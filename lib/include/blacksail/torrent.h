#ifndef _BLACKSAIL_TORRENT_H
#define _BLACKSAIL_TORRENT_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <time.h>

enum torrent_encoding {
	UTF8,
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
	const char *name;
	const char *comment;
	const char *created_by;
	const struct tm date_created;
	const uint8_t encoding;
	bool is_private;

	bool is_completed;
	bool is_started;

	const char **trackers;

	const char *download_dir;
	struct torrent_file *files;

	size_t total_size;	
	int block_size;
	int piece_size;
	int piece_count;
	int verified_piece_count;

	uint8_t infohash[20];
	uint8_t *piece_hashes[20];

	double verified_ratio;

	size_t uploaded;
	size_t downloaded;
};

#endif /* _BLACKSAIL_TORRENT_H */
