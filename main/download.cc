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

tFileInfo::tFileInfo(){
	name=body=NULL;
};

void tFileInfo::set_name(char *what){
	if (name) delete(name);
	name=copy_string(what);
};

void tFileInfo::set_body(char *what){
	if (body) delete(body);
	body=copy_string(what);
};

char *tFileInfo::get_name(){
	return name;
};

char *tFileInfo::get_body(){
	return body;
};

tFileInfo::~tFileInfo(){
	if (name) delete(name);
	if (body) delete(body);
};
/* ---------------------------------------- */

tCfg::tCfg() {
	proxy_host = proxy_user = proxy_pass = user_agent = NULL;
	save_path = save_name = NULL;
	speed=0;
	proxy_type=0;
	link_as_file=leave_server=0;
};

void tCfg::set_proxy_user(char *what) {
	if (proxy_user)	delete(proxy_user);
	proxy_user=copy_string(what);
};

void tCfg::set_proxy_host(char *what) {
	if (proxy_host)	delete(proxy_host);
	proxy_host=copy_string(what);
};

void tCfg::set_proxy_pass(char *what) {
	if (proxy_pass)	delete(proxy_pass);
	proxy_pass=copy_string(what);
};

void tCfg::set_user_agent(char *what) {
	if (user_agent)	delete(user_agent);
	user_agent=copy_string(what);
};

void tCfg::set_save_name(char *what) {
	if (save_name)	delete(save_name);
	save_name=copy_string(what);
};

void tCfg::set_save_path(char *what) {
	if (save_path)	delete(save_path);
	save_path=copy_string(what);
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
	set_proxy_host(src->proxy_host);
	set_proxy_user(src->proxy_user);
	set_proxy_pass(src->proxy_pass);
	set_user_agent(src->user_agent);
};

void tCfg::reset_proxy() {
	if (proxy_user) delete(proxy_user);
	if (proxy_pass) delete(proxy_pass);
	if (proxy_host) delete(proxy_host);
	proxy_user=proxy_pass=proxy_host=NULL;
};

void tCfg::save_to_config(int fd){
	f_wstr_lf(fd,"Cfg:");
	if (proxy_host){
		write_named_string(fd,"Proxy:",proxy_host);
		write_named_integer(fd,"proxy_port:",proxy_port);
		write_named_integer(fd,"proxy_type:",proxy_type);
		if(proxy_pass!=NULL && proxy_user!=NULL){
			write_named_string(fd,"Proxy_pass:",proxy_pass);
			write_named_string(fd,"Proxy_user:",proxy_user);
		};
	};
	if (user_agent && *user_agent)
		write_named_string(fd,"User_agent:",user_agent);
		
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
	if (save_name && *save_name)
		write_named_string(fd,"save_name:",save_name);
	if (save_path && *save_path)
		write_named_string(fd,"save_path:",save_path);
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
		"EndCfg:" //22
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
			set_user_agent(buf);
			break;
		};
		case 1:{
			if (f_rstr(fd,buf,MAX_LEN)<0) return -1;
			set_proxy_host(buf);
			break;
		};
		case 2:{
			if (f_rstr(fd,buf,MAX_LEN)<0) return -1;
			set_proxy_pass(buf);
			break;
		};
		case 3:{
			if (f_rstr(fd,buf,MAX_LEN)<0) return -1;
			set_proxy_user(buf);
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
			set_save_name(buf);
			break;
		};
		case 21:{
			if (f_rstr(fd,buf,MAX_LEN)<0) return -1;
			set_save_path(buf);
			break;
		};
		case 22:{
			return 0;
		};
		};
	};
	return -1;
};

tCfg::~tCfg() {
	reset_proxy();
	if (user_agent) delete(user_agent);
	if (save_name) delete(save_name);
	if (save_path) delete(save_path);
};
/* Downloader::
 */

void tDownloader::print_error(int error_code){
	switch(error_code){
	case ERROR_ATTEMPT_LIMIT:{
		LOG->add(_("Max amount of retries was reached!"),LOG_ERROR);
		break;
	};
	case ERROR_ATTEMPT:{
		if (config.number_of_attempts)
			LOG->myprintf(LOG_OK,_("Retrying %i of %i...."),RetrNum,config.number_of_attempts);
		else
			LOG->myprintf(LOG_OK,_("Retrying %i ..."),RetrNum);
		break;
	};
	case ERROR_DIRECTORY:{
		LOG->add(_("Directory already created!:))"),LOG_ERROR);
		break;
	};
	default:{
		LOG->add(_("Warning! Probably you found the BUG!!!"),LOG_ERROR);
		LOG->add(_("If you see this message please report to mdem@chat.ru"),LOG_ERROR);
		break;
	};
	};
};

tDownloader::tDownloader(){
	LOG=NULL;
	HOST=USER=PASS=D_PATH=NULL;
	D_FILE.perm=get_permisions_from_int(CFG.DEFAULT_PERMISIONS);
	StartSize=D_FILE.size=D_FILE.type=D_FILE.fdesc=0;
	Status=D_NOTHING;
};

void tDownloader::short_init(tLog *log,tCfg *cfg) {
	LOG=log;
	config.set_save_name(cfg->get_save_name());
	config.set_save_path(cfg->get_save_path());
};

int tDownloader::file_type() {
	return D_FILE.type;
};

char *tDownloader::get_real_name() {
	return D_FILE.get_name();
};


char * tDownloader::get_new_url() {
	return NULL;
};

