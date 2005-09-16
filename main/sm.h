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
#include <vector>

namespace d4x{
	struct OldSocket{
		d4x::URL info;
		tSocket *sock;
		time_t birth;
		OldSocket(const d4x::URL &a, tSocket *s);
		~OldSocket();
	};
	
	class SocketsHistory{
		d4xMutex my_lock;
		typedef std::vector<OldSocket *> OldSockList;
		OldSockList lst;
		void del(OldSocket *what);
	public:
		void insert(const URL &u,tSocket *s);
		tSocket *find(const d4x::URL &info);
		void kill_old();
		~SocketsHistory();
	};

};

#endif /* ifndef _D4X_SOCKETS_LIST_ */
