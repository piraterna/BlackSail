#include <blacksail/torrent.h>
#include <blacksail/bencode.h>
#include <sha1.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define TORRENT_BLOCK_SIZE 16384 // 16 KB

struct torrent *blacksail_add_torrent_file(const char *torrent_filepath, const char *download_path)
{
	FILE *torrent = fopen(torrent_filepath, "rb");
	if (torrent == NULL) {
		return NULL;
	}

	fseek(torrent, 0, SEEK_END);
	size_t len = ftell(torrent);
	rewind(torrent);

	char *buf = malloc(sizeof(char) * len);
	if (buf == NULL) {
		return NULL;
	}

	fread(buf, len, 1, torrent);
	fclose(torrent);

	struct bencode_item *bencode = blacksail_parse_bencode(buf, len);
	free(buf);
	if (bencode == NULL) {
		return NULL;
	}

	return blacksail_add_torrent(bencode, download_path);
}

struct torrent *blacksail_add_torrent(struct bencode_item *bencode, const char *download_path)
{
	struct torrent *t = malloc(sizeof(struct torrent));
	if (t == NULL) {
		return NULL;
	}
	
	// set initial values
	t->bencode = bencode;
	t->name = blacksail_bencode_find_dvalue_str(bencode->data, "name", NULL);
	t->comment = blacksail_bencode_find_dvalue_str(bencode->data, "comment", NULL);
	t->created_by = blacksail_bencode_find_dvalue_str(bencode->data, "created by", NULL);

	time_t timestamp = blacksail_bencode_find_dvalue_int(bencode->data, "creation date");
	struct tm *date_created = localtime(&timestamp);
	t->date_created = date_created;

	char *encoding = blacksail_bencode_find_dvalue_str(bencode->data, "encoding", NULL);
	if (encoding == NULL) {
		t->encoding = TORRENT_ENCODING_UTF8;
	}
	free(encoding);

	t->is_private = (blacksail_bencode_find_dvalue_int(bencode->data, "private") == 1) ? true : false;
	t->is_completed = false;
	t->is_started = false;

	t->trackers = blacksail_bencode_find_dvalue_str(bencode->data, "announce", NULL);
	
	t->download_dir = malloc(strlen(download_path));
	strcpy(t->download_dir, download_path);

	t->files = NULL;

	// TODO: get info hash
	// -> SHA1 (use hash_sha1(str, size)) all raw bytes of the info bencode section
	//t->infohash = {0};

	size_t piece_count = 0;
	t->piece_hashes = (uint8_t **)blacksail_bencode_find_dvalue_str(bencode->data, "pieces", &piece_count);

	t->total_size = blacksail_bencode_find_dvalue_int(bencode->data, "length");
	t->block_size = TORRENT_BLOCK_SIZE;
	t->piece_size = blacksail_bencode_find_dvalue_int(bencode->data, "piece length");
	t->piece_count = piece_count / 20;

	t->verified_piece_count = 0;
	t->verified_ratio = 0.00;

	return t;
}

void blacksail_remove_torrent(struct torrent *t)
{
	blacksail_free_bencode_item(t->bencode);
	free(t->download_dir);
	free(t);
}