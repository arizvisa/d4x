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

#include "httpd.h"
#include "ftpd.h"
#include "locstr.h"
#include "var.h"
#include "ntlocale.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>

enum HTTP_ANSWERS_ENUM{
	H_CONTENT_LENGTH,
	H_CONTENT_RANGE,
	H_CONTENT_TYPE,
	H_CONTENT_DISPOSITION,
	H_ACCEPT_RANGE,
	H_ETAG,
	H_WWW_AUTHENTICATE,
	H_LAST_MODIFIED
};

char *http_answers[]={
	"content-length:",
	"content-range:",
	"content-type:",
	"content-disposition:",
	"accept-ranges:",
	"etag:",
	"www-authenticate:",
	"last-modified:"
};

tHttpDownload::tHttpDownload():tDownloader(){
	answer=NULL;
	HTTP=NULL;
	OldETag=ETag=Auth=NULL;
	content_type=NULL;
	REQUESTED_URL=NULL;
};

void tHttpDownload::print_error(int error_code){
	switch(error_code){
	case ERROR_BAD_ANSWER:
		LOG->log(LOG_ERROR,_("Couldn't get normal answer!"));
		break;
	default:
		tDownloader::print_error(error_code);
	};
};

int tHttpDownload::init(tAddr *hostinfo,tWriterLoger *log,tCfg *cfg) {
	LOG=log;
	HTTP=new tHttpClient(cfg);
	RetrNum=0;
	ADDR.copy(hostinfo);
	answer=NULL;
	ETag=NULL;
	Auth=NULL;
	D_FILE.type=T_FILE; //we don't know any other when download via http
	config.copy_ints(cfg);
	if (cfg->split){
		config.retry=0;
		config.rollback=0;
	};
	HTTP->init(ADDR.host.get(),LOG,ADDR.port,config.timeout);
	config.user_agent.set(cfg->user_agent.get());
	config.referer.set(cfg->referer.get());
	HTTP->set_user_agent(config.user_agent.get(),config.referer.get());
	HTTP->registr(ADDR.username.get(),ADDR.pass.get());
	REQUESTED_URL=make_name();
	return reconnect();
};

int tHttpDownload::reconnect() {
	int success=1;
	Status=D_QUERYING;
	while (success) {
		if (config.number_of_attempts &&
		    RetrNum>=config.number_of_attempts+1) {
			print_error(ERROR_ATTEMPT_LIMIT);
			return -1;
		};
		RetrNum++;
		print_error(ERROR_ATTEMPT);
		HTTP->down();
		if (RetrNum>1) {
			LOG->log(LOG_WARNING,_("Sleeping"));
			sleep(config.time_for_sleep+1);
		};
		if (HTTP->reinit()==0)
			success=0;
	};
	return 0;
};

char * tHttpDownload::get_new_url() {
	tString *temp=answer->last();
	char *LOCATION="location:";
	while(temp) {
		string_to_low(temp->body,':');
		if (begin_string(temp->body,LOCATION)) {
			char *str=extract_from_prefixed_string(temp->body,LOCATION);
			del_crlf(str);
			return(str);
		};
		temp=answer->next();
	};
	return NULL;
};

tStringList *tHttpDownload::dir() {
	return answer;
};

