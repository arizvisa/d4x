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

typedef long int fsize_t;

struct tSpeed: public tNode{
	private:
int last_gived;
	public:
	pthread_mutex_t lock,lock1;
	fsize_t bytes,base;
	tSpeed();
	virtual void print();
	fsize_t init(fsize_t a);
	void set(fsize_t a);
	void decrement(fsize_t a);
	~tSpeed();
};


class tSpeedQueue:public tQueue{
	public:
		tSpeedQueue();
		virtual tSpeed *last();
		virtual tSpeed *first();
		virtual tSpeed *next();
		virtual tSpeed *prev();
		void schedule(fsize_t a,int flag);
		void schedule(unsigned int period);
		~tSpeedQueue();
};
#endif
