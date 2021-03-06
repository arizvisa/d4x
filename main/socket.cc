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

#if (defined(__unix__) || defined(unix)) && !defined(USG)
#include <sys/param.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/time.h>
#include <fcntl.h>
#include "socket.h"
#include "var.h"
#include "signal.h"

#ifndef INADDR_NONE
#define INADDR_NONE ((unsigned long) -1)
#endif

int my_get_host_by_name(const char *host,int port,
			sockaddr_in *info,
			hostent *hp,
			char *buf,
			int bufsize,
			int *rvalbuf){
	info->sin_family=AF_INET;
	if (host) {
		info->sin_addr.s_addr = inet_addr(host);
		if (info->sin_addr.s_addr == INADDR_NONE){
			hostent *hpr=hp;
			*rvalbuf=0;
#if !(defined(BSD) && (BSD >= 199306))
#if (defined(__sparc__) || defined(__mips__)) && !(defined(__linux__))
			int tmpr=gethostbyname_r(host,hp,buf,bufsize,rvalbuf);
#else /* (defined(__sparc__) || defined(__mips__)) && !(defined(__linux__)) */
			int tmpr=gethostbyname_r(host,hp,buf,bufsize,&hpr,rvalbuf);
#endif
			if (tmpr!=0 && *rvalbuf) return -1;
			memcpy((char *)&(info->sin_addr),hpr->h_addr_list[0],
			       (size_t) hpr->h_length);
#else /* !(defined(BSD) && (BSD >= 199306)) */
			hostent *hpa=gethostbyname(host);
			if (!hpa) return -1;
			memcpy((char *)&(info->sin_addr),hpa->h_addr_list[0],
			       (size_t) hpa->h_length);
#endif /* !(defined(BSD) && (BSD >= 199306)) */
		};
	} else info->sin_addr.s_addr=INADDR_ANY;
	info->sin_port=htons(port);
	return sizeof(sockaddr_in);
};

/******************************************************************/
tSocket::tSocket() {
	fd=-1;
	RBytes=0;
	SBytes=0;
	buffer=NULL;
	con_flag=0;
};

bool tSocket::constr_name(const char *host,guint16 port) {
	info.sin_family=AF_INET;
	if (host) {
		d4x::USR1On2Off sig_locker_;
		info.sin_addr.s_addr = inet_addr(host);
		if (info.sin_addr.s_addr == INADDR_NONE){
			if (!buffer) buffer=new char[MAX_LEN];
			hostent *hpr=&hp;
			temp_variable=0;
#if !(defined(BSD) && (BSD >= 199306))
#if (defined(__sparc__) || defined(__mips__)) && !(defined(__linux__))
			gethostbyname_r(host,&hp,buffer,MAX_LEN,&temp_variable);
#else
			gethostbyname_r(host,&hp,buffer,MAX_LEN,&hpr,&temp_variable);
#endif
			if (temp_variable){
				return false;
			};
			memcpy((char *)&info.sin_addr,hpr->h_addr_list[0],(size_t) hpr->h_length);
#else /* !(defined(BSD) && (BSD >= 199306)) */
/* It seems that reentrant variant
   of gethostbyname is not available under BSD
*/
			hostent *hpa=gethostbyname(host);
			if (!hpa){
				return false;
			};
			memcpy((char *)&info.sin_addr,hpa->h_addr_list[0],(size_t) hpa->h_length);
#endif /* !(defined(BSD) && (BSD >= 199306)) */
		};
	} else info.sin_addr.s_addr=INADDR_ANY;
	info.sin_port=htons(port);
	return true;
};
//***************************************************/

unsigned int tSocket::get_addr() {
	unsigned int my_addr=0;
#if defined(__sparc__) && !(defined(__linux__))
	int len;
#else
	socklen_t len;
#endif
	len = sizeof(info);
	if (getsockname(fd, (struct sockaddr* )&info,&len) == -1)
		return 0;
	memcpy(&my_addr, (char *)&info.sin_addr.s_addr,sizeof(my_addr));
	my_addr=htonl(my_addr);
	return my_addr;
}

unsigned short int tSocket::get_port() {
	return htons(info.sin_port);
};

