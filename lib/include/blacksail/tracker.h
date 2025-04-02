#ifndef _BLACKSAIL_TRACKER_H
#define _BLACKSAIL_TRACKER_H

#include <blacksail/torrent.h>
#include <stdbool.h>

bool blacksail_announce_torrent(struct torrent *t);

#endif /* _BLACKSAIL_TRACKER_H */
