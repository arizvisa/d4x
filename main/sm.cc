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

#include "sm.h"

d4xOldSocket::d4xOldSocket(){
	info=NULL;
	sock=NULL;
};

d4xOldSocket::d4xOldSocket(tAddr *a, tSocket *s){
	info=a;
	sock=s;
};

void d4xOldSocket::print(){
};

d4xOldSocket::~d4xOldSocket(){
	if (info && info->proto==D_PROTO_FTP && sock)
		sock->direct_send("QUIT\r\n");
	if (sock) delete(sock);
	if (info) delete(info);
};

/*********************************************************/

d4xSocketsHistory::d4xSocketsHistory():tQueue(){
	MaxNum=50; /* FIXME: it's bad to use such constants*/
};

d4xSocketsHistory::~d4xSocketsHistory(){
};


void d4xSocketsHistory::insert(d4xOldSocket *what){
	my_lock.lock();
	what->birth=time(NULL);
	tQueue::insert(what);
	my_lock.unlock();
};

void d4xSocketsHistory::del(d4xOldSocket *what){
	my_lock.lock();
	tQueue::del(what);
	my_lock.unlock();
};


tSocket *d4xSocketsHistory::find(tAddr *info){
	my_lock.lock();
	d4xOldSocket *s=(d4xOldSocket *)First;
	while(s){
		if (equal(s->info->host.get(),info->host.get()) &&
		    s->info->port==info->port &&
		    s->info->proto==info->proto &&
		    equal(s->info->username.get(),info->username.get())){
			tQueue::del(s);
			tSocket *rval=s->sock;
			s->sock=NULL;
			delete(s);
			my_lock.unlock();
			return(rval);
		}
		s=(d4xOldSocket *)(s->prev);
	};
	my_lock.unlock();
	return(NULL);
};

void d4xSocketsHistory::kill_old(){
	time_t now=time(NULL);
	my_lock.lock();
	while(First){
		d4xOldSocket *s=(d4xOldSocket *)First;
		time_t diff=s->birth-now;
		/* FIXME: constants again */
		if (diff<-50 || diff>50){
			tQueue::del(s);
			delete(s);
		}else
			break;
	};
	my_lock.unlock();
};
