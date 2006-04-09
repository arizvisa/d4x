/*	WebDownloader for X-Window
 *	Copyright (C) 1999-2006 Koshelev Maxim
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


/*
OldSocket::~OldSocket(){
	if (info.proto==D_PROTO_FTP && sock)
		sock->direct_send("QUIT\r\n");
	if (sock) delete(sock);
};
*/

/*********************************************************/


void SocketsHistory::insert(const URL &url,const SocketPtr &what){
	MutexLocker gain(my_lock);
	Smap[url]=OldSocket(what);
};

SocketPtr SocketsHistory::find_and_remove(const URL &url){
	MutexLocker gain(my_lock);
	std::map<URL,OldSocket>::iterator it=Smap.find(url);
	SocketPtr rval;
	if (it!=Smap.end()){
		rval=it->second.sock;
		Smap.erase(it);
	};
	return rval;
};

static bool time_too_old_(time_t now,const OldSocket &s){
	time_t d=s.birth-now;
	return d<-50 || d>50;
};

void SocketsHistory::kill_old(){
	time_t now=time(NULL);
	MutexLocker gain(my_lock);

	std::map<URL,OldSocket>::iterator it=Smap.begin(),end=Smap.end(),tmp;
	while(it!=end){
		tmp=it;
		++it;
		if (time_too_old_(now,it->second))
			Smap.erase(tmp);
		++it;
	};
};
