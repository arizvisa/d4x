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
#ifndef __D4X_SOCKS_HEADER__
#define __D4X_SOCKS_HEADER__

#include "socket.h"
#include "locstr.h"

class tSocksSocket:public tSocket{
	tPStr socks_host;
	guint16 socks_port;
	tPStr user,pass;
	unsigned char socks_buf[10];
	guint32 bnd_host;
	guint16 bnd_port;
	void socks_init();
	int socks_connect_reply();
	int socks_bind_reply();
 public:
	tSocksSocket();
	tSocksSocket(char *host,guint16 port,char *use=(char *)NULL,char *pas=(char *)NULL);
	int open_any(char * host);
	int open_any(guint32 host);
	int accepting(char * host);
	int open_port(char * host,guint16 port);
	int open_port(guint32 host,guint16 port);
	unsigned int get_addr();
	unsigned short int get_port();
	~tSocksSocket(); 
};

#endif
