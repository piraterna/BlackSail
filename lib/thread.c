#include <blacksail_utils.h>
#include <blacksail/tracker.h>
#include <torrent_thread.h>
#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>

extern int next_thread_id;

__thread struct torrent_thread *self;

void thread_die(int arg)
{
	//pthread_cancel();
	fprintf(stderr, "(%i): Oh no! I should die now!\n", self->id);
	while (1) {
		sleep(10);
	}
}

void *thread_update(void *arg)
{
	self = arg;

	// register signal handlers
	signal(THREAD_DIE, thread_die);

	// assign a new id to let the main thread know we're ready to roll
	self->id = next_thread_id;

	fprintf(stderr, "[*] Hello, I'm a new torrent with ID %i!\n", self->id);

	// update loop
	while (1) {
		fprintf(stderr, "(%i): Still alive!\n", self->id);
		usleep(MS_TO_US(1000));
	}

	return NULL;
}