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
#include "ntlocale.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>

//****************************************/
void ftp_extract_link(char *src,char *dst) {
	char *delim=" -> ";
	if (src) {
		char *tmp=strstr(src,delim);
		if (tmp) {
			strcpy(dst,tmp+strlen(delim));
		} else dst[0]=0;
	};
};

int ftp_type_from_str(char *data) {
	if (*data=='d')
		return T_DIR;
	if (*data=='l')
		return T_LINK;
	if (*data=='c' || *data=='b')
		return T_DEVICE;
	return T_FILE;
};

int ftp_permisions_from_str(char *a) {
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

int ftp_date_from_str(char *src) {
	char *data=new char[strlen(src)+1];
	char *tmp;
	int month,day;

	tmp=extract_string(src,data,5);
	if (is_string(data)){
		tmp=extract_string(tmp,data);
	};

	time_t NOW=time(NULL);
	struct tm *date=new tm;
	localtime_r(&NOW,date);
	date->tm_isdst=-1;
	month=date->tm_mon;
	day=date->tm_mday;
	if (tmp && *tmp) {
		tmp=extract_string(tmp,data);
		date->tm_mon=convert_month(data);
	};
	if (tmp && *tmp) {
		tmp=extract_string(tmp,data);
		sscanf(data,"%i",&(date->tm_mday));
	};
	if (tmp && *tmp) {
		extract_string(tmp,data);
		if (index(data,':')) {
			sscanf(data,"%i:%i",&(date->tm_hour),&(date->tm_min));
			if (month<date->tm_mon ||
			    (month==date->tm_mon && day<date->tm_mday))
				date->tm_year-=1;
		} else {
			sscanf(data,"%i",&(date->tm_year));
			date->tm_year-=1900;
		};
	};
	NOW=mktime(date);
	delete date;
	delete data;
	return NOW;
};

/* parsing string of LIST -la command
   char *src - string for parsing
   tFileInfo *dst - where put result
   int flag - need put strings (name and name of link) to result or not
 */
void ftp_cut_string_list(char *src,tFileInfo *dst,int flag) {
	if (src==NULL || dst==NULL) return;
	del_crlf(src);
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
		dst->type=ftp_type_from_str(src);
		if (dst->type!=T_DEVICE) {
			dst->perm=ftp_permisions_from_str(src);
			dst->date=ftp_date_from_str(src);
		};
		if (flag) {
			if (tmp) dst->name.set(tmp);
			else dst->name.set(name);
		};
		if (dst->type==T_LINK) {
			ftp_extract_link(src,name);
			dst->body.set(name);
			if (flag) {
				tmp=strstr(dst->name.get()," -> ");
				if (tmp) *tmp=0;
			};
		}else dst->body.set(NULL);
	}else{
// dos style listing
// we don't get date and time because they are not Y2K compilance
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
			dst->name.set(new_src);
			dst->body.set(NULL);
		};
	};

	/* Next cycle extract name from string of
	 * directory listing
	 */
	delete str1;
	delete name;
};
//****************************************/
void tFtpDownload::print_error(int error_code){
	switch(error_code){
	case ERROR_DATA_CONNECT:
		LOG->log(LOG_ERROR,_("Can't establish data connection"));
		break;
	case ERROR_CWD:
		LOG->log(LOG_ERROR,_("Can't change directory"));
		break;
	case ERROR_TOO_MANY_USERS:
		LOG->log(LOG_WARNING,_("Server refused login probably too many users of your class"));
		break;
	default:
		tDownloader::print_error(error_code);
		break;
	};
};

int tFtpDownload::change_dir() {
	int rvalue=0;
	if (CWDFlag) return 0;
	if (!equal(ADDR.username.get(),DEFAULT_USER)){
		if ((rvalue=FTP->change_dir("/"))){
			print_error(ERROR_CWD);
			return(rvalue);
		};
	};
	if ((rvalue=FTP->change_dir(ADDR.path.get()))){
		print_error(ERROR_CWD);
		return(rvalue);
	};
	CWDFlag=1;
	return RVALUE_OK;
};

//****************************************/
tFtpDownload::tFtpDownload() {
	LOG=NULL;
	D_FILE.perm=get_permisions_from_int(CFG.DEFAULT_PERMISIONS);
	StartSize=D_FILE.size=D_FILE.type=0;
	Status=D_NOTHING;

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
			print_error(ERROR_TOO_MANY_USERS);
		};
		if (config.number_of_attempts &&
		    RetrNum>=config.number_of_attempts+1) {
			print_error(ERROR_ATTEMPT_LIMIT);
			return -1;
		};
		print_error(ERROR_ATTEMPT);
		FTP->down();
		if (RetrNum>1) {
			if (FTP->test_reget() || config.retry) {
				LOG->log(LOG_OK,_("Sleeping"));
				sleep(config.time_for_sleep+1);
			}
			else return -1;
		};
		if (FTP->reinit()==0){
			success=0;
			if (config.proxy_user.get() && config.proxy_pass.get()){
				FTP->registr(config.proxy_user.get(), config.proxy_pass.get());
				success=FTP->connect();
			};
			if (success==0){
				FTP->registr(ADDR.username.get(),ADDR.pass.get());
				success=FTP->connect();
			};
		};
	};
	CWDFlag=0;
	return 0;
};

