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
#ifndef _D4X_SOCKETS_LIST_
#define _D4X_SOCKETS_LIST_

#include "queue.h"
#include "addr.h"
#include "socket.h"
#include <pthread.h>
#include <time.h>

struct d4xOldSocket:public tNode{
	tAddr *info;
	tSocket *sock;
	time_t birth;
	d4xOldSocket();
	d4xOldSocket(tAddr *a, tSocket *s);
	void print();
	~d4xOldSocket();
};

class d4xSocketsHistory:public tQueue{
	pthread_mutex_t my_lock;
 public:
	d4xSocketsHistory();
	void insert(d4xOldSocket *what);
	void del(d4xOldSocket *what);
	tSocket *find(tAddr *info);
	void kill_old();
	~d4xSocketsHistory();
};

#endif /* ifndef _D4X_SOCKETS_LIST_ */
