#ifndef _D4X_MUTEX_HEADER_
#define _D4X_MUTEX_HEADER_
#include <pthread.h>

struct d4xMutex{
	pthread_mutex_t m;
	d4xMutex();
	~d4xMutex();
	void lock();
	void unlock();
};

#endif
