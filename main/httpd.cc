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

#include "httpd.h"
#include "ftpd.h"
#include "locstr.h"
#include "var.h"
#include "ntlocale.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

tHtmlTeg HTML_TEGS[]={
	{"a",		"href",		(char *)NULL,	0},
	{"!--",		(char *)NULL,	"--!",		0},
	{"img",		"src",		(char *)NULL,	0},
	{"link",	"href",		(char *)NULL,	0},
	{"base",	"target",	(char *)NULL,	1},
	{"frame",	"src",		(char *)NULL,	0},
	{"/base>",	(char *)NULL,	(char *)NULL,	1}
};
const int HTML_TEGS_NUM=sizeof(HTML_TEGS)/sizeof(tHtmlTeg);

tHttpDownload::tHttpDownload() {
	LOG=NULL;
	D_FILE.name=HOST=USER=PASS=D_PATH=NULL;
	D_FILE.perm=get_permisions_from_int(CFG.DEFAULT_PERMISIONS);
	StartSize=D_FILE.size=D_FILE.type=D_FILE.fdesc=0;
	Status=D_NOTHING;

	answer=NULL;
	HTTP=NULL;
	ETag=Auth=NULL;
	RealName=NULL;
	NewRealName=NULL;
	content_type=NULL;
};

int tHttpDownload::init(tAddr *hostinfo,tLog *log,tCfg *cfg) {
	LOG=log;
	HTTP=new tHttpClient;
	RetrNum=0;
	HOST=hostinfo->host;
	USER=hostinfo->username;
	PASS=hostinfo->pass;
	D_PORT=hostinfo->port;
	answer=NULL;
	ETag=NULL;
	Auth=NULL;
	D_FILE.name=NULL;
	D_FILE.fdesc=0;
	RealName=NewRealName=NULL;
	data=0;
	first=1;
	config.copy_ints(cfg);
	HTTP->init(HOST,LOG,D_PORT,config.timeout);
	config.set_user_agent(cfg->get_user_agent());
	HTTP->set_user_agent(config.get_user_agent());
	HTTP->registr(USER,PASS);
	return reconnect();
};

void tHttpDownload::init_download(char *path,char *file) {
	tDownloader::init_download(path,file);
	NewRealName=NULL;
	if (RealName) delete RealName;
	RealName=NULL;
	RealName=parse_percents(D_FILE.name);
};

int tHttpDownload::reconnect() {
	int success=1;
	Status=D_QUERYING;
	while (success) {
		RetrNum++;
		char data[MAX_LEN];
//		if (HTTP->get_status()==STATUS_FATAL) return -1;
		if (config.number_of_attempts)
			sprintf(data,_("Retrying %i of %i...."),RetrNum,config.number_of_attempts);
		else
			sprintf(data,_("Retrying %i"),RetrNum);
		LOG->add(data,LOG_OK);
		if (RetrNum==config.number_of_attempts) {
			LOG->add(_("Max amount of retries was reached!"),LOG_ERROR);
			return -1;
		};
		HTTP->down();
		if (RetrNum>1) {
			if (HTTP->test_reget() || config.retry) {
				LOG->add(_("Sleeping"),LOG_WARNING);
				sleep(config.time_for_sleep+1);
			}
			else return -1;
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
			return(extract_from_prefixed_string(temp->body,LOCATION));
		};
		temp=answer->next();
	};
	return NULL;
};

