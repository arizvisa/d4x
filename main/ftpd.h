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
#ifndef T_FTP_DOWNLOAD
#define T_FTP_DOWNLOAD

#include "liststr.h"
#include "ftp.h"
#include "download.h"

class tFtpDownload:public tDownloader{
    protected:
    tFtpClient *FTP;
    tStringList *DIR,*list;
    int CWDFlag;
    int change_dir();
    int reconnect();
    int download_dir();
    void print_error(int error_code);
    public:
    	tFtpDownload();
    	int init(tAddr *hostinfo,tWriterLoger *log,tCfg *cfg);
	void init_download(char *path,char *file);
    	int download(fsize_t len);
    	fsize_t get_size();
    	void done();

    	fsize_t get_start_size();
    	fsize_t get_readed();
    	int get_child_status();
    	int reget();
     	fsize_t another_way_get_size();
    	tStringList *dir();
    	~tFtpDownload();
};

int ftp_type_from_str(char *data);
void ftp_extract_link(char *src,char *dst);
void ftp_cut_string_list(char *src,tFileInfo *dst,int flag);
#endif
