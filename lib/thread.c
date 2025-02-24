#include <blacksail_utils.h>
#include <blacksail/tracker.h>
#include <torrent_thread.h>
#include <pthread.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

extern int next_thread_id;
extern int next_torrent_id;

__thread struct torrent_thread *self;

void thread_die(int arg)
{
	self->id = 0;
	pthread_exit(0);
}

size_t thread_update(void)
{
	// TODO: Set this to the nearest torrent announce interval
	size_t next_update = 2000; // ms
	fprintf(stderr, "(%i): Got an update message, checking for news...\n", self->id);

	return next_update;
}

void *thread_init(void *arg)
{
	self = arg;

	// register signal handlers
	signal(THREAD_DIE, thread_die);
	signal(THREAD_UPDATE, SIG_IGN);

	// assign a new id to let the main thread know we're ready to roll
	self->id = next_thread_id;

	// update loop
	while (1) {
		size_t wait_time = thread_update();
		usleep(MS_TO_US(wait_time));
	}

	return NULL;
}
