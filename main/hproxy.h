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
#ifndef MY_HTTP_PROXY
#define MY_HTTP_PROXY
#include "http.h"
#include "httpd.h"

class tHProxyClient:public tHttpClient{
	std::string real_host,cookie_path;
	std::string username_proxy,userword_proxy;
	int no_cache;
	fsize_t get_size_sub(tStringList *list);
public:
	tHProxyClient();
	tHProxyClient(tCfg *cfg,d4x::SocketPtr ctrl=d4x::SocketPtr());
	void setup_data(const std::string &host,int cache);
	void set_cookie_search(const std::string &what);
	fsize_t get_size_only(const std::string &filename,tStringList *list);
	fsize_t get_size(const std::string &filename,tStringList *list);
	void proxy_registr(char *user,char *password);
	~tHProxyClient();
};

class tProxyDownload:public tHttpDownload{
	int D_PROTO;
	std::string make_name();
public:
	tProxyDownload();
	tProxyDownload(tWriterLoger *log);
	int init(const d4x::URL &hostinfo,tCfg *cfg,d4x::SocketPtr s=d4x::SocketPtr());
	fsize_t get_size_only();
	fsize_t get_size();
	~tProxyDownload();
};
#endif
