/*	WebDownloader for X-Window
 *	Copyright (C) 1999 Koshelev Maxim
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

struct tHtmlTeg{
	char *tag,*field,*closetag;
	int mod;
};

extern tHtmlTeg HTML_TEGS[];

class tHttpDownload:public tDownloader{
	protected:
	tHttpClient *HTTP;
	tStringList *answer;
	int ReGet,MustNoReget,first,ETagChanged;
	char *RealName,*NewRealName,*content_type;
	char *ETag,*Auth;
	int analize_answer();
	char *get_field(char *field);
	void skip_for_tag(char *tag);
	void make_full_pathes(const char *path,char *another_name,char **name,char **guess);
   	void make_full_pathes(const char *path,char **name,char **guess);
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
		void rollback_before();
		char *get_content_type();
		tStringList *dir();
		int download(unsigned int from,unsigned int len);
		void done();
		~tHttpDownload();
};

#endif
