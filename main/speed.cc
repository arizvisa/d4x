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

#include <stdio.h>
#include "speed.h"

tSpeed::tSpeed() {
	pthread_mutex_init(&lock,NULL);
	pthread_mutex_init(&lock1,NULL);
	base=bytes=0;
};

void tSpeed::print() {
	printf("%i\n",bytes);
};

int tSpeed::init(int a) {
	pthread_mutex_lock(&lock);
	if (bytes<0) {
		bytes=a;
		pthread_mutex_unlock(&lock);
		pthread_mutex_unlock(&lock1);
		return 0;
	};
	int temp=a-bytes;
	if (temp<=0) {
		pthread_mutex_unlock(&lock);
		return a;
	};
	bytes+=temp;
	pthread_mutex_unlock(&lock);
	return a-temp;
};

void tSpeed::decrement(int a) {
	pthread_mutex_lock(&lock);
	bytes-=a;
	if (bytes<0) {
		pthread_mutex_lock(&lock1);
		pthread_mutex_unlock(&lock);
		pthread_mutex_lock(&lock1);
		pthread_mutex_unlock(&lock1);
	} else {
		pthread_mutex_unlock(&lock);
	};
};

tSpeed::~tSpeed() {
	pthread_mutex_destroy(&lock);
	pthread_mutex_destroy(&lock1);
};

/*------------------------------------------------------------
 */
tSpeedQueue::tSpeedQueue() {
	MaxNum=0;
	Num=0;
};

tSpeed *tSpeedQueue::last() {
	return (tSpeed *)(tQueue::last());
};

tSpeed *tSpeedQueue::first() {
	return (tSpeed *)(tQueue::first());
};

tSpeed *tSpeedQueue::next() {
	return (tSpeed *)(tQueue::next());
};

tSpeed *tSpeedQueue::prev() {
	return (tSpeed *)(tQueue::prev());
};

void tSpeedQueue::schedule(int a) {
	if (Num==0) return;
	if (a){
		int part=a / Num;
		if (part<=0) part=1;
		tSpeed *temp=last();
		while (temp) {
			temp->init(part);
			temp=next();
		};
	}else{
		tSpeed *temp=last();
		while (temp) {
			temp->init(temp->base);
			temp=next();
		};
	};
};

tSpeedQueue::~tSpeedQueue() {};
