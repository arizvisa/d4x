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
#include "log.h"

struct tFileInfo{
    char *name;
    char *body;
    int size;
    int type,oldtype;
    int fdesc;
    int perm;
    int date;
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
	int passive;
	int retry;
	int permisions;
	int get_date;
	int full_server_loading;
/* proxy
 */
	int proxy_port;
	int proxy_type;
private:
	char *proxy_host;
	char *proxy_user;
	char *proxy_pass;
	char *user_agent;
public:
	tCfg();
	void set_proxy_user(char *what);
	void set_proxy_host(char *what);
	void set_proxy_pass(char *what);
	void set_user_agent(char *what);
	char *get_proxy_user(){return(proxy_user);};
	char *get_proxy_host(){return(proxy_host);};
	char *get_proxy_pass(){return(proxy_pass);};
	char *get_user_agent(){return(user_agent);};
	int get_flags();
	void set_flags(int what);
	void reset_proxy();
	void copy(tCfg *src);
	void copy_ints(tCfg *src);
	void save_to_config(int fd);
	int load_from_config(int fd);
	~tCfg();
};

struct tAddr;

class tDownloader{
    protected:
    tCfg config;
    tLog *LOG;
    int RetrNum;
    int Status;
    char *HOST,*USER,*PASS,*D_PATH;
    tFileInfo D_FILE;
    int D_PORT;
    int MASK;
    int StartSize;
    int data;
    int rollback(int offset);
    virtual void make_full_pathes(const char *path,char *another_name,char **name,char **guess);
    virtual void make_full_pathes(const char *path,char **name,char **guess);
 public:
    	tDownloader();
    	int treat();
     	int get_status();
     	virtual int get_start_size();
     	virtual void init_download(char *file,char *path);
     	void set_data(int a);
     	void short_init(tLog *log);
     	virtual int reconnect()=0;
    	virtual int init(tAddr *hostinfo,tLog *log,tCfg *cfg)=0;
    	void make_file_visible(char *where,char *another_name);
    	virtual int create_file(char *where,char *another_name);
    	virtual int delete_file(char *where);
    	virtual void set_date_file(char *where,char *another_name);
    	virtual int get_readed()=0;
    	virtual int file_type();
    	virtual void set_file_info(tFileInfo *what);
    	virtual int get_child_status()=0;
    	virtual int get_size()=0;
	virtual	void rollback_before();
     	virtual char *get_new_url();
     	virtual int reget()=0;
	virtual tStringList *dir()=0;
     	virtual int another_way_get_size();
     	virtual char *get_real_name();
    	virtual int download(unsigned int from,unsigned int len)=0;
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
#endif