int tFtpDownload::init(tAddr *hostinfo,tWriterLoger *log,tCfg *cfg) {
	LOG=log;
	FTP=new tFtpClient;
	RetrNum=0;
	ADDR.copy(hostinfo);
	if (ADDR.username.get()==NULL)
		ADDR.username.set(DEFAULT_USER);
	if (ADDR.pass.get()==NULL)
		ADDR.pass.set(DEFAULT_PASS);
	DIR=NULL;
	list=NULL;

	config.copy_ints(cfg);
	config.proxy_host.set(cfg->proxy_host.get());
	config.proxy_user.set(cfg->proxy_user.get());
	config.proxy_pass.set(cfg->proxy_pass.get());

	if (config.proxy_host.get() && config.proxy_port) {
		FTP->init(config.proxy_host.get(),LOG,config.proxy_port,config.timeout);
		char port[MAX_LEN];
		port[0]=0;
		if (ADDR.port!=get_port_by_proto(D_PROTO_FTP))
			sprintf(port,":%i",ADDR.port);
		char *temp=sum_strings(ADDR.username.get(),"@",
				       ADDR.host.get(),port,NULL);
		ADDR.username.set(temp);
		delete(temp);
	} else
		FTP->init(ADDR.host.get(),LOG,ADDR.port,config.timeout);
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
	while (1) {
		if (!change_dir()) {
			if (!FTP->stand_data_connection()) {
				int a=0;
				if (ADDR.mask)
					a=FTP->get_size(NULL,list);
				else
					a=FTP->get_size(ADDR.file.get(),list);
				if (a==0 && list->count()<=2) {
					tString *last=list->last();
					if (!last) {
						D_FILE.type=T_FILE;
						D_FILE.size=0;
						D_FILE.perm=S_IRUSR|S_IWUSR;
						return 0;
					};
					if (equal_first(last->body,"total")) {
						D_FILE.type=T_DIR;
						D_FILE.size=1;
						if (DIR) delete (DIR);
						DIR=list;
						list=NULL;
						D_FILE.perm=S_IRUSR|S_IWUSR|S_IXUSR;
						return 0;
					};
					ftp_cut_string_list(last->body,&D_FILE,0);
					LOG->log_printf(LOG_OK,_("Length is %i"),D_FILE.size);
					return D_FILE.size;
				};
				if (a==0 && list->count()>2) {
					LOG->log(LOG_WARNING,_("This is a directory!"));
					D_FILE.size=1;
					if (DIR) delete DIR;
					DIR=list;
					list=NULL;
					D_FILE.type=T_DIR;
					D_FILE.perm=S_IRUSR|S_IWUSR;
					return 1;
				};
				LOG->log(LOG_WARNING,_("Couldn't get size :(("));
			} else {
				print_error(ERROR_DATA_CONNECT);
			};
/*
		} else {
			int s=FTP->get_status();
			if (s!=STATUS_TIMEOUT)
				break;
*/
		};
		if (reconnect())
			break;
	};
	return -1;
};

int tFtpDownload::download_dir() {
	int ind=0;
	LOG->log(LOG_OK,_("Loading directory..."));
	if (!DIR) {
		DIR=new tStringList;
		DIR->init(0);

		while(1) {
			if (!change_dir()) {
				if (!FTP->stand_data_connection()) {
					if (!FTP->test_reget()) {
						DIR->done();
					};
					Status=D_DOWNLOAD;
					if (ADDR.mask)
						ind=FTP->get_size(NULL,DIR);
					else
						ind=FTP->get_size(ADDR.file.get(),DIR);
					if (ind==0) {
						LOG->log(LOG_OK,_("Listing was loaded"));
						return 0;
					};
					LOG->log(LOG_WARNING,_("Listing not loaded completelly"));
				} else {
					print_error(ERROR_DATA_CONNECT);
				};
/*
			} else {
				int s=FTP->get_status();
				if (s!=STATUS_TIMEOUT && (s!=STATUS_CMD_ERR || s!=STATUS_UNSPEC_ERR))
					return -1;
*/
			};
			if (reconnect()) {
				return -1;
			};
		};
	};
	return 0;
};

int tFtpDownload::download(int len) {
	int rvalue=0;
	switch (D_FILE.type){
	case T_DIR: {
		rvalue=download_dir();
		break;
	};
	case T_LINK: {
		LOG->log(LOG_OK,_("Link was loaded :))"));
		rvalue=0;
		break;
	};
	default:{
		int length_to_load=len>0?LOADED+len:0;
		int ind=0;
		while(1) {
			if (!change_dir()) {
				if (!FTP->stand_data_connection()) {
					StartSize=rollback();
					Status=D_DOWNLOAD;
					int to_load=len>0?length_to_load-LOADED:0;
					ind=FTP->get_file_from(ADDR.file.get(),LOADED,to_load);
					if (!FTP->test_reget())
						StartSize=LOADED=0;
					if (ind>0) {
						LOADED+=ind;
						LOG->log_printf(LOG_OK,_("%i bytes loaded."),ind);
					};
					if (!FTP->get_status()) {
						rvalue=0;
						break;
					};
				} else {
					print_error(ERROR_DATA_CONNECT);
				};
/*			} else {
				int s=FTP->get_status();
				if (s!=STATUS_TIMEOUT && (s!=STATUS_CMD_ERR || s!=STATUS_UNSPEC_ERR)){
					rvalue=-1;
					break;
				};
*/
			};
			if (reconnect()) {
				rvalue=-1;
				break;
			};
		};
	};
	};
	return rvalue;
};

int tFtpDownload::get_readed() {
	if (D_FILE.type==T_FILE) return (FTP->get_readed());
	if (DIR) return DIR->size();
	if (list) return list->size();
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
};

tFtpDownload::~tFtpDownload() {
	if (FTP) delete(FTP);
	if (DIR) delete(DIR);
	if (list) delete(list);
};