char *tHttpDownload::get_field(char *field) {
	int buff_len=MAX_LEN;
	char *buff=new char [buff_len+1];
	int len=strlen(field);
	int failed=0;
	for (int i=0;i<len-1;i++){
		if (read(D_FILE.fdesc,buff+i,1)<1)
			failed=1;
		if (buff[i]=='>')
			failed=1;
		if (failed){
			delete(buff);return NULL;
		};
	};
	failed=0;
	buff[len-1]=0;
	while(read(D_FILE.fdesc,buff+len-1,1)>0) {
		buff[len]=0;
		string_to_low(buff);
		if (equal(field,buff)) {
			do {
				if (read(D_FILE.fdesc,buff,1)<=0){
					delete buff;return NULL;
				};
			} while(*buff!='=');
			char *cur=buff;
			char close_symbol='>';
			do {
				if (read(D_FILE.fdesc,buff,1)<=0){
					delete buff;return NULL;
				};
				if (*buff=='>'){
					delete buff;return NULL;
				};
				if (*buff=='\"'){
					close_symbol='\"';
					break;
				};
				if (*buff=='\''){
					close_symbol='\'';
					break;
				};
				if (*buff > ' ')
					if ((*buff>='a' && *buff<='z') || (*buff>=' ' && *buff<='Z')) {
						cur+=1;
						break;
					};
			} while(1);
			while(read(D_FILE.fdesc,cur,1)>0) {
				if (*cur==close_symbol || *cur<=' ' || *cur=='>') break;
				cur+=1;
				if (cur-buff>buff_len){ //reallocate buffer
					delete buff;return NULL; //field too long
				};
			};
			*cur=0;
			char *rvalue=copy_string(buff);
			delete buff;return rvalue;
		};
		if (buff[len-1]=='>') {
			delete buff;return NULL;
		};
		for (int i=0;i<=len;i++)
			buff[i]=buff[i+1];
	};
	delete buff;return NULL;
};

void tHttpDownload::skip_for_tag(char *tag) {
	if (tag==NULL || *tag==0) return;
	char buff[MAX_LEN];
	int len=strlen(tag);
	while (read(D_FILE.fdesc,buff+len,1)==1) {
		buff[len]=0;
		if (equal(tag,buff)) break;
		for (int i=0;i<len;i++) buff[i]=buff[i+1];
	};
};

void tHttpDownload::analize_html() {
	char buff[MAX_LEN];
	char *base=NULL;
	int MAX_TAG_LEN=6;
	answer->done();
	answer->init(0);
	lseek(D_FILE.fdesc,0,SEEK_SET);
	while(read(D_FILE.fdesc,buff,1)>0) {
		if (*buff=='<') {
			if (read(D_FILE.fdesc,buff,MAX_TAG_LEN)<MAX_TAG_LEN) break;
			buff[MAX_TAG_LEN]=0;
			char *link=NULL;
			int not_found=0;
			for (int i=0;i<HTML_TEGS_NUM;i++){
				if (begin_string_uncase(buff,HTML_TEGS[i].tag) && buff[strlen(HTML_TEGS[i].tag)]<=' '){
					lseek(D_FILE.fdesc,strlen(HTML_TEGS[i].tag)-MAX_TAG_LEN,SEEK_CUR);
					if (HTML_TEGS[i].field)
						link=get_field(HTML_TEGS[i].field);
					else
						skip_for_tag(HTML_TEGS[i].closetag);
					if (HTML_TEGS[i].mod){
						if (base) delete(base);
						base=link;
						link=NULL;
					};
					not_found=1;
				};
			};
			if (not_found)
				lseek(D_FILE.fdesc,-MAX_TAG_LEN,SEEK_CUR);
			if (link) {
				if (base && !global_url(link)) {
					char *tmp;
					tmp=compose_path(base,link);
					delete link;
					link=tmp;
				};
				answer->add(link);
				delete(link);
			};
		};
	};
	if (base) delete(base);
};

tStringList *tHttpDownload::dir() {
	return answer;
};

