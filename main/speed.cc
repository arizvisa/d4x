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

#include <stdio.h>
#include "speed.h"
#include "signal.h"
#include "dqueue.h"
#include <list>
#include <functional>
#include <algorithm>

using namespace d4x;

Speed::Speed():last_gived(0),bytes(0),base(0),base2(0){
};

fsize_t Speed::init(fsize_t a) {
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

void Speed::set(fsize_t a){
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

void Speed::decrement(fsize_t a) {
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

/*------------------------------------------------------------
 */
SpeedQueue::SpeedQueue(){
};

void SpeedQueue::insert(d4xDownloadQueue *q){
	queues.insert(q);
};
void SpeedQueue::del(d4xDownloadQueue *q){
	tDownload *d=q->first(DL_RUN);
	while (d){
		if (d->SpeedLimit) d->SpeedLimit->base2=0;
		if (d->split){
			tDownload *c=d->split->next_part;
			while(c){
				if (c->SpeedLimit) c->SpeedLimit->base2=0;
				c=c->split?c->split->next_part:0;
			};
		};
		d=(tDownload *)(d->prev);
	}
	queues.erase(q);
};

void SpeedQueue::insert(Speed *s){
	tolimit.insert(s);
};
void SpeedQueue::del(Speed *s){
	tolimit.erase(s);
};

static void _set_limitation_(unsigned int period,Speed *s){
	s->set((s->base*period)/1000);
};

namespace{
	struct TmpStore{
		SpeedQueue *q;
		fsize_t part;
	};
};

inline static void _fix_speed_limit_(std::list<Speed *>&spdlst,tDownload *d){
	if (d->SpeedLimit) spdlst.push_back(d->SpeedLimit);
	if (d->split){
		tDownload *c=d->split->next_part;
		while(c){
			if (c->SpeedLimit) spdlst.push_back(c->SpeedLimit);
			c=c->split?c->split->next_part:0;
		};
	};
};

static void _schedule_(TmpStore *tmp,d4xDownloadQueue *q){
	if (q->SpdLmt<=0) return;
	std::list<Speed *> spdlst;
	tDownload *d=q->first(DL_RUN);
	while (d){
		_fix_speed_limit_(spdlst,d);
		d=(tDownload *)(d->prev);
	};
	d=q->first(DL_STOPWAIT);
	while (d){
		_fix_speed_limit_(spdlst,d);
		d=(tDownload *)(d->prev);
	};
	d=q->first(DL_SIZEQUERY);
	while (d){
		_fix_speed_limit_(spdlst,d);
		d=(tDownload *)(d->prev);
	};
	tmp->q->schedule(spdlst,(q->SpdLmt*tmp->part)/1000);
};

void SpeedQueue::schedule(unsigned int period) {
	TmpStore tmp={this,period};
	std::for_each(queues.begin(),queues.end(),
		      std::bind1st(std::ptr_fun(_schedule_),&tmp));
	
	for_each(tolimit.begin(),tolimit.end(),
		 std::bind1st(std::ptr_fun(_set_limitation_),period));

	std::copy(qskip.begin(),qskip.end(),std::inserter(tolimit,tolimit.begin()));
	qskip.clear();
};


static void _update_limitation_(TmpStore *t,Speed *s){
	s->init(t->part);
	t->q->insert(s);
};

void SpeedQueue::schedule(fsize_t a,int flag) {
	if (tolimit.empty()) return;
	int Num=tolimit.size();
	if (a){
		fsize_t part=a / Num;
		fsize_t Full=0;
		if (part<=0) part=1;
		std::list<Speed *> skiplst;
		int size=Num;
		for(std::set<Speed*>::iterator it=tolimit.begin();it!=tolimit.end();){
			if ((*it)->bytes<0 && flag){
				std::set<Speed*>::iterator nxt=it;
				std::advance(nxt,1);
				skiplst.push_back(*it);
				tolimit.erase(it);
				it=nxt;
			}else{
				Full+=(*it)->init(part);
				it++;
			};
		};
		Num=tolimit.size();
		if (size-Num>0)
			part+=Full/(size-Num);
//			part+=int(((float(Full)*float(0.7)))/(size-Num));
		TmpStore t={this,part};
		std::for_each(skiplst.begin(),skiplst.end(),std::bind1st(std::ptr_fun(_update_limitation_),&t));
	};
};

static void _update_limitation2_(fsize_t part,Speed *s){
	s->init(part);
};

void SpeedQueue::schedule(std::list<Speed*> &lst,fsize_t a){
	if (lst.empty()) return;
	int Num=lst.size();
	if (a){
		fsize_t part=a / Num;
		fsize_t Full=0;
		if (part<=0) part=1;
		std::list<Speed *> skiplst;
		int size=Num;
		for(std::list<Speed*>::iterator it=lst.begin();it!=lst.end();){
			(*it)->base2=part;
			tolimit.erase(*it);
			qskip.push_back(*it);
			if ((*it)->bytes<0){
				std::list<Speed*>::iterator nxt=it;
				std::advance(nxt,1);
				skiplst.push_back(*it);
				lst.erase(it);
				it=nxt;
			}else{
				Full+=(*it)->init(part);
				it++;
			};
		};
		Num=lst.size();
		if (size-Num>0)
			part+=Full/(size-Num);
		std::for_each(skiplst.begin(),skiplst.end(),std::bind1st(std::ptr_fun(_update_limitation2_),part));
	};
	
};

SpeedQueue::~SpeedQueue() {};
