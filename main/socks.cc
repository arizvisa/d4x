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
#include "socks.h"
#include "dbc.h"
#include <sys/types.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
enum D4X_SOCKS_AUTH{
	SOCKS_AUTH_NONE=0,
	SOCKS_AUTH_GSSAPI,
	SOCKS_AUTH_USER
};

#define SOCKS_AUTH_FAILED 0xFF

enum D4X_SOCKS_CMDS{
	SOCKS_CMD_UNKNOWN=0,
	SOCKS_CMD_CONNECT,
	SOCKS_CMD_BIND,
	SOCKS_CMD_UDP
};

enum D4X_SOCKS_ATYPES{
	SOCKS_ATYPE_UNKNOWN=0,
	SOCKS_ATYPE_IPV4,
	SOCKS_ATYPE_RESERVED,
	SOCKS_ATYPE_DOMAIN,
	SOCKS_ATYPE_IPV6
};

enum D4X_SOCKS_REPLIES{
	SOCKS_REP_SUCCESS,
	SOCKS_REP_FAILURE,
	SOCKS_REP_FORBIDEN,
	SOCKS_REP_NETFAILED,
	SOCKS_REP_HOSTFAILED,
	SOCKS_REP_REFUSED,
	SOCKS_REP_EXPIRED,
	SOCKS_REP_NOTSUPPORTED,
	SOCKS_REP_WRONGTYPE
};

#define SOCKS_VERSION 5

/****************************************************************/

tSocksSocket::tSocksSocket():tSocket(){
	socks_port=0;
};

tSocksSocket::tSocksSocket(char *host,unsigned short int port,
			   char *use=NULL,
			   char *pas=NULL):tSocket(){
	user.set(use);
	pass.set(pas);
	socks_host.set(host);
	socks_port=port;
};

tSocksSocket::~tSocksSocket(){
};

void tSocksSocket::socks_init(){
	DBC_RETURN_IF_FAIL(socks_host.get()!=NULL);
	if (tSocket::open_port(socks_host.get(),socks_port)){
		down();
		return;
	};
		
	if (user.get()==NULL || pass.get()==NULL){
		unsigned char request[]={SOCKS_VERSION,1,SOCKS_AUTH_NONE};
		write(fd,request,sizeof(request));
	}else{
		unsigned char request[]={SOCKS_VERSION,2,
				       SOCKS_AUTH_NONE,
				       SOCKS_AUTH_USER};
		write(fd,request,sizeof(request));
	};
	if (read(fd,socks_buf,2)<2){
		down();
		return;
	};
	if (socks_buf[1]==SOCKS_AUTH_NONE)
		return; //successfull initiated
	if (socks_buf[1]==SOCKS_AUTH_USER && user.get() && pass.get()){
		unsigned char tmp=1;
		write(fd,&tmp,sizeof(tmp));
		tmp=strlen(user.get());
		write(fd,&tmp,sizeof(tmp));
		write(fd,user.get(),tmp);
		tmp=strlen(pass.get());
		write(fd,&tmp,sizeof(tmp));
		write(fd,pass.get(),tmp);
		if (read(fd,&socks_buf,2)<2){
			down();
			return;
		};
		if (socks_buf[1]==0)
			return;
	};
	down(); //failed to connect
};

int tSocksSocket::socks_connect_reply(){
	int repsize=read(fd,socks_buf,10);
	if (repsize<10){
		down();
		return(SOCKET_CANT_CONNECT);
	};
	switch(socks_buf[1]){
	case SOCKS_REP_SUCCESS:{
		break;
	};
	case SOCKS_REP_FAILURE:
	case SOCKS_REP_NETFAILED:
	case SOCKS_REP_HOSTFAILED:
		return(SOCKET_UNKNOWN_HOST);
	default:
		return(SOCKET_CANT_CONNECT);
	};		
	return 0;
};

