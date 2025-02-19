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
void blacksail_free_bencode_list(struct bencode_list *list);
void blacksail_free_bencode_dict(struct bencode_dictionary *dict);

#endif /* _BLACKSAIL_BENCODE_H */
