/*	WebDownloader for X-Window
 *	Copyright (C) 1999 Koshelev Maxim
 *	This Program is free but not GPL!!! You can't modify it
 *	without agreement with autor. You can't distribute modified
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
tClient::~tClient() {}
;

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
	if (FillSize<0) return -1;
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
	char temp[maxlen];
	char *cur=temp;
	do {
		*cur=0;
		int err=sock->rec_string(cur,1,timeout);
		if (socket_err_handler(err)) return -1;
		if (err==0 && temp==cur) return 1;
	} while(cur-temp<maxlen && *(cur++)!='\n');
	*cur=0;
	list->add(temp);
	return 0;
};

int tClient::socket_err_handler(int err) {
	if (err==STATUS_TIMEOUT) {
		Status=STATUS_TIMEOUT;
		LOG->add(_("Timeout when socket read!"),LOG_ERROR);
		return -1;
	};
	if (err<0) {
		Status=STATUS_TRIVIAL;
		LOG->add(_("Error when reading from socket!"),LOG_ERROR);
		return -1;
	};
	return 0;
};

int tClient::reinit() {
	Status=0;
	int err=-1;
	LOG->add(_("Trying to connect..."),LOG_OK);
	if (hostname && (err=CtrlSocket.open_port(hostname,port))==0) {
		LOG->add(_("Socket was opened!"),LOG_WARNING);
		return 0;
	};
	switch (err) {
		case -1:
			{
				LOG->add(_("Host not found!"),LOG_ERROR);
				Status=STATUS_FATAL;
				break;
			};
		case -2:
			{
				LOG->add(_("Can't allocate socket"),LOG_ERROR);
				Status=STATUS_FATAL;
				break;
			};
		case -3:
			{
				LOG->add(_("Can't connect"),LOG_ERROR);
				Status=STATUS_TRIVIAL;
				break;
			};
	};
	return -1;
};

int tClient::write_buffer(int fd) {
	return (FillSize-write(fd,buffer,FillSize));
};


int tClient::get_status() {
	return Status;
};
//**************************************************/
