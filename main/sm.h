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
#ifndef _D4X_SOCKETS_LIST_
#define _D4X_SOCKETS_LIST_

#include "addr.h"
#include "socket.h"
#include "mutex.h"
#include <time.h>
#include <map>
#include <boost/smart_ptr.hpp>

namespace d4x{
	struct OldSocket{
		SocketPtr sock;
		time_t birth;
		OldSocket(){};
		OldSocket(const OldSocket &s):sock(s.sock),birth(s.birth){};
		OldSocket(const SocketPtr &s):sock(s),birth(time(NULL)){};
		OldSocket(tSocket *s):sock(s),birth(time(NULL)){};
	};
	
	class SocketsHistory{
		Mutex my_lock;
		typedef std::map<URL,OldSocket> OldSockMap;
		OldSockMap Smap;
	public:
		void insert(const URL &u,const SocketPtr  &s);
		SocketPtr find_and_remove(const URL &info);
		void kill_old();
	};

};

#endif /* ifndef _D4X_SOCKETS_LIST_ */
