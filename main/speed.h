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
#ifndef DOWNLOADER_SPEEDS_HEADER
#define DOWNLOADER_SPEEDS_HEADER

#include "queue.h"
#include <pthread.h>
#include "mutex.h"

typedef long long fsize_t;

struct tSpeed: public tNode{
	private:
int last_gived;
	public:
	d4xMutex lock,lock1;
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

class d4xSpeedCalc{
	d4xMutex lock;
	fsize_t loaded;
	time_t start;
	int counter;
public:
	d4xSpeedCalc():loaded(0),counter(0){};
	void inc(fsize_t val){
		lock.lock();
		loaded+=val;
		if(counter++>1024){
			time_t r=time(NULL);
			time_t period=r-start;
			if (period){
				loaded=loaded/period;
				start=r-1;
			};
			counter=0;
		};
		lock.unlock();
	};
	void reset(){
		loaded=0;
		start=time(NULL);
	};
	fsize_t speed(){
		time_t period=time(NULL)-start;
		if (period) return(loaded/period);
		return(0);
	};
};

#endif
