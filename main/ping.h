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
#ifndef __D4X_PING_CLASS_HEADER__
#define __D4X_PING_CLASS_HEADER__

#include <sys/poll.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include "dlist.h"
#include "var.h"

struct d4xAccessSpeed{
	tDownload *ref;
};

class d4xPing{
	/* tmp data for gethostbyname */
	sockaddr_in info;
	hostent hp;
	char buf[MAX_LEN];
	int rval;
	/* info about hosts */
	int size;
	int TOTAL;
	d4xAccessSpeed *data;
	struct pollfd *pf;
 public:
	d4xPing();
	void run(tDList *list,tWriterLoger *WL);
	~d4xPing();
};

#endif
