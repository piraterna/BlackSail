#ifndef _BLACKSAIL_BENCODE_H
#define _BLACKSAIL_BENCODE_H

#include <stdint.h>
#include <stddef.h>

enum {
	BEN_STRING,
	BEN_INTEGER,
	BEN_LIST,
	BEN_DICTIONARY,

	BEN_INVALID
};

struct bencode_item {
	uint8_t type;
    const char *b_end;
    size_t size;
    void *data;
};

struct bencode_list {
	struct bencode_item *val;

	struct bencode_list *next;
};

struct bencode_dictionary {
	struct bencode_item *key;
	struct bencode_item *val;

	struct bencode_dictionary *next;
};

struct bencode_item *blacksail_parse_bencode(const char *ben_str, size_t len);
void blacksail_free_bencode_item(struct bencode_item *item);

char *blacksail_bencode_find_dvalue_str(struct bencode_dictionary *d, const char *key, size_t *value_size);
size_t blacksail_bencode_find_dvalue_int(struct bencode_dictionary *d, const char *key);

#endif /* _BLACKSAIL_BENCODE_H */