void tDownloader::set_file_info(tFileInfo *what) {
	D_FILE.type=what->type;
	if (D_FILE.type==T_LINK)
		D_FILE.set_body(what->get_body());
	D_FILE.perm=what->perm;
	D_FILE.date=what->date;
};

int tDownloader::rollback(int offset){
	int curent_offset=lseek(D_FILE.fdesc,0,SEEK_CUR);
	int real_roll_back = curent_offset - (curent_offset<config.rollback?curent_offset:config.rollback);
	lseek(D_FILE.fdesc,real_roll_back,SEEK_SET);
	return real_roll_back;
};

void tDownloader::init_download(char *path,char *file) {
	D_FILE.set_name(file);
	D_PATH=copy_string(path);
};

void tDownloader::set_data(int a) {
	data=a;
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
	temp=sum_strings(".",D_FILE.get_name(),NULL);
	*name=compose_path(path,temp);
	*guess=compose_path(path,D_FILE.get_name());
	delete temp;
};

void tDownloader::make_full_pathes(const char *path,char *another_name,char **name,char **guess) {
	char *temp=sum_strings(".",another_name,NULL);
	*name=compose_path(path,temp);
	*guess=compose_path(path,another_name);
	delete temp;
};

int tDownloader::create_file() {
	if (D_FILE.type==T_LINK && config.link_as_file)
		D_FILE.type=T_FILE;
	int rvalue=0;
	make_dir_hier(config.get_save_path());
	char *name;
	char *guess;
	if (config.get_save_name() && strlen(config.get_save_name()))
		make_full_pathes(config.get_save_path(),
				 config.get_save_name(),
				 &name,&guess);
	else
		make_full_pathes(config.get_save_path(),&name,&guess);
	switch (D_FILE.type) {
		case T_LINK:
			{ //this is a link
				LOG->add(_("Trying to create a link"),LOG_WARNING);
				int err=symlink(D_FILE.get_body(),guess);
				if (err) {
					if (errno!=EEXIST) {
						LOG->add(_("Can't create link"),LOG_ERROR);
						return -1;
					};
					LOG->add(_("Link already created!:))"),LOG_ERROR);
				};
				chmod(guess,D_FILE.perm  | S_IWUSR);
				break;
			};
		case T_FILE:
			{ //this is a file
				LOG->add(_("Trying to create a file"),LOG_WARNING);
				D_FILE.fdesc=open(guess,O_RDWR,S_IRUSR | S_IWUSR );
				if (D_FILE.fdesc<0) {
					D_FILE.fdesc=open(name,O_RDWR|O_CREAT,S_IRUSR | S_IWUSR );
					if (D_FILE.fdesc<0) {
						LOG->add(_("Can't create file at the path:"),LOG_ERROR);
						LOG->add(config.get_save_path(),LOG_ERROR);
						LOG->add(_("wich has name:"),LOG_ERROR);
						LOG->add(name,LOG_ERROR);
						rvalue=-1;
						break;
					};
				};
				LOG->add(_("File was created!"),LOG_OK);
				rvalue=StartSize=lseek(D_FILE.fdesc,0,SEEK_END);
				break;
			};
		case T_DIR:
			{ //this is a directory
				LOG->add(_("Trying to create a dir"),LOG_WARNING);
				if (strlen(D_FILE.get_name())==0){
					print_error(ERROR_DIRECTORY);
					break;
				};
				int temp=0;
				if (strlen(guess))
					temp=mkdir(guess,S_IRWXU);
				if (temp) {
					if (errno!=EEXIST) {
						LOG->add(_("Can't create directory!"),LOG_OK);
						rvalue=-1;
						break;
					};
					print_error(ERROR_DIRECTORY);
				};
				chmod(guess,D_FILE.perm | S_IWUSR |S_IXUSR);
				break;
			};
		case T_DEVICE:
			{ //this is device
				LOG->add(_("Downloader can't create devices..."),LOG_WARNING);
				break;
			};
		default:{
			print_error(ERROR_UNKNOWN);
		};
	};
	delete name;
	delete guess;
	return rvalue;
};

void tDownloader::rollback_before(){
	//do nothing
};


void tDownloader::make_file_visible() {
	if (D_FILE.type==T_FILE) {
		char *oldname,*newname;
		if (config.get_save_name() && strlen(config.get_save_name())){
			make_full_pathes(config.get_save_path(),
					 config.get_save_name(),
					 &oldname,&newname);
		} else {
			make_full_pathes(config.get_save_path(),
					 &oldname,&newname);
		};
		rename(oldname,newname);
		delete oldname;
		delete newname;
	};
};

void tDownloader::set_date_file() {
	if (config.get_date) {
		char *name,*guess;
		if (config.get_save_name() && strlen(config.get_save_name()))
			make_full_pathes(config.get_save_path(),
					 config.get_save_name(),
					 &name,&guess);
		else
			make_full_pathes(config.get_save_path(),&name,&guess);
		struct utimbuf dates;
		dates.actime=D_FILE.date;
		dates.modtime=D_FILE.date;
		utime(name,&dates);
		utime(guess,&dates);
		delete name;
		delete guess;
	};
};

int tDownloader::delete_file() {
	int rvalue=0;
	if (D_FILE.type==T_FILE) {
		char *name,*guess;
		make_full_pathes(config.get_save_path(),&name,&guess);
		if (D_FILE.fdesc>0) {
			close(D_FILE.fdesc);
			D_FILE.fdesc=0;
		};
		if (remove(guess) && remove(name))
			rvalue=-1;
		delete name;
		delete guess;
	};
	return rvalue;
};


tDownloader::~tDownloader() {
	// do nothing
};
