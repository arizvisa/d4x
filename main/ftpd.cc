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

#include "ftpd.h"
#include "ftp.h"
#include "client.h"
#include "liststr.h"
#include "var.h"
#include "locstr.h"
#include "log.h"
#include "ntlocale.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>

//****************************************/
void extract_link(char *src,char *dst) {
	char *delim=" -> ";
	if (src) {
		char *tmp=strstr(src,delim);
		if (tmp) {
			strcpy(dst,tmp+strlen(delim));
			del_crlf(dst);
		} else dst[0]=0;
	};
};

int type_from_str(char *data) {
	if (*data=='d')
		return T_DIR;
	if (*data=='l')
		return T_LINK;
	if (*data=='c' || *data=='b')
		return T_DEVICE;
	return T_FILE;
};

int permisions_from_str(char *a) {
	if (a==NULL || strlen(a)<10) return(S_IRUSR|S_IWUSR);
	int perm=0;
	if (a[1]=='r') perm|=S_IRUSR;
	if (a[2]=='w') perm|=S_IWUSR;
	if (a[3]=='x' || a[3]=='s' || a[3]=='S') perm|=S_IXUSR ;
	if (a[4]=='r') perm|=S_IRGRP;
	if (a[5]=='w') perm|=S_IWGRP;
	if (a[6]=='x' || a[6]=='s' || a[6]=='S') perm|=S_IXGRP;
	if (a[7]=='r') perm|=S_IROTH;
	if (a[8]=='w') perm|=S_IWOTH;
	if (a[9]=='x') perm|=S_IXOTH;
	return perm;
};

int date_from_str(char *src) {
	char *data=new char[strlen(src)+1];
	char *tmp;
	int month,day;

	tmp=extract_string(src,data,5);
	if (is_string(data)){
		tmp=extract_string(tmp,data);
	};

	time_t NOW=time(NULL);
	struct tm date;
	localtime_r(&NOW,&date);
	date.tm_isdst=-1;
	month=date.tm_mon;
	day=date.tm_mday;
	if (tmp && *tmp) {
		tmp=extract_string(tmp,data);
		date.tm_mon=convert_month(data);
	};
	if (tmp && *tmp) {
		tmp=extract_string(tmp,data);
		sscanf(data,"%i",&date.tm_mday);
	};
	if (tmp && *tmp) {
		extract_string(tmp,data);
		if (index(data,':')) {
			sscanf(data,"%i:%i",&date.tm_hour,&date.tm_min);
			if (month<date.tm_mon || (month==date.tm_mon && day<date.tm_mday))
				date.tm_year-=1;
		} else {
			sscanf(data,"%i",&date.tm_year);
			date.tm_year-=1900;
		};
	};
	delete data;
	return mktime(&date);
};

/* parsing string of LIST -la command
   char *src - string for parsing
   tFileInfo *dst - where put result
   int flag - need put strings (name and name of link) to result or not
 */
