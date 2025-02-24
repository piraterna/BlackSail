#ifndef _BLACKSAIL_PEER_H
#define _BLACKSAIL_PEER_H

#include <stdint.h>

struct peer {
	char *ip;
	uint16_t port;
};

#endif /* _BLACKSAIL_PEER_H */