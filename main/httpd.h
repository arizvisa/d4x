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
#ifndef HTTP_DOWNLOADER
#define HTTP_DOWNLOADER

#include "download.h"
#include "http.h"
#include "liststr.h"

class tHttpDownload:public tDownloader{
	protected:
	tHttpClient *HTTP;
	tStringList *answer;
	int ReGet,MustNoReget,first,ETagChanged;
	char *RealName,*NewRealName,*content_type;
	char *ETag,*Auth;
	int analize_answer();
	char *get_field(char *field);
	public:
		tHttpDownload();
		void init_download(char *path,char *file);
		void analize_html();
		int reconnect();
		int create_file(char *where,char *another_name);
		int delete_file(char *where);
    	int init(tAddr *hostinfo,tLog *log,tCfg *cfg);
		int get_size();
		int get_readed();
		int get_child_status();
		char *get_new_url();
		int reget();
		char *get_real_name();
		char *get_content_type();
		tStringList *dir();
		int download(unsigned int from,unsigned int len);
		void done();
		~tHttpDownload();
};

#endif