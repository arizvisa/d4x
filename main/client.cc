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

#include "socket.h"
#include "liststr.h"
#include "client.h"
#include "var.h"
#include "ntlocale.h"
#include <unistd.h>
#include <stdarg.h>
#include <sys/types.h>

tWriterLoger::tWriterLoger(){};

fsize_t tWriterLoger::shift(fsize_t len){
	return(shift(len,SEEK_SET));
};

tWriterLoger::~tWriterLoger(){};

void tWriterLoger::log_printf(int type,const char *fmt,...){
	DBC_RETURN_IF_FAIL(fmt!=NULL);
	char str[MAX_LEN+1];
	char *cur=str;
	va_list ap;
	va_start(ap,fmt);
	*cur=0;
	while (*fmt && cur-str<MAX_LEN){
		if (*fmt=='%'){
			fmt+=1;
			switch(*fmt){
			case 's':{
				char *s=va_arg(ap,char *);
				if (s)
					g_snprintf(cur,MAX_LEN-(cur-str),"%s",s);
				else
					g_snprintf(cur,MAX_LEN-(cur-str),"%s","NULL");
				break;
			};
			case 'i':{
				g_snprintf(cur,MAX_LEN-(cur-str),"%i",va_arg(ap,int));
				break;
			};
			default:{
				*cur=*fmt;
				cur+=1;
				*cur=0;			       
			};
			};
			if (*fmt==0) break;
			while(*cur) cur+=1;
		}else{
			*cur=*fmt;
			cur+=1;
			*cur=0;
		};
		fmt+=1;
	};
	va_end(ap);
	log(type,str);
};

void tWriterLoger::truncate(){
};

char * tWriterLoger::cookie(const char *host, const char *path){
	return NULL;
};

/* tClient 
 */

tClient::tClient(){
	hostname=username=userword=buffer=NULL;
	FileLoaded=0;
};

tClient::~tClient(){
	if (buffer) delete buffer;
};

void tClient::init(char *host,tWriterLoger *log,int prt,int time_out) {
	DBC_RETURN_IF_FAIL(host!=NULL);
	DBC_RETURN_IF_FAIL(log!=NULL);

	Status=0;
	LOG=log;
	port=prt;
	hostname=host;
	FileLoaded=0;
	timeout=time_out;
};

int tClient::read_data() {
	FillSize=read_data(buffer,BLOCK_READ);
	if (FillSize<0) return RVALUE_TIMEOUT;
	DSize+=FillSize;
	return FillSize;
};

int tClient::test_reget() {
	return ReGet;
};


int tClient::get_readed() {
	return FileLoaded;
};

int tClient::read_string(tSocket *sock,tStringList *list,int maxlen) {
	DBC_RETVAL_IF_FAIL(sock!=NULL,0);
	DBC_RETVAL_IF_FAIL(list!=NULL,0);

	int rvalue;
	char temp[maxlen+1];
	char *cur=temp;
	do {
		*cur=0;
		int err=sock->rec_string(cur,1,timeout);
		if ((rvalue=socket_err_handler(err))) return(rvalue);
		if (err==0 && temp==cur) return RVALUE_COMPLETED;
	} while(cur-temp<maxlen && *(cur++)!='\n');
	*cur=0;
	list->add(temp);
	return RVALUE_OK;
};

int tClient::socket_err_handler(int err) {
	if (err==STATUS_TIMEOUT) {
		Status=STATUS_TIMEOUT;
		LOG->log(LOG_ERROR,_("Timeout while reading from socket!"));
		return RVALUE_TIMEOUT;
	};
	if (err<0) {
		Status=STATUS_TRIVIAL;
		LOG->log(LOG_ERROR,_("Error while reading from socket!"));
		return RVALUE_TIMEOUT;
	};
	return RVALUE_OK;
};

int tClient::reinit() {
	Status=0;
	int err=-1;
	LOG->log(LOG_OK,_("Trying to connect..."));
	if (hostname && (err=CtrlSocket.open_port(hostname,port))==0) {
		LOG->log(LOG_WARNING,_("Socket was opened!"));
		return RVALUE_OK;
	};
	switch (err) {
	case SOCKET_UNKNOWN_HOST:{
		LOG->log(LOG_ERROR,_("Host not found!"));
		Status=STATUS_FATAL;
		break;
	};
	case SOCKET_CANT_ALLOCATE:{
		LOG->log(LOG_ERROR,_("Can't allocate socket"));
		Status=STATUS_FATAL;
		break;
	};
	case SOCKET_CANT_CONNECT:{
		LOG->log(LOG_ERROR,_("Can't connect"));
		Status=STATUS_TRIVIAL;
		break;
	};
	};
	return RVALUE_TIMEOUT;
};

int tClient::write_buffer() {
	return (FillSize-LOG->write(buffer,FillSize));
};


int tClient::get_status() {
	return Status;
};
//**************************************************/
