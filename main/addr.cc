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
#include "addr.h"
#include "locstr.h"
#include "var.h"
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "dbc.h"

struct tProtoInfo{
	char *name;
	int port;
	int proto;
};

tProtoInfo begin_protos[]={
	{"cid:",0,D_PROTO_UNKNOWN},
	{"clsid:",0,D_PROTO_UNKNOWN},
	{"file:",0,D_PROTO_UNKNOWN},
	{"finger:",0,D_PROTO_UNKNOWN},
	{"ftp:",21,D_PROTO_FTP},
	{"gopher:",0,D_PROTO_UNKNOWN},
	{"hdl:",0,D_PROTO_UNKNOWN},
	{"http:",80,D_PROTO_HTTP},
	{"https:",0,D_PROTO_HTTPS},
	{"ilu:",0,D_PROTO_UNKNOWN},
	{"ior:",0,D_PROTO_UNKNOWN},
	{"irc:",0,D_PROTO_UNKNOWN},
	{"java:",0,D_PROTO_UNKNOWN},
	{"javascript:",0,D_PROTO_UNKNOWN},
	{"lifn:",0,D_PROTO_UNKNOWN},
	{"mail:",0,D_PROTO_UNKNOWN},
	{"mailto:",0,D_PROTO_UNKNOWN},
	{"mid:",0,D_PROTO_UNKNOWN},
	{"news:",0,D_PROTO_UNKNOWN},
	{"nntp:",0,D_PROTO_UNKNOWN},
	{"path:",0,D_PROTO_UNKNOWN},
	{"prospero:",0,D_PROTO_UNKNOWN},
	{"rlogin:",0,D_PROTO_UNKNOWN},
	{"service:",0,D_PROTO_UNKNOWN},
	{"shttp:",0,D_PROTO_UNKNOWN},
	{"snews:",0,D_PROTO_UNKNOWN},
	{"stanf:",0,D_PROTO_UNKNOWN},
	{"telnet:",0,D_PROTO_UNKNOWN},
	{"tn3270:",0,D_PROTO_UNKNOWN},
	{"wais:",0,D_PROTO_UNKNOWN},
	{"whois++:",0,D_PROTO_UNKNOWN}
};

tProtoInfo proto_infos[]={
	{"?",0,D_PROTO_UNKNOWN},
	{"ftp",21,D_PROTO_FTP},
	{"http",80,D_PROTO_HTTP},
	{"https",443,D_PROTO_HTTPS},
	{"search",80,D_PROTO_SEARCH}
};

int global_url(char *url) {
	DBC_RETVAL_IF_FAIL(url!=NULL,0);
	for (unsigned int i=0;i<sizeof(begin_protos)/sizeof(tProtoInfo);i++){
		if (begin_string_uncase(url,begin_protos[i].name)){
			return(1);
		};
	};
	return(0);
};

/* parsing url */
struct tTwoStrings{
    char *one;
    char *two;
    tTwoStrings();
    void zero();
    ~tTwoStrings();
};

tTwoStrings::tTwoStrings() {
	one=two=NULL;
};

void tTwoStrings::zero() {
	one=two=NULL;
};

tTwoStrings::~tTwoStrings() {};

int get_proto_by_name(char *str){
	if (str==NULL) return D_PROTO_UNKNOWN;
	for (int i=D_PROTO_FTP;i<D_PROTO_LAST;i++)
		if (equal_uncase(str,proto_infos[i].name))
			return i;
	return D_PROTO_UNKNOWN;
};

int get_port_by_proto(int proto){
	return(proto_infos[proto].port);
};

char *get_name_by_proto(int proto){
	if (proto>=D_PROTO_LAST || proto<D_PROTO_UNKNOWN)
		return(proto_infos[D_PROTO_UNKNOWN].name);
	return(proto_infos[proto].name);
};

