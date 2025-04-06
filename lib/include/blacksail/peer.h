#ifndef _BLACKSAIL_PEER_H
#define _BLACKSAIL_PEER_H

#include <blacksail/torrent.h>
#include <arpa/inet.h>
#include <stdint.h>

#define PEER_BUFFER_SIZE 256

struct peer {
	char *ip;
	uint16_t port;

	char *local_id;
	char *id;

	struct sockaddr_in peer_addr;
	int socket_num;

	struct torrent *torrent;
};

#endif /* _BLACKSAIL_PEER_H */