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

#include "hproxy.h"
#include "locstr.h"
#include "var.h"
#include "base64.h"
#include "ntlocale.h"

using namespace d4x;

tHProxyClient::tHProxyClient():tHttpClient(){
};

tHProxyClient::tHProxyClient(tCfg *cfg,SocketPtr ctrl):tHttpClient(cfg,ctrl){
};

void tHProxyClient::setup_data(const std::string &host,int cache) {
	real_host=host;
	no_cache=cache;
};

fsize_t tHProxyClient::get_size_sub(tStringList *list){
	if (!referer.empty())
		send_request("Referer",referer);
	send_request("Accept: */*");
	if (Offset)
		send_request("Range",
			     std::string("bytes=")+boost::lexical_cast<std::string>(Offset)+"-");

	if (!user_agent.empty()){
		if (user_agent=="%version")
			send_request("User-Agent",VERSION_NAME);
		else
			send_request("User-Agent",user_agent);
	};
	send_request("Host",real_host);

	if (!username_proxy.empty()) {
		char *pass=string_to_base64((username_proxy+":"+userword_proxy).c_str());
		send_request("Proxy-Authorization",std::string("Basic ")+pass);
		delete[] pass;
	};
	if (!username.empty() && !userword.empty()) {
		char *pass=string_to_base64((username+":"+userword).c_str());
		send_request("Authorization",std::string("Basic ")+pass);
		delete[] pass;
	};
	if (no_cache)
		send_request("Pragma","no-cache");
	send_cookies(real_host,cookie_path);
	send_request("");
	return read_answer(list);
};

fsize_t tHProxyClient::get_size_only(const std::string &filename,tStringList *list) {
	send_request(std::string("HEAD ")+filename+" HTTP/1.0");
	return(get_size_sub(list));
};

fsize_t tHProxyClient::get_size(const std::string &filename,tStringList *list) {
	send_request(std::string("GET ")+filename+" HTTP/1.0");
	return(get_size_sub(list));
};

void tHProxyClient::proxy_registr(char *user,char *password) {
	username_proxy=user?user:"";
	userword_proxy=password?password:"";
};

void tHProxyClient::set_cookie_search(const std::string &what){
	cookie_path=what;
};

tHProxyClient::~tHProxyClient() {
};

/* ---------------------------------------------
 */
tProxyDownload::tProxyDownload():tHttpDownload(){
};

tProxyDownload::tProxyDownload(tWriterLoger *log):tHttpDownload(log){
};

int tProxyDownload::init(const d4x::URL &hostinfo,tCfg *cfg,SocketPtr s) {
	DBC_RETVAL_IF_FAIL(cfg!=NULL,-1);
	HTTP=new tHProxyClient(cfg);
	RetrNum=0;
	ADDR=hostinfo;
	answer=NULL;
	ETag=NULL;
	Auth=NULL;
	D_FILE.type=T_FILE; //we don't know any other when download via http
	config.copy_ints(cfg);
	config.socks_port = cfg->socks_port;
	config.socks_host.set(cfg->socks_host.get());
	config.socks_user.set(cfg->socks_user.get());
	config.socks_pass.set(cfg->socks_pass.get());
	config.proxy.no_cache = cfg->proxy.no_cache;
	if (hostinfo.proto==D_PROTO_FTP){
		config.proxy.http_port = cfg->proxy.ftp_port;
		config.proxy.http_host.set(cfg->proxy.ftp_host.get());
		config.proxy.http_user.set(cfg->proxy.ftp_user.get());
		config.proxy.http_pass.set(cfg->proxy.ftp_pass.get());
	}else{
		config.proxy.http_port = cfg->proxy.http_port;
		config.proxy.http_host.set(cfg->proxy.http_host.get());
		config.proxy.http_user.set(cfg->proxy.http_user.get());
		config.proxy.http_pass.set(cfg->proxy.http_pass.get());
	};

	D_PROTO=hostinfo.proto;
	HTTP->init(config.proxy.http_host.get(),LOG,config.proxy.http_port,config.timeout);
	config.user_agent.set(cfg->user_agent.get());
	config.referer.set(cfg->referer.get());
	HTTP->set_user_agent(config.user_agent.get(),config.referer.get());
	if (ADDR.proto==D_PROTO_FTP)
		HTTP->registr("","");
	else
		HTTP->registr(ADDR.user,ADDR.pass);
	((tHProxyClient *)(HTTP))->proxy_registr(config.proxy.http_user.get(),config.proxy.http_pass.get());
	((tHProxyClient *)(HTTP))->setup_data(ADDR.host.c_str(),cfg->proxy.no_cache);
	REQUESTED_URL=make_name();
	return reconnect();
};

std::string tProxyDownload::make_name(){
	return std::string(ADDR);
};

fsize_t tProxyDownload::get_size_only() {
	// Make a URL from available data
	((tHProxyClient *)HTTP)->set_cookie_search(ADDR.path/ADDR.file);
	//begin request
	if (!answer) {
		answer=new tStringList;
		answer->init(0);
	};
	while (1) {
		answer->done();
		HTTP->set_offset(0);
		LOG->log_printf(LOG_OK,_("Sending request to proxy (%s:%i)"),config.proxy.http_host.get(),config.proxy.http_port);
		fsize_t temp=HTTP->get_size_only(REQUESTED_URL.c_str(),answer);
		switch (temp){
		case 0:{
			LOG->log(LOG_OK,_("Answer read ok"));
			D_FILE.size=analize_answer();
			return D_FILE.size;
		};
		case 1: // redirection
			analize_answer(); // to avoid lost cookies;
			return -1;
		case -2: // bad answer
			LOG->log(LOG_ERROR,_("File not found on the server"));
			break;
		case -1: // timout
			break;
		};
		if (reconnect()) break;
	};
	return -2;
};

fsize_t tProxyDownload::get_size() {
	// Make a URL from available data
	((tHProxyClient *)HTTP)->set_cookie_search(ADDR.path/ADDR.file);
	//begin request
	if (!answer) {
		answer=new tStringList;
		answer->init(0);
	};
	while (1) {
		answer->done();
		HTTP->set_offset(LOADED);
		LOG->log_printf(LOG_OK,_("Sending request to proxy (%s:%i)"),config.proxy.http_host.get(),config.proxy.http_port);
		fsize_t temp=HTTP->get_size(REQUESTED_URL.c_str(),answer);
		switch (temp){
		case 0:{
			LOG->log(LOG_OK,_("Answer read ok"));
			D_FILE.size=analize_answer();
			if (ReGet && D_FILE.size>=0)
				D_FILE.size+=LOADED;
			return D_FILE.size;
		};
		case 1: // redirection
			analize_answer(); // to avoid lost cookies;
			return -1;
		case -2: // bad answer
			if (HTTP->ERROR_CODE==404){
				LOG->log(LOG_ERROR,_("File not found on the server"));
				return -2;
			};
			LOG->log(LOG_OK,_("Probably it's problem of proxy server, retrying"));
			break;
		case -1: // timout
			break;
		};
		if (reconnect()) break;
	};
	return -2;
};

tProxyDownload::~tProxyDownload() {
};
