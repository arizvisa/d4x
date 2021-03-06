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
#include "signal.h"

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
	case ERROR_FILE_UPDATED:{
		LOG->log(LOG_WARNING,_("File on a server is newer then local one. Restarting from begin\n"));
		break;
	};
	default:{
		LOG->log_printf(LOG_ERROR,_("Error code %i"),error_code);
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

tDownloader::tDownloader(tWriterLoger *log){
	LOG=NULL;
	D_FILE.perm=get_permisions_from_int(CFG.DEFAULT_PERMISIONS);
	StartSize=-1;
	D_FILE.size=D_FILE.type=0;
	Status=D_NOTHING;
	local_filetime=0;
	LOG=log;
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

fsize_t tDownloader::rollback(){
	LOADED = LOADED<config.rollback ? 0 : LOADED-config.rollback;
	LOG->shift(LOADED);
	if (config.rollback>0)
		LOG->truncate();
	return(LOADED);
};

void tDownloader::init_download(const std::string &path,const std::string &file) {
	ADDR.file=file;
	ADDR.path=path;
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

int tDownloader::reconnect(){
	tDownload **download=my_pthread_key_get();
	if (download!=NULL && *download!=NULL){
		if ((*download)->split)
			download=&((*download)->split->grandparent);
		if (*download){
			if ((*download)->Attempt<RetrNum)
				(*download)->Attempt=RetrNum;
			D4X_UPDATE.add(*download);
		};
	};
	return(0);
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

	d4x::Path temp(path);
	temp/=std::string(".")+ADDR.file;
	*name=copy_string(temp.c_str());
	temp=d4x::Path(path)/ADDR.file;
	*guess=copy_string(temp.c_str());
};

void tDownloader::make_full_pathes(const char *path,const char *another_name,char **name,char **guess) {
	DBC_RETURN_IF_FAIL(path!=NULL);
	DBC_RETURN_IF_FAIL(another_name!=NULL);
	DBC_RETURN_IF_FAIL(guess!=NULL);
	DBC_RETURN_IF_FAIL(name!=NULL);

	d4x::Path temp(path);
	temp/=std::string(".")+another_name;
	*name=copy_string(temp.c_str());
	temp=d4x::Path(path)/std::string(another_name);
	*guess=copy_string(temp.c_str());
};

tDownloader::~tDownloader() {
	// do nothing
};
