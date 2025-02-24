#include <blacksail/blacksail.h>
#include <blacksail/torrent.h>
#include <blacksail/bencode.h>
#include <torrent_thread.h>
#include <blacksail_utils.h>
#include <openssl/sha.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

#define TORRENT_BLOCK_SIZE 16384 // 16 KB

struct torrent_thread *threads = NULL;
int next_torrent_id = 1; // 0 is reserved
int next_thread_id = 1; // 0 is reserved

struct torrent *add_torrent(struct bencode_item *bencode, const char *download_path)
{
	struct torrent *t = malloc(sizeof(struct torrent));
	if (t == NULL) {
		return NULL;
	}
	
	// set initial values
	t->id = next_torrent_id++;
	t->bencode = bencode;
	t->name = blacksail_bencode_find_dvalue_str(bencode->data, "name", NULL);
	t->comment = blacksail_bencode_find_dvalue_str(bencode->data, "comment", NULL);
	t->created_by = blacksail_bencode_find_dvalue_str(bencode->data, "created by", NULL);

	time_t timestamp = blacksail_bencode_find_dvalue_int(bencode->data, "creation date");
	struct tm *date_created = localtime(&timestamp);
	t->date_created = date_created;

	char *encoding = blacksail_bencode_find_dvalue_str(bencode->data, "encoding", NULL);
	if (encoding == NULL) {
		t->encoding = TORRENT_ENCODING_UTF8;
	}
	free(encoding);

	t->is_private = (blacksail_bencode_find_dvalue_int(bencode->data, "private") == 1) ? true : false;
	t->status = TORRENT_STARTED;

	t->trackers = blacksail_bencode_find_dvalue_str(bencode->data, "announce", NULL);
	
	t->download_dir = malloc(strlen(download_path));
	strcpy(t->download_dir, download_path);

	t->files = NULL;

	struct bencode_dictionary *d = (struct bencode_dictionary *)bencode->data;
	const uint8_t *info_section = NULL;
	size_t info_size = 0;
	while (d->next != NULL) {
		if (strcmp(d->key->data, "info") == 0) {
			info_section = (uint8_t *)d->val->b_start - 1;
			info_size = d->val->b_end - d->val->b_start + 1;
			break;
		}

		d = d->next;
	}
	
	SHA1(info_section, info_size, t->infohash);

	// convert infohash hex to a URL-friendly string    
    BYTES_TO_URL(t->infohash, t->infohash_url, 20);

	size_t piece_count = 0;
	t->piece_hashes = (uint8_t **)blacksail_bencode_find_dvalue_str(bencode->data, "pieces", &piece_count);

	t->total_size = blacksail_bencode_find_dvalue_int(bencode->data, "length");
	t->block_size = TORRENT_BLOCK_SIZE;
	t->piece_size = blacksail_bencode_find_dvalue_int(bencode->data, "piece length");
	t->piece_count = piece_count / 20;

	t->verified_piece_count = 0;
	t->verified_ratio = 0.00;

	return t;
}

bool create_torrent_thread(struct torrent *t)
{
	if (t == NULL) {
		return false;
	}

	struct torrent_thread **last = &threads;

	while ((*last) != NULL) {
		last = &(*last)->next;
	}

	struct torrent_thread *new = malloc(sizeof(struct torrent_thread));
	if (new == NULL) {
		return false;
	}


	new->id = 0;
	pthread_create(&new->thread, NULL, thread_init, new);

	// new->prev = *last;
	new->next = NULL;
	new->torrent[0] = t;

	// wait until the new thread sets the new id itself to indicate
	// it's ready to start
	while (new->id == 0) {
		usleep(MS_TO_US(20));
	}

	(*last) = new;

	return true;
}

void remove_torrent(struct torrent *t)
{
	blacksail_free_bencode_item(t->bencode);
	free(t->download_dir);
	free(t);
}

int blacksail_add_torrentf(const char *torrent_filepath, const char *download_path)
{
	FILE *torrent = fopen(torrent_filepath, "rb");
	if (torrent == NULL) {
		return 0;
	}

	fseek(torrent, 0, SEEK_END);
	size_t len = ftell(torrent);
	rewind(torrent);

	char *buf = malloc(sizeof(char) * len);
	if (buf == NULL) {
		return 0;
	}

	fread(buf, len, 1, torrent);
	fclose(torrent);

	struct bencode_item *bencode = blacksail_parse_bencode((uint8_t *)buf, len);
	if (bencode == NULL) {
		free(buf);
		return 0;
	}

	struct torrent *t = add_torrent(bencode, download_path);
	free(buf);

	// look if there's a free thread
	struct torrent_thread *cur = threads;
	while (cur != NULL) {
		for (int i = 0; i < BLACKSAIL_TORRENTS_PER_THREAD; i++) {
			if (cur->torrent[i] == NULL) {
				fprintf(stderr, "Found a free thread, joining the torrent and telling the thread.\n");
				cur->torrent[i] = t;
				pthread_kill(cur->thread, THREAD_UPDATE);
				// wait until the thread registered the new torrent.
				while (cur->torrent[i]->id == 0) {
					usleep(MS_TO_US(20));
				}
				return t->id;
			}
		}

		cur = cur->next;
	}

	// no free thread found, create one
	fprintf(stderr, "No free thread found, creating a new one.\n");
	int id = create_torrent_thread(t);
	if (id == 0) {
		remove_torrent(t);
	}

	return t->id;
}

void blacksail_remove_torrent(int id)
{
	struct torrent_thread *thread = threads;
	while (thread != NULL) {
		for (int i = 0; i < BLACKSAIL_TORRENTS_PER_THREAD; i++) {
			if (thread->torrent[i] == NULL)
				continue;

			if (thread->torrent[i]->id == id) {
				fprintf(stderr, "Attempting to kill torrent with ID %i\n", id);
				remove_torrent(thread->torrent[i]);
				thread->torrent[i] = NULL;
				break;
			}

		}

		// is this the last torrent in a thread?
		for (int i = 0; i < BLACKSAIL_TORRENTS_PER_THREAD; i++) {
			if (thread->torrent[i])
				break;

			fprintf(stderr, "No torrents in thread ID %i, killing...\n", thread->id);
			pthread_kill(thread->thread, THREAD_DIE);

			if (thread->prev != NULL) {
				thread->prev->next = thread->next;
			}
			thread->next->prev = thread->prev;

			free(thread);
			return;
		}

		thread = thread->next;
	}
}

void blacksail_remove_all_torrents(void)
{
	struct torrent_thread *thread = threads;
	struct torrent_thread *next;

	while (thread != NULL) {
		for (int i = 0; i < BLACKSAIL_TORRENTS_PER_THREAD; i++) {
			if (thread->torrent[i] == NULL)
				continue;

			fprintf(stderr, "Removing torrent with ID %i\n", thread->torrent[i]->id);
			remove_torrent(thread->torrent[i]);
		}

		next = thread->next;

		pthread_kill(thread->thread, THREAD_DIE);

		// wait until the thread clears its ID to let us know
		// it is dead
		while (thread->id != 0) {
			usleep(MS_TO_US(20));
		}

		free(thread);

		thread = next;
	}
}