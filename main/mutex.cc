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

