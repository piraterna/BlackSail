#include <blacksail/bencode.h>
#include <blacksail/tracker.h>
#include <blacksail/torrent.h>
#include <blacksail/peer.h>
#include <blacksail/config.h>
#include <curl/curl.h>
#include <curl_buffer.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

extern struct blacksail_config cfg;

CURL *curl = NULL;

struct curl_buffer buffer;

extern char *build_handshake(struct torrent *t, char *peer_id);

size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct curl_buffer *mem = (struct curl_buffer *)userp;
    
    char *ptr = realloc(mem->buf, mem->size + realsize + 1);
    if (ptr == NULL) {
        return CURL_WRITEFUNC_ERROR;
    }

    mem->buf = ptr;
    memcpy(&(mem->buf[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->buf[mem->size] = 0;

    return realsize;
}

bool blacksail_announce_torrent(struct torrent *t)
{
	if (t == NULL || curl == NULL) {
		return false;
	}

	// construct the announce URL
	char *get_url;
	asprintf(&get_url, "%s?info_hash=%s&peer_id=%s&port=%u&uploaded=%zu&downloaded=%zu&left=%zu&compact=1",
		t->trackers, t->infohash_url, cfg.client_id, cfg.port, t->uploaded, t->downloaded, t->total_size - t->downloaded);
	
	curl_easy_setopt(curl, CURLOPT_URL, get_url);

	CURLcode res;
	if ((res = curl_easy_perform(curl)) != CURLE_OK) {
		fprintf(stderr, "curl_easy_perform(): Failed: %s\n", curl_easy_strerror(res));
	}

	free(get_url);

	struct bencode_item *bencode = blacksail_parse_bencode((const uint8_t *)buffer.buf, buffer.size);
	buffer.size = 0;

	struct bencode_dictionary *dict = bencode->type == BEN_DICTIONARY ? bencode->data : NULL;
	if (dict == NULL) {
		// there's nothing we can do
		return false;
	}

	int min_interval = blacksail_bencode_find_dvalue_int(dict, "min interval");
	int interval = blacksail_bencode_find_dvalue_int(dict, "interval");
	if (min_interval == -1 && interval == -1) {
		interval = 60;
	}

	size_t peerlist_size;
	char *peers = blacksail_bencode_find_dvalue_str(dict, "peers", &peerlist_size);

	if (peerlist_size % 6 != 0) {
		// ??? tracker bad (or ipv6)
		return false;
	}

	t->peers = (struct peer *)malloc((peerlist_size / 6) * sizeof(struct peer));
	t->peer_count = peerlist_size / 6;

	for (int i = 0; i < peerlist_size / 6; i++) {
		if (asprintf(&t->peers[i].ip, "%d.%d.%d.%d",
            ((uint8_t *)peers)[6 * i + 0],
            ((uint8_t *)peers)[6 * i + 1],
            ((uint8_t *)peers)[6 * i + 2],
            ((uint8_t *)peers)[6 * i + 3]) > 16) {
        	return false;
    	}

		t->peers[i].port = 256 * ((uint8_t *)peers)[6 * i + 4] + ((uint8_t *)peers)[6 * i + 5];
	}

	t->min_interval = min_interval;
	t->next_interval = interval;
	build_handshake(t, "qwertyuiopasdfghjklz");

	return true;
}