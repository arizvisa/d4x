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
#include "addr.h"
#include "locstr.h"
#include "var.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

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

int get_port_by_proto(char *proto) {
	if (proto) {
		if (equal_uncase(proto,"ftp")) return 21;
		if (equal_uncase(proto,"http")) return 80;
	};
	return 21;
};

static void split_string(char *what,char *delim,tTwoStrings *out) {
	if (what==NULL || delim==NULL) return;
	char * where=strstr(what,delim);
	if (where) {
		int len=strlen(where),len1=strlen(delim);
		out->two=copy_string(where+len1);
		len1=strlen(what)-len;
		out->one=copy_string(what,len1);
	} else {
		out->two=copy_string(what);
		out->one=NULL;
	};
	delete(what);
};

static void rsplit_string(char *what,char delim,tTwoStrings *out) {
	if (what==NULL) return;
	char * where=rindex(what,delim);
	if (where) {
		int len=strlen(where),len1=1;
		out->two=copy_string(where+len1);
		len1=strlen(what)-len;
		out->one=copy_string(what,len1);
	} else {
		out->two=copy_string(what);
		out->one=NULL;
	};
	delete(what);
};

/*------------------ end of temporary functions ------------ */

tAddr::tAddr() {
	protocol=host=username=pass=path=file=NULL;
	port=0;
	mask=0;
};

tAddr::tAddr(char *str){
	protocol=host=username=pass=path=file=NULL;
	port=0;
	mask=0;
	if (str==NULL) return;
	char *what=copy_string(str);
	tTwoStrings pair;
	split_string(what,"://",&pair);
	if (pair.one) {
		protocol=pair.one;
	} else {
		protocol=copy_string(DEFAULT_PROTO);
	};
	host=pair.two;
	if (!host) {
		return;
	};
	split_string(host,"/",&pair);
	if (pair.one) {
		host=pair.one;
		file=pair.two;
	} else {
		host=pair.two;
		file=pair.one;
	};
	rsplit_string(host,'@',&pair);
	host=pair.two;
	username=pair.one;
	if (username) {
		split_string(username,":",&pair);
		username=pair.one;
		pass=pair.two;
	} else {
		username=NULL;
		pass=NULL;
	};
	if (file) {
		char *tmp=parse_percents(file);
		if (tmp) {
			delete file;
			file=tmp;
		} else
			delete tmp;
		char *prom=rindex(file,'/');
		if (prom) {
			path=copy_string(prom+1);
			*prom=0;
			prom=path;
			path=copy_string(file);
			delete file;
			file=prom;
		};
	} else {
		file=copy_string("");
	};
	if (!path) path=copy_string("");
	split_string(host,":",&pair);
	if (pair.one) {
		sscanf(pair.two,"%i",&port);
		delete pair.two;
		host=pair.one;
	} else {
		port=0;
		host=pair.two;
	};
	if (equal_uncase(protocol,"ftp") && index(file,'*'))
		mask=1;
	/* Parse # in http urls
	 */
	if (equal_uncase(protocol,"http") && file!=NULL) {
		char *tmp=index(file,'#');
		if (tmp) {
			*tmp=0;
			tmp=file;
			file=copy_string(tmp);
			delete(tmp);
		};
	};
	if (port==0)
		port=get_port_by_proto(protocol);
};

void tAddr::print() {
	if (protocol) printf("protocol: %s\n",protocol);
	if (host) printf("host: %s\n",host);
	if (path) printf("path: %s\n",path);
	if (file) printf("file: %s\n",file);
	if (username) printf("username: %s\n",username);
	if (pass) printf("pass: %s\n",pass);
	printf("port: %i\n",port);
};

void tAddr::save_to_config(int fd){
	write(fd,"URL:\n",strlen("URL:\n"));
	write(fd,protocol,strlen(protocol));
	write(fd,"://",strlen("://"));
	if (username && !equal(username,DEFAULT_USER)){
		write(fd,username,strlen(username));
		write(fd,":",strlen(":"));
		write(fd,pass,strlen(pass));
		write(fd,"@",strlen("@"));
	};
	write(fd,host,strlen(host));
	char port_str[MAX_LEN];
	g_snprintf(port_str,MAX_LEN,"%d",port);
	write(fd,":",strlen(":"));
	write(fd,port_str,strlen(port_str));
	if (path && *path!='/')
		write(fd,"/",strlen("/"));
	int temp=path==NULL ? 0:strlen(path);
	write(fd,path,temp);
	if (path && temp && path[temp-1]!='/')
		write(fd,"/",strlen("/"));
	write(fd,file,strlen(file));
	write(fd,"\n",strlen("\n"));
};

void tAddr::set_host(char *what){
	if (host) delete(host);
	host=copy_string(what);
};
void tAddr::set_proto(char *what){	
	if (protocol) delete(protocol);
	protocol=copy_string(what);
};
void tAddr::set_username(char *what){	
	if (username) delete(username);
	username=copy_string(what);
};
void tAddr::set_pass(char *what){	
	if (pass) delete(pass);
	pass=copy_string(what);
};
void tAddr::set_path(char *what){	
	if (path) delete(path);
	path=copy_string(what);
};
void tAddr::set_file(char *what){	
	if (file) delete(file);
	file=copy_string(what);
};

void tAddr::compose_path(char *aa, char *bb){
	if (path) delete(path);
	path=::compose_path(aa,bb);
};

void tAddr::file_del_sq(){
	char *tmp=index(file,'#');
	if (tmp) {
		*tmp=0;
		tmp=file;
		file=copy_string(tmp);
		delete(tmp);
	};
};

void tAddr::make_url(char *where){
	*where=0;
	strcat(where,protocol);
	strcat(where,"://");
	if (username && !equal(username,"anonymous")) {
		strcat(where,username);
		strcat(where,":");
		if (pass) strcat(where,pass);
		strcat(where,"@");
	};
	strcat(where,host);
	if ((equal_uncase(protocol,"ftp")  && port!=21) ||
	        (equal_uncase(protocol,"http") && port!=80)) {
		char data[MAX_LEN];
		sprintf(data,":%i",port);
		strcat(where,data);
	};
	strcat(where,path);
	if (path[strlen(path)-1]!='/')
		strcat(where,"/");
	strcat(where,file);
};

char *tAddr::url() {
	char *URL=new char[strlen(protocol)+strlen(host)+
	                   strlen(path)+strlen(file)+7];
	*URL=0;
	/* Easy way to make URL from info  field
	 */
	if (protocol) strcat(URL,protocol);
	strcat(URL,"://");
	if (host) strcat(URL,host);
	if (path){
		if (path[0]!='/') strcat(URL,"/");
		strcat(URL,path);
		int len=strlen(path);
		if (len>0 && path[len-1]!='/')
			strcat(URL,"/");
	};
	if (file) strcat(URL,file);
	return URL;
};

void tAddr::copy_host(tAddr *what){
	set_proto(what->protocol);
	set_host(what->host);
	set_pass(what->pass);
	set_username(what->username);
	port=what->port;
};

tAddr::~tAddr() {
	if (protocol) delete(protocol);
	if (path) delete(path);
	if (pass) delete(pass);
	if (username) delete(username);
	if (host) delete(host);
	if (file) delete(file);
};

/**********************************************/