void cut_string_list(char *src,tFileInfo *dst,int flag) {
	if (src==NULL || dst==NULL) return;
	int srclen=strlen(src)+1;
	char *str1=new char[srclen];
	char *name=new char[srclen];
	*str1=0;
	*name=0;
	int par1;
	extract_string(src,str1,5);
	if (strlen(str1)){
// unix style listing
		char *tmp;
		if (!is_string(str1)){
			tmp=skip_strings(src,8);
			sscanf(src,"%s %u %s %s %u %s %u %s %s",
	       		str1,&par1,str1,str1,&dst->size,str1,&par1,str1,name);
		}else{
			tmp=skip_strings(src,7);
			sscanf(src,"%s %u %s %u %s %u %s %s",
	       		str1,&par1,str1,&dst->size,str1,&par1,str1,name);
		};
		dst->type=type_from_str(src);
		if (dst->type!=T_DEVICE) {
			dst->perm=permisions_from_str(src);
			dst->date=date_from_str(src);
		};
		if (flag) {
			if (tmp) dst->name=copy_string(tmp);
			else dst->name=copy_string(name);
			del_crlf(dst->name);
		};
		if (dst->type==T_LINK) {
			extract_link(src,name);
			dst->body=copy_string(name);
			if (flag) {
				tmp=strstr(dst->name," -> ");
				if (tmp) *tmp=0;
			};
		} else dst->body=NULL;
	}else{
// dos style listing
// we don't get date and time because they are not Y2K
		char *new_src=extract_string(src,str1);
		new_src=extract_string(new_src,str1);
		dst->date=time(NULL);
		new_src=extract_string(new_src,str1);

		if (is_string(str1)){
			dst->type=T_DIR;
			dst->perm=0775;
		}else{
			sscanf(str1,"%u",&(dst->size));
			dst->type=T_FILE;
			dst->perm=0664;
		};
		
		if (flag && new_src && *new_src) {
			while (*new_src==' ') new_src+=1;
			dst->name=copy_string(new_src);
			del_crlf(dst->name);
			dst->body=NULL;
		};
	};

	/* Next cycle extract name from string of
	 * directory listing
	 */
	delete str1;
	delete name;
};
//****************************************/

int tFtpDownload::change_dir() {
	int rvalue=0;
	if (CWDFlag) return 0;
	if (!equal(USER,DEFAULT_USER)){
		if ((rvalue=FTP->change_dir("/"))) return(rvalue);
	};
	if ((rvalue=FTP->change_dir(D_PATH))) return(rvalue);
	CWDFlag=1;
	return RVALUE_OK;
};

void tFtpDownload::print_reget(int offset) {
	if (!FTP->test_reget()) {
		LOG->add(_("Reget is not supported!!!"),LOG_WARNING);
	};
};

void tFtpDownload::check_for_repeated(tStringList *LIST) {
	if (!LIST) return;
	tString *temp;
	temp=LIST->last();
	while(temp) {
		tString *temp2=(tString *)(temp->next);
		while (temp2) {
			if (equal(temp->body,temp2->body)) {
				tString *prev=(tString *)(temp2->prev);
				LIST->del(temp2);
				delete(temp2);
				temp2=(tString *)(prev->next);
			} else
				temp2=(tString *)(temp2->next);
		};
		temp=(tString *)(temp->next);
	};
};
//****************************************/
tFtpDownload::tFtpDownload() {
	LOG=NULL;
	D_FILE.name=HOST=USER=PASS=D_PATH=NULL;
	D_FILE.perm=get_permisions_from_int(CFG.DEFAULT_PERMISIONS);
	StartSize=D_FILE.size=D_FILE.type=D_FILE.fdesc=0;
	Status=D_NOTHING;

	D_FILE.body=NULL;
	FTP=NULL;
	DIR=list=NULL;
	RetrNum=0;
};

int tFtpDownload::reconnect() {
	int success=1;
	Status=D_QUERYING;
	while (success) {
		RetrNum++;
		if (FTP->get_status()!=STATUS_TIMEOUT && FTP->get_status()!=0) {
			LOG->add(_("Server refused login probably too many users of your class"),LOG_WARNING);
		};
		if (config.number_of_attempts)
			LOG->myprintf(LOG_OK,_("Retrying %i of %i...."),RetrNum,config.number_of_attempts);
		else
			LOG->myprintf(LOG_OK,_("Retrying %i ..."),RetrNum);
		if (RetrNum==config.number_of_attempts) {
			LOG->add(_("Max amount of retries was reached!"),LOG_ERROR);
			return -1;
		};
		FTP->down();
		if (RetrNum>1) {
			if (FTP->test_reget() || config.retry) {
				LOG->add(_("Sleeping"),LOG_OK);
				sleep(config.time_for_sleep+1);
			}
			else return -1;
		};
		if (FTP->reinit()==0 && FTP->connect()==0) {
			success=0;
		};
	};
	CWDFlag=0;
	return 0;
};

