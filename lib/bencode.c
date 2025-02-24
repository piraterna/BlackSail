#include <blacksail/bencode.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void blacksail_free_bencode_list(struct bencode_list *list);
void blacksail_free_bencode_dict(struct bencode_dictionary *dict);

struct bencode_item *blacksail_parse_bencode(const uint8_t *ben_str, size_t len)
{
	if (ben_str == NULL || len == 0) {
		return NULL;
	}

	char start = ben_str[0];
	const uint8_t *end = ben_str + len;
	const uint8_t *str = (const uint8_t *)ben_str + 1;


	switch (start) {
		// BEN_STRING
		case '0' ... '9': {
			size_t length = start - '0';
			const uint8_t *b_start = (const uint8_t *)str + 1;

			while (str && str < end && *str != ':') {
				size_t next = 10 * length + (*str - '0');
				if (next < length) {
					return NULL;
				}

				length = next;
				str++;
			}

			if ((!str || str >= end || *str != ':') ||
				(length > (end - str))) {
				return NULL;
			}

			void *data = malloc(length);
			if (data == NULL) {
				return NULL;
			}

			memcpy(data, str + 1, length);

			struct bencode_item *ret = malloc(sizeof(struct bencode_item));
			if (ret == NULL) {
				free(data);
				return NULL;
			}

			ret->type = BEN_STRING;
			ret->b_start = b_start;
			ret->b_end = (const uint8_t *)str + length + 1;
			ret->size = length;
			ret->data = data;
			return ret;
			break;
		}

		// BEN_INTEGER
		case 'i': {
			size_t num = 0;
			const uint8_t *b_start = (const uint8_t *)str + 1;

			str += *str == '-' ? 1 : 0;

			while (str < end && *str != 'e') {
				num = 10 * num + (*(str++) - '0');
			}

			if (ben_str[1] == '-') {
				num = -num;
			}

			struct bencode_item *ret = malloc(sizeof(struct bencode_item));
			if (ret == NULL) {
				return NULL;
			}

			ret->type = BEN_INTEGER;
			ret->b_start = b_start;
			ret->b_end = (const uint8_t *)str + 1;
			ret->size = num;
			ret->data = NULL;
			return ret;
			break;
		}

		// BEN_LIST
		case 'l': {
			struct bencode_list *list = malloc(sizeof(struct bencode_list));
			if (list == NULL) {
				return NULL;
			}

			const uint8_t *b_start = (const uint8_t *)str + 1;

			struct bencode_list *data = list;
			size_t size = 0;

			while (str && str < end && *str != 'e') {
				size++;

				list->val = blacksail_parse_bencode(str, end - str);
				if (list->val == NULL || list->val->type == BEN_INVALID) {
					blacksail_free_bencode_list(data);
					free(data);
					return NULL;
				}

				str = (const uint8_t *)list->val->b_end;
				if (str && str < end && *str != 'e') {
					list->next = malloc(sizeof(struct bencode_list));
					if (list->next == NULL) {
						blacksail_free_bencode_list(data);
						free(data);
						return NULL;
					}

					list = list->next;
				}
			}

			struct bencode_item *ret = malloc(sizeof(struct bencode_item));
			if (ret == NULL) {
				blacksail_free_bencode_list(data);
				free(data);
				return NULL;
			}

			ret->type = BEN_LIST;
			ret->b_start = b_start;
			ret->b_end = (const uint8_t *)str + 1;
			ret->size = size;
			ret->data = data;
			return ret;
			break;
		}

		// BEN_DICTIONARY
		case 'd': {
			struct bencode_dictionary *dict = malloc(sizeof(struct bencode_dictionary));
			if (dict == NULL) {
				return NULL;
			}

			const uint8_t *b_start = (const uint8_t *)str;

			struct bencode_dictionary *data = dict;
			size_t size = 0;

			while (str && str < end && *str != 'e') {
				size++;

				dict->key = blacksail_parse_bencode(str, end - str);
				if (dict->key == NULL || dict->key->type != BEN_STRING) {
					blacksail_free_bencode_dict(data);
					free(data);
					return NULL;
				}

				str = (const uint8_t *)dict->key->b_end;
				if (str == NULL || str >= end || *str == 'e') {
					blacksail_free_bencode_dict(data);
					free(data);
					return NULL;
				}

				dict->val = blacksail_parse_bencode(str, end - str);
				if (dict->val == NULL || dict->val->type == BEN_INVALID) {
					blacksail_free_bencode_dict(data);
					free(data);
					return NULL;
				}

				str = (const uint8_t *)dict->val->b_end;
				if (str && str < end && *str != 'e') {
					dict->next = malloc(sizeof(struct bencode_dictionary));
					if (dict->next == NULL) {
						blacksail_free_bencode_dict(data);
						free(data);
						return NULL;
					}
					dict = dict->next;
				}
			}

			struct bencode_item *ret = malloc(sizeof(struct bencode_item));
			if (ret == NULL) {
				blacksail_free_bencode_dict(data);
				free(data);
				return NULL;
			}

			ret->type = BEN_DICTIONARY;
			ret->b_start = b_start;
			ret->b_end = (const uint8_t *)str + 1;
			ret->size = size;
			ret->data = data;
			return ret;
			break;
		}

		// BEN_INVALID
		default: {
			return NULL;
			break;
		}
	}

