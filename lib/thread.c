#include <blacksail_utils.h>
#include <blacksail/tracker.h>
#include <blacksail/config.h>
#include <torrent_thread.h>
#include <pthread.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

extern struct blacksail_config cfg;
extern int next_thread_id;
extern int next_torrent_id;

__thread struct torrent_thread *self;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void thread_die(int arg)
{
	self->id = 0;
	pthread_exit(0);
}

void thread_announce_torrents()
{
	for (int i = 0; i < BLACKSAIL_TORRENTS_PER_THREAD; i++) {
		if (self->torrent[i] == NULL)
			continue;

		fprintf(stderr, "Sending update for torrent ID %i\n", self->torrent[i]->id);
		if (self->torrent[i]->status == TORRENT_STARTED) {
			blacksail_announce_torrent(self->torrent[i], &mutex);
		}
	}
}

void thread_update(void)
{
	fprintf(stderr, "(%i): Got an update message, checking for news...\n", self->id);
}

void *thread_init(void *arg)
{
	self = arg;

	// register signal handlers
	signal(THREAD_DIE, thread_die);
	signal(THREAD_UPDATE, SIG_IGN);

	// assign a new id to let the main thread know we're ready to roll
	self->id = next_thread_id++;

	// update loop
	while (1) {
		thread_announce_torrents();

		// calculate time until next interval
		usleep(MS_TO_US(2000));
	}

	return NULL;
}
