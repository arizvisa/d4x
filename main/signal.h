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
#ifndef THREAD_MANGER
#define THREAD_MANGER
#include "dlist.h"
#include <pthread.h>

void init_signal_handler();
void real_stop_thread(tDownload *what);
int stop_thread(tDownload *what);
void my_pthread_key_init();
tDownload **my_pthread_key_get();
void my_pthread_key_set(tDownload *what);
void my_pthreads_mutex_init(pthread_mutex_t *lock);

namespace d4x{
	class USR1Off2On{
	public:
		USR1Off2On();
		~USR1Off2On();
	};
	
	class USR1On2Off{
	public:
		USR1On2Off();
		~USR1On2Off();
	};
};

#endif
