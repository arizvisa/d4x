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
#ifndef T_DOWNLOADER
#define T_DOWNLOADER

#include "queue.h"
#include "liststr.h"
#include "locstr.h"
#include "client.h"

class tFileInfo{
 public:
	tPStr name,body;
	int size;
	int type,oldtype;
	int perm;
	time_t date;
};

struct tCfg{
	int timeout;
	int time_for_sleep;
	int number_of_attempts;
	int ftp_recurse_depth,http_recurse_depth;
	int rollback;
	int speed;
/* flags
 */
	int http_recursing; //temporary flag
	int leave_server,dont_leave_dir;
	int passive;
	int retry;
	int permisions;
	int get_date;
	int full_server_loading;
	int link_as_file;
	int restart_from_begin;
/* proxy
 */
	int proxy_port;
	int proxy_type;
	int proxy_no_cache;
	tPStr proxy_host;
	tPStr proxy_user;
	tPStr proxy_pass;
	tPStr user_agent,referer;
	tPStr save_name,save_path;
	tCfg();
	int get_flags();
	void set_flags(int what);
	void reset_proxy();
	void copy(tCfg *src);
	void copy_ints(tCfg *src);
	void save_to_config(int fd);
	int load_from_config(int fd);
	~tCfg();
};

#include "addr.h"

class tDownloader{
 protected:
	tCfg config;
	tWriterLoger *LOG;
	int RetrNum;
	int Status;
	tAddr ADDR;
	tFileInfo D_FILE;
	time_t local_filetime;
	int StartSize;
	int LOADED;
 public:
    	tDownloader();
    	int treat();
     	int get_status();
     	virtual int get_start_size();
     	virtual void init_download(char *file,char *path);
     	void set_loaded(int a);
    	virtual void set_file_info(tFileInfo *what);
	void set_local_filetime(time_t lt);
	virtual int remote_file_changed();
    	virtual tFileInfo *get_file_info();
     	virtual char *get_new_url();
     	virtual int another_way_get_size();

	int rollback();
	virtual void make_full_pathes(const char *path,char *another_name,char **name,char **guess);
	virtual void make_full_pathes(const char *path,char **name,char **guess);
	virtual void print_error(int error_code);

     	virtual int reconnect()=0;
    	virtual int init(tAddr *hostinfo,tWriterLoger *log,tCfg *cfg)=0;
    	virtual int get_readed()=0;
    	virtual int get_child_status()=0;
    	virtual int get_size()=0;
     	virtual int reget()=0;
	virtual tStringList *dir()=0;
    	virtual int download(int len)=0;
    	virtual void done()=0;

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