void set_nonblock_flag (int desc, int value){
	int oldflags = fcntl (desc, F_GETFL, 0);
	/* If reading the flags failed, return error indication now. */
	if (oldflags == -1)
		return;
	/* Set just the flag we want to set. */
	if (value != 0)
		oldflags |= O_NONBLOCK;
	else
		oldflags &= ~O_NONBLOCK;
	/* Store modified flag word in the descriptor. */
	fcntl (desc, F_SETFL, oldflags);
};


bool tSocket::connect_impl(){
	set_nonblock_flag(fd,1);
	int len=sizeof(info);
	connect(fd, (struct sockaddr *)&info, len);
	if (wait_for_write(50))
		return false;
	int a;
	len=sizeof(a);
	if (getsockopt(fd,SOL_SOCKET,SO_ERROR,&a,(socklen_t*)&len) || a)
		return false;
	return true;
};

int tSocket::open_port(const char *host, guint16 port) {
	DBC_RETVAL_IF_FAIL(host!=NULL,SOCKET_CANT_CONNECT);
	d4x::USR1Off2On sig_unlocker_;
	if (!constr_name(host,port)) return SOCKET_UNKNOWN_HOST;
	if ((fd = socket(info.sin_family,SOCK_STREAM, 0)) < 0)
		return(SOCKET_CANT_ALLOCATE);
	int a=1;
	setsockopt(fd,SOL_SOCKET,SO_KEEPALIVE,(char *)&a,sizeof(a));
	setsockopt(fd,SOL_SOCKET,SO_KEEPALIVE,(char *)&a,sizeof(a));
	
	size_t sl=2000; //set receive buffer to default+30% MTU size
	setsockopt(fd,SOL_SOCKET,SO_RCVBUF,(char *)&sl,sizeof(sl));

	if (!connect_impl())
		return(SOCKET_CANT_CONNECT);
	con_flag=1;
	return 0;
}

int tSocket::open_port(int *ftp_addr) {
	DBC_RETVAL_IF_FAIL(ftp_addr!=NULL,SOCKET_CANT_CONNECT);
	unsigned char addr[6];
	for (int i=0;i<6;i++) addr[i]=(unsigned char)(ftp_addr[i]&0xff);
	unsigned long int host=0;
	unsigned short int port=0;
	memcpy((char *)&host,addr,4);
	memcpy((char *)&port,addr+4,2);	      
	return(open_port(host,port));
}

int tSocket::open_port(guint32 host,guint16 port) {
	d4x::USR1Off2On sig_unlocker_;
	port=htons(port);
//	host=htonl(host);
	if (!constr_name(NULL,port)) return SOCKET_UNKNOWN_HOST;
	memcpy((char *)&info.sin_addr.s_addr,&host, sizeof(host));
	if ((fd = socket(info.sin_family,SOCK_STREAM, 0)) < 0)
		return(SOCKET_CANT_ALLOCATE);
	int a=1;
	setsockopt(fd,SOL_SOCKET,SO_KEEPALIVE,(char *)&a,sizeof(a));
	
	if (!connect_impl())
		return(SOCKET_CANT_CONNECT);
	con_flag=1;
	return 0;
}

int tSocket::open_any(const char *host) {
	d4x::USR1Off2On sig_unlocker_;
	DBC_RETVAL_IF_FAIL(host!=NULL,SOCKET_CANT_CONNECT);
	constr_name(NULL,0);
	if ((fd = socket(info.sin_family,SOCK_STREAM, 0)) < 0)
		return(-1);
	int a=1;
	setsockopt(fd,SOL_SOCKET,SO_KEEPALIVE,(char *)&a,sizeof(a));
	if (!constr_name(host,0)) return -2;
	if (bind(fd, (struct sockaddr *)&info, (socklen_t)sizeof(info)) < 0)
		return(-3);
	if (listen(fd,1)) return(-4);
	return 0;
};

int tSocket::open_any(guint32 host) {
	d4x::USR1Off2On sig_unlocker_;
	constr_name(NULL,0);
	host=htonl(host);
	memcpy((char *)&info.sin_addr.s_addr,&host, sizeof(host));
	if ((fd = socket(info.sin_family,SOCK_STREAM, 0)) < 0)
		return(-1);
	int a=1;
	setsockopt(fd,SOL_SOCKET,SO_KEEPALIVE,(char *)&a,sizeof(a));
	if (bind(fd, (struct sockaddr *)&info, sizeof(info)) < 0)
		return(-2);
	if (listen(fd,1))
		return(-3);
	return 0;
};

