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
#ifndef MY_HTTP_PROXY
#define MY_HTTP_PROXY
#include "http.h"
#include "httpd.h"

class tHProxyClient:public tHttpClient{
	char *real_host;
	char *cookie_path;
	public:
		tHProxyClient();
		void setup_host(char *host);
		void set_cookie_search(char *what);
		int get_size(char *filename,tStringList *list);
		~tHProxyClient();
};

class tProxyDownload:public tHttpDownload{
	char *D_PROTO;
	public:
		tProxyDownload();
		int init(tAddr *hostinfo,tWriterLoger *log,tCfg *cfg);
		int get_size();
		~tProxyDownload();
};
#endif
