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

	fprintf(stderr, "[*] Total size: %.2f MB\n", (float)(t->total_size / 1024 / 1024));
	fprintf(stderr, "[*] Piece size: %d KB\n", t->piece_size / 1024);
	fprintf(stderr, "[*] Piece count: %d\n", t->piece_count);
	fprintf(stderr, "[*] Block size: 16384\n");
	fprintf(stderr, "[*] Block count: %d\n", t->block_count);

	if (!blacksail_announce_torrent(t)) {
		fprintf(stderr, "[!] Failed to announce torrent!\n");
	}

	if (!blacksail_download_piece_from_peer(t, 0, 0)) {
		fprintf(stderr, "[!] Failed to download torrent!\n");
	}

	blacksail_shutdown();
	return 0;
}