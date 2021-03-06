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
#ifndef T_DOWNLOADER
#define T_DOWNLOADER

#include "queue.h"
#include "liststr.h"
#include "locstr.h"
#include "client.h"

class tFileInfo{
 public:
	tPStr name,body;
	fsize_t size;
	int type,oldtype;
	int perm;
	time_t date;
};


#include "addr.h"

class tDownloader{
 protected:
	tCfg config;
	tWriterLoger *LOG;
	int RetrNum;
	int Status;
	d4x::URL ADDR;
	tFileInfo D_FILE;
	time_t local_filetime;
	fsize_t StartSize,LOADED;
 public:
    	tDownloader();
    	tDownloader(tWriterLoger *log);
    	int treat();
     	int get_status();
     	virtual fsize_t get_start_size();
     	virtual void init_download(const std::string&file,const std::string &path);
     	void set_loaded(fsize_t a);
    	virtual void set_file_info(tFileInfo *what);
	void set_local_filetime(time_t lt);
	virtual int remote_file_changed();
    	virtual tFileInfo *get_file_info();
     	virtual char *get_new_url();
     	virtual fsize_t another_way_get_size()=0;

	fsize_t rollback();
	virtual void make_full_pathes(const char *path,const char *another_name,char **name,char **guess);
	virtual void make_full_pathes(const char *path,char **name,char **guess);
	virtual void print_error(int error_code);

    	virtual int init(const d4x::URL &hostinfo,tCfg *cfg,d4x::SocketPtr s=d4x::SocketPtr())=0;
    	virtual fsize_t get_readed()=0;
    	virtual fsize_t get_size_only()=0;
    	virtual fsize_t get_size()=0;
    	virtual int get_child_status()=0;
     	virtual int reget()=0;
	virtual tStringList *dir()=0;
    	virtual int download(fsize_t len)=0;
    	virtual void done()=0;
	virtual int reconnect();
	virtual d4x::SocketPtr export_ctrl_socket()=0;

    	virtual ~tDownloader();
};

enum{
	T_NONE,
	T_DIR,
	T_FILE,
	T_LINK,
	T_REDIRECT,
	T_DEVICE
};
/*
 * Statuses of tDownloader
 */

enum{
	D_NOTHING,
	D_QUERYING,
	D_DOWNLOAD,
	D_DOWNLOAD_BAD
};

enum ERROR_CODES{
	ERROR_UNKNOWN,
	ERROR_TIMEOUT,
	ERROR_CWD,
	ERROR_DATA_CONNECT,
	ERROR_ATTEMPT_LIMIT,
	ERROR_BAD_ANSWER,
	ERROR_ATTEMPT,
	ERROR_REGET,
	ERROR_TOO_MANY_USERS,
	ERROR_DIRECTORY,
	ERROR_ACCESS,
	ERROR_NO_SPACE,
	ERROR_FILE_UPDATED
};
#endif
