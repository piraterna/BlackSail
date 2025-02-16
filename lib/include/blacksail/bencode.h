#ifndef _BLACKSAIL_BENCODE_H
#define _BLACKSAIL_BENCODE_H

#include <stddef.h>

struct bencode {
	void *data;
};

struct bencode *blacksail_parse_bencode(char *str, size_t len);

#endif /* _BLACKSAIL_BENCODE_H */
