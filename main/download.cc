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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <utime.h>
#include "var.h"
#include "download.h"
#include "locstr.h"
#include "ntlocale.h"

/* ---------------------------------------- */

tCfg::tCfg() {
	speed=0;
	proxy_type=0;
	link_as_file=leave_server=0;
	dont_leave_dir=0;
	restart_from_begin=0;
};

void tCfg::copy_ints(tCfg *src){
	http_recursing=src->http_recursing;
	speed=src->speed;
	ftp_recurse_depth=src->ftp_recurse_depth;
	http_recurse_depth=src->http_recurse_depth;
	timeout = src->timeout;
	time_for_sleep = src->time_for_sleep;
	number_of_attempts = src->number_of_attempts;

	passive = src->passive;
	permisions = src->permisions;
	get_date = src->get_date;
	retry = src->retry;
	full_server_loading=src->full_server_loading;
	dont_leave_dir=src->dont_leave_dir;

	proxy_type = src->proxy_type;
	proxy_port = src->proxy_port;
	rollback = src->rollback;
	link_as_file = src->link_as_file;
	leave_server = src->leave_server;
};

int tCfg::get_flags(){
	int rvalue=0;
	if (full_server_loading) rvalue|=1;
	if (retry) rvalue|=2;
	if (get_date) rvalue|=4;
	if (passive) rvalue|=8;
	if (permisions) rvalue|=16;
	return rvalue;
};

void tCfg::set_flags(int what){
	full_server_loading = ((what & 1)!=0);
	retry = ((what & 2) != 0);
	get_date = ((what & 4)!=0);
	passive = ((what & 8)!=0);
	permisions = ((what & 16)!=0);
};

void tCfg::copy(tCfg *src) {
	copy_ints(src);
	proxy_host.set(src->proxy_host.get());
	proxy_user.set(src->proxy_user.get());
	proxy_pass.set(src->proxy_pass.get());
	user_agent.set(src->user_agent.get());
};

void tCfg::reset_proxy() {
	proxy_user.set(NULL);
	proxy_pass.set(NULL);
	proxy_host.set(NULL);
};

void tCfg::save_to_config(int fd){
	f_wstr_lf(fd,"Cfg:");
	if (proxy_host.get()){
		write_named_string(fd,"Proxy:",proxy_host.get());
		write_named_integer(fd,"proxy_port:",proxy_port);
		write_named_integer(fd,"proxy_type:",proxy_type);
		if(proxy_pass.get()!=NULL && proxy_user.get()!=NULL){
			write_named_string(fd,"Proxy_pass:",proxy_pass.get());
			write_named_string(fd,"Proxy_user:",proxy_user.get());
		};
	};
	if (user_agent.get() && *(user_agent.get()))
		write_named_string(fd,"User_agent:",user_agent.get());
		
	write_named_integer(fd,"Timeout:",timeout);
	write_named_integer(fd,"time_for_sleep:",time_for_sleep);
	write_named_integer(fd,"number_of_attempts:",number_of_attempts);
	write_named_integer(fd,"ftp_recurse_depth:",ftp_recurse_depth);
	write_named_integer(fd,"http_recurse_depth:",http_recurse_depth);
	write_named_integer(fd,"rollback:",rollback);
	write_named_integer(fd,"speed:",speed);
	write_named_integer(fd,"passive:",passive);
	write_named_integer(fd,"retry:",retry);
	write_named_integer(fd,"permisions:",permisions);
	write_named_integer(fd,"get_date:",get_date);
	write_named_integer(fd,"http_recursing:",http_recursing);
	write_named_integer(fd,"link_as_file:",link_as_file);
	write_named_integer(fd,"leave_server:",leave_server);
	write_named_integer(fd,"dont_leave_dir:",dont_leave_dir);
	if (save_name.get() && *(save_name.get()))
		write_named_string(fd,"save_name:",save_name.get());
	if (save_path.get() && *(save_path.get()))
		write_named_string(fd,"save_path:",save_path.get());
	f_wstr_lf(fd,"EndCfg:");
};

