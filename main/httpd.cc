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
#include "signal.h"
#include "sslsocket.h"

using namespace d4x;

enum HTTP_ANSWERS_ENUM{
	H_CONTENT_LENGTH,
	H_CONTENT_RANGE,
	H_CONTENT_TYPE,
	H_CONTENT_DISPOSITION,
	H_ACCEPT_RANGE,
	H_ETAG,
	H_WWW_AUTHENTICATE,
	H_LAST_MODIFIED,
	H_SET_COOKIE,
	H_TRANSFER_ENCODING,
	H_CONNECTION,
	H_PROXY_CONNECTION
};

char *http_answers[]={
	"content-length:",
	"content-range:",
	"content-type:",
	"content-disposition:",
	"accept-ranges:",
	"etag:",
	"www-authenticate:",
	"last-modified:",
	"set-cookie:",
	"transfer-encoding:",
	"connection:",
	"proxy-connection:"
};


/*************************************************************/

enum CONTENDDISPOSITION_ENUM{
	CDE_FILENAME,
	CDE_M_DATE,
	CDE_R_DATE,
	CDE_C_DATE,
	CDE_SIZE,
	CDE_NONE
};

d4xContentDisposition::d4xContentDisposition(){
	size=-1;
	c_date=m_date=r_date=0;
};

d4xContentDisposition::d4xContentDisposition(char *httpstr){
	/* skip first parm cos it's 'inline' or 'attachment' */
	char *tmp=index(httpstr,';');
	while (tmp){
		tmp+=1;
		char *name=get_name(tmp);
		char *value=get_value(tmp);
		if (value){
			switch(get_code(name)){
			case CDE_FILENAME:
				filename.set(value);
				break;
			case CDE_M_DATE:
				m_date=ctime_to_time(value);
				break;
			case CDE_C_DATE:
				c_date=ctime_to_time(value);
				break;
			case CDE_R_DATE:
				r_date=ctime_to_time(value);
				break;
			case CDE_SIZE:
				sscanf(value,"%lli",&size);
				break;
			};
			delete[] value;
		};
		if (name) delete[] name;
		tmp=index(tmp,';');
	};
};

int d4xContentDisposition::get_code(char *name){
	char *parm_names[]={
		"filename",
		"modification-date",
		"read-date",
		"creation-date",
		"size"
	};
	if (name==NULL) return(CDE_NONE);
	for (unsigned int i=0;i<sizeof(parm_names)/sizeof(char*);i++)
		if (equal_uncase(parm_names[i],name))
			return(i);
	return(CDE_NONE);
};

char *d4xContentDisposition::get_value(char *s){
	char *tmp=index(s,'=');
	if (tmp==NULL) return(NULL);
	tmp=skip_spaces(tmp+1);
	char *sqs=index(s,';');
	if (sqs==NULL) sqs=tmp+strlen(tmp)-1;
	else sqs-=1;
	int len=sqs-tmp;
	if (len<=0) return(NULL);
	char *rval=copy_string2(tmp,len);
	for (int i=len-1;i>=0;i++){
		if (isspace(rval[i])) rval[i]=0;
		else break;
	};
//	printf("value='%s'\n",rval);
	return(rval);
};

char *d4xContentDisposition::get_name(char *s){
	char *tmp=skip_spaces(s);
	char *eqs=index(s,'=');
	if (eqs==NULL) return(NULL);
	int len=eqs-tmp;
	char *rval=copy_string2(tmp,len);
	for (int i=len-1;i>=0;i++){
		if (isspace(rval[i])) rval[i]=0;
		else break;
	};
//	printf("name='%s'\n",rval);
	return(rval);
};

d4xContentDisposition::~d4xContentDisposition(){
	/* nothing to do yet */
};

/*************************************************************/

tHttpDownload::tHttpDownload():tDownloader(){
	answer=NULL;
	HTTP=NULL;
	OldETag=ETag=Auth=NULL;
	content_type=NULL;
	content_disp=NULL;
};

