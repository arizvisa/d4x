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
#ifndef T_CLIENT
#define T_CLIENT

#include "socket.h"
#include "liststr.h"

class tWriterLoger{
 public:
	tWriterLoger();
	virtual int write(const void *buf,int len)=0;
	virtual int shift(int shift)=0;
	virtual void log(int type,const char *str)=0;
	virtual void log_printf(int type, const char *format,...);
	virtual char *cookie(const char *host, const char *path);
	virtual ~tWriterLoger();
};

class tClient{
    protected:
    char *hostname,*username,*userword;
    int port;
    int timeout;
    int FillSize,FileLoaded;
    unsigned int DSize;
    int ReGet,Status;
    tSocket CtrlSocket;
    unsigned int BuffSize;
    char *buffer;
    tWriterLoger *LOG;
//-----------------------------------------------
    int read_string(tSocket *sock,tStringList *list,int maxlen);
    int socket_err_handler(int err);
    virtual int read_data();
    virtual int read_data(char *dst,int len)=0;
    int write_buffer();
    public:
    	tClient();
    	int get_readed();
    	virtual void init(char *host,tWriterLoger *log,int prt,int time_out);
        virtual int reinit();
    	virtual void down()=0;
    	virtual int registr(char *user,char *password)=0;
    	virtual int get_size(char *filename,tStringList *list)=0;
	virtual int get_file_from(char *what,unsigned int begin,int len)=0;
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
