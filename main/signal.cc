/*	WebDownloader for X-Window
 *	Copyright (C) 1999-2000 Koshelev Maxim
 *	This Program is free but not GPL!!! You can't modify it
 *	without agreement with author. You can't distribute modified
 *	program but you can distribute unmodified program.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include "dlist.h"
#include "face/log.h"
#include "var.h"
#include "ntlocale.h"

static pthread_key_t THREADS_KEY;
static pthread_once_t THREADS_KEY_ONCE=PTHREAD_ONCE_INIT;

static void my_pthread_key_destroy(void *key) {
	char *temp=(char *)key;
	if (temp) delete[] temp;
};

static void my_pthread_key_alloc() {
	pthread_key_create(&THREADS_KEY,my_pthread_key_destroy);
};

void my_pthread_key_init() {
	tDownload *temp;
	pthread_once(&THREADS_KEY_ONCE,my_pthread_key_alloc);
	pthread_setspecific(THREADS_KEY,new char[sizeof(temp)]);
};

tDownload **my_pthread_key_get() {
	return (tDownload **)pthread_getspecific(THREADS_KEY);
};


void my_pthread_key_set(tDownload *what){
	tDownload **temp=my_pthread_key_get();
	if (temp) *temp=what;
};

void download_set_block(int a){
	tDownload **temp=(my_pthread_key_get());
	if (temp && *temp){
		if ((*temp)->BLOCKED==2){
			(*temp)->LOG->add(_("Download  was stopped by user"),LOG_WARNING);
			(*temp)->status=DOWNLOAD_REAL_STOP;
			pthread_exit(NULL);
		};
		(*temp)->BLOCKED=a;
	};
};

void signal_handler(int num) {
	tDownload **temp=(my_pthread_key_get());
	if (temp){
		// check for entering in gethostbyname routine
		// or any other glibc routine which locks global lock
		if ((*temp)->BLOCKED){
			(*temp)->BLOCKED=2;
			return;
		};
		// to avoid locking if currently string adding in log
		(*temp)->LOG->unlock();
		(*temp)->LOG->add(_("Download  was stopped by user"),LOG_WARNING);
		//	temp->who->done();
		(*temp)->status=DOWNLOAD_REAL_STOP;
	};
	pthread_exit(NULL);
};

void init_signal_handler() {
	struct sigaction action,old_action;
	action.sa_handler=signal_handler;
	action.sa_flags=0;
	sigaction(SIGUSR1,&action,&old_action);
	sigset_t oldmask,newmask;
	sigemptyset(&newmask);
	sigaddset(&newmask,SIGINT);
	sigaddset(&newmask,SIGTERM);
	sigaddset(&newmask,SIGUSR2);
	pthread_sigmask(SIG_BLOCK,&newmask,&oldmask);
};

int stop_thread(tDownload *what) {
	if (what->thread_id==0)  return 1;
	return(pthread_kill(what->thread_id,SIGUSR1));
};

void real_stop_thread(tDownload *what) {
	int *rc;
	if (what->thread_id)
		pthread_join(what->thread_id,(void **)&rc);
	what->delete_who();
	what->thread_id=0;
};


void my_pthreads_mutex_init(pthread_mutex_t *lock){
#if defined(__linux__)
/* manual page for mutexes said that mutexes in linux is fast by
   default...
 */
//	pthread_mutexattr_setkind_np(&ma,PTHREAD_MUTEX_FAST_NP);
	pthread_mutex_init(lock,NULL);
#else
	pthread_mutexattr_t ma;
	pthread_mutexattr_init(&ma);
#if !defined (__sparc__) && !defined(__mips__)
	pthread_mutexattr_settype(&ma,MUTEX_TYPE_FAST);
#elif defined(__mips__)
	pthread_mutexattr_settype(&ma,MUTEX_TYPE_NORMAL);	
#endif
	pthread_mutex_init(lock,&ma);
	pthread_mutexattr_destroy(&ma);
#endif
};
