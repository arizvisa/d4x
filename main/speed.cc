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
#if defined(__linux__)
/* manual page for mutexes said that mutexes in linux is fast by
   default...
 */
//	pthread_mutexattr_setkind_np(&ma,PTHREAD_MUTEX_FAST_NP);
	pthread_mutex_init(&lock1,NULL);
#else
	pthread_mutexattr_t ma;
	pthread_mutexattr_init(&ma);
	pthread_mutexattr_settype(&ma,MUTEX_TYPE_FAST);
	pthread_mutex_init(&lock1,&ma);
	pthread_mutexattr_destroy(&ma);
#endif
	last_gived=base=bytes=0;
};

void tSpeed::print() {
	printf("%i\n",bytes);
};

int tSpeed::init(int a) {
	pthread_mutex_lock(&lock);
	if (bytes<0) {
		last_gived=bytes=a;
		pthread_mutex_unlock(&lock);
		pthread_mutex_unlock(&lock1);
		return 0;
	};
	int temp=(last_gived>0?last_gived:a)-bytes;
	if((bytes=temp)<=0)
		bytes=1;
	last_gived=bytes;
	int rvalue=a-bytes;
	pthread_mutex_unlock(&lock);
	return(rvalue);
};

void tSpeed::set(int a){
	pthread_mutex_lock(&lock);
	if (bytes<0) {
		last_gived=bytes=a;
		pthread_mutex_unlock(&lock);
		pthread_mutex_unlock(&lock1);
	}else{
		last_gived=bytes=a;		
		pthread_mutex_unlock(&lock);
	};
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
	First=Last=Curent;
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

void tSpeedQueue::schedule(unsigned int period) {
	tSpeed *temp=last();
	while (temp) {
		temp->set((temp->base*period)/1000);
		temp=next();
	};
};

void tSpeedQueue::schedule(int a,int flag) {
	if (Num==0) return;
	if (a){
		int part=a / Num;
		int Full=0;
		if (part<=0) part=1;
		tSpeed *temp=last();
		tSpeed *tmpbeg=NULL;
		int size=Num;
		while (temp) {
			tSpeed *tmpnext=next();
			if (temp->bytes<0 && flag){
				del(temp);
				temp->next=tmpbeg;
				tmpbeg=temp;
			}else{
				Full+=temp->init(part);
			};
			temp=tmpnext;
		};
		if (size-Num>0)
			part+=Full/(size-Num);
//			part+=int(((float(Full)*float(0.7)))/(size-Num));
		while(tmpbeg){
			tSpeed *tmpnext=(tSpeed *)(tmpbeg->next);
			tmpbeg->init(part);
			insert(tmpbeg);
			tmpbeg=tmpnext;
		};
	};
};

tSpeedQueue::~tSpeedQueue() {};
