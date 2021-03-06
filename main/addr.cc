/*	Downloader for X
 *	Copyright (C) 1999-2005 Koshelev Maxim
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
	{"whois++:",0,D_PROTO_UNKNOWN},
	{"socks:",0,D_PROTO_SOCKS}
};

tProtoInfo proto_infos[]={
	{"?",0,D_PROTO_UNKNOWN},
	{"ftp",21,D_PROTO_FTP},
	{"http",80,D_PROTO_HTTP},
	{"https",443,D_PROTO_HTTPS},
	{"search",80,D_PROTO_SEARCH},
	{"socks",0,D_PROTO_SOCKS}
};

int get_port_by_proto(int proto){
	return(proto_infos[proto].port);
};

int get_proto_by_name(const char *str){
	if (str==NULL) return D_PROTO_UNKNOWN;
	for (int i=D_PROTO_FTP;i<D_PROTO_LAST;i++)
		if (equal_uncase(str,proto_infos[i].name))
			return i;
	return D_PROTO_UNKNOWN;
};

const char *get_name_by_proto(int proto){
	if (proto>=D_PROTO_LAST || proto<D_PROTO_UNKNOWN)
		return(proto_infos[D_PROTO_UNKNOWN].name);
	return(proto_infos[proto].name);
};

static int get_proto_by_string(const char *str){
	if (str==NULL) return D_PROTO_UNKNOWN;
	if (begin_string_uncase(str,"www")) return D_PROTO_HTTP;
	if (begin_string_uncase(str,"ftp")) return D_PROTO_FTP;
	return D_PROTO_FTP;
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

#include <iostream>

namespace d4x{
	
	ShortURL::operator std::string() const{
		std::string portstr;
		if (port!=get_port_by_proto(proto)){
			char buf[30];
			sprintf(buf,":%i",port);
			portstr=buf;
		};
		if (proto==D_PROTO_FTP || params.empty())
			return(std::string(proto_infos[proto].name)+"://"+host+portstr+hexed_string(path/file));
		return(std::string(proto_infos[proto].name)+"://"+host+portstr+hexed_string(path/file)+"?"+hexed_string(params));
	};
	
	
	URL::URL(const std::string &_s){
		// PROTO://USER:PASS@HOST/PATH/FILE?PARAMS
		std::string::size_type p=_s.find("://");
		std::string protostr;
		if (p!=std::string::npos && p!=0){
			protostr=_s.substr(0,p);
			p+=3;
		}else{
			p=0;
		};
		
		if (protostr.empty())
			proto=get_proto_by_string(_s.c_str());
		else
			proto=get_proto_by_name(protostr.c_str());
		
		std::string::size_type p1=_s.find('/',p);
		if (p1!=std::string::npos){
			host=_s.substr(p,p1-p);
			p=_s.find('?',p1);
			std::string::size_type p2 = (proto==D_PROTO_HTTP||proto==D_PROTO_HTTPS)?_s.rfind('#'):std::string::npos;
			if (p!=std::string::npos){
				path=unhexed_string(_s.substr(p1,p-p1));
				if(p2==std::string::npos)
					params=_s.substr(p+1);
				else
					params=_s.substr(p+1,p2-p-1);
			}else{
				if (p2==std::string::npos)
					path=unhexed_string(_s.substr(p1));
				else
					path=unhexed_string(_s.substr(p1,p2-p1));
			};
		}else{
			host=_s.substr(p);
		};
		
		// host is USER:PASS@HOST:PORT
		// path is PATH/FILE

		p=host.find('@');
		if (p!=std::string::npos){
			user=host.substr(0,p);
			host=host.substr(p+1);
		};
		p=user.find(':');
		if (p!=std::string::npos){
			pass=user.substr(p+1);
			user=user.substr(0,p);
		};
		p=host.find(':');
		if (p!=std::string::npos){
			port=atoi(host.substr(p+1).c_str());
			host=host.substr(0,p);
		};
		p=path.rfind('/');
		if (p!=std::string::npos){
			file=path.substr(p+1);
			path=path.substr(0,p);
		};
		path=d4x::Path("/")/path;
		
		if (!port)
			port=get_port_by_proto(proto);
		
		if (proto==D_PROTO_FTP  && file.find('*')!=std::string::npos)
			mask=true;
		else
			mask=false;
	};
		
	URL &URL::operator=(const URL &_u){
		host=_u.host;
		user=_u.user;
		pass=_u.pass;
		path=_u.path;
		file=_u.file;
		params=_u.params;
		tag=_u.tag;
		proto=_u.proto;
		port=_u.port;
		mask=_u.mask;
		return *this;
	};
	
	bool URL::operator==(const URL &_u) const{
		if (strcasecmp(host.c_str(),_u.host.c_str()) ||
		    path!=_u.path || file!=_u.file || params!=_u.params ||
		    proto!=_u.proto || port!=port || user!=user)
			return false;
		return true;
	};
	bool URL::operator<(const URL &u_) const{
		return operator std::string()<std::string(u_);
	};
	
	bool URL::is_valid(){
		if (host.empty() || path.empty() || port==0) return false;
		return true;
	};
	
	URL::operator std::string() const{
		std::string portstr;
		std::string userstr;
		if (port!=get_port_by_proto(proto)){
			char buf[30];
			sprintf(buf,":%i",port);
			portstr=buf;
		};
		if (!user.empty() || !pass.empty()){
			if (pass.empty())
				userstr=user+"@";
			else
				userstr=user+":"+pass+"@";
		};
		if (proto==D_PROTO_FTP || params.empty())
			return(std::string(proto_infos[proto].name)+"://"+userstr+host+portstr+hexed_string(path/file));
		return(std::string(proto_infos[proto].name)+"://"+userstr+host+portstr+hexed_string(path/file)+"?"+hexed_string(params));
	};

	void URL::copy_host(const URL&_u){
		host=_u.host;
		pass=_u.pass;
		user=_u.user;
		port=_u.port;
		proto=_u.proto;
	};
	
	void URL::clear(){
		pass.clear();
		user.clear();
		host.clear();
		path.clear();
		file.clear();
		params.clear();
		mask=false;
		port=0;
		proto=D_PROTO_UNKNOWN;
	};
	
};
/*

/* parsing url 
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

/*------------------ end of temporary functions ------------ 

tAddr::tAddr() {
	proto=D_PROTO_UNKNOWN;
	port=0;
	mask=0;
};

tAddr::tAddr(const tAddr *a) {
	proto=D_PROTO_UNKNOWN;
	port=0;
	mask=0;
	copy(a);
};


tAddr::tAddr(const char *str){
	from_string(str);
};

void tAddr::from_string(const char *str){
	clear();
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
		const char *tmp=str+strlen(proto_name);
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
// parsing %xx -> CHAR and vice verse
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
// Parse # in http urls
	if (port==0)
		port=proto_infos[proto].port;
	host.set(host1);if (host1) delete[] host1;
	username.set(username1);if (username1) delete[] username1;
	pass.set(pass1);if (pass1) delete[] pass1;
	path.set(path1);if (path1) delete[] path1;
	file.set(file1);if (file1) delete[] file1;
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
void tAddr::compose_path2(char *aa, char *bb){
	char *cc=sum_strings(bb,"/",NULL);
	char *tmp=::compose_path(aa,cc);
	delete[] cc;
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
	if (host.get()==NULL) return (copy_string(""));
	char *URL=new char[strlen(proto_infos[proto].name)+strlen(host.get())+
	                   strlen(path.get())+strlen(file.get())+7+params_len+port_len];
	*URL=0;
	// Easy way to make URL from info  field
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

char *tAddr::url_parsed() {
	int params_len=(params.get()?strlen(params.get())+1:0);
	int port_len=port==proto_infos[proto].port?0:(int_to_strin_len(port)+1);
	char *rpath=unparse_percents(path.get());
	char *rfile=unparse_percents(file.get());;
	char *URL=new char[strlen(proto_infos[proto].name)+strlen(host.get())+
	                   strlen(rpath)+strlen(rfile)+7+params_len+port_len];
	*URL=0;
	// Easy way to make URL from info  field
	strcat(URL,proto_infos[proto].name);
	strcat(URL,"://");
	if (host.get()) strcat(URL,host.get());
	if (port_len)
		sprintf(URL+strlen(URL),":%i",port);
	if (rpath){
		if (!_str_first_char(rpath,'/')) strcat(URL,"/");
		strcat(URL,rpath);
		if (!_str_last_char(URL,'/')) strcat(URL,"/");
	};
	if (rfile) strcat(URL,rfile);
	if (params.get()){
		strcat(URL,"?");
		strcat(URL,params.get());
	};
	delete[] rfile;
	delete[] rpath;
	return URL;
};

char *tAddr::url_full(){
	int params_len=(params.get()?strlen(params.get())+1:0);
	int port_len=port==proto_infos[proto].port?0:(int_to_strin_len(port)+1);
	int auth_len=0;
	if (host.get()==NULL) return (copy_string(""));
	if (pass.get() && username.get())
		auth_len=strlen(pass.get())+strlen(username.get());
	char *URL=new char[strlen(proto_infos[proto].name)+strlen(host.get())+
			  strlen(path.get())+strlen(file.get())+7+params_len+
			  port_len+auth_len];
	*URL=0;
	//Easy way to make URL from info  field
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

void tAddr::copy_host(const tAddr *what){
	DBC_RETURN_IF_FAIL(what!=NULL);
	host.set(what->host.get());
	pass.set(what->pass.get());
	username.set(what->username.get());
	proto=what->proto;
	port=what->port;
};

void tAddr::copy(const tAddr *what){
	DBC_RETURN_IF_FAIL(what!=NULL);
	copy_host(what);
	file.set(what->file.get());
	path.set(what->path.get());
	params.set(what->params.get());
	mask=what->mask;
};

tAddr::~tAddr() {
};

void tAddr::clear(){
	port=0;
	host.set(NULL);
	path.set(NULL);
	file.set(NULL);
	username.set(NULL);
	pass.set(NULL);
	tag.set(NULL);
	params.set(NULL);
	mask=0;
	proto=0;
};
*/
/**********************************************/
