/*	WebDownloader for X-Window
 *	Copyright (C) 1999 Koshelev Maxim
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

tClient::tClient() {
	hostname=username=userword=buffer=NULL;
	FileLoaded=0;
};

tClient::~tClient() {
};

void tClient::init(char *host,tLog *log,int prt,int time_out) {
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
	int rvalue;
	char temp[maxlen];
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
		LOG->add(_("Timeout when socket read!"),LOG_ERROR);
		return RVALUE_TIMEOUT;
	};
	if (err<0) {
		Status=STATUS_TRIVIAL;
		LOG->add(_("Error when reading from socket!"),LOG_ERROR);
		return RVALUE_TIMEOUT;
	};
	return RVALUE_OK;
};

int tClient::reinit() {
	Status=0;
	int err=-1;
	LOG->add(_("Trying to connect..."),LOG_OK);
	if (hostname && (err=CtrlSocket.open_port(hostname,port))==0) {
		LOG->add(_("Socket was opened!"),LOG_WARNING);
		return RVALUE_OK;
	};
	switch (err) {
	case SOCKET_UNKNOWN_HOST:{
		LOG->add(_("Host not found!"),LOG_ERROR);
		Status=STATUS_FATAL;
		break;
	};
	case SOCKET_CANT_ALLOCATE:{
		LOG->add(_("Can't allocate socket"),LOG_ERROR);
		Status=STATUS_FATAL;
		break;
	};
	case SOCKET_CANT_CONNECT:{
		LOG->add(_("Can't connect"),LOG_ERROR);
		Status=STATUS_TRIVIAL;
		break;
	};
	};
	return RVALUE_TIMEOUT;
};

int tClient::write_buffer(int fd) {
	return (FillSize-write(fd,buffer,FillSize));
};


int tClient::get_status() {
	return Status;
};
//**************************************************/
