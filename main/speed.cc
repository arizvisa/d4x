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
#include <package_config.h>
#include <stdio.h>
#include "speed.h"
#include "signal.h"

tSpeed::tSpeed() {
	last_gived=base=bytes=0;
};

void tSpeed::print() {
	printf("%li\n",bytes);
};

fsize_t tSpeed::init(fsize_t a) {
	lock.lock();
	if (bytes<0) {
		last_gived=bytes=a;
		lock.unlock();
		lock1.unlock();
		return 0;
	};
	fsize_t temp=(last_gived>0?last_gived:a)-bytes;
	if((bytes=temp)<=0)
		bytes=1;
	last_gived=bytes;
	fsize_t rvalue=a-bytes;
	lock.unlock();
	return(rvalue);
};

void tSpeed::set(fsize_t a){
	lock.lock();
	if (bytes<0) {
		last_gived=bytes=a;
		lock.unlock();
		lock1.unlock();
	}else{
		last_gived=bytes=a;
		lock.unlock();
	};
};

void tSpeed::decrement(fsize_t a) {
	lock.lock();
	bytes-=a;
	if (bytes<0) {
		lock1.lock();
		lock.unlock();
		lock1.lock();
		lock1.unlock();
	} else {
		lock.unlock();
	};
};

tSpeed::~tSpeed() {
};

/*------------------------------------------------------------
 */
tSpeedQueue::tSpeedQueue():tQueue(){
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

void tSpeedQueue::schedule(fsize_t a,int flag) {
	if (Num==0) return;
	if (a){
		fsize_t part=a / Num;
		fsize_t Full=0;
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
