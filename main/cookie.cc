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
#include "cookie.h"
#include "locstr.h"
#include "config.h"
#include "var.h"
#include "ntlocale.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

tCookie::tCookie(){
	host=path=name=value=NULL;
	time_of_life=time_t(0);
};

void tCookie::set_time(char *what){
	sscanf(what,"%lu",&time_of_life);
};

void tCookie::set_host(char *what){
	if (host) delete(host);
	host=copy_string(what);
};

void tCookie::set_path(char *what){
	if (path) delete(path);
	path=copy_string(what);
};

void tCookie::set_name(char *what){
	if (name) delete(name);
	name=copy_string(what);
};

void tCookie::set_value(char *what){
	if (value) delete(value);
	value=copy_string(what);
};

void tCookie::init(char *a,char *b,char *c,char *d){
	set_host(a);
	set_path(b);
	set_name(c);
	set_value(d);
};

time_t tCookie::get_time(){ return time_of_life;};
char *tCookie::get_host(){ return host;};
char *tCookie::get_path(){ return path;};
char *tCookie::get_name(){ return name;};
char *tCookie::get_value(){ return value;};

void tCookie::print(){
	if (host) puts(host);
	if (path) puts(path);
	if (name) puts(name);
	if (value) puts(value);
};

tCookie::~tCookie(){
	if (host) delete(host);
	if (path) delete(path);
	if (name) delete(name);
	if (value) delete(value);
};

/*
 */
int tCookiesTree::compare_nodes(tAbstractSortNode *a,tAbstractSortNode *b){
	return(string_ended(((tCookie *)a)->get_host(),((tCookie *)b)->get_host()));
};

tCookie *tCookiesTree::find(char *what) {
	return find((tCookie **)(&Top),what);
};
	
tCookie *tCookiesTree::find(tCookie **begin,char *what) {
	tCookie **temp=begin;
	while (*temp) {
		int a=string_ended((*temp)->get_host(),what);
		if (a<0)
			temp=(tCookie **)&((*temp)->more);
		else {
			if (a==0) {
				return *temp;
			};
			temp=(tCookie **)&((*temp)->less);
		};
	};
	return NULL;
};

void tCookiesTree::load_cookies(){
	MainLog->add(_("Loading cookies"),LOG_WARNING | LOG_DETAILED);
	char *path=compose_path(HOME_VARIABLE,".netscape/cookies");
	int fd=open(path,O_RDONLY);
	if (fd>=0){
		char temp[MAX_LEN];
		while (read_string(fd,temp,MAX_LEN)){
			if (*temp!='#'){
				tCookie *cookie=new tCookie;
				char *data=new char[strlen(temp)+1];
				char *next_for_parse=extract_string(temp,data); //host
				cookie->set_host(data);
				next_for_parse=extract_string(next_for_parse,data);//path
				next_for_parse=extract_string(next_for_parse,data);
				cookie->set_path(data);
				next_for_parse=extract_string(next_for_parse,data);
				next_for_parse=extract_string(next_for_parse,data);
				cookie->set_time(data);
				next_for_parse=extract_string(next_for_parse,data);//name
				cookie->set_name(data);
				next_for_parse=extract_string(next_for_parse,data);//value
				cookie->set_value(data);
				if (*(cookie->get_path())!='/')
					delete(cookie);
				else
					add(cookie);
				delete(data);
			};
		};
		close(fd);
		MainLog->myprintf(LOG_OK|LOG_DETAILED,_("%i cookies loaded"),NUM);
	}else{
		MainLog->add(_("Can't open cookies file!"),LOG_ERROR|LOG_DETAILED);
	};
	delete(path);
};

tCookiesTree::~tCookiesTree(){
	while (Top){
		tCookie *temp=(tCookie *)Top;
		del(Top);
		delete(temp);
	};
};
