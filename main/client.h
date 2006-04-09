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
#ifndef T_CLIENT
#define T_CLIENT

#include "socket.h"
#include "liststr.h"
#include "locstr.h"
#include "cookie.h"

struct tSimplyCfg{
	int timeout;
	int time_for_sleep;
	int sleep_before_complete;
	int number_of_attempts;
	int ftp_recurse_depth,http_recurse_depth;
	fsize_t rollback;
	int speed;
	int con_limit; //used only by URL-manager
/* flags
 */
	int http_recursing; //temporary flag
	int leave_server,dont_leave_dir;
	int change_links;
	int quest_sign_replace;
	int passive;
	int retry;
	int permisions;
	int get_date;
	int full_server_loading;
	int follow_link; //ftp only: 0-normal loading,1-follow symlinks,2-load as file
	int dont_send_quit;
	int check_time;
	int ftp_dirontop;
	int ihate_etag;
	/* temporary flags */
	int split; 
	int redirect_count;
	void copy_ints(tSimplyCfg *src);
};

struct d4xProxyCfg{
	int type;
	int no_cache;
	tPStr http_host;
	tPStr http_user;
	tPStr http_pass;
	int http_port;
	tPStr ftp_host;
	tPStr ftp_user;
	tPStr ftp_pass;
	int ftp_port;
	d4xProxyCfg();
	void copy(d4xProxyCfg *src);
	void reset();
};

struct tCfg:public tSimplyCfg{
	int socks_port;
	tPStr socks_host;
	tPStr socks_user,socks_pass;
	d4xProxyCfg proxy;
	tPStr user_agent,referer,cookie; /* HTTP items */
	tPStr save_path;
	tPStr log_save_path;
	tPStr Filter;
	int isdefault;
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
	virtual int is_overlaped();
	virtual fsize_t shift(fsize_t shift);
	virtual fsize_t shift(fsize_t shift,int mode)=0; // for html parser
	virtual fsize_t read(void *dst,fsize_t len)=0; //for html parser
	virtual void log(int type,const char *str)=0;
	virtual void log_printf(int type, const char *format,...);
	virtual std::string cookie(const char *host, const char *path);
	virtual void cookie_set(tCookie *cookie);
	virtual void truncate();
	virtual ~tWriterLoger();
};

class tClient{
    protected:
	std::string hostname,username,userword;
	int port;
	int timeout;
	fsize_t FillSize,FileLoaded;
	fsize_t DSize;
	int ReGet,Status;
	d4x::SocketPtr CtrlSocket;
	fsize_t BuffSize;
	char *buffer;
	tWriterLoger *LOG;
//-----------------------------------------------
	char *read_string(d4x::SocketPtr sock,fsize_t maxlen);
	int read_string(d4x::SocketPtr sock,tStringList *list,fsize_t maxlen);
	int socket_err_handler(int err);
	virtual fsize_t read_data(fsize_t len);
	virtual fsize_t read_data(char *dst,fsize_t len)=0;
	int write_buffer();
public:
    	tClient();
    	tClient(tCfg *cfg,d4x::SocketPtr ctrl=d4x::SocketPtr());
    	fsize_t get_readed();
    	virtual void init(const std::string &host,tWriterLoger *log,int prt,int time_out);
        virtual int reinit();
    	virtual void down()=0;
    	virtual int registr(const std::string &user,const std::string &password)=0;
    	virtual fsize_t get_size(const std::string &filename,tStringList *list)=0;
	virtual fsize_t get_file_from(const char *what,fsize_t begin,fsize_t len)=0;
    	int get_status();
    	int test_reget();
    	virtual void done()=0;
    	virtual ~tClient();
	virtual d4x::SocketPtr export_ctrl_socket();
	virtual void import_ctrl_socket(d4x::SocketPtr s);
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
