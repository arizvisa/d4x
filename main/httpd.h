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
#ifndef HTTP_DOWNLOADER
#define HTTP_DOWNLOADER

#include "download.h"
#include "http.h"
#include "liststr.h"

class tHttpDownload:public tDownloader{
	protected:
	tHttpClient *HTTP;
	tStringList *answer;
	int ReGet,MustNoReget,ETagChanged;
	char *content_type,*FULL_NAME_TEMP;
	char *ETag,*OldETag,*Auth;
	int analize_answer();
	char *make_name();
	virtual void print_error(int error_code);
	public:
		tHttpDownload();
		int reconnect();
		int init(tAddr *hostinfo,tWriterLoger *log,tCfg *cfg);
		int get_size();
		int get_readed();
		int get_child_status();
		char *get_new_url();
		int reget();
		char *get_content_type();
		tStringList *dir();
		int download(int len);
		void make_full_pathes(const char *path,char *another_name,char **name,char **guess);
		void make_full_pathes(const char *path,char **name,char **guess);
		void done();
		~tHttpDownload();
};

#endif
