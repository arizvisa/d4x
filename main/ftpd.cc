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
#include <strings.h>
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

time_t ftp_date_from_dos(char *src){
	struct tm *date=new tm;
	time_t NOW=time(NULL);
	localtime_r(&NOW,date);
	date->tm_sec=0;
	date->tm_isdst=-1;
	char *tmp=src;
	/* day-month-year min:hour[aa]*/
	sscanf_int(tmp,&(date->tm_mday));
	tmp+=3;
	sscanf_int(tmp,&(date->tm_mon));
	tmp+=3;
	int year=date->tm_year;
	sscanf_int(tmp,&(year));
	if (year+100<=date->tm_year)
		date->tm_year=year+100;
	else
		date->tm_year=year;
	if (date->tm_mon>12){
		year=date->tm_mon;
		date->tm_mon=date->tm_mday;
		date->tm_mday=year;
	};
	date->tm_mon-=1;
	/* parsing minutes and hours */
	tmp=index(src,' ');
	if (tmp){
		while (*tmp && *tmp==' ') tmp+=1;
		sscanf_int(tmp,&(date->tm_hour));
		tmp+=3;
		sscanf_int(tmp,&(date->tm_min));
		tmp+=2;
		if (*tmp=='p' || *tmp=='P')
			date->tm_hour+=12;
	};
	NOW=mktime(date);
	delete(date);
	return(NOW);
};

time_t ftp_date_from_str(char *src) {
	char *data=new char[strlen(src)+1];
	char *tmp;
	int month,day;

	tmp=extract_string(src,data,5);
	if (is_string(data)){
		tmp=extract_string(tmp,data);
		if (!is_string(data)){
			tmp=extract_string(src,data,4);
		};
	};

	time_t NOW=time(NULL);
	struct tm *date=new tm;
	localtime_r(&NOW,date);
	date->tm_sec=0;
	date->tm_min=0;
	date->tm_hour=0;
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
			/* very ugly way to skip first zero */
			char *tmpdata=data;
			sscanf_int(tmpdata,&(date->tm_hour));
			tmpdata+=3;
			sscanf_int(tmpdata,&(date->tm_min));
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
	delete[] data;
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
		extract_string(src,name,1);
		char *rsrc;
		if (is_string(name))
			rsrc=src;
		else{
			rsrc=skip_strings(src,2);
			extract_string(rsrc,str1,5);
		};
		char *tmp;
		if (!is_string(str1)){
			tmp=skip_strings(rsrc,8);
			sscanf(rsrc,"%s %u %s %s %lli %s %u %s %s",
	       		str1,&par1,str1,str1,&dst->size,str1,&par1,str1,name);
		}else{
			tmp=skip_strings(rsrc,7);
			sscanf(rsrc,"%s %u %s %lli %s %u %s %s",
	       		str1,&par1,str1,&dst->size,str1,&par1,str1,name);
		};
		dst->type=ftp_type_from_str(rsrc);
		if (dst->type!=T_DEVICE) {
			dst->perm=ftp_permisions_from_str(rsrc);
			dst->date=ftp_date_from_str(rsrc);
		};
		if (flag) {
			if (tmp) dst->name.set(tmp);
			else dst->name.set(name);
		};
		if (dst->type==T_LINK) {
			ftp_extract_link(rsrc,name);
			dst->body.set(name);
			if (flag) {
				tmp=strstr(dst->name.get()," -> ");
				if (tmp) *tmp=0;
			};
		}else dst->body.set(NULL);
	}else{
// dos style listing

		char *new_src=extract_string(src,str1);
		new_src=extract_string(new_src,str1);
		new_src=extract_string(new_src,str1);
		if (new_src && *new_src){
			dst->date=ftp_date_from_dos(src);
			if (strstr(str1,"DIR")){
				dst->type=T_DIR;
				dst->perm=0775;
			}else{
				sscanf(str1,"%lli",&(dst->size));
				dst->type=T_FILE;
				dst->perm=0664;
			};
			
			if (flag && new_src && *new_src) {
				while (*new_src==' ') new_src+=1;
				dst->name.set(new_src);
				dst->body.set(NULL);
			};
		};
	};
	delete[] str1;
	delete[] name;
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
		LOG->log(LOG_WARNING,_("Server refused login, perhaps there are too many users of your class"));
		break;
	default:
		tDownloader::print_error(error_code);
		break;
	};
};

