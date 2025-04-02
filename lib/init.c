#include <blacksail/blacksail.h>
#include <blacksail/torrent.h>
#include <curl/curl.h>
#include <curl_buffer.h>
#include <stdlib.h>
#include <stddef.h>

extern CURL *curl;
extern struct curl_buffer buffer;

extern size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp);

void blacksail_init(void)
{
	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();

	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "BlackSail/"BLACKSAIL_VERSION_STR);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

	buffer.buf = malloc(1);
	buffer.size = 0;
}

void blacksail_shutdown(void)
{
	free(buffer.buf);
	curl_easy_cleanup(curl);
	curl_global_cleanup();
}