static int get_proto_by_string(char *str){
	if (str==NULL) return D_PROTO_UNKNOWN;
	if (begin_string_uncase(str,"www")) return D_PROTO_HTTP;
	if (begin_string_uncase(str,"ftp")) return D_PROTO_FTP;
	return D_PROTO_FTP;
};

static void split_string(char *what,char *delim,tTwoStrings *out) {
	if (what==NULL || delim==NULL) return;
	char * where=strstr(what,delim);
	if (where) {
		int len=strlen(where),len1=strlen(delim);
		out->two=copy_string(where+len1);
		len1=strlen(what)-len;
		out->one=copy_string2(what,len1);
	} else {
		out->two=copy_string(what);
		out->one=NULL;
	};
	delete[] what;
};

static void rsplit_string(char *what,char delim,tTwoStrings *out) {
	if (what==NULL) return;
	char * where=rindex(what,delim);
	if (where) {
		int len=strlen(where),len1=1;
		out->two=copy_string(where+len1);
		len1=strlen(what)-len;
		out->one=copy_string2(what,len1);
	} else {
		out->two=copy_string(what);
		out->one=NULL;
	};
	delete[] what;
};

/*------------------ end of temporary functions ------------ */

tAddr::tAddr() {
	proto=D_PROTO_UNKNOWN;
	port=0;
	mask=0;
};

void tAddr::from_string(char *str){
	char *host1=NULL,*username1=NULL,*pass1=NULL,*path1=NULL,*file1=NULL;
	char *proto_name=NULL;
	proto=D_PROTO_UNKNOWN;
	port=0;
	mask=0;
	if (str==NULL) return;
	for (unsigned int i=0;i<sizeof(begin_protos)/sizeof(tProtoInfo);i++){
		if (begin_string_uncase(str,begin_protos[i].name)){
			proto_name=begin_protos[i].name;
			port=begin_protos[i].port;
			proto=begin_protos[i].proto;
			break;
		};
	};
	if (proto_name) {
		char *tmp=str+strlen(proto_name);
		while (*tmp=='/') tmp+=1;
		host1=copy_string(tmp);
	} else {
		proto=get_proto_by_string(str);
		host1=copy_string(str);
	};
	tTwoStrings pair;
	if (!host1) {
		return;
	};
	split_string(host1,"/",&pair);
	if (pair.one) {
		host1=pair.one;
		file1=pair.two;
	} else {
		host1=pair.two;
		file1=pair.one;
	};
	rsplit_string(host1,'@',&pair);
	host1=pair.two;
	username1=pair.one;
	if (username1) {
		split_string(username1,":",&pair);
		username1=pair.one;
		pass1=pair.two;
	} else {
		username1=NULL;
		pass1=NULL;
	};
	if (file1) {
		if (proto==D_PROTO_HTTP){
			char *tmp=rindex(file1,'#');
			if (tmp) {
				*tmp=0;
				tmp=file1;
				file1=copy_string(tmp);
				delete[] tmp;
			};
			char *prom=index(file1,'?');
			if (prom){
				params.set(prom+1);
				*prom=0;
			};
		};
/* parsing %xx -> CHAR and vice verse */
		char *prom=parse_percents(file1);
		delete[] file1;
		file1=prom;
		prom=rindex(file1,'/');
		if (prom) {
			path1=copy_string(prom+1);
			*prom=0;
			prom=path1;
			path1=copy_string(file1);
			delete[] file1;
			file1=prom;
		};
	} else {
		file1=copy_string("");
	};
	if (!path1) path1=copy_string("");
	split_string(host1,":",&pair);
	if (pair.one) {
		sscanf(pair.two,"%i",&port);
		delete[] pair.two;
		host1=pair.one;
	} else {
		port=0;
		host1=pair.two;
	};
	if (proto==D_PROTO_FTP  && index(file1,'*'))
		mask=1;
	/* Parse # in http urls
	 */
	if (port==0)
		port=proto_infos[proto].port;
	host.set(host1);if (host1) delete[] host1;
	username.set(username1);if (username1) delete[] username1;
	pass.set(pass1);if (pass1) delete[] pass1;
	path.set(path1);if (path1) delete[] path1;
	file.set(file1);if (file1) delete[] file1;
};

