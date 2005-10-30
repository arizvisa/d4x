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

#include "http.h"
#include "locstr.h"
#include "liststr.h"
#include "var.h"
#include "stdio.h"
#include "base64.h"
#include "ntlocale.h"
#include "signal.h"

tHttpClient::tHttpClient():tClient(){
	user_agent=NULL;
	pass_first=0;
};

tHttpClient::tHttpClient(tCfg *cfg,tSocket *ctrl):tClient(cfg,ctrl){
	user_agent=NULL;
	pass_first=0;
};

void tHttpClient::pass_first_segment(){
	pass_first=1;
};

void tHttpClient::init(const std::string &host,tWriterLoger *log,int prt,int time_out) {
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

int tHttpClient::send_request(const char *what) {
	DBC_RETVAL_IF_FAIL(what!=NULL,-1);
	LOG->log(LOG_TO_SERVER,what);
	return CtrlSocket->send_string(what,timeout);
};

int tHttpClient::send_request(const char *begin, const char *center,const char *end){
	DBC_RETVAL_IF_FAIL(begin!=NULL,-1);
	char *tmp=sum_strings(begin,center,end,NULL);
	int rvalue=send_request(tmp);
	delete[] tmp;
	return(rvalue);
};

fsize_t tHttpClient::read_data(char *where,fsize_t len) {
	DBC_RETVAL_IF_FAIL(where!=NULL,RVALUE_TIMEOUT);
	fsize_t all=CtrlSocket->rec_string(where,len,timeout);
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
	HTTP_VER=1;
	HTTP_SUBVER=0;
	if (last) {
		LOG->log(LOG_FROM_SERVER,last->body);
		char *str1=new char[strlen(last->body)+1];
		char *str2=new char[strlen(last->body)+1];
		sscanf(last->body,"%s %s",str1,str2);
		sscanf(str2,"%i",&ERROR_CODE);
		if (!equal_first("HTTP",str1)) {
			LOG->log(LOG_WARNING,_("It is not HTTP server!!!"));
		}else{
			char *tmp=index(str1,'/');
			if (tmp)
				sscanf(tmp+1,"%i.%i",&HTTP_VER,&HTTP_SUBVER);
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
			rvalue = -2;
			break;
		};
		default:{
			Status=STATUS_BAD_ANSWER;
			LOG->log(LOG_ERROR,_("Server return bad answer:(("));
			rvalue = -2;
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

void tHttpClient::send_cookies(const char *host,const char *path){
	std::string request_string=LOG->cookie(host,path);
	if (!request_string.empty()){
		send_request("Cookie: ",request_string.c_str(),"\r\n");
	};
};

fsize_t tHttpClient::get_size_sub(tStringList *list){
	char data[MAX_LEN];
	send_request("Accept: */*\r\n");
	if (Offset){
		sprintf(data,"%Li",Offset);
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

	send_request("Host: ",hostname.c_str(),"\r\n");
	if (!username.empty() && !userword.empty()) {
		char *pass=string_to_base64((username+":"+userword).c_str());
		send_request("Authorization: Basic ",pass,"\r\n");
		delete[] pass;
	};
	send_request("\r\n");
	return read_answer(list);
};

fsize_t tHttpClient::get_size_only(const char *filename,tStringList *list) {
	DBC_RETVAL_IF_FAIL(filename!=NULL,-1);
	DBC_RETVAL_IF_FAIL(list!=NULL,-1);
	send_request("HEAD ",filename," HTTP/1.1\r\n");
	send_cookies(hostname.c_str(),filename);
	return(get_size_sub(list));
};


fsize_t tHttpClient::get_size(const char *filename,tStringList *list) {
	DBC_RETVAL_IF_FAIL(filename!=NULL,-1);
	DBC_RETVAL_IF_FAIL(list!=NULL,-1);
	send_request("GET ",filename," HTTP/1.1\r\n");
	send_cookies(hostname.c_str(),filename);
	return(get_size_sub(list));
};


fsize_t tHttpClient::get_file_from(const char *what,fsize_t begin,fsize_t len) {
	DSize=0;
	fsize_t llen=len;
	do{
		if (CHUNKED){
			char *str=read_string(CtrlSocket,MAX_LEN);
			if (str){
//				LOG->log(LOG_FROM_SERVER,str);
				fsize_t l=0;
				sscanf(str,"%Lx",&l);
				llen=l;
				delete[] str;
			}else{
				LOG->log(LOG_ERROR,_("Wrong chunk size!"));
				return(0);
			};
			LOG->log_printf(LOG_OK,_("Chunk size %ll"),llen);
			if (!llen){
				LOG->log(LOG_OK,_("It's last chunk!"));
				/*skip for last string*/
				str=read_string(CtrlSocket,MAX_LEN);
//				if (str) LOG->log(LOG_FROM_SERVER,str);
				while(str!=NULL && !empty_string(str)){
					delete[] str;
					str=read_string(CtrlSocket,MAX_LEN);
				};
				if (str) delete[] str;
				break;
			};
			len=llen;
		};
		int complete=1;
		FileLoaded=begin;
		do {
			if ((complete=tClient::read_data((BLOCK_READ>llen && llen>0)?llen:BLOCK_READ))<0) {
				LOG->log(LOG_WARNING,_("Data connection closed."));
				break;
			};
			if (len && FillSize>llen) FillSize=llen;
			FileLoaded+=FillSize;
			if (write_buffer()) {
				LOG->log(LOG_ERROR,_("Error have happened during writing buffer to disk!"));
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
			if (pass_first==0 && LOG->is_overlaped()){
				LOG->log(LOG_OK,_("Segment was loaded! Complete this thread."));
				return DSize;
			};
			pass_first=0;
		} while (complete!=0);
		if (complete==0){
			LOG->log(LOG_WARNING,_("EOF recieved from server!"));
			break;
		};
		if (CHUNKED) tClient::read_data(2);
	}while(CHUNKED);
	return DSize;
};

int tHttpClient::registr(const std::string &user,const std::string &password) {
	username=user;
	userword=password;
	return 0;
};

void tHttpClient::down() {
	if (CtrlSocket) CtrlSocket->down();
};

void tHttpClient::done() {
	down();
};

tHttpClient::~tHttpClient() {
	down();
};
