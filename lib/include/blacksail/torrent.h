#ifndef _BLACKSAIL_TORRENT_H
#define _BLACKSAIL_TORRENT_H

#include <blacksail/bencode.h>
#include <blacksail/peer.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <time.h>

enum torrent_encoding {
	TORRENT_ENCODING_UTF8,
};

enum torrent_status {
	TORRENT_STARTED,
	TORRENT_PAUSED,
	TORRENT_STOPPED,

	TORRENT_DOWNLOADING,
	TORRENT_SEEDING,
	TORRENT_STALLED,
};

struct torrent_file {
	const char *path;
	size_t offset;
	size_t size;
};

struct torrent {
	struct bencode_item *bencode;

	const char *name;
	const char *comment;
	const char *created_by;
	struct tm *date_created;
	uint8_t encoding;
	bool is_private;

	uint8_t status;

	//const char **trackers;
	const char *trackers;
	size_t min_interval;
	size_t next_interval;

	struct peer *peers;
	size_t peer_count;

	char *download_dir;
	struct torrent_file *files;

	uint8_t infohash[20];
	char infohash_url[20 * 3 + 1];
	uint8_t **piece_hashes;

	size_t total_size;	
	int block_size;
	int piece_size;
	int piece_count;

	int verified_piece_count;
	double verified_ratio;

	size_t uploaded;
	size_t downloaded;
};

struct torrent *blacksail_add_torrentf(const char *torrent_filepath, const char *download_path);
void blacksail_remove_torrent(struct torrent *t);

bool blacksail_download_from_peer(struct torrent *t, int peer_idx);

#endif /* _BLACKSAIL_TORRENT_H */
