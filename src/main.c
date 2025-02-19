#include <blacksail/bencode.h>
#include <stdlib.h>
#include <stdio.h>

#include <string.h>

int main(int argc, char **argv)
{
	if (argc < 2) {
		fprintf(stderr, "[!] No input torrent supplied!\n");
		fprintf(stderr, "Usage: %s <torrent>\n\n", argv[0]);

		return 1;
	}

	FILE *torrent = fopen(argv[1], "rb");
	if (!torrent) {
		fprintf(stderr, "[!] Failed to open torrent file!\n");
		return 1;
	}

	fseek(torrent, 0, SEEK_END);
	size_t len = ftell(torrent);
	rewind(torrent);

	char *buf = malloc(sizeof(char) * len);
	if (!buf) {
		fprintf(stderr, "[!] Failed to allocate memory for buffer!\n");
		return 1;
	}

	fprintf(stderr, "[*] Reading %zu bytes...\n", len);
	fread(buf, len, 1, torrent);

	blacksail_parse_bencode(buf, len);

	fclose(torrent);
	free(buf);

	return 0;
}