int tSocksSocket::open_port(char *host, u_int16_t port) {
	DBC_RETVAL_IF_FAIL(host!=NULL,SOCKET_CANT_CONNECT);
	socks_init();
	if (fd<0) return(SOCKET_CANT_CONNECT);
	socks_buf[0]=SOCKS_VERSION;
	socks_buf[1]=SOCKS_CMD_CONNECT;
	socks_buf[2]=0;
	socks_buf[3]=SOCKS_ATYPE_DOMAIN;
	write(fd,socks_buf,4);
	unsigned char tmp=strlen(host);
	write(fd,&tmp,1);
	write(fd,host,tmp);
	port=htons(port);
	write(fd,&port,2);
	return(socks_connect_reply());
};

int tSocksSocket::open_port(u_int32_t host,u_int16_t port) {
//	port=htons(port);
//	host=htonl(host);
	socks_init();
	if (fd<0) return(SOCKET_CANT_CONNECT);
	socks_buf[0]=SOCKS_VERSION;
	socks_buf[1]=SOCKS_CMD_CONNECT;
	socks_buf[2]=0;
	socks_buf[3]=SOCKS_ATYPE_IPV4;
	write(fd,socks_buf,4);
	write(fd,&host,4);
	write(fd,&port,2);
	return(socks_connect_reply());
};

unsigned int tSocksSocket::get_addr(){
	return(bnd_host);
};

unsigned short int tSocksSocket::get_port(){
	return(bnd_port);
};

int tSocksSocket::socks_bind_reply(){
	int repsize=read(fd,socks_buf,10);
	if (repsize<10){
		down();
		return(SOCKET_CANT_CONNECT);
	};
	switch(socks_buf[1]){
	case SOCKS_REP_SUCCESS:{
		break;
	};
	case SOCKS_REP_FAILURE:
	case SOCKS_REP_NETFAILED:
	case SOCKS_REP_HOSTFAILED:
		return(SOCKET_UNKNOWN_HOST);
	default:
		return(SOCKET_CANT_CONNECT);
	};		
	memcpy(&bnd_host,socks_buf+4,4);
	memcpy(&bnd_port,socks_buf+8,2);
	bnd_host=ntohl(bnd_host);
	bnd_port=ntohs(bnd_port);
	return 0;
};

int tSocksSocket::accepting(char *host){
	DBC_RETVAL_IF_FAIL(host!=NULL,-1);
	int repsize=read(fd,socks_buf,10);	
	if (repsize<10){
		down();
		return(-1);
	};
	if (socks_buf[1]!=SOCKS_REP_SUCCESS){
		down();
		return(-1);
	};
	return(0);
};

int tSocksSocket::open_any(char * host){
	DBC_RETVAL_IF_FAIL(host!=NULL,SOCKET_CANT_CONNECT);
	socks_init();
	if (fd<0) return(SOCKET_CANT_CONNECT);
	socks_buf[0]=SOCKS_VERSION;
	socks_buf[1]=SOCKS_CMD_BIND;
	socks_buf[2]=0;
	socks_buf[3]=SOCKS_ATYPE_DOMAIN;
	write(fd,socks_buf,4);
	unsigned char tmp=strlen(host);
	write(fd,&tmp,1);
	write(fd,host,tmp);
	u_int16_t port=0;
	write(fd,&port,2);
	return(socks_bind_reply());
};

int tSocksSocket::open_any(u_int32_t host) {
	host=htonl(host);
	socks_init();
	if (fd<0) return(SOCKET_CANT_CONNECT);
	socks_buf[0]=SOCKS_VERSION;
	socks_buf[1]=SOCKS_CMD_BIND;
	socks_buf[2]=0;
	socks_buf[3]=SOCKS_ATYPE_IPV4;
	write(fd,socks_buf,4);
	write(fd,&host,4);
	u_int16_t port=0;
	write(fd,&port,2);
	return(socks_bind_reply());
};