int tSocket::wait_for_read(int len) {
	FD_ZERO(&set);
	FD_SET(fd,&set);
	timeval tv;
	tv.tv_sec=len;
	tv.tv_usec=0;
	if (select(fd+1,&set,NULL,NULL,&tv)>0) return 0;
	con_flag=0;
	return -1;
};

int tSocket::wait_for_write(int len) {
	FD_ZERO(&set);
	FD_SET(fd,&set);
	timeval tv;
	tv.tv_sec=len;
	tv.tv_usec=0;
	if (select(fd+1,NULL,&set,NULL,&tv)>0) return 0;
	con_flag=0;
	return -1;
};

void tSocket::flush(){
	char *a;
	FD_ZERO(&set);
	FD_SET(fd,&set);
	timeval tv;
	tv.tv_sec=0;
	tv.tv_usec=0;
	while (select(1,&set,NULL,NULL,&tv)>0)
		read(fd,&a,1);
};

int tSocket::accepting(const char * host) {
	DBC_RETVAL_IF_FAIL(host!=NULL,-1);
	sockaddr_in addr;
#if defined(__sparc__) && !(defined(__linux__))
	int len=sizeof(addr);
#else
	socklen_t len=sizeof(addr);
#endif
	int oldfd=fd;
	if ((fd=accept(fd,(sockaddr *)&addr,&len))<0) {
		return -1;
	}
	if (oldfd>0) {
		close(oldfd);
	};
	con_flag=1;
	return 0;
};

int tSocket::direct_send(char *what){
	return(send(fd,what,strlen(what),0));
};

int tSocket::send_string(const char * what,int timeout) {
	DBC_RETVAL_IF_FAIL(what!=NULL,-1);
	int a=strlen(what);
	int b=send(fd,what,a,0);
	if (b<0) return -1;
	if (wait_for_write(timeout)) {
		return STATUS_TIMEOUT;
	};
	SBytes+=a-b;
	return a-b;
};

fsize_t tSocket::lowlevel_read(char *where,fsize_t bytes){
	return recv(fd,where,bytes,0);
};

fsize_t tSocket::rec_string(char * where,fsize_t len,int timeout) {
	DBC_RETVAL_IF_FAIL(where!=NULL,-1);
	d4x::USR1Off2On sig_unlocker_;
	if (wait_for_read(timeout))
		return STATUS_TIMEOUT;
	tDownload **download=my_pthread_key_get();
	fsize_t bytes;
	if (download!=NULL && *download!=NULL && (CFG.SPEED_LIMIT!=3 || (*download)->SpeedLimit->base>0 || (*download)->SpeedLimit->base2>0))
		bytes=(*download)->SpeedLimit->bytes >= len ? len: (*download)->SpeedLimit->bytes+1;
	else
		bytes=len;
	fsize_t temp=lowlevel_read(where,bytes);
	if (temp>0) {
		d4x::USR1On2Off sig_locker_();
		RBytes+=temp;
		GVARS.MUTEX.lock();
		GVARS.READED_BYTES+=temp;
		GVARS.MUTEX.unlock();
		if (download!=NULL && *download!=NULL){
			tDownload *dwn=*download;
			if (dwn->split && dwn->split->grandparent){
				dwn->split->grandparent->SpeedCalc.inc(temp);
				tDownload *par=dwn->split->grandparent;
				if (par->myowner && par->myowner->PAPA)
					par->myowner->PAPA->speed.inc(temp);
			}else{
				dwn->SpeedCalc.inc(temp);
				if (dwn->myowner && dwn->myowner->PAPA)
					dwn->myowner->PAPA->speed.inc(temp);
			};
			if (CFG.SPEED_LIMIT!=3 || dwn->SpeedLimit->base>0 || dwn->SpeedLimit->base2>0)
				dwn->SpeedLimit->decrement(temp);
			D4X_UPDATE.add(dwn);
		};
	};
	return temp;
};

void tSocket::down() {
	if (fd>=0) {
		shutdown(fd,2);
		close(fd);
	};
	fd=-1;
	RBytes=SBytes=0;
};

int tSocket::connected(){
	return(con_flag);
};

int tSocket::readed_bytes() {
	return RBytes;
};

tSocket::~tSocket() {
	if (buffer) delete[] buffer;
	down();
};

