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
#include "msgqueue.h"

tMsgQueue::tMsgQueue():tQueue(){
};

void tMsgQueue::insert(tNode *what){
	mylock.lock();
	tQueue::insert(what);
	mylock.unlock();
};

void tMsgQueue::insert_before(tNode *what,tNode *where){
	mylock.lock();
	tQueue::insert_before(what,where);
	mylock.unlock();
};

void tMsgQueue::del(tNode *what){
	mylock.lock();
	tQueue::del(what);
	mylock.unlock();
};

tMsgQueue::~tMsgQueue(){
};
