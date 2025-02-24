#ifndef _BLACKSAIL_CURL_BUFFER_H
#define _BLACKSAIL_CURL_BUFFER_H

#include <stddef.h>

struct curl_buffer {
	char *buf;
	size_t size;
};

#endif /* _BLACKSAIL_CURL_BUFFER_H */
