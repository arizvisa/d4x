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
#include "savedvar.h"


void tSimplyCfg::copy_ints(tSimplyCfg *src){
	DBC_RETURN_IF_FAIL(src!=NULL);
	http_recursing=src->http_recursing;
	change_links=src->change_links;
	speed=src->speed;
	ftp_recurse_depth=src->ftp_recurse_depth;
	http_recurse_depth=src->http_recurse_depth;
	timeout = src->timeout;
	time_for_sleep = src->time_for_sleep;
	number_of_attempts = src->number_of_attempts;

	passive = src->passive;
	dont_send_quit = src->dont_send_quit;
	permisions = src->permisions;
	get_date = src->get_date;
	retry = src->retry;
	full_server_loading=src->full_server_loading;
	dont_leave_dir=src->dont_leave_dir;

	rollback = src->rollback;
	link_as_file = src->link_as_file;
	leave_server = src->leave_server;
	sleep_before_complete = src->sleep_before_complete;
	
	check_time = src->check_time;
};

/* ---------------------------------------- */

tCfg::tCfg() {
	speed=0;
	proxy_no_cache=proxy_type=0;
	link_as_file=leave_server=0;
	dont_leave_dir=0;
	restart_from_begin=0;
	sleep_before_complete=0;
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

void tCfg::copy_proxy(tCfg *src){
	proxy_type = src->proxy_type;
	proxy_port = src->proxy_port;
	proxy_no_cache = src->proxy_no_cache;
	proxy_host.set(src->proxy_host.get());
	proxy_user.set(src->proxy_user.get());
	proxy_pass.set(src->proxy_pass.get());
};

void tCfg::copy(tCfg *src) {
	copy_ints(src);
	copy_proxy(src);
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
	write_named_integer(fd,"proxy_no_cache:",proxy_no_cache);
	if (user_agent.get() && *(user_agent.get()))
		write_named_string(fd,"User_agent:",user_agent.get());
		
	write_named_integer(fd,"Timeout:",timeout);
	write_named_integer(fd,"time_for_sleep:",time_for_sleep);
	write_named_integer(fd,"number_of_attempts:",number_of_attempts);
	write_named_integer(fd,"sleep_before_complete:",sleep_before_complete);
	write_named_integer(fd,"ftp_recurse_depth:",ftp_recurse_depth);
	write_named_integer(fd,"http_recurse_depth:",http_recurse_depth);
	write_named_integer(fd,"rollback:",rollback);
	write_named_integer(fd,"speed:",speed);
	write_named_integer(fd,"passive:",passive);
	write_named_integer(fd,"dont_send_quit:",dont_send_quit);
	write_named_integer(fd,"retry:",retry);
	write_named_integer(fd,"permisions:",permisions);
	write_named_integer(fd,"get_date:",get_date);
	write_named_integer(fd,"http_recursing:",http_recursing);
	write_named_integer(fd,"link_as_file:",link_as_file);
	write_named_integer(fd,"leave_server:",leave_server);
	write_named_integer(fd,"dont_leave_dir:",dont_leave_dir);
	write_named_integer(fd,"check_time:",check_time);
	write_named_integer(fd,"change_links:",change_links);
	if (restart_from_begin)
		write_named_integer(fd,"restart_from_begin:",restart_from_begin);
	if (save_name.get() && *(save_name.get()))
		write_named_string(fd,"save_name:",save_name.get());
	if (save_path.get() && *(save_path.get()))
		write_named_string(fd,"save_path:",save_path.get());
	if (referer.get())
		write_named_string(fd,"referer:",referer.get());
	if (log_save_path.get())
		write_named_string(fd,"log_save_path:",log_save_path.get());
	f_wstr_lf(fd,"EndCfg:");
};

int tCfg::load_from_config(int fd){
	tSavedVar table_of_fields[]={
		{"User_agent:", SV_TYPE_PSTR,	&user_agent}, 
		{"Proxy:",	SV_TYPE_PSTR,	&proxy_host},
		{"proxy_pass:",	SV_TYPE_PSTR,	&proxy_pass},
		{"proxy_user:",	SV_TYPE_PSTR,	&proxy_user},
		{"proxy_port:",	SV_TYPE_INT,	&proxy_port},
		{"proxy_type:",	SV_TYPE_INT,	&proxy_type},
		{"timeout:",	SV_TYPE_INT,	&timeout},
		{"time_for_sleep:",SV_TYPE_INT,	&time_for_sleep},
		{"sleep_before_complete:",SV_TYPE_INT,	&sleep_before_complete},
		{"number_of_attempts:",SV_TYPE_INT,&number_of_attempts},
		{"ftp_recurse_depth:",SV_TYPE_INT,&ftp_recurse_depth},
		{"http_recurse_depth:",SV_TYPE_INT,&http_recurse_depth},
		{"rollback:",	SV_TYPE_INT,	&rollback},
		{"speed:",	SV_TYPE_INT,	&speed},
		{"passive:",	SV_TYPE_INT,	&passive},
		{"dont_send_quit:",	SV_TYPE_INT,	&dont_send_quit},
		{"retry:",	SV_TYPE_INT,	&retry},
		{"permisions:",	SV_TYPE_INT,	&permisions},
		{"get_date:",	SV_TYPE_INT,	&get_date},
		{"http_recursing:",SV_TYPE_INT,	&http_recursing},
		{"link_as_file:",SV_TYPE_INT,	&link_as_file},
		{"leave_server:",SV_TYPE_INT,	&leave_server},
		{"save_name:",	SV_TYPE_PSTR,	&save_name},
		{"save_path:",	SV_TYPE_PSTR,	&save_path},
		{"dont_leave_dir:",SV_TYPE_INT,	&dont_leave_dir},
		{"referer:",	SV_TYPE_PSTR,	&referer},
		{"proxy_no_cache:",SV_TYPE_INT,	&proxy_no_cache},
		{"EndCfg:",	SV_TYPE_END,	NULL},
		{"restart_from_begin:",SV_TYPE_INT,&restart_from_begin},
		{"check_time:",SV_TYPE_INT,&check_time},
		{"change_links:",SV_TYPE_INT,&change_links},
		{"log_save_path:",SV_TYPE_PSTR,	&log_save_path}
	};
	char buf[MAX_LEN];
	while(f_rstr(fd,buf,MAX_LEN)>0){
		unsigned int i;
		for (i=0;i<sizeof(table_of_fields)/sizeof(tSavedVar);i++){
			if (equal_uncase(buf,table_of_fields[i].name)){
				if (table_of_fields[i].type==SV_TYPE_END){
					return(0);
				}else{
					if (sv_parse_file(fd,&(table_of_fields[i]),buf,MAX_LEN))
						return(-1);
				};
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
		LOG->log(LOG_ERROR,_("Maximum number of retries reached!"));
		break;
	};
	case ERROR_ATTEMPT:{
		if (config.number_of_attempts)
			LOG->log_printf(LOG_OK,_("Retry %i of %i...."),RetrNum,config.number_of_attempts);
		else
			LOG->log_printf(LOG_OK,_("Retry %i ..."),RetrNum);
		break;
	};
	case ERROR_FILE_UPDATED:
		LOG->log(LOG_WARNING,_("File on a server is newer then local one. Restarting from begin\n"));
		break;
	default:{
		LOG->log(LOG_ERROR,_("Warning! Probably you found the BUG!!!"));
		LOG->log(LOG_ERROR,_("If you see this message please report to mdem@chat.ru"));
		break;
	};
	};
};

tDownloader::tDownloader(){
	LOG=NULL;
	D_FILE.perm=get_permisions_from_int(CFG.DEFAULT_PERMISIONS);
	StartSize=-1;
	D_FILE.size=D_FILE.type=0;
	Status=D_NOTHING;
	local_filetime=0;
};

char * tDownloader::get_new_url() {
	return NULL;
};

void tDownloader::set_file_info(tFileInfo *what) {
	DBC_RETURN_IF_FAIL(what!=NULL);

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
	if (config.rollback>0)
		LOG->truncate();
	return(LOADED);
};

void tDownloader::init_download(char *path,char *file) {
	ADDR.file.set(file);
	ADDR.path.set(path);
};

void tDownloader::set_loaded(fsize_t a) {
	LOADED=a;
};

void tDownloader::set_local_filetime(time_t lt){
	local_filetime=lt;
};

int tDownloader::remote_file_changed(){
	if (config.check_time && local_filetime && local_filetime<D_FILE.date)
		return 1;
	return 0;
};

int tDownloader::treat() {
	return RetrNum;
};

fsize_t tDownloader::another_way_get_size() {
	return 0;
};

int tDownloader::get_status() {
	return(Status);
};

fsize_t tDownloader::get_start_size() {
	return(StartSize);
};

void tDownloader::make_full_pathes(const char *path,char **name,char **guess) {
	DBC_RETURN_IF_FAIL(path!=NULL);
	DBC_RETURN_IF_FAIL(guess!=NULL);
	DBC_RETURN_IF_FAIL(name!=NULL);

	char *temp;
	temp=sum_strings(".",ADDR.file.get(),NULL);
	*name=compose_path(path,temp);
	*guess=compose_path(path,ADDR.file.get());
	delete temp;
};

void tDownloader::make_full_pathes(const char *path,char *another_name,char **name,char **guess) {
	DBC_RETURN_IF_FAIL(path!=NULL);
	DBC_RETURN_IF_FAIL(another_name!=NULL);
	DBC_RETURN_IF_FAIL(guess!=NULL);
	DBC_RETURN_IF_FAIL(name!=NULL);

	char *temp=sum_strings(".",another_name,NULL);
	*name=compose_path(path,temp);
	*guess=compose_path(path,another_name);
	delete temp;
};

tDownloader::~tDownloader() {
	// do nothing
};