fsize_t tHttpDownload::analize_answer() {
	/*  Here we need few analisation of http answer
	 *	AcceptRanges: bytes
	 *	Content-Length: (int)
	 *	Content-Range: bytes (int)-(int)/(int)
	 *	Last-Modified: 
	 */
	ETagChanged=0;
	OldETag=ETag;
	ETag=NULL;
	tString *temp=answer->last();
	if (!temp) return -1;
	fsize_t rvalue=0;
	ReGet=0;
	MustNoReget=0;
	while(temp) {
		char *STR=NULL;
		unsigned int i;
		for (i=0;i<sizeof(http_answers)/sizeof(char*);i++){
			STR=http_answers[i];
			if (begin_string_uncase(temp->body,STR))
				break;
		};
//		printf("i=%i\n",i);
		switch(i){
		case H_CONTENT_TYPE:{
			if (content_type) delete [] content_type;
			content_type=extract_from_prefixed_string(temp->body,STR);
			if (content_type && strstr(content_type,"multipart"))
				ReGet=1;
			break;
		};
		case H_ACCEPT_RANGE:{
			if (strstr(temp->body+strlen(STR),"bytes"))
				Status=0;
			break;
		};
		case H_CONTENT_LENGTH:{
			sscanf(temp->body+strlen(STR),"%li",&rvalue);
			break;
		};
		case H_CONTENT_RANGE:{
			char *a=strstr(temp->body+strlen(STR),"bytes");
			if (a){
				a+=strlen("bytes");
				while (*a && !isdigit(*a))
					a+=1;
			}else
				a=temp->body+strlen(STR);
			int b[3];
			if (sscanf(a,"%i-%i/%i",&b[0],&b[1],&b[2])==3){
				ReGet=1;
				rvalue=b[2]-b[0];
			};
			break;
		};
		case H_LAST_MODIFIED:{
			/* Need to extract date
			 */
			char *tmp=temp->body+strlen(STR);
			while (*tmp==' ') tmp++;
			D_FILE.date=ctime_to_time(tmp);
			break;
		};
		case H_ETAG:{
			ETag=extract_from_prefixed_string(temp->body,STR);
			char *tmp=index(ETag,':');
			if (tmp) *tmp=0;
			if (OldETag) {
				if (!equal(OldETag,ETag)) {
					MustNoReget=ETagChanged=1;
				};
			};
			break;
		};
		case H_CONTENT_DISPOSITION:{
			break;
		};
		case  H_WWW_AUTHENTICATE:{
			if (!Auth) {
				Auth=extract_from_prefixed_string(temp->body,STR);
			};
			break;
		};
		};
		temp=answer->next();
	};
	if (rvalue>0)
		LOG->log_printf(LOG_OK,_("Size for download is %i bytes"),rvalue);
	if (LOADED==0) ReGet=1;
	if (MustNoReget) ReGet=0;
	if (OldETag){ /* if etag disappeared */
		delete[] OldETag;
		OldETag=NULL;
	};
	return rvalue;
};

char *tHttpDownload::make_name(){
	char *parsed_name=unparse_percents(ADDR.file.get());
	char *parsed_path=unparse_percents(ADDR.path.get());
	char *rvalue=new char[strlen(parsed_path)+strlen(parsed_name)+
			     (ADDR.params.get() ? strlen(ADDR.params.get())+1:0)+
			     strlen("//")+1];
	*rvalue=0;
	strcat(rvalue,"/");
	strcat(rvalue,parsed_path);
	int len=strlen(parsed_path);
	if (*parsed_path!=0 && len && parsed_path[len-1]!='/')
		strcat(rvalue,"/");
	strcat(rvalue,parsed_name);
	if (ADDR.params.get()){
		strcat(rvalue,"?");
		strcat(rvalue,ADDR.params.get());
	};
	delete[] parsed_path;
	delete[] parsed_name;
	return rvalue;
};

fsize_t tHttpDownload::get_size() {
	if (!answer) {
		answer=new tStringList;
		answer->init(0);
	};
	while (1) {
		answer->done();
		HTTP->set_offset(LOADED);
		LOG->log(LOG_OK,_("Sending HTTP request..."));
		int temp=HTTP->get_size(REQUESTED_URL,answer);
		if (temp==0) {
			LOG->log(LOG_OK,_("Answer read ok"));
			D_FILE.size=analize_answer();
			if (ReGet && D_FILE.size>=0)
				D_FILE.size+=LOADED;
			return D_FILE.size;
		};
		if (temp==1){
			return -1;
		};
		if (HTTP->get_status()!=STATUS_TIMEOUT &&
		    HTTP->get_status()!=STATUS_BAD_ANSWER) break;
		if (reconnect()) break;
	};
	print_error(ERROR_BAD_ANSWER);
	return -2;
};

