#include <blacksail/blacksail.h>
#include <blacksail/torrent.h>
#include <blacksail/config.h>
#include <stdio.h>

char *build_handshake(struct torrent *t, char *peer_id)
{
	// 1  byte:  protocol string length
	// 19 bytes: protocol string
	// 8  bytes: reserved
	// 20 bytes: torrent infohash
	// 20 bytes: peer id
	// 1  byte:  NULL char (will not be sent to the peer)
	///
	// This nicely rounds it to 69.
	//

	char ret[69];

	if (strlen(peer_id) != 20) {
		return NULL;
	}

	snprintf(ret, 69, "%uBitTorrent protocol%s%s%s", 0x19, "\x0\x0\x0\x0\x0\x0\x0\x0", t->infohash, peer_id);
	return ret;
}