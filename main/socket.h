/*	WebDownloader for X-Window
 *	Copyright (C) 1999-2000 Koshelev Maxim
 *	This Program is free but not GPL!!! You can't modify it
 *	without agreement with author. You can't distribute modified
 *	program but you can distribute unmodified program.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */
#ifndef T_SOCKET
#define T_SOCKET

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

#include "fcntl.h"

class tSocket{
	fd_set set;
	int fd;
	sockaddr_in info;
	hostent hp;
	int temp_variable;
	char *buffer;
	int RBytes,SBytes;
	int constr_name(char *host,int port);
	int wait_for_read(int len);
	int wait_for_write(int len); 
 public:
	tSocket();
	unsigned int get_addr();
	unsigned short int get_port();
	int readed_bytes();
	int open_any(char * host);
	int open_any(unsigned int host);
	int accepting(char * host);
	int open_port(char * host,int port);
	int open_port(unsigned long int host,unsigned short int port);
	int open_port(int *ftp_addr);
	int send_string(char *what,int timeout);
	int rec_string(char * where,int len,int timeout);
	void flush();
	void down();
	~tSocket(); 
};

#define SOCKET_UNKNOWN_HOST -1
#define SOCKET_CANT_ALLOCATE -2
#define SOCKET_CANT_CONNECT -3

#endif
