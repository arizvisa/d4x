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
#include "hproxy.h"
#include "locstr.h"
#include "var.h"
#include "base64.h"
#include "dlist.h"
#include "ntlocale.h"

tHProxyClient::tHProxyClient() {
	real_host=NULL;
	hostname=userword=username=buffer=NULL;
};

void tHProxyClient::setup_host(char *host) {
	real_host=host;
};

int tHProxyClient::get_size(char *filename,tStringList *list) {
	char *real_filename=unparse_percents(filename);
	char *data2=new char[strlen("GET  HTTP/1.0\r\n")+strlen(real_filename)+1];
	sprintf(data2,"GET %s HTTP/1.0\r\n",real_filename);
	send_request(data2);
	delete real_filename;
	delete data2;

	char data[MAX_LEN];
	if (user_agent && strlen(user_agent)){
		if (equal(user_agent,"%version"))
			sprintf(data,"User-Agent: %s\r\n",VERSION_NAME);
		else
			sprintf(data,"User-Agent: %s\r\n",user_agent);
		send_request(data);
	};
	send_request("Accept: */*\r\n");
	sprintf(data,"Range: bytes=%i-\r\n",Offset);
	send_request(data);
	sprintf(data,"Refer: %s\r\n",HOME_PAGE);
	send_request(data);
	sprintf(data,"Host: %s\r\n",real_host);
	send_request(data);
	if (username && userword) {
		char *tmp=sum_strings(username,":",userword);
		char *pass=string_to_base64(tmp);
		delete tmp;
		sprintf(data,"Proxy-Authorization: Basic %s\r\n",pass);
		delete pass;
		send_request(data);
	};
	send_request("\r\n");
	return read_answer(list);
};

tHProxyClient::~tHProxyClient() {
};

/* ---------------------------------------------
 */
tProxyDownload::tProxyDownload() {
	LOG=NULL;
	D_FILE.name=HOST=USER=PASS=D_PATH=NULL;
	D_FILE.perm=get_permisions_from_int(CFG.DEFAULT_PERMISIONS);
	StartSize=D_FILE.size=D_FILE.type=D_FILE.fdesc=0;
	Status=D_NOTHING;

	D_PROTO=NULL;
};

int tProxyDownload::init(tAddr *hostinfo,tLog *log,tCfg *cfg) {
	LOG=log;
	HTTP=new tHProxyClient;
	RetrNum=0;
	HOST=hostinfo->host;
	USER=hostinfo->username;
	PASS=hostinfo->pass;
	D_PORT=hostinfo->port;
	answer=NULL;
	ETag=NULL;
	Auth=NULL;
	D_PATH=NULL;
	D_FILE.name=NULL;
	D_FILE.fdesc=0;
	RealName=NewRealName=NULL;
	data=0;
	first=1;
	config.copy_ints(cfg);
	config.set_proxy_host(cfg->get_proxy_host());
	config.set_proxy_user(cfg->get_proxy_user());
	config.set_proxy_pass(cfg->get_proxy_pass());
	D_PROTO=copy_string(hostinfo->protocol);
	HTTP->init(config.get_proxy_host(),LOG,config.proxy_port,config.timeout);
	config.set_user_agent(cfg->get_user_agent());
	HTTP->set_user_agent(config.get_user_agent());
	HTTP->registr(config.get_proxy_user(),config.get_proxy_pass());
	((tHProxyClient *)(HTTP))->setup_host(HOST);
	return reconnect();
};

int tProxyDownload::get_size() {
	// Make a URL from available data
	char fullname[MAX_LEN];
	fullname[0]=0;
	strcat(fullname,D_PROTO);
	strcat(fullname,"://");
	if (USER) {
		strcat(fullname,USER);
		strcat(fullname,":");
		if (PASS) strcat(fullname,PASS);
		strcat(fullname,"@");
	};
	strcat(fullname,HOST);
	char temp[MAX_LEN];
	sprintf(temp,":%i",D_PORT);
	strcat(fullname,temp);
	strcat(fullname,D_PATH);
	if (D_PATH[strlen(D_PATH)-1]!='/')
		strcat(fullname,"/");
	strcat(fullname,D_FILE.name);
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
		if (USER && PASS) HTTP->set_auth(1);
		int temp=HTTP->get_size(fullname,answer);
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
	LOG->add(_("Could'nt get normal answer!"),LOG_ERROR);
	return -2;
};

tProxyDownload::~tProxyDownload() {
	if (D_PROTO) delete D_PROTO;
};
