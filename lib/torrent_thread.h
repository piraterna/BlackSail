#ifndef _BLACKSAIL_THREAD_H
#define _BLACKSAIL_THREAD_H

#include <blacksail/blacksail.h>
#include <signal.h>
#include <pthread.h>

#define THREAD_DIE SIGUSR1
#define THREAD_UPDATE SIGUSR2

struct torrent_thread {
	int id;
	pthread_t thread;
	struct torrent *torrent[BLACKSAIL_TORRENTS_PER_THREAD];

	struct torrent_thread *prev;
	struct torrent_thread *next;
};

void *thread_init(void *arg);

#endif /* _BLACKSAIL_THREAD_H */