tAddr::tAddr(char *str){
	from_string(str);
};

void tAddr::print() {
        printf("protocol: %s\n",proto_infos[proto].name);
	if (host.get()) printf("host: %s\n",host.get());
	if (path.get()) printf("path: %s\n",path.get());
	if (file.get()) printf("file: %s\n",file.get());
	if (username.get()) printf("username: %s\n",username.get());
	if (pass.get()) printf("pass: %s\n",pass.get());
	printf("port: %i\n",port);
};

static int _str_first_char(const char *a,char b){
	if (a && *a==b) return 1;
	return 0;
};

static int _str_last_char(const char *a,char b){
	if (a){
		int len=strlen(a);
		if (len && a[len-1]==b) return 1;
	};
	return 0;
};

void tAddr::save_to_config(int fd){
	f_wstr_lf(fd,"URL:");
	f_wstr(fd,proto_infos[proto].name);
	f_wstr(fd,"://");
	if (username.get() && !equal(username.get(),DEFAULT_USER)){
		f_wstr(fd,username.get());
		f_wstr(fd,":");
		f_wstr(fd,pass.get());
		f_wstr(fd,"@");
	};
	f_wstr(fd,host.get());
	if (port!=proto_infos[proto].port){
		char port_str[MAX_LEN];
		g_snprintf(port_str,MAX_LEN,"%d",port);
		f_wstr(fd,":");
		f_wstr(fd,port_str);
	};
	if (!_str_first_char(path.get(),'/'))
		f_wstr(fd,"/");
	if (path.get())
		f_wstr(fd,path.get());
	if (!_str_last_char(path.get(),'/'))
		f_wstr(fd,"/");
	f_wstr(fd,file.get());
	if (params.get()){
		f_wstr(fd,"?");
		f_wstr(fd,params.get());
	};
	f_wstr(fd,"\n");
};

void tAddr::save_to_description(int fd){
	f_wstr(fd,proto_infos[proto].name);
	f_wstr(fd,"://");
	f_wstr(fd,host.get());
	if (port!=proto_infos[proto].port){
		char port_str[MAX_LEN];
		g_snprintf(port_str,MAX_LEN,"%d",port);
		f_wstr(fd,":");
		f_wstr(fd,port_str);
	};
	if (!_str_first_char(path.get(),'/'))
		f_wstr(fd,"/");
	if (path.get())
		f_wstr(fd,path.get());
	if (!_str_last_char(path.get(),'/'))
		f_wstr(fd,"/");
	f_wstr(fd,file.get());
	if (params.get()){
		f_wstr(fd,"?");
		f_wstr(fd,params.get());
	};
};


void tAddr::compose_path(char *aa, char *bb){
	char *tmp=::compose_path(aa,bb);
	if (tmp && *tmp=='/')
		path.set(tmp+1);
	else
		path.set(tmp);
	if (tmp) delete[] tmp;
};

void tAddr::file_del_sq(){
	char *tmp=index(file.get(),'#');
	if (tmp) {
		*tmp=0;
	};
};

void tAddr::make_url(char *where){
	DBC_RETURN_IF_FAIL(where!=NULL);
	*where=0;
	strcat(where,proto_infos[proto].name);
	strcat(where,"://");
	if (username.get() && !equal(username.get(),DEFAULT_USER)) {
		strcat(where,username.get());
		strcat(where,":");
		if (pass.get()) strcat(where,pass.get());
		strcat(where,"@");
	};
	strcat(where,host.get());
	if (port!=proto_infos[proto].port){
		char data[MAX_LEN];
		sprintf(data,":%i",port);
		strcat(where,data);
	};
	strcat(where,path.get());
	if (!_str_last_char(path.get(),'/'))
		strcat(where,"/");
	strcat(where,file.get());
};

