#include <blacksail/torrent.h>
#include <stdio.h>

int main(int argc, char **argv)
{
	if (argc < 2) {
		fprintf(stderr, "[!] No input torrent supplied!\n");
		fprintf(stderr, "Usage: %s <torrent>\n\n", argv[0]);

		return 1;
	}

	struct torrent *t = blacksail_add_torrent_file(argv[1], "");
	if (t == NULL) {
		fprintf(stderr, "Failed to open torrent file!\n");
	}

	// print some nice debug info
	fprintf(stderr, "[*] Torrent name: %s\n", t->name);
	fprintf(stderr, "[*] Comment: %s\n", t->comment);
	fprintf(stderr, "[*] Created by: %s\n", t->created_by);
	fprintf(stderr, "[*] Date created: %u/%u/%u %u:%u:%u %s\n", t->date_created->tm_mday, t->date_created->tm_mon, t->date_created->tm_year + 1900, t->date_created->tm_hour, t->date_created->tm_min, t->date_created->tm_sec, t->date_created->tm_zone);
	fprintf(stderr, "[*] Encoding: %s\n", t->encoding == TORRENT_ENCODING_UTF8 ? "UTF-8" : "Unknown");
	fprintf(stderr, "[*] Is Private: %s\n", t->is_private ? "Yes" : "No");
	fprintf(stderr, "[*] Download directory: \"%s\"\n", t->download_dir);
	fprintf(stderr, "[*] Info hash: ");
	for (int i = 0; i < 20; i++) {
		fprintf(stderr, "%02x", t->infohash[i]);
	}
	fprintf(stderr, "\n");
	fprintf(stderr, "[*] Primary tracker: %s\n", t->trackers);
	fprintf(stderr, "[*] Total size: %zu MB\n", t->total_size / 1024 / 1024);
	fprintf(stderr, "[*] Block size: %i KB\n", t->block_size / 1024);
	fprintf(stderr, "[*] Piece size: %i KB\n", t->piece_size / 1024);
	fprintf(stderr, "[*] Piece count: %i\n", t->piece_count);

	blacksail_remove_torrent(t);
	return 0;
}