int tHttpDownload::analize_answer() {
	/*  Here we need few analisation of http answer
	 *	AcceptRanges: bytes
	 *	Content-Length: (int)
	 *	Content-Range: bytes (int)-(int)/(int)
	 *	Last-Modified: 
	 */
	ETagChanged=0;
	tString *temp=answer->last();
	if (!temp) return -1;
	char *CL="content-length:";
	char *AR="accept-range:";
	char *CR="content-range:";
	char *CD="content-disposition:";
	char *ET="etag:";
	char *WA="www-authenticate:";
	char *LM="last-modified:";
	char *CT="content-type:";
	int rvalue=0;
	ReGet=0;
	MustNoReget=0;
	while(temp) {
		string_to_low(temp->body,':');
		if (begin_string(temp->body,CT)) {
			if (content_type) delete (content_type);
			content_type=extract_from_prefixed_string(temp->body,CT);
			string_to_low(content_type);
		};
		if (begin_string(temp->body,AR)) {
			if (strstr(temp->body+strlen(AR),"bytes"))
				Status=0;
		};
		if (begin_string(temp->body,CL)) {
			sscanf(temp->body+strlen(CL),"%i",&rvalue);
			char data[MAX_LEN];
			sprintf(data,_("Size for download is %i bytes"),rvalue);
			LOG->add(data,LOG_OK);
		};
		if (begin_string(temp->body,CR)) {
			if (strstr(temp->body+strlen(CR),"bytes"))
				ReGet=1;
		};
		if (begin_string(temp->body,LM)) {
			/* Need to extract date
			 */
			char *tmp=temp->body+strlen(LM);
			while (*tmp==' ') tmp++;
			D_FILE.date=ctime_to_time(tmp);
		};
		if (begin_string(temp->body,ET)) {
			char *ETag1=extract_from_prefixed_string(temp->body,ET);
			char *tmp=index(ETag1,':');
			if (tmp) *tmp=0;
			if (ETag) {
				if (!equal(ETag,ETag1)) {
					MustNoReget=1;
					ETagChanged=1;
					delete ETag;
					ETag=ETag1;
				} else delete(ETag1);
			} else {
				ETag=ETag1;
			};
		};
		if (begin_string(temp->body,CD)) {
			char *point=strstr(temp->body,"filename");
			if (point) {
				point+=strlen("filename");
				point=index(point,'=');
				if (point) {
					point++;
					NewRealName=copy_string(point);
					int len=strlen(NewRealName);
					NewRealName[len-2]=0;
					point=rindex(NewRealName,'/');
					if (point) {
						char *tmp=copy_string(point+1);
						delete(NewRealName);
						NewRealName=tmp;
					};
				};
			};
		};
		if (begin_string(temp->body,WA)) {
			if (!Auth) {
				Auth=extract_from_prefixed_string(temp->body,WA);
			};
		};
		temp=answer->next();
	};
	if (MustNoReget) ReGet=0;
	return rvalue;
};

int tHttpDownload::get_size() {
	char *fullname;
	if (strlen(D_FILE.name))
		fullname=compose_path(D_PATH,D_FILE.name);
	else
		fullname=sum_strings(D_PATH,"/");
	if (!answer) {
		answer=new tStringList;
		answer->init(0);
	};
	while (1) {
		answer->done();
		HTTP->set_offset(data);
		LOG->add(_("Sending http request..."),LOG_OK);
		if (USER && PASS) HTTP->set_auth(1);
		int temp=HTTP->get_size(fullname,answer);
		if (temp==0) {
			LOG->add(_("Answer read ok"),LOG_OK);
			D_FILE.size=analize_answer();
			if (ReGet && D_FILE.size>=0)
				D_FILE.size+=data;
			delete fullname;
			return D_FILE.size;
		};
		if (temp==1){
			delete fullname;
			return -1;
		};
		if (HTTP->get_status()!=STATUS_TIMEOUT) break;
		if (reconnect()) break;
	};
	delete fullname;
	LOG->add(_("Could'nt get normal answer!"),LOG_ERROR);
	return -2;
};

void tHttpDownload::rollback_before(){
	StartSize=data=rollback(data);
};

