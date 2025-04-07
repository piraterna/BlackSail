#ifndef _PEER_H
#define _PEER_H

#include <stdint.h>

enum peer_message {
	CHOKE = 0,
	UNCHOKE = 1,
	INTERESTED = 2,
	NOT_INTERESTED = 3,
	HAVE = 4,
	BITFIELD = 5,
	REQUEST = 6,
	PIECE = 7,
	CANCEL = 8,
};

struct peer_unchoke {
	uint8_t id;
	uint32_t payload_len;
} __attribute__((packed));

struct peer_request {
	uint8_t id; // always 6
	uint32_t payload_len;
	uint32_t piece_index;
	uint32_t block_offset;
	uint32_t block_length;
} __attribute__((packed));

struct peer_piece {
	uint8_t id; // always 7
	uint32_t payload_len;
	uint32_t index;
	uint32_t begin;
} __attribute__((packed));

char *build_handshake(struct torrent *t, char *peer_id);
bool send_handshake(struct peer *p, const char *handshake);

#endif /* _PEER_H */
