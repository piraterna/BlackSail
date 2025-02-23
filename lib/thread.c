#include <blacksail_utils.h>
#include <blacksail/tracker.h>
#include <torrent_thread.h>
#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>

extern int next_thread_id;

__thread struct torrent_thread *self;

void thread_update(int arg);

void thread_die(int arg)
{
	self->id = 0;
	pthread_exit(0);
}

void *thread_init(void *arg)
{
	self = arg;

	// register signal handlers
	signal(THREAD_DIE, thread_die);
	signal(THREAD_UPDATE, thread_update);

	// assign a new id to let the main thread know we're ready to roll
	self->id = next_thread_id;

	// update loop
	while (1) {
		thread_update(0);
		usleep(MS_TO_US(10000));
	}

	return NULL;
}

void thread_update(int arg)
{
	//if (cur_time < self->torrent->earliest_interval) {
	//	return;
	//}
}