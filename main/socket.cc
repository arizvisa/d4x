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

tSocket::tSocket() {
	fd=0;
	RBytes=0;
	SBytes=0;
	buffer=NULL;
};

int tSocket::constr_name(char *host,int port) {
	info.sin_family=AF_INET;
	if (host) {
		info.sin_addr.s_addr = inet_addr(host);
		if (info.sin_addr.s_addr == INADDR_NONE){
			if (!buffer) buffer=new char[MAX_LEN];
			hostent *hpr=&hp;
			temp_variable=0;
#if !(defined(BSD) && (BSD >= 199306))
#if defined(__sparc__) && !(defined(__linux__))
			gethostbyname_r(host,&hp,buffer,MAX_LEN,&temp_variable);
#else
			gethostbyname_r(host,&hp,buffer,MAX_LEN,&hpr,&temp_variable);
#endif
			if (temp_variable) return -1;
			memcpy((char *)&info.sin_addr,hpr->h_addr_list[0],(size_t) hpr->h_length);
#else /* !(defined(BSD) && (BSD >= 199306)) */
/* It seems that reentrant variant
   of gethostbyname is not available under BSD
*/
			hostent *hpa=gethostbyname(host);
			if (!hpa) return -1;
			memcpy((char *)&info.sin_addr,hpa->h_addr_list[0],(size_t) hpa->h_length);
#endif /* !(defined(BSD) && (BSD >= 199306)) */
		};
	} else info.sin_addr.s_addr=INADDR_ANY;
	info.sin_port=htons(port);
	return sizeof(info);
};
//***************************************************/

unsigned int tSocket::get_addr() {
	unsigned int my_addr=0;
	unsigned int len;
	len = sizeof(info);
	if (getsockname(fd, (struct sockaddr* )&info, &len) == -1)
		return 0;
	memcpy(&my_addr, (char *)&info.sin_addr.s_addr,sizeof(my_addr));
	my_addr=htonl(my_addr);
	return my_addr;
}

unsigned short int tSocket::get_port() {
	return htons(info.sin_port);
};

int tSocket::open_port(char *host, int port) {
	int len=constr_name(host,port);
	if (len<0) return SOCKET_UNKNOWN_HOST;
	if ((fd = socket(info.sin_family,SOCK_STREAM, 0)) < 0)
		return(SOCKET_CANT_ALLOCATE);
	int a=1;
	setsockopt(fd,SOL_SOCKET,SO_KEEPALIVE,(char *)&a,sizeof(a));
	if (connect(fd, (struct sockaddr *)&info, len) < 0)
		return(SOCKET_CANT_CONNECT);
	return 0;
}
int tSocket::open_port(int *ftp_addr) {
	unsigned char addr[6];
	for (int i=0;i<6;i++) addr[i]=(unsigned char)(ftp_addr[i]&0xff);
	unsigned long int host=0;
	unsigned short int port=0;
	memcpy((char *)&host,addr,4);
	memcpy((char *)&port,addr+4,2);	      
	return(open_port(host,port));
}

int tSocket::open_port(unsigned long int host,unsigned short int port) {
	port=htons(port);
//	host=htonl(host);
	int len=constr_name(NULL,port);
	if (len<0) return SOCKET_UNKNOWN_HOST;
	memcpy((char *)&info.sin_addr.s_addr,&host, sizeof(host));
	if ((fd = socket(info.sin_family,SOCK_STREAM, 0)) < 0)
		return(SOCKET_CANT_ALLOCATE);
	int a=1;
	setsockopt(fd,SOL_SOCKET,SO_KEEPALIVE,(char *)&a,sizeof(a));
	if (connect(fd, (struct sockaddr *)&info, len) < 0)
		return(SOCKET_CANT_CONNECT);
	return 0;
}

int tSocket::open_any(char *host) {
	constr_name(NULL,0);
	if ((fd = socket(info.sin_family,SOCK_STREAM, 0)) < 0)
		return(-1);
	int a=1;
	setsockopt(fd,SOL_SOCKET,SO_KEEPALIVE,(char *)&a,sizeof(a));
	int len=constr_name(host,0);
	if (len<0) return -2;
	if (bind(fd, (struct sockaddr *)&info, len) < 0)
		return(-3);
	if (listen(fd,1)) return(-4);
	return 0;
};

int tSocket::open_any(unsigned int host) {
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
	return -1;
};

int tSocket::wait_for_write(int len) {
	FD_ZERO(&set);
	FD_SET(fd,&set);
	timeval tv;
	tv.tv_sec=len;
	tv.tv_usec=0;
	if (select(fd+1,NULL,&set,NULL,&tv)>0) return 0;
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

int tSocket::accepting(char * host) {
	sockaddr_in addr;
	unsigned int len=sizeof(addr);
	int oldfd=fd;
	if ((fd=accept(fd,(sockaddr *)&addr,&len))<0) {
		return -1;
	}
	if (oldfd>0) {
		close(oldfd);
	};
	return 0;
};

int tSocket::send_string(char * what,int timeout) {
	int a=strlen(what);
	int b=send(fd,what,a,0);
	if (b<0) return -1;
	if (wait_for_write(timeout)) {
		return STATUS_TIMEOUT;
	};
	SBytes+=a-b;
	return a-b;
};

int tSocket::rec_string(char * where,int len,int timeout) {
	if (wait_for_read(timeout))
		return STATUS_TIMEOUT;
	tDownload **download=my_pthread_key_get();
	int speed_limit=CFG.SPEED_LIMIT;
	int bytes;
	if (download!=NULL && *download!=NULL && (speed_limit!=3 || (*download)->SpeedLimit->base>0))
		bytes=(*download)->SpeedLimit->bytes >= len ? len: (*download)->SpeedLimit->bytes+1;
	else
		bytes=len;
	int temp=recv(fd,where,bytes,0);
	if (temp>0) {
		RBytes+=temp;
		pthread_mutex_lock(&GVARS.READED_BYTES_MUTEX);
		GVARS.READED_BYTES+=temp;
		pthread_mutex_unlock(&GVARS.READED_BYTES_MUTEX);
		if (download!=NULL && *download!=NULL && (speed_limit!=3 || (*download)->SpeedLimit->base>0))
			(*download)->SpeedLimit->decrement(temp);
	};
	return temp;
};

void tSocket::down() {
	if (fd>0) {
		shutdown(fd,2);
		close(fd);
	};
	fd=0;
	RBytes=SBytes=0;
};

int tSocket::readed_bytes() {
	return RBytes;
};

tSocket::~tSocket() {
	if (buffer) delete(buffer);
	down();
};