tHttpDownload::tHttpDownload(tWriterLoger *log):tDownloader(log){
	answer=NULL;
	HTTP=NULL;
	OldETag=ETag=Auth=NULL;
	content_type=NULL;
	content_disp=NULL;
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

int tHttpDownload::init(const d4x::URL &hostinfo,tCfg *cfg,SocketPtr s) {
	Persistent=0;
#ifdef HAVE_SSL
	if (hostinfo.proto==D_PROTO_HTTPS && !s)
		s.reset(new d4x::SSLSocket());
	HTTP=new tHttpClient(cfg,s);
#else
	HTTP=new tHttpClient(cfg);
#endif //HAVE_SSL	
	RetrNum=0;
	ADDR=hostinfo;
	answer=NULL;
	ETag=NULL;
	Auth=NULL;
	D_FILE.type=T_FILE; //we don't know any other when download via http
	config.copy_ints(cfg);
	if (cfg->split){
		config.retry=0;
		config.rollback=0;
	};
	HTTP->init(ADDR.host,LOG,ADDR.port,config.timeout);
	config.user_agent.set(cfg->user_agent.get());
	config.referer.set(cfg->referer.get());
	HTTP->set_user_agent(config.user_agent.get(),config.referer.get());
	HTTP->registr(ADDR.user,ADDR.pass);
	REQUESTED_URL=make_name();
	if (!s)
		return reconnect();
	HTTP->import_ctrl_socket(s);
	RetrNum=1;
	tDownloader::reconnect();
	return(0);
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
		tDownloader::reconnect();
		HTTP->down();
		if (RetrNum>1) {
			LOG->log(LOG_WARNING,_("Sleeping"));
			d4x::USR1Off2On sig_unlocker;
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

int tHttpDownload::persistent(){
	return(Persistent);
};

fsize_t tHttpDownload::another_way_get_size() {
	return 0;
};

fsize_t tHttpDownload::analize_answer() {
	/*  Here we need few analisation of http answer
	 *	AcceptRanges: bytes
	 *	Content-Length: (int)
	 *	Content-Range: bytes (int)-(int)/(int)
	 *	Last-Modified: 
	 */
	if (HTTP->HTTP_SUBVER==1 || HTTP->HTTP_VER>1)
		Persistent=1;
	ETagChanged=0;
	OldETag=ETag;
	ETag=NULL;
	tString *temp=answer->last();
	if (!temp) return -1;
	fsize_t rvalue=0;
	ReGet=0;
	MustNoReget=0;
	HTTP->CHUNKED=0;
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
			sscanf(temp->body+strlen(STR),"%Li",&rvalue);
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
			fsize_t b[3];
			if (sscanf(a,"%Li-%Li/%Li",&b[0],&b[1],&b[2])==3){
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
			if (OldETag && config.ihate_etag==0) {
				if (!equal(OldETag,ETag)) {
					MustNoReget=ETagChanged=1;
				};
			};
			break;
		};
		case H_CONTENT_DISPOSITION:{
			if (content_disp) delete(content_disp);
			content_disp=new d4xContentDisposition(temp->body+strlen(STR));
			break;
		};
		case  H_WWW_AUTHENTICATE:{
			if (!Auth) {
				Auth=extract_from_prefixed_string(temp->body,STR);
			};
			break;
		};
		case H_SET_COOKIE:{
// Set-Cookie: zzzplayuniq=free3-chi-48-2001-Dec-28_04:57:07; expires=Sat, 31 Dec 2005 00:00:00 GMT; path=/; domain=.playboy.com;
			/* FIXME: to avoid lost cookies, parse answer before redirection */
			tCookie *cookie=new tCookie;
			if (cookie->parse(temp->body+strlen(STR),ADDR.host.c_str(),ADDR.path.c_str())==0){
				LOG->cookie_set(cookie);
			};
			break;
		};
		case H_TRANSFER_ENCODING:{
			if (strstr(temp->body,"chunked")){
				HTTP->CHUNKED=1;
			};
			break;
		};
		case H_PROXY_CONNECTION:
		case H_CONNECTION:{
			if (strstr(temp->body,"close")){
				Persistent=0;
			};
			break;
		};
		};
		temp=answer->next();
	};
	if (rvalue>0)
		LOG->log_printf(LOG_OK,_("Size for download is %ll bytes"),rvalue);
	if (LOADED==0) ReGet=1;
	if (MustNoReget) ReGet=0;
	if (OldETag){ /* if etag disappeared */
		delete[] OldETag;
		OldETag=NULL;
	};
	return rvalue;
};

std::string tHttpDownload::make_name(){
	return hexed_string((ADDR.path/ADDR.file)+(ADDR.params.empty()?std::string():std::string("?")+ADDR.params));
};

fsize_t tHttpDownload::get_size_only() {
	if (!answer) {
		answer=new tStringList;
		answer->init(0);
	};
	while (1) {
		answer->done();
		HTTP->set_offset(0);
		LOG->log(LOG_OK,_("Sending HTTP request..."));
		int temp=HTTP->get_size_only(REQUESTED_URL.c_str(),answer);
		switch (temp){
		case 0:{ // all ok
			LOG->log(LOG_OK,_("Answer read ok"));
			D_FILE.size=analize_answer();
			return D_FILE.size;
		};
		case -2: // bad error code
			print_error(ERROR_BAD_ANSWER);
			return -2;
		case -1: // timeout
			break;
		case 1: // redirection
			return -1;
		};
		if (reconnect()) break;
	};
	return -2;
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
		int temp=HTTP->get_size(REQUESTED_URL.c_str(),answer);
		switch (temp){
		case 0:{ // all ok
			LOG->log(LOG_OK,_("Answer read ok"));
			D_FILE.size=analize_answer();
			if (ReGet && D_FILE.size>=0)
				D_FILE.size+=LOADED;
			return D_FILE.size;
		};
		case -2: // bad error code
			print_error(ERROR_BAD_ANSWER);
			return -2;
		case -1: // timeout
			break;
		case 1: // redirection
			analize_answer(); // to avoid lost cookies;
			return -1;
		};
		if (reconnect()) break;
	};
	return -2;
};

