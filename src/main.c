#include <blacksail/blacksail.h>
#include <blacksail/torrent.h>
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

	if (!blacksail_add_torrentf(argv[1], "")) {
		fprintf(stderr, "[!] Failed to add a new torrent!\n");
	}

	sleep(30);

	blacksail_shutdown();
	return 0;
}