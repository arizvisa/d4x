#include "mutex.h"
#include "signal.h"

/* simple mutex */

d4xMutex::d4xMutex(){
//	my_pthreads_mutex_init(&m);
	pthread_mutex_init(&m,NULL);
};

d4xMutex::~d4xMutex(){
	pthread_mutex_destroy(&m);
};

void d4xMutex::lock(){
	pthread_mutex_lock(&m);
};

void d4xMutex::unlock(){
	pthread_mutex_unlock(&m);
};

