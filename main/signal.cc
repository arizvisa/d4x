/*	WebDownloader for X-Window
 *	Copyright (C) 1999-2002 Koshelev Maxim
 *	This Program is free but not GPL!!! You can't modify it
 *	without agreement with author. You can't distribute modified
 *	program but you can distribute unmodified program.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <pthread.h>
#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include "dlist.h"
#include "face/log.h"
#include "var.h"
#include "ntlocale.h"
#include  "signal.h"

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

void signal_handler(int num) {
	tDownload **temp=(my_pthread_key_get());
	if (temp){
		(*temp)->LOG->add(_("Download  was stopped by user"),LOG_WARNING);
		if ((*temp)->segments)
			(*temp)->segments->save();
		D4X_UPDATE.add(*temp,DOWNLOAD_REAL_STOP);
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
	sigaddset(&newmask,SIGUSR1);
	pthread_sigmask(SIG_BLOCK,&newmask,&oldmask);
};


int stop_thread(tDownload *what) {
	if (what->thread_id==0)  return 1;
	what->STOPPED_BY_USER=true;
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


using namespace d4x;

USR1Off2On::USR1Off2On(){
	tDownload **temp=(my_pthread_key_get());
	if (temp && (*temp)->STOPPED_BY_USER)
		signal_handler(SIGUSR1);
	
	sigset_t oldmask,newmask;
	sigemptyset(&newmask);
	sigaddset(&newmask,SIGUSR1);
	pthread_sigmask(SIG_UNBLOCK,&newmask,&oldmask);
};

USR1Off2On::~USR1Off2On(){
	sigset_t oldmask,newmask;
	sigemptyset(&newmask);
	sigaddset(&newmask,SIGUSR1);
	pthread_sigmask(SIG_BLOCK,&newmask,&oldmask);
};


USR1On2Off::USR1On2Off(){
	sigset_t oldmask,newmask;
	sigemptyset(&newmask);
	sigaddset(&newmask,SIGUSR1);
	pthread_sigmask(SIG_UNBLOCK,&newmask,&oldmask);
};

USR1On2Off::~USR1On2Off(){
	tDownload **temp=(my_pthread_key_get());
	if (temp && (*temp)->STOPPED_BY_USER)
		signal_handler(SIGUSR1);
	
	sigset_t oldmask,newmask;
	sigemptyset(&newmask);
	sigaddset(&newmask,SIGUSR1);
	pthread_sigmask(SIG_BLOCK,&newmask,&oldmask);
};
