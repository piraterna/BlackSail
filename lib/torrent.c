#include <blacksail/blacksail.h>
#include <blacksail/config.h>
#include <blacksail/torrent.h>
#include <blacksail/bencode.h>
#include <blacksail_utils.h>
#include <peer.h>
#include <openssl/sha.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

#define TORRENT_BLOCK_SIZE 16384 // 16 KB

struct torrent *add_torrent(struct bencode_item *bencode, const char *download_path)
{
	struct torrent *t = malloc(sizeof(struct torrent));
	if (t == NULL) {
		return NULL;
	}
	
	// set initial values
	t->bencode = bencode;
	t->name = blacksail_bencode_find_dvalue_str(bencode->data, "name", NULL);
	t->comment = blacksail_bencode_find_dvalue_str(bencode->data, "comment", NULL);
	t->created_by = blacksail_bencode_find_dvalue_str(bencode->data, "created by", NULL);

	time_t timestamp = blacksail_bencode_find_dvalue_int(bencode->data, "creation date");
	struct tm *date_created = localtime(&timestamp);
	t->date_created = date_created;

	char *encoding = blacksail_bencode_find_dvalue_str(bencode->data, "encoding", NULL);
	if (encoding == NULL) {
		t->encoding = TORRENT_ENCODING_UTF8;
	}
	free(encoding);

	t->is_private = (blacksail_bencode_find_dvalue_int(bencode->data, "private") == 1) ? true : false;
	t->status = TORRENT_STARTED;

	t->trackers = blacksail_bencode_find_dvalue_str(bencode->data, "announce", NULL);
	
	t->download_dir = malloc(strlen(download_path));
	strcpy(t->download_dir, download_path);

	t->files = NULL;

	struct bencode_dictionary *d = (struct bencode_dictionary *)bencode->data;
	const uint8_t *info_section = NULL;
	size_t info_size = 0;
	while (d->next != NULL) {
		if (strcmp(d->key->data, "info") == 0) {
			info_section = (uint8_t *)d->val->b_start - 1;
			info_size = d->val->b_end - d->val->b_start + 1;
			break;
		}

		d = d->next;
	}
	
	SHA1(info_section, info_size, t->infohash);

	// convert infohash hex to a URL-friendly string    
    BYTES_TO_URL(t->infohash, t->infohash_url, 20);

	size_t piece_count = 0;
	t->piece_hashes = (uint8_t **)blacksail_bencode_find_dvalue_str(bencode->data, "pieces", &piece_count);

	t->total_size = blacksail_bencode_find_dvalue_int(bencode->data, "length");
	t->piece_size = blacksail_bencode_find_dvalue_int(bencode->data, "piece length");
	t->piece_count = piece_count / 20;
	t->block_size = TORRENT_BLOCK_SIZE;
	t->block_count = (t->piece_size / t->block_size) * t->piece_count;

	t->verified_piece_count = 0;
	t->verified_ratio = 0.00;

	return t;
}

struct torrent *blacksail_add_torrentf(const char *torrent_filepath, const char *download_path)
{
	FILE *torrent = fopen(torrent_filepath, "rb");
	if (torrent == NULL) {
		return 0;
	}

	fseek(torrent, 0, SEEK_END);
	size_t len = ftell(torrent);
	rewind(torrent);

	char *buf = malloc(sizeof(char) * len);
	if (buf == NULL) {
		return 0;
	}

	fread(buf, len, 1, torrent);
	fclose(torrent);

	struct bencode_item *bencode = blacksail_parse_bencode((uint8_t *)buf, len);
	if (bencode == NULL) {
		free(buf);
		return 0;
	}

	struct torrent *t = add_torrent(bencode, download_path);
	free(buf);

	return t;
}

void blacksail_remove_torrent(struct torrent *t)
{
	blacksail_free_bencode_item(t->bencode);
	free(t->download_dir);

	for (int i = 0; i < t->peer_count; i++) {
		free(t->peers[i].ip);
	}
	
	free(t->peers);
	free(t);
}

