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
#ifndef HTTP_DOWNLOADER
#define HTTP_DOWNLOADER

#include "download.h"
#include "http.h"
#include "liststr.h"

struct d4xContentDisposition{
	time_t c_date,m_date,r_date;
	fsize_t size;
	tPStr filename;
	d4xContentDisposition();
	d4xContentDisposition(char *httpstr);
	~d4xContentDisposition();
private:
	int get_code(char *name);
	char *get_value(char *name);
	char *get_name(char *name);
};

class tHttpDownload:public tDownloader{
	protected:
	tHttpClient *HTTP;
	tStringList *answer;
	int ReGet,MustNoReget,ETagChanged,Persistent;
	d4xContentDisposition *content_disp;
	char *content_type;
	char *REQUESTED_URL;
	char *ETag,*OldETag,*Auth;
	fsize_t analize_answer();
	char *make_name();
	virtual void print_error(int error_code);
public:
	tHttpDownload();
	tHttpDownload(tWriterLoger *log);
	void pass_first_segment();
	int reconnect();
	int init(tAddr *hostinfo,tCfg *cfg,tSocket *s=(tSocket *)NULL);
	fsize_t get_size();
	fsize_t get_size_only();
	fsize_t get_readed();
	fsize_t another_way_get_size();
	int get_child_status();
	char *get_new_url();
	int reget();
	char *get_content_type();
	d4xContentDisposition *get_content_disp();
	tStringList *dir();
	int download(fsize_t len);
	void make_full_pathes(const char *path,char *another_name,char **name,char **guess);
	void make_full_pathes(const char *path,char **name,char **guess);
	void done();
	int persistent();
	tSocket *export_ctrl_socket();
	~tHttpDownload();
};

#endif
