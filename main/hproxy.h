/*	WebDownloader for X-Window
 *	Copyright (C) 1999-2001 Koshelev Maxim
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
	char *username_proxy,*userword_proxy;
	int no_cache;
	public:
		tHProxyClient();
		tHProxyClient(tCfg *cfg,tSocket *ctrl=NULL);
		void setup_data(char *host,int cache);
		void set_cookie_search(char *what);
		fsize_t get_size(char *filename,tStringList *list);
		void proxy_registr(char *user,char *password);
		~tHProxyClient();
};

class tProxyDownload:public tHttpDownload{
	int D_PROTO;
	char *make_name();
public:
	tProxyDownload();
	tProxyDownload(tWriterLoger *log);
	int init(tAddr *hostinfo,tCfg *cfg,tSocket *s=NULL);
	fsize_t get_size();
	~tProxyDownload();
};
#endif
