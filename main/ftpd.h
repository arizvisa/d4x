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
#ifndef T_FTP_DOWNLOAD
#define T_FTP_DOWNLOAD

#include "liststr.h"
#include "ftp.h"
#include "download.h"
#include "log.h"
#include "dlist.h"

class tFtpDownload:public tDownloader{
    protected:
    tFtpClient *FTP;
    tStringList *DIR,*list;
    int CWDFlag;
    int change_dir();
    int reconnect();
    int download_dir();
    void print_reget(int offset);
    void check_for_repeated(tStringList *LIST);
    public:
    	tFtpDownload();
    	int get_start_size();
    	int init(tAddr *hostinfo,tLog *log,tCfg *cfg);
    	void set_passive(int a);
    	int download(unsigned int from,unsigned int len);
    	int get_size();
    	int get_readed();
    	int get_child_status();
    	int reget();
     	int another_way_get_size();
    	void done();
    	tStringList *dir();
    	~tFtpDownload();
};

int type_from_str(char *data);
void extract_link(char *src,char *dst);
void cut_string_list(char *src,tFileInfo *dst,int flag);
#endif