int tFtpDownload::init(tAddr *hostinfo,tLog *log,tCfg *cfg) {
	LOG=log;
	FTP=new tFtpClient;
	RetrNum=0;
	HOST=hostinfo->host;
	if (hostinfo->username)
		USER=copy_string(hostinfo->username);
	else
		USER=copy_string(DEFAULT_USER);
	if (hostinfo->pass)
		PASS=copy_string(hostinfo->pass);
	else
		PASS=copy_string(DEFAULT_PASS);
	D_PORT=hostinfo->port;
	DIR=NULL;
	list=NULL;
	D_FILE.fdesc=0;
	MASK=hostinfo->mask;

	config.copy_ints(cfg);
	config.set_proxy_host(cfg->get_proxy_host());

	if (config.get_proxy_host() && config.proxy_port) {
		FTP->init(config.get_proxy_host(),LOG,config.proxy_port,config.timeout);
		char port[MAX_LEN];
		port[0]=0;
		if (D_PORT!=21) sprintf(port,":%i",D_PORT);
		char *temp=new char[strlen(USER)+strlen(HOST)+strlen("@")+strlen(port)+1];
		sprintf(temp,"%s@%s%s",USER,HOST,port);
		delete USER;
		USER=temp;
	} else
		FTP->init(HOST,LOG,D_PORT,config.timeout);
	FTP->registr(USER,PASS);
	FTP->set_passive(config.passive);
	return reconnect();
};

tStringList *tFtpDownload::dir() {
	return DIR;
};

int tFtpDownload::get_size() {
	if (!list) {
		list=new tStringList;
		list->init(0);
	} else list->done();
	int needcheck=0;
	while (1) {
		if (!change_dir()) {
			if (!FTP->stand_data_connection()) {
				int a;
				if (MASK)
					a=FTP->get_size(NULL,list);
				else
					a=FTP->get_size(D_FILE.name,list);
				if (a==0 && list->count()<=2) {
					tString *last=list->last();
					if (!last) {
						D_FILE.type=T_FILE;
						D_FILE.size=0;
						D_FILE.body=NULL;
						D_FILE.perm=S_IRUSR|S_IWUSR;
						return 0;
					};
					if (equal_first(last->body,"total")) {
						D_FILE.type=T_DIR;
						D_FILE.size=1;
						DIR=list;
						list=NULL;
						D_FILE.body=NULL;
						D_FILE.perm=S_IRUSR|S_IWUSR|S_IXUSR;
						return 0;
					};
					tFileInfo temp;
					cut_string_list(last->body,&temp,0);
					D_FILE.type=temp.type;
					D_FILE.body=temp.body;
					D_FILE.size=temp.size;
					D_FILE.perm=temp.perm;
					D_FILE.date=temp.date;
					LOG->myprintf(LOG_OK,_("Length is %i"),D_FILE.size);
					return D_FILE.size;
				};
				if (a==0 && list->count()>2) {
					LOG->add(_("This is a directory!"),LOG_WARNING);
					D_FILE.size=1;
					if (DIR) delete DIR;
					DIR=list;
					list=NULL;
					D_FILE.type=T_DIR;
					D_FILE.perm=S_IRUSR|S_IWUSR;
					if (needcheck) check_for_repeated(DIR);
					return 1;
				};
				needcheck=1;
				LOG->add(_("Couldn't get size :(("),LOG_WARNING);
			} else {
				LOG->add(_("Can't establish data connection"),LOG_ERROR);
			};
		} else {
			LOG->add(_("Can't change dir!"),LOG_ERROR);
			int s=FTP->get_status();
			if (s!=STATUS_TIMEOUT && (s!=STATUS_CMD_ERR || s!=STATUS_UNSPEC_ERR))
				break;
		};
		if (reconnect())
			break;
	};
	return -1;
};

