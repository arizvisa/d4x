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
#include "hproxy.h"
#include "locstr.h"
#include "var.h"
#include "base64.h"
#include "ntlocale.h"

tHProxyClient::tHProxyClient() {
	real_host=NULL;
	hostname=userword=username=buffer=NULL;
	cookie_path=NULL;
};

void tHProxyClient::setup_host(char *host) {
	real_host=host;
};

int tHProxyClient::get_size(char *filename,tStringList *list) {
	send_request("GET ",filename," HTTP/1.0\r\n");

//	send_request("Referer: ",HOME_PAGE,"\r\n");
	if (referer)
		send_request("Referer: ",referer,"\r\n");
	else
		send_request("Referer: ",filename,"\r\n");

	char data[MAX_LEN];
	send_request("Accept: */*\r\n");
	sprintf(data,"%i",Offset);
	if (Offset){
		sprintf(data,"%i",Offset);
		send_request("Range: bytes=",data,"-\r\n");
	};

	if (user_agent && strlen(user_agent)){
		if (equal(user_agent,"%version"))
			send_request("User-Agent: ",VERSION_NAME,"\r\n");
		else
			send_request("User-Agent: ",user_agent,"\r\n");
	};
	send_request("Host: ",real_host,"\r\n");

	if (username && userword) {
		char *tmp=sum_strings(username,":",userword,NULL);
		char *pass=string_to_base64(tmp);
		delete tmp;
		send_request("Proxy-Authorization: Basic ",pass,"\r\n");
		delete pass;
	};
	send_cookies(real_host,cookie_path);
	send_request("\r\n");
	return read_answer(list);
};

void tHProxyClient::set_cookie_search(char *what){
	if (cookie_path) delete(cookie_path);
	cookie_path=what;
};

tHProxyClient::~tHProxyClient() {
	if (cookie_path) delete(cookie_path);
};

/* ---------------------------------------------
 */
tProxyDownload::tProxyDownload() {
	LOG=NULL;
	HOST=USER=PASS=D_PATH=NULL;
	D_FILE.perm=get_permisions_from_int(CFG.DEFAULT_PERMISIONS);
	StartSize=D_FILE.size=D_FILE.type=0;
	Status=D_NOTHING;
	FULL_NAME_TEMP=NULL;
	answer=NULL;
};

int tProxyDownload::init(tAddr *hostinfo,tWriterLoger *log,tCfg *cfg) {
	LOG=log;
	HTTP=new tHProxyClient;
	RetrNum=0;
	HOST=hostinfo->host.get();
	USER=hostinfo->username.get();
	PASS=hostinfo->pass.get();
	PARAMS=hostinfo->params.get();
	D_PORT=hostinfo->port;
	answer=NULL;
	ETag=NULL;
	Auth=NULL;
	D_PATH=NULL;
	D_FILE.type=T_FILE; //we don't know any other when download via http
	config.copy_ints(cfg);
	config.proxy_host.set(cfg->proxy_host.get());
	config.proxy_user.set(cfg->proxy_user.get());
	config.proxy_pass.set(cfg->proxy_pass.get());
	D_PROTO=hostinfo->proto;
	HTTP->init(config.proxy_host.get(),LOG,config.proxy_port,config.timeout);
	config.user_agent.set(cfg->user_agent.get());
	config.referer.set(cfg->referer.get());
	HTTP->set_user_agent(config.user_agent.get(),config.referer.get());
	HTTP->registr(config.proxy_user.get(),config.proxy_pass.get());
	((tHProxyClient *)(HTTP))->setup_host(HOST);
	return reconnect();
};

char *tProxyDownload::make_name(){
	int port_len = get_port_by_proto(D_PROTO)!=D_PORT ? int_to_strin_len(D_PORT)+1 : 0;
	char *rvalue=new char[strlen(get_name_by_proto(D_PROTO))+strlen(D_PATH)+
			     strlen(HOST)+strlen(D_FILE.name.get())+
			     (USER && PASS ? strlen(USER)+strlen(PASS)+2:0)+
			     (PARAMS ? strlen(PARAMS)+1:0)+strlen(":////")+
			     port_len+1];
	*rvalue=0;
	strcat(rvalue,get_name_by_proto(D_PROTO));
	strcat(rvalue,"://");
	if (USER && PASS){
		strcat(rvalue,USER);
		strcat(rvalue,":");
		strcat(rvalue,PASS);
		strcat(rvalue,"@");
	};
	strcat(rvalue,HOST);
	if (port_len)
		sprintf(rvalue+strlen(rvalue),":%i",D_PORT);
	strcat(rvalue,"/");
	strcat(rvalue,D_PATH);
	if (*D_PATH && D_PATH[strlen(D_PATH)-1]!='/')
		strcat(rvalue,"/");
	strcat(rvalue,D_FILE.name.get());
	if (PARAMS){
		strcat(rvalue,"?");
		strcat(rvalue,PARAMS);
	};
	return rvalue;
};

int tProxyDownload::get_size() {
	// Make a URL from available data
	if (FULL_NAME_TEMP) delete (FULL_NAME_TEMP);
	FULL_NAME_TEMP=make_name();
	((tHProxyClient *)HTTP)->set_cookie_search(sum_strings("/",D_PATH,"/",D_FILE.name.get(),NULL));
	//begin request
	if (!answer) {
		answer=new tStringList;
		answer->init(0);
	};
	while (1) {
		answer->done();
		HTTP->set_offset(LOADED);
		LOG->log(LOG_OK,_("Connection to the internet via proxy"));

		LOG->log(LOG_OK,_("Sending request to proxy"));
		int temp=HTTP->get_size(FULL_NAME_TEMP,answer);
		if (temp==0) {
			LOG->log(LOG_OK,_("Answer read ok"));
			D_FILE.size=analize_answer();
			if (ReGet && D_FILE.size>=0)
				D_FILE.size+=LOADED;
			return D_FILE.size;
		};
		if (temp==1) return -1;
		if (HTTP->get_status()!=STATUS_TIMEOUT) break;
		if (reconnect()) break;
	};
	print_error(ERROR_BAD_ANSWER);
	return -2;
};

tProxyDownload::~tProxyDownload() {
};
