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

#include "http.h"
#include "locstr.h"
#include "liststr.h"
#include "var.h"
#include "stdio.h"
#include "base64.h"
#include "ntlocale.h"

tHttpClient::tHttpClient() {
	hostname=userword=username=buffer=NULL;
	user_agent=NULL;
};

void tHttpClient::init(char *host,tWriterLoger *log,int prt,int time_out) {
	tClient::init(host,log,prt,time_out);
	BuffSize=MAX_LEN;
	buffer=new char[BuffSize];
};

void tHttpClient::set_user_agent(char *what){
	user_agent=what;
};

void tHttpClient::set_offset(int a) {
	FileLoaded=Offset=a;
};

int tHttpClient::send_request(char *what) {
	LOG->log(LOG_TO_SERVER,what);
	return CtrlSocket.send_string(what,timeout);
};

int tHttpClient::send_request(char *begin, char *center,char *end){
	char *tmp=sum_strings(begin,center,end,NULL);
	int rvalue=send_request(tmp);
	delete(tmp);
	return(rvalue);
};

int tHttpClient::read_data(char *where,int len) {
	int all=CtrlSocket.rec_string(where,len,timeout);
	if (socket_err_handler(all)) {
		LOG->log(LOG_ERROR,_("Socket lost!"));
		return RVALUE_TIMEOUT;
	};
	return all;
};

int tHttpClient::read_answer(tStringList *list) {
	list->done();
	int rvalue=0;
	LOG->log(LOG_OK,_("Request was sent, waiting for the answer"));
	if (read_string(&CtrlSocket,list,MAX_LEN)!=0) return -1;
	tString *last=list->last();
	if (last) {
		LOG->log(LOG_FROM_SERVER,last->body);
		char *str1=new char[strlen(last->body)+1];
		char *str2=new char[strlen(last->body)+1];
		sscanf(last->body,"%s %s",str1,str2);
		if (!equal_first("HTTP",str1)) {
			LOG->log(LOG_WARNING,_("It is not HTTP server!!!"));
			delete str1;
			delete str2;
			return -1;
		};
		switch (str2[0]) {
			case '2':{
					LOG->log(LOG_OK,_("All ok, reading file"));
					break;
				};
			case '3':{
					rvalue=1;
					break;
				};
			case '4':{
					if (equal_first("401",str2)) {
						LOG->log(LOG_WARNING,_("It seems to me that you need a password :)"));
					};
				};
			default:{
					Status=STATUS_BAD_ANSWER;
					LOG->log(LOG_ERROR,_("Server return bad answer:(("));
					rvalue = -1;
				};
		};
		list->del(last);
		delete str1;
		delete str2;
		delete last;
		do{
			if (read_string(&CtrlSocket,list,MAX_LEN)) return -1;
			last=list->last();
			LOG->log(LOG_FROM_SERVER,last->body);
		}while (!empty_string(last->body));
		return rvalue;
	};
	return -1;
};

void tHttpClient::send_cookies(char *host,char *path){
	char *request_string=LOG->cookie(host,path);
	if (request_string){
		send_request("Cookie: ",request_string,"\r\n");
		delete(request_string);
	};
};

int tHttpClient::get_size(char *filename,tStringList *list) {
	char *real_filename=unparse_percents(filename);
	send_request("GET ",real_filename," HTTP/1.0\r\n");
	delete real_filename;
	char data[MAX_LEN];
	send_request("Accept: */*\r\n");
	if (Offset){
		sprintf(data,"%i",Offset);
		send_request("Range: bytes=",data,"-\r\n");
	};
	send_request("Referer: ",HOME_PAGE,"\r\n");
	if (user_agent && strlen(user_agent)){
		if (equal(user_agent,"%version"))
			send_request("User-Agent: ",VERSION_NAME,"\r\n");
		else
			send_request("User-Agent: ",user_agent,"\r\n");
	};

	send_request("Host: ",hostname,"\r\n");
	if (username && userword) {
		char *tmp=sum_strings(username,":",userword,NULL);
		char *pass=string_to_base64(tmp);
		delete tmp;
		send_request("Authorization: Basic ",pass,"\r\n");
		delete pass;
	};
	send_cookies(hostname,filename);
	send_request("\r\n");
	return read_answer(list);
};


int tHttpClient::get_file_from(char *what,unsigned int begin,int len) {
	DSize=0;
	int complete=1;
	FileLoaded=begin;
	int llen=len;
	do {
		if ((complete=tClient::read_data())<0) {
			LOG->log(LOG_WARNING,_("Data connection closed."));
			break;
		};
		if (len && FillSize>llen) FillSize=llen;
		FileLoaded+=complete;
		if (write_buffer()) {
			Status=STATUS_FATAL;
			break;
		};
		if (len){
			llen -=FillSize;
			if (llen==0){
				LOG->log(LOG_OK,_("Requested size was loaded"));
				break;
			};
		};
	} while (complete!=0);
	return DSize;
};

int tHttpClient::registr(char *user,char *password) {
	username=user;
	userword=password;
	return 0;
};

void tHttpClient::down() {
	CtrlSocket.down();
};

void tHttpClient::done() {
	down();
};

tHttpClient::~tHttpClient() {
	down();
	if (buffer) delete buffer;
};
