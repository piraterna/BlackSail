#ifndef _SHA1_H_
#define _SHA1_H_

#include <stdint.h>
#include <stddef.h>

uint8_t *hash_sha1(const uint8_t *str, size_t size);

#endif /* _SHA1_H_ */