#include <openssl/sha.h>
#include <blacksail/torrent.h>
#include <blacksail/bencode.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define TORRENT_BLOCK_SIZE 16384 // 16 KB

struct torrent *blacksail_add_torrent_file(const char *torrent_filepath, const char *download_path)
{
	FILE *torrent = fopen(torrent_filepath, "rb");
	if (torrent == NULL) {
		return NULL;
	}

	fseek(torrent, 0, SEEK_END);
	size_t len = ftell(torrent);
	rewind(torrent);

	char *buf = malloc(sizeof(char) * len);
	if (buf == NULL) {
		return NULL;
	}

	fread(buf, len, 1, torrent);
	fclose(torrent);

	struct bencode_item *bencode = blacksail_parse_bencode((uint8_t *)buf, len);
	if (bencode == NULL) {
		free(buf);
		return NULL;
	}

	struct torrent *ret = blacksail_add_torrent(bencode, download_path);
	free(buf);

	return ret;
}

struct torrent *blacksail_add_torrent(struct bencode_item *bencode, const char *download_path)
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
	t->is_completed = false;
	t->is_started = false;

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
	sprintf(t->infohash_url, "%%%X%%%X%%%X%%%X%%%X%%%X%%%X%%%X%%%X%%%X%%%X%%%X%%%X%%%X%%%X%%%X%%%X%%%X%%%X%%%X",
		t->infohash[0],
		t->infohash[1],
		t->infohash[2],
		t->infohash[3],
		t->infohash[4],
		t->infohash[5],
		t->infohash[6],
		t->infohash[7],
		t->infohash[8],
		t->infohash[9],
		t->infohash[10],
		t->infohash[11],
		t->infohash[12],
		t->infohash[13],
		t->infohash[14],
		t->infohash[15],
		t->infohash[16],
		t->infohash[17],
		t->infohash[18],
		t->infohash[19]);

	size_t piece_count = 0;
	t->piece_hashes = (uint8_t **)blacksail_bencode_find_dvalue_str(bencode->data, "pieces", &piece_count);

	t->total_size = blacksail_bencode_find_dvalue_int(bencode->data, "length");
	t->block_size = TORRENT_BLOCK_SIZE;
	t->piece_size = blacksail_bencode_find_dvalue_int(bencode->data, "piece length");
	t->piece_count = piece_count / 20;

	t->verified_piece_count = 0;
	t->verified_ratio = 0.00;

	return t;
}

void blacksail_remove_torrent(struct torrent *t)
{
	blacksail_free_bencode_item(t->bencode);
	free(t->download_dir);
	free(t);
}