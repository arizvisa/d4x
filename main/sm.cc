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
#include <functional>

using namespace d4x;

OldSocket::OldSocket(const d4x::URL &a, tSocket *s):info(a),sock(s),birth(time(NULL)){
};

OldSocket::~OldSocket(){
	if (info.proto==D_PROTO_FTP && sock)
		sock->direct_send("QUIT\r\n");
	if (sock) delete(sock);
};

/*********************************************************/

static void _destroy_(OldSocket *s){
	delete s;
};


SocketsHistory::~SocketsHistory(){
	std::for_each(lst.begin(),lst.end(),_destroy_);
};


void SocketsHistory::insert(const URL&_u,tSocket *what){
	my_lock.lock();
	lst.push_back(new OldSocket(_u,what));
	my_lock.unlock();
};


void SocketsHistory::del(OldSocket *what){
	my_lock.lock();
	lst.erase(std::remove(lst.begin(),lst.end(),what),lst.end());
	my_lock.unlock();
};

static bool _cmp_with_url_(URL *info,OldSocket *s){
	return *info==s->info;
};

tSocket *SocketsHistory::find(const URL &info){
	my_lock.lock();
	tSocket *rval=0;
	OldSockList::iterator it=find_if(lst.begin(),lst.end(),std::bind1st(std::ptr_fun(_cmp_with_url_),&info));
	if (it!=lst.end()){
		OldSocket *old=*it;
		rval=(*it)->sock;
		(*it)->sock=0;
		lst.erase(it);
		delete old;
	};
	my_lock.unlock();
	return rval;
};

static bool _time_too_old_(time_t now,OldSocket *s){
	time_t d=s->birth-now;
	return d<-50 || d>50;
};

void SocketsHistory::kill_old(){
	time_t now=time(NULL);
	my_lock.lock();
	OldSockList::iterator it=std::remove_if(lst.begin(),lst.end(),std::bind1st(std::ptr_fun(_time_too_old_),now));
	std::for_each(it,lst.end(),_destroy_);
	lst.erase(it,lst.end());
	my_lock.unlock();
};
