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

#include "httpd.h"
#include "ftpd.h"
#include "locstr.h"
#include "var.h"
#include "ntlocale.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

tHttpDownload::tHttpDownload() {
	answer=NULL;
	HTTP=NULL;
	ETag=Auth=NULL;
	RealName=NULL;
	NewRealName=NULL;
	Auth=NULL;
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
	config.timeout=cfg->timeout;
	config.time_for_sleep=cfg->time_for_sleep;
	config.number_of_attempts=cfg->number_of_attempts;
	config.get_date=cfg->get_date;
	config.retry=cfg->retry;
	HTTP->init(HOST,LOG,D_PORT,config.timeout);
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
		if (HTTP->get_status()==STATUS_FATAL) return -1;
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
				sleep(config.time_for_sleep);
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
	char buff[MAX_LEN];
	int len=strlen(field);
	if (read(D_FILE.fdesc,buff,len-1)<len-1) return NULL;
	buff[len-1]=0;
	if (index(buff,'>')){
//		lseek(D_FILE.fdesc,buff-close_bracket,SEEK_CUR);
		return NULL;
	};
	while(read(D_FILE.fdesc,buff+len-1,1)>0) {
		buff[len]=0;
		string_to_low(buff);
		if (equal(field,buff)) {
			do {
				if (read(D_FILE.fdesc,buff,1)<=0) return NULL;
			} while(*buff!='=');
			char *cur=buff;
			do {
				if (read(D_FILE.fdesc,buff,1)<=0) return NULL;
				if (*buff=='>') return NULL;
				if (*buff=='\"') break;
				if (*buff!=' ')
					if ((*buff>='a' && *buff<='z') || (*buff>=' ' && *buff<='Z')) {
						cur+=1;
						break;
					};
			} while(1);
			while(read(D_FILE.fdesc,cur,1)>0) {
				if (*cur=='\"' || *cur<=' ' || *cur=='>') break;
				cur+=1;
				if (cur-buff>MAX_LEN) return NULL; // too long field
			};
			*cur=0;
			return copy_string(buff);
		};
		char *close_bracket=index(buff,'>');
		if (close_bracket) {
			lseek(D_FILE.fdesc,buff-close_bracket,SEEK_CUR);
			return NULL;
		};
		for (int i=0;i<=len;i++)
			buff[i]=buff[i+1];
	};
	return NULL;
};


void tHttpDownload::analize_html() {
	char *teg3="a "; //2
	char *teg7="!--"; //3
	char *teg1="img "; //4
	char *teg2="link "; //5
	char *teg5="base "; //5
	char *teg4="frame "; //6
	char *teg6="/base>"; //6
	char buff[MAX_LEN];
	char *base=NULL;
	answer->done();
	answer->init(0);
	lseek(D_FILE.fdesc,0,SEEK_SET);
	while(read(D_FILE.fdesc,buff,1)>0) {
		if (*buff=='<') {
			if (read(D_FILE.fdesc,buff,6)<6)
				break;
			buff[6]=0;
			string_to_low(buff);
			char *link=NULL;
			if (equal_first(buff,teg3)) {
				lseek(D_FILE.fdesc,-4,SEEK_CUR);
				link=get_field("href");
			} else
				if (equal_first(buff,teg2)) {
					lseek(D_FILE.fdesc,-1,SEEK_CUR);
					link=get_field("href");
				} else
					if (equal_first(buff,teg1)) {
						lseek(D_FILE.fdesc,-2,SEEK_CUR);
						link=get_field("src");
					} else
						if (equal_first(buff,teg4)) {
							link=get_field("src");
						} else
							//skip coments
							if (equal_first(buff,teg7)) {
								lseek(D_FILE.fdesc,-3,SEEK_CUR);
								if (read(D_FILE.fdesc,buff,2)!=2) break;
								while (read(D_FILE.fdesc,buff+2,1)==1) {
									buff[3]=0;
									if (equal("-->",buff)) break;
									for (int i=0;i<3;i++)
										buff[i]=buff[i+1];
								};
							} else
								// <base>
								if (equal_first(buff,teg5)) {
									char *newbase=get_field("target");
									if (newbase) {
										if (base) delete(base);
										base=newbase;
									};
								} else
									//</base>
									if (equal_first(buff,teg6)) {
										delete base;
										base=NULL;
									} else {
										lseek(D_FILE.fdesc,-6,SEEK_CUR);
									};
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
	char fullname[MAX_LEN];
	fullname[0]=0;
	strcat(fullname,D_PATH);
	strcat(fullname,"/");
	strcat(fullname,D_FILE.name);
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
			return D_FILE.size;
		};
		if (temp==1) return -1;
		if (HTTP->get_status()!=STATUS_TIMEOUT) break;
		if (reconnect()) break;
	};
	LOG->add(_("Could'nt get normal answer!"),LOG_ERROR);
	return -2;
};

int tHttpDownload::download(unsigned int from,unsigned int len) {
	int success=1;
	int offset=from;
	first=1;
	while(success) {
		HTTP->set_offset(offset);
		while (first || get_size()>=0) {
			if (!ReGet) {
				if (offset) LOG->add(_("It is seemed REGET not supported! Loading from begin.."),LOG_WARNING);
				StartSize=data=offset=0;
				if (ETagChanged) break;
			};
			if (lseek(D_FILE.fdesc,offset,SEEK_SET)<0) {
				LOG->add(_("Problems with lseek()"),LOG_ERROR);
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
		data=offset;
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
	if (RealName) {
		delete RealName;
		RealName=NULL;
	};
	if (NewRealName) {
		delete NewRealName;
		NewRealName=NULL;
	};
	if (HTTP) delete HTTP;
	if (ETag) delete ETag;
	if (Auth) delete Auth;
	if (D_PATH) delete D_PATH;
	if (D_FILE.name) delete (D_FILE.name);
	if (D_FILE.fdesc) close(D_FILE.fdesc);
	if (answer) delete(answer);
	if (RealName) delete RealName;
	if (NewRealName) delete NewRealName;
	if (content_type) delete (content_type);
};
