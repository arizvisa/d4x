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

#include "http.h"
#include "locstr.h"
#include "liststr.h"
#include "log.h"
#include "var.h"
#include "stdio.h"
#include "base64.h"
#include "ntlocale.h"

tHttpClient::tHttpClient() {
	hostname=userword=username=buffer=NULL;
};

void tHttpClient::init(char *host,tLog *log,int prt,int time_out) {
	tClient::init(host,log,prt,time_out);
	BuffSize=MAX_LEN;
	buffer=new char[BuffSize];
	Auth=0;
};

void tHttpClient::set_offset(int a) {
	FileLoaded=Offset=a;
};

int tHttpClient::send_request(char *what) {
	LOG->add(what,LOG_TO_SERVER);
	return CtrlSocket.send_string(what,timeout);
};

int tHttpClient::read_data(char *where,int len) {
	int all=CtrlSocket.rec_string(where,len,timeout);
	if (socket_err_handler(all)) {
		LOG->add(_("Socket lost!"),LOG_ERROR);
		return -1;
	};
	return all;
};

int tHttpClient::read_answer(tStringList *list) {
	list->done();
	int rvalue=0;
	if (read_string(&CtrlSocket,list,MAX_LEN)!=0) return -1;
	tString *last=list->last();
	if (last) {
		char str1[MAX_LEN],str2[MAX_LEN];
		sscanf(last->body,"%s %s",str1,str2);
		if (!equal_first("HTTP",str1)) {
			LOG->add(_("It is not HTTP server!!!"),LOG_WARNING);
			return -1;
		};
		switch (str2[0]) {
			case '2':
				{
					LOG->add(_("All ok, reading file"),LOG_OK);
					break;
				};
			case '3':
				{
					LOG->add(_("Redirect detected..."),LOG_WARNING);
					rvalue=1;
					break;
				};
			case '4':
				{
					if (equal_first("401",str2)) {
						LOG->add(_("It seems to me that you need a password :)"),LOG_WARNING);
						if (!Auth) {
							Auth=1;
						};
					};
				};
			default:
				{
					Status=STATUS_BAD_ANSWER;
					LOG->add(_("Server return bad answer:(("),LOG_ERROR);
					rvalue = -1;
				};
		};
		while (!empty_string(last->body)) {
			if (read_string(&CtrlSocket,list,MAX_LEN)) return -1;
			LOG->add(last->body,LOG_FROM_SERVER);
			last=list->last();
		};
		return rvalue;
	};
	return -1;
};

int tHttpClient::get_size(char *filename,tStringList *list) {
	char data[MAX_LEN];
	sprintf(data,"GET %s HTTP/1.0\r\n",filename);
	send_request(data);
	sprintf(data,"User-Agent: %s\r\n",VERSION_NAME);
	send_request(data);
	send_request("Accept: */*\r\n");
	sprintf(data,"Range: bytes=%i-\r\n",Offset);
	send_request(data);
	sprintf(data,"Refer: %s\r\n",HOME_PAGE);
	send_request(data);
	sprintf(data,"Host: %s\r\n",hostname);
	send_request(data);
	if (Auth && username && userword) {
		char *tmp=sum_strings(username,":",userword);
		char *pass=string_to_base64(tmp);
		delete tmp;
		sprintf(data,"Authorization: Basic %s\r\n",pass);
		delete pass;
		send_request(data);
	};
	send_request("\r\n");
	return read_answer(list);
};


int tHttpClient::get_file_from(char *what,unsigned int begin,unsigned int len,int fd) {
	DSize=0;
	int complete=1;
	FileLoaded=begin;
	do {
		if ((complete=tClient::read_data())<0) {
			LOG->add(_("Connection closed."),LOG_ERROR);
			break;
		};
		FileLoaded+=complete;
		if (write_buffer(fd)) {
			LOG->add(_("Can't write to file"),LOG_ERROR);
			Status=STATUS_FATAL;
			break;
		};
	} while (complete!=0);
	return DSize;
};

int tHttpClient::registr(char *user,char *password) {
	username=user;
	userword=password;
	return 0;
};

void tHttpClient::set_auth(int what) {
	Auth=what;
};

int tHttpClient::get_auth() {
	return Auth;
};

void tHttpClient::down() {
	CtrlSocket.down();
};

void tHttpClient::done() {
	down();
};

tHttpClient::~tHttpClient() {
	if (buffer) delete buffer;
	down();
};
