/*	WebDownloader for X-Window
 *	Copyright (C) 1999-2001 Koshelev Maxim
 *	This Program is free but not GPL!!! You can't modify it
 *	without agreement with author. You can't distribute modified
 *	program but you can distribute unmodified program.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */
#ifndef T_CLIENT
#define T_CLIENT

#include "socket.h"
#include "liststr.h"
#include "locstr.h"

struct tSimplyCfg{
	int timeout;
	int time_for_sleep;
	int sleep_before_complete;
	int number_of_attempts;
	int ftp_recurse_depth,http_recurse_depth;
	int rollback;
	int speed;
/* flags
 */
	int http_recursing; //temporary flag
	int leave_server,dont_leave_dir;
	int change_links;
	int passive;
	int retry;
	int permisions;
	int get_date;
	int full_server_loading;
	int link_as_file;
	int restart_from_begin;
	int dont_send_quit;
	int check_time;
	/* temporary flags */
	int split; 
	int redirect_count;
	void copy_ints(tSimplyCfg *src);
};

struct tCfg:public tSimplyCfg{
	int socks_port;
	tPStr socks_host;
	tPStr socks_user,socks_pass;
	int proxy_port;
	int proxy_type;
	int proxy_no_cache;
	tPStr proxy_host;
	tPStr proxy_user;
	tPStr proxy_pass;
	tPStr user_agent,referer,cookie; /* HTTP items */
	tPStr save_name,save_path;
	tPStr log_save_path;
	tCfg();
	int get_flags();
	void set_flags(int what);
	void reset_proxy();
	void copy_proxy(tCfg *src);
	void copy(tCfg *src);
	void save_to_config(int fd);
	int load_from_config(int fd);
	~tCfg();
};

class tWriterLoger{
 public:
	tWriterLoger();
	virtual fsize_t write(const void *buf,fsize_t len)=0;
	virtual fsize_t shift(fsize_t shift);
	virtual fsize_t shift(fsize_t shift,int mode)=0; // for html parser
	virtual fsize_t read(void *dst,fsize_t len)=0; //for html parser
	virtual void log(int type,const char *str)=0;
	virtual void log_printf(int type, const char *format,...);
	virtual char *cookie(const char *host, const char *path);
	virtual void truncate();
	virtual ~tWriterLoger();
};

class tClient{
    protected:
    char *hostname,*username,*userword;
    int port;
    int timeout;
    fsize_t FillSize,FileLoaded;
    unsigned int DSize;
    int ReGet,Status;
    tSocket *CtrlSocket;
    unsigned int BuffSize;
    char *buffer;
    tWriterLoger *LOG;
//-----------------------------------------------
    int read_string(tSocket *sock,tStringList *list,int maxlen);
    int socket_err_handler(int err);
    virtual int read_data();
    virtual int read_data(char *dst,fsize_t len)=0;
    int write_buffer();
    public:
    	tClient();
    	tClient(tCfg *cfg);
    	int get_readed();
    	virtual void init(char *host,tWriterLoger *log,int prt,int time_out);
        virtual int reinit();
    	virtual void down()=0;
    	virtual int registr(char *user,char *password)=0;
    	virtual fsize_t get_size(char *filename,tStringList *list)=0;
	virtual int get_file_from(char *what,unsigned int begin,fsize_t len)=0;
    	int get_status();
    	int test_reget();
    	virtual void done()=0;
    	virtual ~tClient();
};


enum NT_LOG_STRING_TYPES{
	LOG_OK=0,
	LOG_WARNING,
	LOG_FROM_SERVER,
	LOG_TO_SERVER,
	LOG_ERROR,
	LOG_DETAILED=64
};

#define RVALUE_COMPLETED 1
#define RVALUE_OK 0
#define RVALUE_TIMEOUT -1
#define RVALUE_BAD_COMMAND -2
#define RVALUE_UNSPEC_ERR -3

#endif
