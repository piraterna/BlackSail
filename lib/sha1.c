#include <openssl/sha.h>
#include <stdint.h>
#include <stddef.h>

uint8_t *hash_sha1(const uint8_t *str, size_t size)
{
    uint8_t *ret;
    SHA1(str, size, ret);

    return ret;
}