	return NULL;
}

void blacksail_free_bencode_item(struct bencode_item *item)
{
	if (item == NULL ||
		item->data == NULL) {
		return;
	}

	switch (item->type) {
		case BEN_STRING:
			free(item->data);
			break;

		case BEN_LIST:
			blacksail_free_bencode_list((struct bencode_list *)item->data);
			free(item->data);
			break;

		case BEN_DICTIONARY:
			blacksail_free_bencode_dict((struct bencode_dictionary *)item->data);
			free(item->data);
			break;

		case BEN_INTEGER:
			break;

		case BEN_INVALID:
			break;
	}

	free(item);
}

char *blacksail_bencode_find_dvalue_str(struct bencode_dictionary *d, const char *key, size_t *value_size)
{
	if (d == NULL || key == NULL) {
		return NULL;
	}

	while (d != NULL) {
		// traverse all dictionaries
		if (d->val->type == BEN_DICTIONARY) {
			char *dret = blacksail_bencode_find_dvalue_str(d->val->data, key, value_size);
			if (dret != NULL) {
				return dret;
			}
		} else if (strncmp((char *)d->key->data, key, d->key->size) == 0) {
			char *ret = malloc(d->val->size * sizeof(char));
			if (ret == NULL) {
				return NULL;
			}

			if (value_size != NULL) {
				*value_size = d->val->size;
			}
			strncpy(ret, (char *)d->val->data, d->val->size);
			return ret;
		}

		d = d->next;
	}

	return NULL;
}

size_t blacksail_bencode_find_dvalue_int(struct bencode_dictionary *d, const char *key)
{
	if (d == NULL || key == NULL) {
		return SIZE_MAX;
	}

	while (d != NULL) {
		// traverse all dictionaries
		if (d->val->type == BEN_DICTIONARY) {
			size_t dret = blacksail_bencode_find_dvalue_int(d->val->data, key);
			if (dret != SIZE_MAX) {
				return dret;
			}
		} else if (strncmp((char *)d->key->data, key, d->key->size) == 0) {
			return d->val->size;
		}

		d = d->next;
	}

	return SIZE_MAX;
}

void blacksail_free_bencode_list(struct bencode_list *list)
{
	if (list == NULL) {
		return;
	}

	blacksail_free_bencode_item(list->val);

	if (list->next) {
		blacksail_free_bencode_list(list->next);
		free(list->next);
	}
}

void blacksail_free_bencode_dict(struct bencode_dictionary *dict)
{
	if (dict == NULL) {
		return;
	}

	blacksail_free_bencode_item(dict->key);
	blacksail_free_bencode_item(dict->val);

	if (dict->next) {
		blacksail_free_bencode_dict(dict->next);
		free(dict->next);
	}
}