int tCfg::load_from_config(int fd){
	char *table_of_fields[]={
		"User_agent:", //0
		"Proxy:",//1
		"proxy_pass:",//2
		"proxy_user:",//3
		"proxy_port:",//4
		"proxy_type:",//5
		"timeout:",//6
		"time_for_sleep:",//7
		"number_of_attempts:",//8
		"ftp_recurse_depth:",//9
		"http_recurse_depth:",//10
		"rollback:",//11
		"speed:",//12
		"passive:",//13
		"retry:",//14
		"permisions:", //15
		"get_date:", //16
		"http_recursing:",//17
		"link_as_file:",//18
		"leave_server:",//19
		"save_name:",//20
		"save_path:",//21
		"dont_leave_dir:",//22
		"EndCfg:" //23
	};
	char buf[MAX_LEN];
	while(f_rstr(fd,buf,MAX_LEN)>0){
		unsigned int i;
		for (i=0;i<sizeof(table_of_fields)/sizeof(char *);i++){
			if (equal_uncase(buf,table_of_fields[i])) break;
		};
		switch(i){
		case 0:{
			if (f_rstr(fd,buf,MAX_LEN)<0) return -1;
			user_agent.set(buf);
			break;
		};
		case 1:{
			if (f_rstr(fd,buf,MAX_LEN)<0) return -1;
			proxy_host.set(buf);
			break;
		};
		case 2:{
			if (f_rstr(fd,buf,MAX_LEN)<0) return -1;
			proxy_pass.set(buf);
			break;
		};
		case 3:{
			if (f_rstr(fd,buf,MAX_LEN)<0) return -1;
			proxy_user.set(buf);
			break;
		};
		case 4:{
			if (f_rstr(fd,buf,MAX_LEN)<0) return -1;
			sscanf(buf,"%d",&proxy_port);
			break;
		};
		case 5:{
			if (f_rstr(fd,buf,MAX_LEN)<0) return -1;
			sscanf(buf,"%d",&proxy_type);
			break;
		};
		case 6:{
			if (f_rstr(fd,buf,MAX_LEN)<0) return -1;
			sscanf(buf,"%d",&timeout);
			break;
		};
		case 7:{
			if (f_rstr(fd,buf,MAX_LEN)<0) return -1;
			sscanf(buf,"%d",&time_for_sleep);
			break;
		};
		case 8:{
			if (f_rstr(fd,buf,MAX_LEN)<0) return -1;
			sscanf(buf,"%d",&number_of_attempts);
			break;
		};
		case 9:{
			if (f_rstr(fd,buf,MAX_LEN)<0) return -1;
			sscanf(buf,"%d",&ftp_recurse_depth);
			break;
		};		
		case 10:{
			if (f_rstr(fd,buf,MAX_LEN)<0) return -1;
			sscanf(buf,"%d",&http_recurse_depth);
			break;
		};		
		case 11:{
			if (f_rstr(fd,buf,MAX_LEN)<0) return -1;
			sscanf(buf,"%d",&rollback);
			break;
		};		
		case 12:{
			if (f_rstr(fd,buf,MAX_LEN)<0) return -1;
			sscanf(buf,"%d",&speed);
			break;
		};		
		case 13:{
			if (f_rstr(fd,buf,MAX_LEN)<0) return -1;
			sscanf(buf,"%d",&passive);
			break;
		};		
		case 14:{
			if (f_rstr(fd,buf,MAX_LEN)<0) return -1;
			sscanf(buf,"%d",&retry);
			break;
		};		
		case 15:{
			if (f_rstr(fd,buf,MAX_LEN)<0) return -1;
			sscanf(buf,"%d",&permisions);
			break;
		};		
		case 16:{
			if (f_rstr(fd,buf,MAX_LEN)<0) return -1;
			sscanf(buf,"%d",&get_date);
			break;
		};		
		case 17:{
			if (f_rstr(fd,buf,MAX_LEN)<0) return -1;
			sscanf(buf,"%d",&http_recursing);
			break;
		};		
		case 18:{
			if (f_rstr(fd,buf,MAX_LEN)<0) return -1;
			sscanf(buf,"%d",&link_as_file);
			break;
		};		
		case 19:{
			if (f_rstr(fd,buf,MAX_LEN)<0) return -1;
			sscanf(buf,"%d",&leave_server);
			break;
		};		
		case 20:{
			if (f_rstr(fd,buf,MAX_LEN)<0) return -1;
			save_name.set(buf);
			break;
		};
		case 21:{
			if (f_rstr(fd,buf,MAX_LEN)<0) return -1;
			save_path.set(buf);
			break;
		};
		case 22:{
			if (f_rstr(fd,buf,MAX_LEN)<0) return -1;
			sscanf(buf,"%d",&dont_leave_dir);
			break;
		};		
		case 23:{
			return 0;
		};
		};
	};
	return -1;
};