int tHttpDownload::download(fsize_t len) {
	int success=1;
	int first=1;
	fsize_t length_to_load=len>0?LOADED+len:0;
	while(success) {
		StartSize=LOADED;
		if (!first) StartSize=rollback();
		HTTP->set_offset(LOADED);
		if (ReGet && len==0 && LOADED &&
		    remote_file_changed()){
			print_error(ERROR_FILE_UPDATED);
			if (config.retry==0) return(-1);
			first=0;
			LOADED=0;
			LOG->shift(0);
			LOG->truncate();
		};
		while (first || get_size()>=0) {
			if (!ReGet) {
				if (LOADED){
					LOG->log(LOG_WARNING,_("It is seemed REGET not supported! Loading from begin.."));
				};
				if (ETagChanged && LOADED!=0){
					LOG->log(LOG_WARNING,_("ETag was changed, restarting again..."));
					StartSize=LOADED=0;
					LOG->shift(0);
					break;
				};
				if (config.retry==0) break;
				StartSize=LOADED=0;
				LOG->shift(0);
				LOG->truncate(); //to avoid displaing wron size
			};
			Status=D_DOWNLOAD;
			fsize_t to_load=len>0?length_to_load-LOADED:0;
			fsize_t ind=HTTP->get_file_from(NULL,LOADED,to_load);
			if (ind>=0) {
				LOADED+=ind;
				LOG->log_printf(LOG_OK,_("%i bytes loaded."),ind);
			};
			break;
		};
		first=0;
//		if (HTTP->get_status()==STATUS_FATAL) return -1;
		if (len>0 && LOADED>=length_to_load) break;
		if (LOADED==D_FILE.size && D_FILE.size!=0) break;
		if (HTTP->get_status()==0 && D_FILE.size==0) break;
		if (reconnect()) return -1;
	};
	return 0;
};

int tHttpDownload::get_child_status() {
	return HTTP->get_status();
};

fsize_t tHttpDownload::get_readed() {
	return (HTTP->get_readed());
};

char *tHttpDownload::get_content_type() {
	return content_type;
};


int tHttpDownload::reget() {
	return ReGet;
};

void tHttpDownload::make_full_pathes(const char *path,char **name,char **guess) {
	DBC_RETURN_IF_FAIL(path!=NULL);
	DBC_RETURN_IF_FAIL(name!=NULL);
	DBC_RETURN_IF_FAIL(guess!=NULL);
	char *full_path;
	int flag=strlen(ADDR.file.get());
	if (config.http_recursing){
		if (config.leave_server){
			char *tmp=compose_path(path,ADDR.host.get());
			full_path=compose_path(tmp,ADDR.path.get());
			delete[] tmp;
		}else
			full_path=compose_path(path,ADDR.path.get());
	}else
		full_path=copy_string(path);
	char *temp;
	if (flag){
		temp=(config.http_recursing && ADDR.params.get())?
			sum_strings(".",ADDR.file.get(),"?",ADDR.params.get(),NULL):
			sum_strings(".",ADDR.file.get(),NULL);
		*name=compose_path(full_path,temp);
		delete[] temp;
		temp=(config.http_recursing && ADDR.params.get())?
			sum_strings(ADDR.file.get(),"?",ADDR.params.get(),NULL):
			sum_strings(ADDR.file.get(),NULL);
		*guess=compose_path(full_path,temp);
	}else{
		temp=(config.http_recursing && ADDR.params.get())?
			sum_strings(".",CFG.DEFAULT_NAME,"?",ADDR.params.get(),NULL):
			sum_strings(".",CFG.DEFAULT_NAME,NULL);
		*name=compose_path(full_path,temp);
		delete[] temp;
		temp=(config.http_recursing && ADDR.params.get())?
			sum_strings(CFG.DEFAULT_NAME,"?",ADDR.params.get(),NULL):
			sum_strings(CFG.DEFAULT_NAME,NULL);
		*guess=compose_path(full_path,temp);
	};
	delete[] full_path;
	delete[] temp;
};

void tHttpDownload::make_full_pathes(const char *path,char *another_name,char **name,char **guess) {
	DBC_RETURN_IF_FAIL(path!=NULL);
	DBC_RETURN_IF_FAIL(another_name!=NULL);
	DBC_RETURN_IF_FAIL(name!=NULL);
	DBC_RETURN_IF_FAIL(guess!=NULL);
	char *temp=sum_strings(".",another_name,NULL);
	char *full_path=NULL;
	if (config.http_recursing)
		full_path=compose_path(path,ADDR.path.get());
	else
		full_path=copy_string(path);
//	char *question_sign=index(full_path,'?');
//	if (question_sign) *question_sign=0;
	*name=compose_path(full_path,temp);
	*guess=compose_path(full_path,another_name);
	delete[] full_path;
	delete[] temp;
};

void tHttpDownload::done() {
	HTTP->done();
};

tHttpDownload::~tHttpDownload() {
	if (REQUESTED_URL) delete[] REQUESTED_URL;
	if (HTTP) delete HTTP;
	if (ETag) delete[] ETag;
	if (OldETag) delete[] OldETag;
	if (Auth) delete[] Auth;
	if (answer) delete(answer);
	if (content_type) delete[] content_type;
};