int tHttpDownload::download(unsigned int from,unsigned int len) {
	int success=1;
	int offset=from;
	first=1;
	while(success) {
		if (!first) StartSize=offset=data=rollback(offset);
		HTTP->set_offset(offset);
		while (first || get_size()>=0) {
			if (!ReGet) {
				if (offset) LOG->add(_("It is seemed REGET not supported! Loading from begin.."),LOG_WARNING);
				StartSize=data=offset=0;
				lseek(D_FILE.fdesc,0,SEEK_SET);
				if (ETagChanged) break;
			};
			Status=D_DOWNLOAD;
			int ind=HTTP->get_file_from(NULL,offset,len,D_FILE.fdesc);
			if (ind>=0) {
				offset+=ind;
				len-=ind;
/*				if (HTTP->get_status()==0) {
					if (D_FILE.size!=0 && offset==D_FILE.size) return 0;
					if (D_FILE.size==0) return 0;
				};
*/
			};
			break;
		};
//		data=offset;
		first=0;
		if (HTTP->get_status()==STATUS_FATAL) return -1;
		if (offset==D_FILE.size && D_FILE.size!=0) break;
		if (HTTP->get_status()==0 && D_FILE.size==0) break;
		if (reconnect()) return -1;
	};
	fchmod(D_FILE.fdesc,D_FILE.perm);
	return 0;
};

int tHttpDownload::get_child_status() {
	return HTTP->get_status();
};

int tHttpDownload::get_readed() {
	return HTTP->get_readed();
};

char *tHttpDownload::get_real_name() {
	if (NewRealName) return NewRealName;
	if (RealName)   return RealName;
	return D_FILE.name;
};

char *tHttpDownload::get_content_type() {
	return content_type;
};


int tHttpDownload::reget() {
	return ReGet;
};

void tHttpDownload::make_full_pathes(const char *path,char **name,char **guess) {
	int flag=strlen(D_FILE.name);
	char *full_path;
	if (config.http_recursing)
		full_path=compose_path(path,D_PATH);
	else
		full_path=copy_string(path);
	char *temp;
	char *question_sign=index(full_path,'?');
	if (question_sign) *question_sign=0;
	if (flag){
		temp=sum_strings(".",D_FILE.name);
		*name=compose_path(full_path,temp);
		*guess=compose_path(full_path,D_FILE.name);
	}else{
		temp=sum_strings(".",CFG.DEFAULT_NAME);
		*name=compose_path(full_path,temp);
		*guess=compose_path(full_path,CFG.DEFAULT_NAME);
	};
	make_dir_hier(full_path);
	delete full_path;
	delete temp;
};

void tHttpDownload::make_full_pathes(const char *path,char *another_name,char **name,char **guess) {
	char *temp=sum_strings(".",another_name);
	char *full_path=compose_path(path,D_PATH);
	char *question_sign=index(full_path,'?');
	if (question_sign) *question_sign=0;
	*name=compose_path(full_path,temp);
	*guess=compose_path(full_path,another_name);
	make_dir_hier(full_path);
	delete full_path;
	delete temp;
};

int tHttpDownload::create_file(char *data,char *another_name) {
	if (NewRealName) {
		if (RealName) delete RealName;
		RealName=NewRealName;
		NewRealName=NULL;
	};
	char *temp=D_FILE.name;
	if (RealName) D_FILE.name=RealName;
	D_FILE.type=T_FILE;
	int rvalue=tDownloader::create_file(data,another_name);
	if (RealName) D_FILE.name=temp;
	return rvalue;
};


int tHttpDownload::delete_file(char *data) {
	char *temp=D_FILE.name;
	if (RealName) D_FILE.name=RealName;
	int rvalue=tDownloader::delete_file(data);
	if (RealName) D_FILE.name=temp;
	return rvalue;
};


void tHttpDownload::done() {
	HTTP->done();
	if (D_FILE.fdesc) {
		close(D_FILE.fdesc);
		D_FILE.fdesc=0;
	};
};

tHttpDownload::~tHttpDownload() {
	if (RealName) delete RealName;
	if (NewRealName) delete NewRealName;
	if (HTTP) delete HTTP;
	if (ETag) delete ETag;
	if (Auth) delete Auth;
	if (D_PATH) delete D_PATH;
	if (D_FILE.name) delete (D_FILE.name);
	if (D_FILE.fdesc) close(D_FILE.fdesc);
	if (answer) delete(answer);
	if (content_type) delete (content_type);
};
