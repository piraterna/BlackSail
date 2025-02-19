#include <blacksail/bencode.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct bencode_item *blacksail_parse_bencode(const char *ben_str, size_t len)
{
	if (ben_str == NULL || len == 0) {
		return NULL;
	}

	char start = ben_str[0];
	const char *end = ben_str + len;
	const char *str = ben_str + 1;

	switch (start) {
		// BEN_STRING
		case '0' ... '9': {
			size_t length = start - '0';

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
			ret->b_end = str + length + 1;
			ret->size = length;
			ret->data = data;
			return ret;
			break;
		}

		// BEN_INTEGER
		case 'i': {
			size_t num = 0;

			str += *str == '-' ? 1 : 0;

			while (str < end && *str != 'e') {
				num = 10 * num + (*(str++) - '0');
			}

			if (ben_str[1] == '-') {
				num = -num;
			}

			struct bencode_item *ret = malloc(sizeof(struct bencode_item));
			if (ret == NULL) {
				return ret;
			}

			ret->type = BEN_INTEGER;
			ret->b_end = str + 1;
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

				str = list->val->b_end;
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
				return ret;
			}

			ret->type = BEN_LIST;
			ret->b_end = str + 1;
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

				str = dict->key->b_end;
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

				str = dict->val->b_end;
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
			ret->b_end = str + 1;
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
