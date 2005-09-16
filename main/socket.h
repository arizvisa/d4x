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
#ifndef D4X_SOCKET_HEADER
#define D4X_SOCKET_HEADER

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/time.h>
#include <fcntl.h>
#include <glib.h>
#include "speed.h"

/*

#include <stdexcept>

namespace d4x{
	class NetException:public std::runtime_error{
	};
	class StopException:public std::runtime_error{
	};
};

*/

class tSocket{
 protected:
	fd_set set;
	int fd;
	sockaddr_in info;
	hostent hp;
	int temp_variable;
	int con_flag;
	char *buffer;
	int RBytes,SBytes;
	int constr_name(const char *host,guint16 port);
	int wait_for_read(int len);
	int wait_for_write(int len);
	virtual fsize_t lowlevel_read(char *where,fsize_t len);
 public:
	tSocket();
	virtual unsigned int get_addr();
	virtual unsigned short int get_port();
	int readed_bytes();
	virtual int open_any(const char * host);
	virtual int open_any(guint32 host);
	virtual int accepting(const char * host);
	virtual int open_port(const char * host,guint16 port);
	virtual int open_port(guint32 host,guint16 port);
	int open_port(int *ftp_addr);
	int direct_send(char *what);
	virtual int send_string(const char *what,int timeout);
	fsize_t rec_string(char * where,fsize_t len,int timeout);
	void flush();
	int connected();
	virtual void down();
	virtual ~tSocket(); 
};

#define SOCKET_UNKNOWN_HOST -1
#define SOCKET_CANT_ALLOCATE -2
#define SOCKET_CANT_CONNECT -3

int my_get_host_by_name(const char *host,
			int port,
			sockaddr_in *info,
			hostent *hp,
			char *buf,
			int bufsize,
			int *rvalbuf);

#endif
