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
#include "var.h"
#include "ntlocale.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

tCookie::tCookie(){
	time_of_life=time_t(0);
};

void tCookie::set_time(char *what){
	sscanf(what,"%lu",&time_of_life);
};

void tCookie::init(char *a,char *b,char *c,char *d){
	host.set(a);
	path.set(b);
	name.set(c);
	value.set(d);
};

time_t tCookie::get_time(){ return time_of_life;};

void tCookie::print(){
	if (host.get()) puts(host.get());
	if (path.get()) puts(path.get());
	if (name.get()) puts(name.get());
	if (value.get()) puts(value.get());
};

int tCookie::cmp(tAbstractSortNode *b){
	DBC_RETVAL_IF_FAIL(b!=NULL,0);
	return(strcmp(((tCookie *)b)->host.get(),host.get()));
/*
	if (r) return(r);
	r=strcmp(((tCookie *)b)->path.get(),path.get());
	if (r) return(r);
	return(strcmp(((tCookie *)b)->name.get(),name.get()));
*/
};

tCookie::~tCookie(){
};

/*
 */

tCookie *tCookiesTree::find(const char *what) {
	DBC_RETVAL_IF_FAIL(what!=NULL,NULL);
	if (Top && string_ended(((tCookie*)Top)->host.get(),what)==0)
		return((tCookie*)Top);
	return find((tCookie **)(&Top),what);
};
	
tCookie *tCookiesTree::find(tCookie **begin,const char *what) {
	tCookie **temp=begin;
	while (*temp) {
//		(*temp)->print();
		int a=strcmp(what,(*temp)->host.get());
		if (a<0)
			temp=(tCookie **)&((*temp)->more);
		else {
			temp=(tCookie **)&((*temp)->less);
		};
		if (*temp && string_ended((*temp)->host.get(),what)==0){
			return *temp;
		};
	};
	return NULL;
};


void tCookiesTree::add(tCookie *what){
	tCookie *tmp=find(what->host.get());
	if (tmp){
		if ((what->next=tmp->next))
			what->next->prev=what;
		what->prev=tmp;
		tmp->next=what;
	}else{
		tAbstractSortTree::add(what);
		what->next=what->prev=NULL;
	};
};

void tCookiesTree::del(tCookie *what){
	tCookie *tmp=find(what->host.get());
	if (tmp){
		if (tmp==what){
			tCookie *stay=(tCookie *)(what->next);
			tAbstractSortTree::del(what);
			if (stay)
				add(stay);
		}else{
			tmp=(tCookie *)(tmp->next);
			while(tmp){
				if (tmp==what){
					if ((tmp->prev->next=tmp->next))
						tmp->next->prev=tmp->prev;
					break;
				};
				tmp=(tCookie *)(tmp->next);
			};
		};
	};
};

void tCookiesTree::load_cookies(){
	MainLog->add(_("Loading cookies"),LOG_WARNING | LOG_DETAILED);
	char *path=compose_path(HOME_VARIABLE,".netscape/cookies");
	int fd=open(path,O_RDONLY);
	if (fd>=0){
		char temp[MAX_LEN];
		while (f_rstr(fd,temp,MAX_LEN)){
			if (*temp!='#'){
				tCookie *cookie=new tCookie;
				char *data=new char[strlen(temp)+1];
				char *next_for_parse=extract_string(temp,data); //host
				cookie->host.set(data);
				next_for_parse=extract_string(next_for_parse,data);//path
				next_for_parse=extract_string(next_for_parse,data);
				cookie->path.set(data);
				next_for_parse=extract_string(next_for_parse,data);
				next_for_parse=extract_string(next_for_parse,data);
				cookie->set_time(data);
				next_for_parse=extract_string(next_for_parse,data);//name
				cookie->name.set(data);
				next_for_parse=extract_string(next_for_parse,data);//value
				cookie->value.set(data);
				if (cookie->path.get() && cookie->path.get()[0]!='/')
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
		del((tCookie*)Top);
		delete(temp);
	};
};