extern struct blacksail_config cfg;

bool blacksail_download_file_from_peer(struct torrent *t, int peer_idx)
{
	if (!t)
		return false;
	
	if (peer_idx >= t->peer_count)
		return false;
	
	/*int peer_socket = connect_to_peer(&t->peers[peer_idx]);
	if (peer_socket == 0)
		return false;*/
	
	const char *handshake = build_handshake(t, cfg.peer_id);
	if (!send_handshake(&t->peers[peer_idx], handshake)) {
		return false;
	}

	return true;
}

bool blacksail_download_piece_from_peer(struct torrent *t, int peer_idx, int piece_idx)
{
	if (!t)
		return false;

	if (peer_idx >= t->peer_count || piece_idx >= t->piece_count)
		return false;
	
	struct peer *peer = &t->peers[peer_idx];

	char *handshake = build_handshake(t, cfg.peer_id);

	if (!send_handshake(peer, handshake)) {
		free(handshake);
		return false;
	}

	free(handshake);

	// wait for message (and hope it's a bitfield)
	uint32_t msg_len;
	if (recv(peer->socket_num, &msg_len, 4, 0) <= 0) {
		fprintf(stderr, "Failed to receive message from peer: %s\n", strerror(errno));
		// TODO: don't do this
		close(peer->socket_num);
		peer->socket_num = 0;
		memset(&peer->peer_addr, 0, sizeof(struct sockaddr_in));
		return false;
	}

	msg_len = ntohl(msg_len);

	uint8_t msg_id;
	if (recv(peer->socket_num, &msg_id, 1, 0) <= 0) {
		fprintf(stderr, "Failed to receive message from peer: %s\n", strerror(errno));
		// TODO: don't do this
		close(peer->socket_num);
		peer->socket_num = 0;
		memset(&peer->peer_addr, 0, sizeof(struct sockaddr_in));
		return false;
	}

	if (msg_id != BITFIELD) {
		fprintf(stderr, "Peer probably doesn't have any pieces downloaded, skipping...\n");
		close(peer->socket_num);
		peer->socket_num = 0;
		memset(&peer->peer_addr, 0, sizeof(struct sockaddr_in));
		return false;
	}

	char *bitfield = malloc(msg_len);
	if (!bitfield) {
		fprintf(stderr, "Failed to allocate memory for bitfield!\n");
		// TODO: don't do this
		close(peer->socket_num);
		peer->socket_num = 0;
		memset(&peer->peer_addr, 0, sizeof(struct sockaddr_in));
		return false;
	}

	if (recv(peer->socket_num, bitfield, msg_len, 0) <= 0) {
		fprintf(stderr, "Failed to receive message from peer: %s\n", strerror(errno));
		// TODO: don't do this
		close(peer->socket_num);
		peer->socket_num = 0;
		memset(&peer->peer_addr, 0, sizeof(struct sockaddr_in));
		return false;
	}

	// verify if the peer has the entire file... for now
	for (int i = 0; i < msg_len - 1; i++) {
		if ((bitfield[i] & 0xff) != 0xff) {
			fprintf(stderr, "Peer doesn't have the entire file, ignoring...\n");
			
			// TODO: don't do this
			close(peer->socket_num);
			peer->socket_num = 0;
			memset(&peer->peer_addr, 0, sizeof(struct sockaddr_in));
			return false;
		}
	}

	// alr cool we can tell the peer we're interested in his goods and request the piece
	// ...
	char interested_msg = INTERESTED;
	if (send(peer->socket_num, &interested_msg, 1, 0) < 0) {
		fprintf(stderr, "Failed to send interested message to peer: %s!\n", strerror(errno));

		// TODO: don't do this
		close(peer->socket_num);
		peer->socket_num = 0;
		memset(&peer->peer_addr, 0, sizeof(struct sockaddr_in));
		return false;
	}

	// wait until we get UNCHOKE back
	struct peer_unchoke unchoke_msg;
	if (recv(peer->socket_num, &unchoke_msg, sizeof(unchoke_msg), 0) < 0) {
		fprintf(stderr, "Peer didn't send unchoke message: %s!\n", strerror(errno));

		// TODO: don't do this
		close(peer->socket_num);
		peer->socket_num = 0;
		memset(&peer->peer_addr, 0, sizeof(struct sockaddr_in));
		return false;
	}

	// this doesn't work
	/*
	if (unchoke_msg.id != UNCHOKE) {
		fprintf(stderr, "Peer didn't send unchoke message!\n");

		// TODO: don't do this
		close(peer->socket_num);
		peer->socket_num = 0;
		memset(&peer->peer_addr, 0, sizeof(struct sockaddr_in));
		return false;
	}
	*/

	// calculate how many blocks we'll need for a single piece
	int blocks_required = t->piece_size / t->block_size;
	char *piece = malloc(t->piece_size);
	if (!piece) {
		close(peer->socket_num);
		peer->socket_num = 0;
		memset(&peer->peer_addr, 0, sizeof(struct sockaddr_in));
		return false;
	}

	int piece_len = t->block_size;
	struct peer_piece piece_msg;

	struct peer_request request_msg;
	request_msg.id = REQUEST;
	request_msg.piece_index = piece_idx;

	for (int i = 0; i < blocks_required; i++) {
		request_msg.block_offset = htonl(i * t->block_size);
		request_msg.block_length = htonl(t->block_size);

		// send REQUEST
		if (send(peer->socket_num, &request_msg, 13, 0) < 0) {
			fprintf(stderr, "Failed to send REQUEST message: %s!\n", strerror(errno));

			// TODO: don't do this
			free(piece);
			close(peer->socket_num);
			peer->socket_num = 0;
			memset(&peer->peer_addr, 0, sizeof(struct sockaddr_in));
			return false;
		}
		
		// wait for PIECE
		if (recv(peer->socket_num, &piece_msg, sizeof(struct peer_piece), 0) < 0) {
			fprintf(stderr, "Failed to receive piece: %s!\n", strerror(errno));
			
			// TODO: don't do this
			free(piece);
			close(peer->socket_num);
			peer->socket_num = 0;
			memset(&peer->peer_addr, 0, sizeof(struct sockaddr_in));
			return false;
		}

		// TODO: Write a better way to handle messages
		if (piece_msg.id != 7) {
			fprintf(stderr, "Peer didn't send piece!\n");
			
			// TODO: don't do this
			free(piece);
			close(peer->socket_num);
			peer->socket_num = 0;
			memset(&peer->peer_addr, 0, sizeof(struct sockaddr_in));
			return false;
		}

		if (htonl(piece_msg.index) != piece_idx) {
			fprintf(stderr, "Peer sent incorrect piece, requesting again...\n");
			i--;
			continue;
		}

		if (htonl(piece_msg.begin) != request_msg.block_offset) {
			fprintf(stderr, "Peer sent incorrect block, requesting again...\n");
			i--;
			continue;
		}

		if (recv(peer->socket_num, &piece[request_msg.block_offset], request_msg.block_length, 0) < 0) {
			fprintf(stderr, "Failed to receive block: %s!\n", strerror(errno));
			
			// TODO: don't do this
			free(piece);
			close(peer->socket_num);
			peer->socket_num = 0;
			memset(&peer->peer_addr, 0, sizeof(struct sockaddr_in));
			return false;
		}
	}
	
	fprintf(stderr, "Received piece index %u\n", piece_idx);

	// TODO: we got the piece, close the connection for now
	close(peer->socket_num);
	peer->socket_num = -1;
	memset(&peer->peer_addr, 0, sizeof(struct sockaddr_in));

	return true;
}