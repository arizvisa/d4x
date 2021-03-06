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
#ifndef __MY_MSG_QUEUE_HEADER__
#define __MY_MSG_QUEUE_HEADER__

#include "queue.h"
#include <pthread.h>
#include "log.h"
#include "mutex.h"

enum MSG_QUEUE_TYPES{
	MQT_MY=1,
	MQT_COM=2
};

struct tLogMsg:public tNode{
	long type;
	tLogString *what;
	tLog *which;
	void print(){};
};

class tMsgQueue:public tQueue{
	d4x::Mutex mylock;
 public:
	tMsgQueue();
	void insert(tNode *what);
    	void insert_before(tNode *what,tNode *where);
	void remove_this_log(tLog *log);
	void del(tNode *what);
	~tMsgQueue();
};

#endif