char *tAddr::pathfile(){
	if (*(path.get()))
		return(sum_strings("/",path.get(),"/",file.get(),NULL));
	return(sum_strings("/",file.get(),NULL));
};

char *tAddr::url() {
	int params_len=(params.get()?strlen(params.get())+1:0);
	int port_len=port==proto_infos[proto].port?0:(int_to_strin_len(port)+1);
	char *URL=new char[strlen(proto_infos[proto].name)+strlen(host.get())+
	                   strlen(path.get())+strlen(file.get())+7+params_len+port_len];
	*URL=0;
	/* Easy way to make URL from info  field
	 */
	strcat(URL,proto_infos[proto].name);
	strcat(URL,"://");
	if (host.get()) strcat(URL,host.get());
	if (port_len)
		sprintf(URL+strlen(URL),":%i",port);
	if (path.get()){
		if (!_str_first_char(path.get(),'/')) strcat(URL,"/");
		strcat(URL,path.get());
		if (!_str_last_char(URL,'/')) strcat(URL,"/");
	};
	if (file.get()) strcat(URL,file.get());
	if (params.get()){
		strcat(URL,"?");
		strcat(URL,params.get());
	};
	return URL;
};

char *tAddr::url_full(){
	int params_len=(params.get()?strlen(params.get())+1:0);
	int port_len=port==proto_infos[proto].port?0:(int_to_strin_len(port)+1);
	int auth_len=0;
	if (pass.get() && username.get())
		auth_len=strlen(pass.get())+strlen(username.get());
	char *URL=new char[strlen(proto_infos[proto].name)+strlen(host.get())+
			  strlen(path.get())+strlen(file.get())+7+params_len+
			  port_len+auth_len];
	*URL=0;
	/* Easy way to make URL from info  field
	 */
	strcat(URL,proto_infos[proto].name);
	strcat(URL,"://");
	if (auth_len){
		strcat(URL,username.get());
		strcat(URL,":");
		strcat(URL,pass.get());
		strcat(URL,"@");
	};
	if (host.get()) strcat(URL,host.get());
	if (port_len)
		sprintf(URL+strlen(URL),":%i",port);
	if (path.get()){
		if (!_str_first_char(path.get(),'/')) strcat(URL,"/");
		strcat(URL,path.get());
		if (!_str_last_char(URL,'/')) strcat(URL,"/");
	};
	if (file.get()) strcat(URL,file.get());
	if (params.get()){
		strcat(URL,"?");
		strcat(URL,params.get());
	};
	return URL;
};

int tAddr::is_valid(){
	if (host.get()==NULL || path.get()==NULL || file.get()==NULL) return 0;
	return 1;
};

int tAddr::cmp(tAddr *b){
	DBC_RETVAL_IF_FAIL(b!=NULL,0);
	if (!equal_uncase(host.get(),b->host.get()) ||
	    !equal(path.get(),b->path.get()) ||
	    !equal(file.get(),b->file.get()) ||
	    !equal(params.get(),b->params.get()) ||
	    proto!=b->proto)
		return 0;
	if (proto==D_PROTO_FTP && !equal(username.get(),b->username.get()))
		return 0;
	return 1;
};

void tAddr::copy_host(tAddr *what){
	DBC_RETURN_IF_FAIL(what!=NULL);
	host.set(what->host.get());
	pass.set(what->pass.get());
	username.set(what->username.get());
	proto=what->proto;
	port=what->port;
};

void tAddr::copy(tAddr *what){
	DBC_RETURN_IF_FAIL(what!=NULL);
	copy_host(what);
	file.set(what->file.get());
	path.set(what->path.get());
	params.set(what->params.get());
	mask=what->mask;
};

tAddr::~tAddr() {
};

/**********************************************/