void tHttpDownload::pass_first_segment(){
	if (HTTP) HTTP->pass_first_segment();
};

int tHttpDownload::download(fsize_t len) {
	int success=1;
	int first=1;
	StartSize=LOADED;
	fsize_t length_to_load=len>0?LOADED+len:0;
	while(success) {
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
				LOG->log_printf(LOG_OK,_("%ll bytes loaded."),ind);
			};
			break;
		};
		first=0;
//		if (HTTP->get_status()==STATUS_FATAL) return -1;
		if (LOG->is_overlaped()) break;
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

d4xContentDisposition *tHttpDownload::get_content_disp(){
	return content_disp;
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
	d4x::Path full_path(path);
	if (config.http_recursing){
		if (config.leave_server){
			full_path/=d4x::Path(ADDR.host)/ADDR.path;
		}else
			full_path/=ADDR.path;
	};
	std::string temp(ADDR.file.empty()?std::string(CFG.DEFAULT_NAME):ADDR.file);
	if (config.http_recursing && !ADDR.params.empty())
		temp+=std::string(config.quest_sign_replace?"_":"?")+ADDR.params;
	*guess=copy_string((full_path/temp).c_str());
	full_path/=std::string(".")+temp;
	*name=copy_string(full_path.c_str());
};

void tHttpDownload::make_full_pathes(const char *path,const char *another_name,char **name,char **guess) {
	DBC_RETURN_IF_FAIL(path!=NULL);
	DBC_RETURN_IF_FAIL(another_name!=NULL);
	DBC_RETURN_IF_FAIL(name!=NULL);
	DBC_RETURN_IF_FAIL(guess!=NULL);
	d4x::Path full_path(path);
	if (config.http_recursing){
		if (config.leave_server){
			full_path/=d4x::Path(ADDR.host)/ADDR.path;
		}else
			full_path/=ADDR.path;
	};
	std::string temp(another_name);
	if (config.http_recursing && !ADDR.params.empty())
    		temp+=std::string(config.quest_sign_replace?"_":"?")+ADDR.params;
	*guess=copy_string((full_path/temp).c_str());
	full_path/=std::string(".")+temp;
	*name=copy_string(full_path.c_str());
};

void tHttpDownload::done() {
	HTTP->done();
};

SocketPtr tHttpDownload::export_ctrl_socket(){
	if (HTTP) return(HTTP->export_ctrl_socket());
	return(SocketPtr());
};

tHttpDownload::~tHttpDownload() {
	if (HTTP) delete HTTP;
	if (ETag) delete[] ETag;
	if (OldETag) delete[] OldETag;
	if (Auth) delete[] Auth;
	if (answer) delete(answer);
	if (content_type) delete[] content_type;
	if (content_disp) delete(content_disp);
};
