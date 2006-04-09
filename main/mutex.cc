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

using namespace d4x;

Mutex::Mutex(){
//	my_pthreads_mutex_init(&m);
	pthread_mutex_init(&m,NULL);
};

Mutex::~Mutex(){
	pthread_mutex_destroy(&m);
};

void Mutex::lock(){
	pthread_mutex_lock(&m);
};

void Mutex::unlock(){
	pthread_mutex_unlock(&m);
};

