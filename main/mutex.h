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
