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
#ifndef DOWNLOADER_SPEEDS_HEADER
#define DOWNLOADER_SPEEDS_HEADER

#include "queue.h"
#include <pthread.h>

struct tSpeed: public tNode{
	pthread_mutex_t lock,lock1;
	int bytes,base;
	tSpeed();
	virtual void print();
	int init(int a);
	void decrement(int a);
	~tSpeed();
};


class tSpeedQueue:public tQueue{
	public:
		tSpeedQueue();
		virtual tSpeed *last();
		virtual tSpeed *first();
		virtual tSpeed *next();
		virtual tSpeed *prev();
		void schedule(int a);
		~tSpeedQueue();
};
#endif
