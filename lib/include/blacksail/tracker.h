#ifndef _BLACKSAIL_TRACKER_H
#define _BLACKSAIL_TRACKER_H

#include <blacksail/torrent.h>
#include <stdint.h>
#include <stddef.h>
#include <time.h>

enum tracker_event {
	Started,
	Paused,
	Stopped
};

struct tracker_response {
	size_t seeder_count;
	size_t leecher_count;
	
	size_t last_announce; // unix timestamp
	size_t min_interval;
	size_t interval;

	uint8_t *peers;
};

struct tracker_response *blacksail_announce_torrent(struct torrent *t);

#endif /* _BLACKSAIL_TRACKER_H */
