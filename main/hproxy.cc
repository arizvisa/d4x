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

tHProxyClient::tHProxyClient():tHttpClient(){
	real_host=NULL;
	cookie_path=NULL;
};

tHProxyClient::tHProxyClient(tCfg *cfg,tSocket *ctrl):tHttpClient(cfg,ctrl){
	real_host=NULL;
	cookie_path=NULL;
};

void tHProxyClient::setup_data(char *host,int cache) {
	real_host=host;
	no_cache=cache;
};

fsize_t tHProxyClient::get_size(char *filename,tStringList *list) {
	DBC_RETVAL_IF_FAIL(filename!=NULL,-1);
	send_request("GET ",filename," HTTP/1.0\r\n");

//	send_request("Referer: ",HOME_PAGE,"\r\n");
	if (referer && *referer)
		send_request("Referer: ",referer,"\r\n");
	char data[MAX_LEN];
	send_request("Accept: */*\r\n");
	if (Offset){
		sprintf(data,"%li",Offset);
		send_request("Range: bytes=",data,"-\r\n");
	};

	if (user_agent && strlen(user_agent)){
		if (equal(user_agent,"%version"))
			send_request("User-Agent: ",VERSION_NAME,"\r\n");
		else
			send_request("User-Agent: ",user_agent,"\r\n");
	};
	send_request("Host: ",real_host,"\r\n");

	if (username_proxy && userword_proxy) {
		char *tmp=sum_strings(username_proxy,":",userword_proxy,NULL);
		char *pass=string_to_base64(tmp);
		delete[] tmp;
		send_request("Proxy-Authorization: Basic ",pass,"\r\n");
		delete[] pass;
	};
	if (username && userword) {
		char *tmp=sum_strings(username,":",userword,NULL);
		char *pass=string_to_base64(tmp);
		delete[] tmp;
		send_request("Authorization: Basic ",pass,"\r\n");
		delete[] pass;
	};
	if (no_cache)
		send_request("Pragma: no-cache\r\n");
	send_cookies(real_host,cookie_path);
	send_request("\r\n");
	return read_answer(list);
};

void tHProxyClient::proxy_registr(char *user,char *password) {
	username_proxy=user;
	userword_proxy=password;
};

void tHProxyClient::set_cookie_search(char *what){
	if (cookie_path) delete[] cookie_path;
	cookie_path=what;
};

tHProxyClient::~tHProxyClient() {
	if (cookie_path) delete[] cookie_path;
};

/* ---------------------------------------------
 */
tProxyDownload::tProxyDownload():tHttpDownload(){
};

tProxyDownload::tProxyDownload(tWriterLoger *log):tHttpDownload(log){
};

int tProxyDownload::init(tAddr *hostinfo,tCfg *cfg,tSocket *s) {
	DBC_RETVAL_IF_FAIL(hostinfo!=NULL,-1);
	DBC_RETVAL_IF_FAIL(cfg!=NULL,-1);
	HTTP=new tHProxyClient(cfg);
	RetrNum=0;
	ADDR.copy(hostinfo);
	answer=NULL;
	ETag=NULL;
	Auth=NULL;
	D_FILE.type=T_FILE; //we don't know any other when download via http
	config.copy_ints(cfg);
	config.socks_port = cfg->socks_port;
	config.socks_host.set(cfg->socks_host.get());
	config.socks_user.set(cfg->socks_user.get());
	config.socks_pass.set(cfg->socks_pass.get());
	config.proxy_no_cache = cfg->proxy_no_cache;
	if (hostinfo->proto==D_PROTO_FTP){
		config.hproxy_port = cfg->fproxy_port;
		config.hproxy_host.set(cfg->fproxy_host.get());
		config.hproxy_user.set(cfg->fproxy_user.get());
		config.hproxy_pass.set(cfg->fproxy_pass.get());
	}else{
		config.hproxy_port = cfg->hproxy_port;
		config.hproxy_host.set(cfg->hproxy_host.get());
		config.hproxy_user.set(cfg->hproxy_user.get());
		config.hproxy_pass.set(cfg->hproxy_pass.get());
	};

	D_PROTO=hostinfo->proto;
	HTTP->init(config.hproxy_host.get(),LOG,config.hproxy_port,config.timeout);
	config.user_agent.set(cfg->user_agent.get());
	config.referer.set(cfg->referer.get());
	HTTP->set_user_agent(config.user_agent.get(),config.referer.get());
	if (ADDR.proto==D_PROTO_FTP)
		HTTP->registr(NULL,NULL);
	else
		HTTP->registr(ADDR.username.get(),ADDR.pass.get());
	((tHProxyClient *)(HTTP))->proxy_registr(config.hproxy_user.get(),config.hproxy_pass.get());
	((tHProxyClient *)(HTTP))->setup_data(ADDR.host.get(),cfg->proxy_no_cache);
	REQUESTED_URL=make_name();
	return reconnect();
};

char *tProxyDownload::make_name(){
	char *parsed_name=unparse_percents(ADDR.file.get());
	char *parsed_path=unparse_percents(ADDR.path.get());
	int auth_len=0;
	if (ADDR.proto==D_PROTO_FTP && ADDR.username.get() &&
	    ADDR.pass.get())
		auth_len=strlen(ADDR.pass.get())+strlen(ADDR.username.get())+2;
	int port_len = get_port_by_proto(ADDR.proto)!=ADDR.port ? int_to_strin_len(ADDR.port)+1 : 0;
	char *rvalue=new char[strlen(get_name_by_proto(ADDR.proto))+strlen(parsed_path)+
			     strlen(ADDR.host.get())+strlen(parsed_name)+
			     (ADDR.params.get() ? strlen(ADDR.params.get())+1:0)+strlen(":////")+
			     port_len+auth_len+1];
	*rvalue=0;
	strcat(rvalue,get_name_by_proto(ADDR.proto));
	strcat(rvalue,"://");
	if (auth_len){
		strcat(rvalue,ADDR.username.get());
		strcat(rvalue,":");
		strcat(rvalue,ADDR.pass.get());
		strcat(rvalue,"@");
	};
	strcat(rvalue,ADDR.host.get());
	if (port_len)
		sprintf(rvalue+strlen(rvalue),":%i",ADDR.port);
	strcat(rvalue,"/");
	strcat(rvalue,parsed_path);
	char *D_PATH=parsed_path;
	if (*D_PATH && D_PATH[strlen(D_PATH)-1]!='/')
		strcat(rvalue,"/");
	strcat(rvalue,parsed_name);
	if (ADDR.params.get()){
		strcat(rvalue,"?");
		strcat(rvalue,ADDR.params.get());
	};
	delete[] parsed_path;
	delete[] parsed_name;
	return rvalue;
};

fsize_t tProxyDownload::get_size() {
	// Make a URL from available data
	((tHProxyClient *)HTTP)->set_cookie_search(ADDR.pathfile());
	//begin request
	if (!answer) {
		answer=new tStringList;
		answer->init(0);
	};
	while (1) {
		answer->done();
		HTTP->set_offset(LOADED);
		LOG->log_printf(LOG_OK,_("Sending request to proxy (%s:%i)"),config.hproxy_host.get(),config.hproxy_port);
		fsize_t temp=HTTP->get_size(REQUESTED_URL,answer);
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
