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
#ifndef MUTEXABLE_DLIST
#define MUTEXABLE_DLIST

#include <pthread.h>
#include "dlist.h"

class tDListMutex:public tDList{
	pthread_mutex_t mutex;
	public:
		void init(int n);
		void insert(tNode *what);
		void del(tNode *what);
		void dispose();
		tDownload *last();
		tDownload *next();
		tDownload *first();
};

#endif
