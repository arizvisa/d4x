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
#ifndef __D4X_SOCKS_HEADER__
#define __D4X_SOCKS_HEADER__

#include "socket.h"
#include "locstr.h"

class tSocksSocket:public tSocket{
	tPStr socks_host;
	u_int16_t socks_port;
	tPStr user,pass;
	unsigned char socks_buf[10];
	u_int32_t bnd_host;
	u_int16_t bnd_port;
	void socks_init();
	int socks_connect_reply();
	int socks_bind_reply();
 public:
	tSocksSocket();
	tSocksSocket(char *host,u_int16_t port,char *use=NULL,char *pas=NULL);
	int open_any(char * host);
	int open_any(u_int32_t host);
	int accepting(char * host);
	int open_port(char * host,u_int16_t port);
	int open_port(u_int32_t host,u_int16_t port);
	unsigned int get_addr();
	unsigned short int get_port();
	~tSocksSocket(); 
};

#endif
