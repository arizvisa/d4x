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
#include "dlist.h"
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
	char *real_filename=unparse_percents(filename);
	send_request("GET ",real_filename," HTTP/1.0\r\n");

//	send_request("Referer: ",HOME_PAGE,"\r\n");
	send_request("Referer: ",real_filename,"\r\n");
	delete real_filename;

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
	StartSize=D_FILE.size=D_FILE.type=D_FILE.fdesc=0;
	Status=D_NOTHING;
	FULL_NAME_TEMP=NULL;
	D_PROTO=NULL;
	answer=NULL;
};

int tProxyDownload::init(tAddr *hostinfo,tLog *log,tCfg *cfg) {
	LOG=log;
	HTTP=new tHProxyClient;
	RetrNum=0;
	HOST=hostinfo->get_host();
	USER=hostinfo->get_username();
	PASS=hostinfo->get_pass();
	D_PORT=hostinfo->port;
	answer=NULL;
	ETag=NULL;
	Auth=NULL;
	D_PATH=NULL;
	D_FILE.fdesc=0;
	D_FILE.type=T_FILE; //we don't know any other when download via http
	data=0;
	first=1;
	config.copy_ints(cfg);
	config.set_proxy_host(cfg->get_proxy_host());
	config.set_proxy_user(cfg->get_proxy_user());
	config.set_proxy_pass(cfg->get_proxy_pass());
	config.set_save_path(cfg->get_save_path());
	config.set_save_name(cfg->get_save_name());
	D_PROTO=copy_string(get_name_by_proto(hostinfo->proto));
	HTTP->init(config.get_proxy_host(),LOG,config.proxy_port,config.timeout);
	config.set_user_agent(cfg->get_user_agent());
	HTTP->set_user_agent(config.get_user_agent());
	HTTP->registr(config.get_proxy_user(),config.get_proxy_pass());
	((tHProxyClient *)(HTTP))->setup_host(HOST);
	return reconnect();
};

int tProxyDownload::get_size() {
	// Make a URL from available data
	if (FULL_NAME_TEMP) delete (FULL_NAME_TEMP);
	FULL_NAME_TEMP=NULL;
	if (D_PATH[0]!=0 && D_PATH[strlen(D_PATH)-1]!='/'){
		if (USER && PASS)
			FULL_NAME_TEMP=sum_strings(D_PROTO,"://",USER,":",PASS,"@",HOST,"/",D_PATH,"/",D_FILE.get_name(),NULL);
		else
			FULL_NAME_TEMP=sum_strings(D_PROTO,"://",HOST,"/",D_PATH,"/",D_FILE.get_name(),NULL);
	}else{
		if (USER && PASS)
			FULL_NAME_TEMP=sum_strings(D_PROTO,"://",USER,":",PASS,"@",HOST,"/",D_PATH,D_FILE.get_name(),NULL);
		else
			FULL_NAME_TEMP=sum_strings(D_PROTO,"://",HOST,"/",D_PATH,D_FILE.get_name(),NULL);
	};
	((tHProxyClient *)HTTP)->set_cookie_search(sum_strings("/",D_PATH,"/",D_FILE.get_name(),NULL));
	//begin request
	if (!answer) {
		answer=new tStringList;
		answer->init(0);
	};
	while (1) {
		answer->done();
		HTTP->set_offset(data);
		LOG->add(_("Connection to the internet via proxy"),LOG_OK);
		LOG->add(_("Sending request to proxy"),LOG_OK);
		int temp=HTTP->get_size(FULL_NAME_TEMP,answer);
		if (temp==0) {
			LOG->add(_("Answer read ok"),LOG_OK);
			D_FILE.size=analize_answer();
			if (ReGet && D_FILE.size>=0)
				D_FILE.size+=data;
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
	if (D_PROTO) delete D_PROTO;
};
