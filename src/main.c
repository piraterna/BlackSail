#include <blacksail/blacksail.h>
#include <blacksail/torrent.h>
#include <blacksail/tracker.h>
#include <blacksail/config.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	if (argc < 2) {
		fprintf(stderr, "[!] No input torrent supplied!\n");
		fprintf(stderr, "Usage: %s <torrent>\n\n", argv[0]);

		return 1;
	}

	// example configuration change
	/*
	struct blacksail_config cfg = {
		.client_id = "BlackSailClient54321",
		.port = 1234
	};

	blacksail_update_config(&cfg);
	*/

	blacksail_init();

	struct torrent *t = blacksail_add_torrentf(argv[1], "");
	if (!t) {
		fprintf(stderr, "[!] Failed to add a new torrent!\n");
	}

	if (!blacksail_announce_torrent(t)) {
		fprintf(stderr, "[!] Failed to announce torrent!\n");
	}

	blacksail_shutdown();
	return 0;
}