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
#ifndef T_CLIENT
#define T_CLIENT

#include "socket.h"
#include "liststr.h"
#include "log.h"

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
    tLog *LOG;
//-----------------------------------------------
    int read_string(tSocket *sock,tStringList *list,int maxlen);
    int socket_err_handler(int err);
    virtual int read_data();
    virtual int read_data(char *dst,int len)=0;
    int write_buffer(int fd);
    public:
    	tClient();
    	int get_readed();
    	virtual void init(char *host,tLog *log,int prt,int time_out);
        virtual int reinit();
    	virtual void down()=0;
    	virtual int registr(char *user,char *password)=0;
    	virtual int get_size(char *filename,tStringList *list)=0;
    	int get_status();
    	int test_reget();
    	virtual void done()=0;
    	virtual ~tClient();
};

#endif
