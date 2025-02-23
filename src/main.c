#include <blacksail/blacksail.h>
#include <blacksail/torrent.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	if (argc < 2) {
		fprintf(stderr, "[!] No input torrent supplied!\n");
		fprintf(stderr, "Usage: %s <torrent>\n\n", argv[0]);

		return 1;
	}

	blacksail_initialize();

	if (!blacksail_add_torrentf(argv[1], "")) {
		fprintf(stderr, "[!] Failed to add a new torrent!\n");
	}

	if (!blacksail_add_torrentf(argv[1], "")) {
		fprintf(stderr, "[!] Failed to add a new torrent!\n");
	}

	fprintf(stderr, "Sleeping for 2 seconds...\n");
	sleep(2);

	blacksail_remove_all_torrents();
	return 0;
}