int tFtpDownload::download_dir() {
	unsigned int offset=0;
	int ind=0,needcheck=0;
	if (!DIR) {
		DIR=new tStringList;
		DIR->init(0);

		while(1) {
			if (!change_dir()) {
				if (!FTP->stand_data_connection()) {
					print_reget(offset);
					if (!FTP->test_reget()) {
						offset=0;
						DIR->done();
					};
					Status=D_DOWNLOAD;
					if (MASK)
						ind=FTP->get_size(NULL,DIR);
					else
						ind=FTP->get_size(D_FILE.name,DIR);
					if (ind==0) {
						LOG->add(_("Listing was loaded"),LOG_OK);
						if (needcheck) check_for_repeated(DIR);
						return 0;
					};
					LOG->add(_("Listing not loaded completelly"),LOG_WARNING);
					needcheck=1;
				} else {
					LOG->add(_("Can't establish data connection"),LOG_ERROR);
				};
			} else {
				LOG->add(_("Can't change directory"),LOG_ERROR);
				int s=FTP->get_status();
				if (s!=STATUS_TIMEOUT && (s!=STATUS_CMD_ERR || s!=STATUS_UNSPEC_ERR))
					return -1;
			};
			if (reconnect()) {
				return -1;
			};
		};
	};
	return 0;
};

int tFtpDownload::download(unsigned int from,unsigned int len) {
	unsigned int length=0,offset=from;
	int ind=0;
	if (D_FILE.type==T_DIR) {
		LOG->add(_("Loading directory..."),LOG_OK);
		return(download_dir());
	};
	if (D_FILE.type==T_LINK) {
		LOG->add(_("Link was loaded :))"),LOG_OK);
		return 0;
	};
	while(1) {
		if (!change_dir()) {
			if (!FTP->stand_data_connection()) {
				int real_offset=rollback(offset);
				print_reget(real_offset);
//				if (config.rollback)
					StartSize=offset=real_offset;
				if (!FTP->test_reget())
					StartSize=offset=0;
				Status=D_DOWNLOAD;
				ind=FTP->get_file_from(D_FILE.name,offset,D_FILE.fdesc);
				if (ind>0) {
					length+=ind;
					offset+=ind;
					LOG->myprintf(LOG_OK,_("%i bytes loaded."),length);
				};
				if (!FTP->get_status()) {
					if (config.permisions) fchmod(D_FILE.fdesc,D_FILE.perm);
					return 0;
				};
			} else {
				LOG->add(_("Can't establish data connection"),LOG_ERROR);
			};
		} else {
			LOG->add(_("Can't change directory"),LOG_ERROR);
			int s=FTP->get_status();
			if (s!=STATUS_TIMEOUT && (s!=STATUS_CMD_ERR || s!=STATUS_UNSPEC_ERR))
				return -1;
		};
		if (reconnect()) {
			return -1;
		};
	};
	return 0;
};

void tFtpDownload::set_passive(int a) {
	if (FTP) {
		FTP->set_passive(a);
	};
};

int tFtpDownload::get_readed() {
	if (D_FILE.type==T_FILE) return (FTP->get_readed());
	if (list) return list->size();
	if (DIR) return DIR->size();
	return 0;
};

int tFtpDownload::another_way_get_size() {
	return FTP->another_way_get_size();
};

int tFtpDownload::get_child_status() {
	return(FTP->get_status());
};

int tFtpDownload::get_start_size() {
	if (FTP && !FTP->test_reget())
		return 0;
	return(StartSize);
};

int tFtpDownload::reget() {
	return FTP->test_reget();
};

void tFtpDownload::done() {
	if (FTP) FTP->done();
	if (D_FILE.fdesc) {
		close(D_FILE.fdesc);
		D_FILE.fdesc=0;
	};
};

tFtpDownload::~tFtpDownload() {
	if (FTP) delete(FTP);
	if (D_FILE.fdesc) close(D_FILE.fdesc);
	if (USER) delete(USER);
	if (PASS) delete(PASS);
	if (D_PATH) delete(D_PATH);
	if (D_FILE.name) delete(D_FILE.name);
	if (D_FILE.body) delete D_FILE.body;
	if (DIR) delete(DIR);
	if (list) delete(list);
};
