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

tCfg::tCfg() {
	proxy_host = proxy_user = proxy_pass = NULL;
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

void tCfg::copy(tCfg *src) {
	timeout = src->timeout;
	time_for_sleep = src->time_for_sleep;
	number_of_attempts = src->number_of_attempts;
	passive = src->passive;
	permisions = src->permisions;
	get_date = src->get_date;
	retry = src->retry;
	proxy_type = src->proxy_type;
	proxy_port = src->proxy_port;
	set_proxy_host(src->proxy_host);
	set_proxy_user(src->proxy_user);
	set_proxy_pass(src->proxy_pass);
};

void tCfg::reset_proxy() {
	if (proxy_user) delete(proxy_user);
	if (proxy_pass) delete(proxy_pass);
	if (proxy_host) delete(proxy_host);
	proxy_user=proxy_pass=proxy_host=NULL;
};


tCfg::~tCfg() {
	reset_proxy();
};
/* Downloader::
 */

tDownloader::tDownloader(){
	LOG=NULL;
	D_FILE.name=HOST=USER=PASS=D_PATH=NULL;
//	D_FILE.perm=S_IWUSR | S_IRUSR;
	D_FILE.perm=get_permisions_from_int(CFG.DEFAULT_PERMISIONS);
	D_FILE.size=D_FILE.type=D_FILE.fdesc=0;
	Status=D_NOTHING;
};

void tDownloader::short_init(tLog *log) {
	LOG=log;
};

int tDownloader::file_type() {
	return D_FILE.type;
};

char *tDownloader::get_real_name() {
	return D_FILE.name;
};


char * tDownloader::get_new_url() {
	return NULL;
};

void tDownloader::set_file_info(tFileInfo *what) {
	D_FILE.type=what->type;
	if (D_FILE.type==T_LINK)
		D_FILE.body=copy_string(what->body);
	D_FILE.perm=what->perm;
	D_FILE.date=what->date;
};

void tDownloader::init_download(char *path,char *file) {
	D_FILE.name=copy_string(file);
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

void tDownloader::make_full_pathes(const char *path,char *name,char *guess) {
	*name=0;
	if (path) {
		strcat(name,path);
		if (strlen(name)) strcat(name,"/");
	};
	int flag=strlen(D_FILE.name);
	if (D_FILE.type==T_FILE) {
		*guess=0;
		strcat(guess,name);
		strcat(name,".");
		if (flag)
			strcat(guess,D_FILE.name);
		else
			strcat(guess,CFG.DEFAULT_NAME);
	};
	if (flag)
		strcat(name,D_FILE.name);
	else
		strcat(name,CFG.DEFAULT_NAME);
};

void tDownloader::make_full_pathes(const char *path,char *another_name,char *name,char *guess) {
	*name=0;
	if (path) {
		strcat(name,path);
		if (strlen(name)) strcat(name,"/");
	};
	if (D_FILE.type==T_FILE) {
		*guess=0;
		strcat(guess,name);
		strcat(name,".");
		strcat(guess,another_name);
	};
	strcat(name,another_name);
};

int tDownloader::create_file(char *where,char *another_name) {
	mkdir(where,S_IRWXU);
	char name[MAX_LEN],guess[MAX_LEN];
	if (another_name && strlen(another_name))
		make_full_pathes(where,another_name,name,guess);
	else
		make_full_pathes(where,name,guess);
	switch (D_FILE.type) {
		case T_FILE:
			{ //this is a file
				LOG->add(_("Trying to create a file"),LOG_WARNING);
				//	            D_FILE.fdesc=open(guess,O_WRONLY,S_IRUSR | S_IWUSR );
				D_FILE.fdesc=open(guess,O_RDWR,S_IRUSR | S_IWUSR );
				if (D_FILE.fdesc<0) {
					//	                D_FILE.fdesc=open(name,O_WRONLY|O_CREAT,S_IRUSR | S_IWUSR );
					D_FILE.fdesc=open(name,O_RDWR|O_CREAT,S_IRUSR | S_IWUSR );
					if (D_FILE.fdesc<0) {
						LOG->add(_("Can't create file at the path:"),LOG_ERROR);
						LOG->add(where,LOG_ERROR);
						LOG->add(_("wich has name:"),LOG_ERROR);
						LOG->add(name,LOG_ERROR);
						return -1;
					};
				};
				LOG->add(_("File was created!"),LOG_OK);
				int temp=StartSize=lseek(D_FILE.fdesc,0,SEEK_END);
				return temp;
			};
		case T_DIR:
			{ //this is a directory
				LOG->add(_("Trying to create a dir"),LOG_WARNING);
				int temp=0;
				if (strlen(name))
					temp=mkdir(name,S_IRWXU);
				if (temp) {
					if (errno!=EEXIST) {
						LOG->add(_("Can't create directory!"),LOG_OK);
						return -1;
					};
					LOG->add(_("Directory already created!:))"),LOG_ERROR);
				};
				chmod(name,D_FILE.perm | S_IWUSR |S_IXUSR);
				return 0;
			};
		case T_LINK:
			{ //this is a link
				LOG->add(_("Trying to create a link"),LOG_WARNING);
				int err=symlink(D_FILE.body,name);
				if (err) {
					if (errno!=EEXIST) {
						LOG->add(_("Can't create link"),LOG_ERROR);
						return -1;
					};
					LOG->add(_("Link already created!:))"),LOG_ERROR);
				};
				chmod(name,D_FILE.perm  | S_IWUSR);
				return 0;
			};
		case T_DEVICE:
			{ //this is device
				LOG->add(_("Downloader can't create devices..."),LOG_WARNING);
				return 0;
			};
		default:
			{
				LOG->add(_("Warning! Probably you found the BUG!!!"),LOG_ERROR);
				LOG->add(_("If you see this message please report to mdem@chat.ru"),LOG_ERROR);
			};
	};
	return 0;
};

void tDownloader::set_date_file(char *where,char *another_name) {
	if (config.get_date) {
		char name[MAX_LEN],guess[MAX_LEN];
		if (another_name && strlen(another_name))
			make_full_pathes(where,another_name,name,guess);
		else
			make_full_pathes(where,name,guess);
		struct utimbuf dates;
		dates.actime=D_FILE.date;
		dates.modtime=D_FILE.date;
		utime(name,&dates);
		utime(guess,&dates);
	};
};

int tDownloader::delete_file(char *where) {
	if (D_FILE.type==T_FILE) {
		char name[MAX_LEN],guess[MAX_LEN];
		make_full_pathes(where,name,guess);
		if (D_FILE.fdesc>0) {
			close(D_FILE.fdesc);
			D_FILE.fdesc=0;
		};
		if (remove(guess) && remove(name))
			return -1;
	};
	return 0;
};


tDownloader::~tDownloader() {
	// do nothing
}
;
