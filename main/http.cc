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

#include "http.h"
#include "locstr.h"
#include "liststr.h"
#include "var.h"
#include "stdio.h"
#include "base64.h"
#include "ntlocale.h"

tHttpClient::tHttpClient():tClient(){
	user_agent=NULL;
};

tHttpClient::tHttpClient(tCfg *cfg):tClient(cfg){
	user_agent=NULL;
};

void tHttpClient::init(char *host,tWriterLoger *log,int prt,int time_out) {
	tClient::init(host,log,prt,time_out);
	BuffSize=BLOCK_READ;
	buffer=new char[BuffSize];
};

void tHttpClient::set_user_agent(char *agent,char *refer){
	user_agent=agent;
	referer=refer;
};

void tHttpClient::set_offset(fsize_t a) {
	FileLoaded=Offset=a;
};

int tHttpClient::send_request(char *what) {
	DBC_RETVAL_IF_FAIL(what!=NULL,-1);
	LOG->log(LOG_TO_SERVER,what);
	return CtrlSocket->send_string(what,timeout);
};

int tHttpClient::send_request(char *begin, char *center,char *end){
	DBC_RETVAL_IF_FAIL(begin!=NULL,-1);
	char *tmp=sum_strings(begin,center,end,NULL);
	int rvalue=send_request(tmp);
	delete[] tmp;
	return(rvalue);
};

int tHttpClient::read_data(char *where,fsize_t len) {
	DBC_RETVAL_IF_FAIL(where!=NULL,RVALUE_TIMEOUT);
	int all=CtrlSocket->rec_string(where,len,timeout);
	if (socket_err_handler(all)) {
		LOG->log(LOG_ERROR,_("Socket lost!"));
		return RVALUE_TIMEOUT;
	};
	return all;
};

int tHttpClient::read_answer(tStringList *list) {
	DBC_RETVAL_IF_FAIL(list!=NULL,-1);
	list->done();
	int rvalue=0;
	LOG->log(LOG_OK,_("Request was sent, waiting for the answer"));
	if (read_string(CtrlSocket,list,MAX_LEN)!=0){
		Status=STATUS_TIMEOUT;
		return -1;
	};
	tString *last=list->last();
	if (last) {
		LOG->log(LOG_FROM_SERVER,last->body);
		char *str1=new char[strlen(last->body)+1];
		char *str2=new char[strlen(last->body)+1];
		sscanf(last->body,"%s %s",str1,str2);
		if (!equal_first("HTTP",str1)) {
			LOG->log(LOG_WARNING,_("It is not HTTP server!!!"));
/*			delete str1;
			delete str2;
			return -1;*/
		};
		switch (*str2) {
		case '2':{
			LOG->log(LOG_OK,_("All ok, reading file"));
			break;
		};
		case '3':{
			rvalue=1;
			break;
			};
		case '4':{
			Status=STATUS_NOT_FOUND;
			if (begin_string(last->body,"401") ||
			    begin_string(last->body,"403"))
				LOG->log(LOG_WARNING,_("It seems to me that you need a password :)"));
			rvalue = -1;
			break;
		};
		default:{
			Status=STATUS_BAD_ANSWER;
			LOG->log(LOG_ERROR,_("Server return bad answer:(("));
			rvalue = -1;
		};
		};
		list->del(last);
		delete[] str1;
		delete[] str2;
		delete last;
		int num_str=32;
		do{
			num_str-=1;
			if (read_string(CtrlSocket,list,MAX_LEN)) return -1;
			last=list->last();
			LOG->log(LOG_FROM_SERVER,last->body);
			/*limit strings in answer to 32*/
		}while (!empty_string(last->body) && num_str>0);
		return rvalue;
	};
	return -1;
};

void tHttpClient::send_cookies(char *host,char *path){
	char *request_string=LOG->cookie(host,path);
	if (request_string){
		send_request("Cookie: ",request_string,"\r\n");
		delete[] request_string;
	};
};

fsize_t tHttpClient::get_size(char *filename,tStringList *list) {
	DBC_RETVAL_IF_FAIL(filename!=NULL,-1);
	DBC_RETVAL_IF_FAIL(list!=NULL,-1);
	send_request("GET ",filename," HTTP/1.0\r\n");
	char data[MAX_LEN];
	send_request("Accept: */*\r\n");
	if (Offset){
		sprintf(data,"%li",Offset);
		send_request("Range: bytes=",data,"-\r\n");
	};
	if (referer && *referer){
//		referer=sum_strings("http://",hostname,filename,NULL);
		send_request("Referer: ",referer,"\r\n");
	};
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
		delete[] tmp;
		send_request("Authorization: Basic ",pass,"\r\n");
		delete[] pass;
	};
	send_cookies(hostname,filename);
	send_request("\r\n");
	return read_answer(list);
};


int tHttpClient::get_file_from(char *what,unsigned int begin,fsize_t len) {
	DSize=0;
	int complete=1;
	FileLoaded=begin;
	fsize_t llen=len;
	do {
		if ((complete=tClient::read_data())<0) {
			LOG->log(LOG_WARNING,_("Data connection closed."));
			break;
		};
		if (len && FillSize>llen) FillSize=llen;
		FileLoaded+=FillSize;
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
	CtrlSocket->down();
};

void tHttpClient::done() {
	down();
};

tHttpClient::~tHttpClient() {
	down();
};