int tFtpDownload::change_dir() {
	int rvalue=0;
	if (DONT_CWD || CWDFlag) return 0;
	/*
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
	*/
	if ((rvalue=FTP->change_dir(ADDR.path.get()))){
		print_error(ERROR_CWD);
		return(rvalue);
	};
		
	CWDFlag=1;
	return RVALUE_OK;
};

//****************************************/
tFtpDownload::tFtpDownload():tDownloader(){
	FTP=NULL;
	DIR=list=NULL;
	RetrNum=0;
	DONT_CWD=0;
	TMP_FILEPATH=NULL;
};

tFtpDownload::tFtpDownload(tWriterLoger *log):tDownloader(log){
	FTP=NULL;
	DIR=list=NULL;
	RetrNum=0;
	DONT_CWD=0;
	TMP_FILEPATH=NULL;
};

void tFtpDownload::dont_cwd(){
	DONT_CWD=1;
};

int tFtpDownload::reconnect() {
	int success=1;
	Status=D_QUERYING;
	while (success) {
		if (FTP->get_status()!=STATUS_TIMEOUT && FTP->get_status()!=0) {
			print_error(ERROR_TOO_MANY_USERS);
		};
		FTP->done();
		if (config.number_of_attempts &&
		    RetrNum>=config.number_of_attempts) {
			print_error(ERROR_ATTEMPT_LIMIT);
			return -1;
		};
		RetrNum++;
		print_error(ERROR_ATTEMPT);
		tDownloader::reconnect();
		if (RetrNum>1) {
			LOG->log(LOG_OK,_("Sleeping"));
			sleep(config.time_for_sleep+1);
		};
		if (FTP->reinit()==0){
			success=0;
			if (config.proxy.ftp_user.get() && config.proxy.ftp_pass.get()){
				FTP->registr(config.proxy.ftp_user.get(), config.proxy.ftp_pass.get());
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

void tFtpDownload::init_download(char *path,char *file) {
	ADDR.file.set(file);
	if (path && *path!='~' && *path!='/'){
		path=sum_strings("/",path,NULL);
		ADDR.path.set(path);
		delete[] path;
	}else
		ADDR.path.set(path);
};

int tFtpDownload::init(tAddr *hostinfo,tCfg *cfg,tSocket *s) {
	FTP=new tFtpClient(cfg);
	RetrNum=0;
	ADDR.copy(hostinfo);
	if (ADDR.username.get()==NULL)
		ADDR.username.set(DEFAULT_USER);
	if (ADDR.pass.get()==NULL)
		ADDR.pass.set(CFG.ANONYMOUS_PASS);
	DIR=NULL;
	list=NULL;

	config.copy_ints(cfg);
	if (cfg->split){
		config.retry=0;
		config.rollback=0;
	};
	config.copy_proxy(cfg);

	if (config.proxy.type==0 && config.proxy.ftp_host.get() && config.proxy.ftp_port) {
		FTP->init(config.proxy.ftp_host.get(),LOG,config.proxy.ftp_port,config.timeout);
		char port[MAX_LEN];
		port[0]=0;
		if (ADDR.port!=get_port_by_proto(D_PROTO_FTP))
			sprintf(port,":%i",ADDR.port);
		char *temp=sum_strings(ADDR.username.get(),"@",
				       ADDR.host.get(),port,NULL);
		ADDR.username.set(temp);
		delete[] temp;
	} else
		FTP->init(ADDR.host.get(),LOG,ADDR.port,config.timeout);
	FTP->set_passive(config.passive);
	FTP->set_retry(config.retry);
	FTP->set_dont_set_quit(config.dont_send_quit);
	if (s){
		FTP->import_ctrl_socket(s);
		CWDFlag=0;
		RetrNum=1;
		tDownloader::reconnect();
		return(0);
	};
	while(reconnect()==0){
		if (cfg->split==0 || FTP->force_reget()==0)
			return(0);
	};
	return(-1);
};

tStringList *tFtpDownload::dir() {
	return DIR;
};

int tFtpDownload::ftp_get_size_no_sdc(tStringList *l){
	if (ADDR.mask)
		return(FTP->get_size(NULL,l));
	if (DONT_CWD){
		if (TMP_FILEPATH) delete[] TMP_FILEPATH;
		TMP_FILEPATH=sum_strings("/",ADDR.path.get(),"/",
					 ADDR.file.get(),NULL);
		normalize_path(TMP_FILEPATH);
		return(FTP->get_size(TMP_FILEPATH,l));
	};
	return(FTP->get_size(ADDR.file.get(),l));
};

int tFtpDownload::ftp_get_size(tStringList *l){
	if (FTP->stand_data_connection()){
		print_error(ERROR_DATA_CONNECT);
		return(-1);
	};
	return(ftp_get_size_no_sdc(l));
};

int tFtpDownload::is_dir(){
	if (FTP->change_dir(ADDR.file.get())==0)
		return(1);
	return(0);
};

fsize_t tFtpDownload::ls_answer_short(){
	tString *last=list->last();
	if (!last) {
		LOG->log(LOG_WARNING,_("Empty answer. Trying to change directory to determine file type."));
		D_FILE.perm=S_IRUSR|S_IWUSR;
		/* try to CWD */
		if (is_dir()){
			D_FILE.type=T_DIR;
			char *a=compose_path(ADDR.path.get(),ADDR.file.get());
			ADDR.path.set(a);
			ADDR.file.set("");
			delete[] a;
		}else
			D_FILE.type=T_FILE;
		return 0;
	};
	D_FILE.type=T_FILE;
	D_FILE.size=0;
	if (equal_first(last->body,"total")) {
		D_FILE.type=T_DIR;
		D_FILE.size=1;
		if (DIR) delete (DIR);
		DIR=list;
		list=NULL;
		D_FILE.perm=S_IRUSR|S_IWUSR|S_IXUSR;
		return 0;
	};
	ftp_cut_string_list(last->body,&D_FILE,1);
	LOG->log_printf(LOG_OK,_("Length is %ll"),D_FILE.size);
	return D_FILE.size;
};

fsize_t tFtpDownload::ls_answer_long(){
	tString *last=list->last();
	while(last){
		if (strstr(last->body,ADDR.file.get()))
			break;
		last=(tString*)(last->next);
	};
	if (is_dir() || ADDR.mask){
		LOG->log(LOG_WARNING,_("This is a directory!"));
		D_FILE.size=1;
		if (DIR) delete DIR;
		DIR=list;
		list=NULL;
		D_FILE.type=T_DIR;
		D_FILE.perm=S_IRUSR|S_IWUSR;
		return 1;
	};
	if (last==NULL){
		LOG->log(LOG_WARNING,_("No such file or directory!"));
		return(-1);
	};
	ftp_cut_string_list(last->body,&D_FILE,0);
	LOG->log_printf(LOG_OK,_("Length is %ll"),D_FILE.size);
	D_FILE.type=T_FILE;
	return(D_FILE.size);
};

fsize_t tFtpDownload::get_size_only() {
	return(get_size());
};

fsize_t tFtpDownload::get_size() {
	if (!list) {
		list=new tStringList;
		list->init(0);
	} else list->done();
	while (1) {
		if (!change_dir()) {
			int a=ftp_get_size(list);
			if (a==0 && list->count()<=2){
				fsize_t sz=ls_answer_short();
				if (equal(D_FILE.name.get(),ADDR.file.get()) ||
				    FTP->METHOD_TO_LIST==1)
					return(sz);
				FTP->METHOD_TO_LIST=1;
				list->done();
				a=ftp_get_size(list);
				if (a==0 && list->count()<=2)
					return(ls_answer_short());
			};
			if (a==0 && list->count()>2) {
				return(ls_answer_long());
			};
			LOG->log(LOG_WARNING,_("Couldn't get size :(("));
		};
		if (FTP->get_status()==STATUS_CMD_ERR ||
		    FTP->get_status()==STATUS_UNSPEC_ERR){
			D_FILE.type=T_FILE;
			break; //an error occured
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
					ind=ftp_get_size_no_sdc(DIR);
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

int tFtpDownload::download(fsize_t len) {
	int rvalue=0;
#ifdef DEBUG_ALL
	LOG->log_printf(LOG_OK,"tFtpDownload::download(%ll)",len);
#endif //DEBUG ALL
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
		if (LOADED && remote_file_changed()){
			print_error(ERROR_FILE_UPDATED);
			if (config.retry==0) return(-1);
			LOADED=0;
			LOG->shift(0);
			LOG->truncate();
		};
		fsize_t length_to_load=len>0?LOADED+len:0;
		fsize_t ind=0;
		while(1) {
			if (!change_dir()) {
				if (!FTP->stand_data_connection()) {
					StartSize=rollback();
					Status=D_DOWNLOAD;
					fsize_t to_load=len>0?length_to_load-LOADED:0;
					if (DONT_CWD){
						if (TMP_FILEPATH) delete[] TMP_FILEPATH;
						TMP_FILEPATH=sum_strings("/",ADDR.path.get(),"/",
									 ADDR.file.get(),NULL);
						normalize_path(TMP_FILEPATH);
						ind=FTP->get_file_from(TMP_FILEPATH,LOADED,to_load);
					}else
						ind=FTP->get_file_from(ADDR.file.get(),LOADED,to_load);
#ifdef DEBUG_ALL
					LOG->log_printf(LOG_OK,"return to tFtpDownload::download with ind=%ll",ind);
#endif //DEBUG ALL
					if (!FTP->test_reget()){
						if (config.retry==0) break;
						StartSize=LOADED=0;
					};
					if (ind>0) {
						LOADED+=ind;
//						LOG->log_printf(LOG_OK,_("%ll bytes loaded."),ind);
						LOG->log_printf(LOG_OK,"%ll bytes loaded.(LOADED=%ll,len=%ll,FTP->get_status()=%i)",
								ind,LOADED,len,FTP->get_status());
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
#ifdef DEBUG_ALL
	LOG->log_printf(LOG_OK,"exit from tFtpDownload::download with %i",rvalue);
#endif //DEBUG ALL
	return rvalue;
};

fsize_t tFtpDownload::get_readed() {
	if (D_FILE.type==T_FILE) return (FTP->get_readed());
	if (DIR) return DIR->size();
	if (list) return list->size();
	return 0;
};

fsize_t tFtpDownload::another_way_get_size() {
	return FTP->another_way_get_size();
};

int tFtpDownload::get_child_status() {
	return(FTP->get_status());
};

fsize_t tFtpDownload::get_start_size() {
	if (FTP && !FTP->test_reget())
		return 0;
	return(StartSize);
};

char *tFtpDownload::get_new_url() {
	return(copy_string(D_FILE.body.get()));
};

int tFtpDownload::reget() {
	if (FTP) return FTP->test_reget();
	return(1);
};

void tFtpDownload::done() {
	if (FTP) FTP->done();
};

tSocket *tFtpDownload::export_ctrl_socket(){
	if (FTP) return(FTP->export_ctrl_socket());
	return(NULL);
};

tFtpDownload::~tFtpDownload() {
	if (FTP) delete(FTP);
	if (DIR) delete(DIR);
	if (list) delete(list);
	if (TMP_FILEPATH) delete[] TMP_FILEPATH;
};
