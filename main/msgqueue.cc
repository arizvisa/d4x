/*	WebDownloader for X-Window
 *	Copyright (C) 1999-2001 Koshelev Maxim
 *	This Program is free but not GPL!!! You can't modify it
 *	without agreement with author. You can't distribute modified
 *	program but you can distribute unmodified program.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */
#include "msgqueue.h"

tMsgQueue::tMsgQueue():tQueue(){
	pthread_mutex_init(&mylock,NULL);
};

void tMsgQueue::insert(tNode *what){
	pthread_mutex_lock(&mylock);
	tQueue::insert(what);
	pthread_mutex_unlock(&mylock);
};

void tMsgQueue::insert_before(tNode *what,tNode *where){
	pthread_mutex_lock(&mylock);
	tQueue::insert_before(what,where);
	pthread_mutex_unlock(&mylock);
};

void tMsgQueue::del(tNode *what){
	pthread_mutex_lock(&mylock);
	tQueue::del(what);
	pthread_mutex_unlock(&mylock);
};

tMsgQueue::~tMsgQueue(){
	pthread_mutex_destroy(&mylock);
};
