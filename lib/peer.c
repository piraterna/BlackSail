#include <blacksail/blacksail.h>
#include <blacksail/torrent.h>
#include <blacksail/config.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

char *build_handshake(struct torrent *t, char *peer_id)
{
	// 1  byte:  protocol string length
	// 19 bytes: protocol string
	// 8  bytes: reserved
	// 20 bytes: torrent infohash
	// 20 bytes: peer id
	// 1  byte:  NULL char (will not be sent to the peer)
	///
	// This nicely rounds it to 69.
	//

	char *ret = malloc(69);
	if (!ret) {
		return NULL;
	}

	const char *proto = "BitTorrent protocol";

	ret[0] = 0x13; // 19
	memcpy(&ret[1], proto, 19);
	for (int i = 0; i < 8; i++) {
		ret[20 + i] = 0;
	}

	memcpy(&ret[28], t->infohash, 20);
	memcpy(&ret[58], peer_id, 20);
	ret[68] = 0;

	//snprintf(ret, 69, "%uBitTorrent protocol%s%s%.*s", 0x13, "\x0\x0\x0\x0\x0\x0\x0\x0", t->infohash, 20, peer_id);
	return ret;
}

bool send_handshake(struct peer *p, const char *handshake)
{
	if (!p)
		return false;
	
	if (!handshake)
		return false;

	int peer_socket;
	struct sockaddr_in peer_addr;
	char peer_response[1024];
	memset(peer_response, 0, 1024);

	peer_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (peer_socket < 0) {
		fprintf(stderr, "Failed to open a socket: %s!\n", strerror(errno));
		return false;
	}

	peer_addr.sin_family = AF_INET;
	peer_addr.sin_port = htons(p->port);
	peer_addr.sin_addr.s_addr = inet_addr(p->ip);

	fprintf(stderr, "Connecting to peer %s:%u... ", p->ip, p->port);

	if (connect(peer_socket, (struct sockaddr *)&peer_addr, sizeof(peer_addr)) < 0){
		fprintf(stderr, "Failed to connect to %s:%u: %s!\n", p->ip, p->port, strerror(errno));
		return false;
	}
	
	fprintf(stderr, "connected!\nSending handshake... ");
	// TODO: Handshake is currently always exactly 68 bytes (nice!),
	//		 but it doesn't have to be hardcoded.
	if (send(peer_socket, handshake, 68, 0) < 0){
		fprintf(stderr, "Failed to send handshake: %s!\n", strerror(errno));
		close(peer_socket);
		return false;
	}

	fprintf(stderr, "done!\nWaiting for response...\n");

	if (recv(peer_socket, peer_response, 1024, 0) <= 0){
		fprintf(stderr, "Failed to receive handshake from peer: %s!\n", strerror(errno));
		close(peer_socket);
		return false;
	}
	
	int proto_len = peer_response[0];

	p->id = malloc(21);
	if (!p->id) {
		fprintf(stderr, "Failed to allocate memory for peer ID!\n");
		close(peer_socket);
		return false;
	}

	char *peer_id = &peer_response[proto_len + 8 + 20];
	strncpy(p->id, peer_id, 20);
	fprintf(stderr, "Peer ID: %x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x\n",
		p->id[0], p->id[1], p->id[2], p->id[3], p->id[4], p->id[5],
		p->id[6], p->id[7], p->id[8], p->id[9], p->id[10], p->id[11],
		p->id[12], p->id[13], p->id[14], p->id[15], p->id[16], p->id[17],
		p->id[18], p->id[19]);
	
	p->socket_num = peer_socket;
	p->peer_addr = peer_addr;
	close(peer_socket);

	return true;
}