tCfg::~tCfg() {
};

/* End config functions.
   Begin tDownloader's functions.
 */

void tDownloader::print_error(int error_code){
	switch(error_code){
	case ERROR_ATTEMPT_LIMIT:{
		LOG->log(LOG_ERROR,_("Max amount of retries was reached!"));
		break;
	};
	case ERROR_ATTEMPT:{
		if (config.number_of_attempts)
			LOG->log_printf(LOG_OK,_("Retrying %i of %i...."),RetrNum,config.number_of_attempts);
		else
			LOG->log_printf(LOG_OK,_("Retrying %i ..."),RetrNum);
		break;
	};
	case ERROR_DIRECTORY:{
		LOG->log(LOG_ERROR,_("Directory already created!:))"));
		break;
	};
	case ERROR_ACCESS:{
		LOG->log_printf(LOG_ERROR,
			      _("You have no permissions to create file at path %s"),
			      config.save_path.get());
		break;
	};
	case ERROR_NO_SPACE:{
		LOG->log_printf(LOG_ERROR,
			      _("You have no space at path %s for creating file"),
			      config.save_path.get());
		break;
	};
	default:{
		LOG->log(LOG_ERROR,_("Warning! Probably you found the BUG!!!"));
		LOG->log(LOG_ERROR,_("If you see this message please report to mdem@chat.ru"));
		break;
	};
	};
};

tDownloader::tDownloader(){
	LOG=NULL;
	HOST=USER=PASS=D_PATH=NULL;
	D_FILE.perm=get_permisions_from_int(CFG.DEFAULT_PERMISIONS);
	StartSize=D_FILE.size=D_FILE.type=0;
	Status=D_NOTHING;
};

char * tDownloader::get_new_url() {
	return NULL;
};

void tDownloader::set_file_info(tFileInfo *what) {
	D_FILE.type=what->type;
	if (D_FILE.type==T_LINK)
		D_FILE.body.set(what->body.get());
	D_FILE.perm=what->perm;
	D_FILE.date=what->date;
};

tFileInfo *tDownloader::get_file_info() {
	return(&D_FILE);
};

int tDownloader::rollback(){
	LOADED = LOADED<config.rollback ? 0 : LOADED-config.rollback;
	LOG->shift(LOADED);
	return(LOADED);
};

void tDownloader::init_download(char *path,char *file) {
	D_FILE.name.set(file);
	if (D_PATH) delete(D_PATH);
	D_PATH=copy_string(path);
};

void tDownloader::set_loaded(int a) {
	LOADED=a;
};

int tDownloader::treat() {
	return RetrNum;
};

int tDownloader::another_way_get_size() {
	return 0;
};

int tDownloader::get_status() {
	return(Status);
};

int tDownloader::get_start_size() {
	return(StartSize);
};

void tDownloader::make_full_pathes(const char *path,char **name,char **guess) {
	char *temp;
	temp=sum_strings(".",D_FILE.name.get(),NULL);
	*name=compose_path(path,temp);
	*guess=compose_path(path,D_FILE.name.get());
	delete temp;
};

void tDownloader::make_full_pathes(const char *path,char *another_name,char **name,char **guess) {
	char *temp=sum_strings(".",another_name,NULL);
	*name=compose_path(path,temp);
	*guess=compose_path(path,another_name);
	delete temp;
};

tDownloader::~tDownloader() {
	// do nothing
};
