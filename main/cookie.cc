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

#include "cookie.h"
#include "locstr.h"
#include "var.h"
#include "ntlocale.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>
#include <ctype.h>

/* determine which cookies file is newer Netscape or Mozilla's one
   It's allowed to use non reentrant variance of readdir() cos here's first
   place where readdir() is used
 */
char *d4x_get_best_in_subdir(const char *path,time_t *a){
	DIR *d=opendir(path);
	char *best=NULL;
	time_t besttime=0;
	if (d){
		struct dirent *de=NULL;
		while((de=readdir(d))){
			if (de->d_name && !equal(de->d_name,"..") &&
			    !equal(de->d_name,".")){
				char *tmppath=sum_strings(path,"/",de->d_name,NULL);
				struct stat s;
				stat(tmppath,&s);
				if (S_ISDIR(s.st_mode)){
					char *p=sum_strings(tmppath,"/","cookies.txt",NULL);
					if (stat(p,&s)==0 && S_ISREG(s.st_mode) &&
					    (best==NULL ||  besttime<s.st_mtime)){
						if (best) delete[] best;
						best=p;
						besttime=s.st_mtime;
					}else
						delete[] p;
				};
				delete[] tmppath;
			};
		};
		closedir(d);
	};
	*a=besttime;
	return(best);
};

char *d4x_get_best_cookies(){
	char *tmppath=sum_strings(HOME_VARIABLE,"/.mozilla",NULL);
	DIR *d=opendir(tmppath);
	delete[] tmppath;
	char *best=NULL;
	time_t besttime=0;
	if (d){
		struct dirent *de=NULL;
		while((de=readdir(d))){
			if (de->d_name && !equal(de->d_name,"..") &&
			    !equal(de->d_name,".")){
				tmppath=sum_strings(HOME_VARIABLE,"/.mozilla/",de->d_name,NULL);
				struct stat s;
				stat(tmppath,&s);
				if (S_ISDIR(s.st_mode)){
					time_t a;
					char *p1=d4x_get_best_in_subdir(tmppath,&a);
					if (p1 && a>besttime){
						if (best) delete[] best;
						best=p1;
						besttime=a;
					};
				};
				delete[] tmppath;
			};
		};
		closedir(d);
	};
	tmppath=sum_strings(HOME_VARIABLE,"/.netscape/cookies",NULL);
	struct stat s;
	if (stat(tmppath,&s)==0 && S_ISREG(s.st_mode) &&
	    (s.st_mtime>besttime || best==NULL)){
		if (best) delete[] best;
		best=tmppath;
	}else
		delete[] tmppath;
	return(best);
};
/*******************************************************/

tCookie::tCookie(){
	time_of_life=time_t(0);
	myown=0;
};

void tCookie::set_time(time_t t){
	time_of_life=t;
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

int tCookie::parse(char *str,char *srchost,char *srcpath){
	char *cur=str;
	while(cur && *cur){
		cur=skip_spaces(cur);
		char *n=index(cur,';');
		if (n) *n=0;
		char *eq=index(cur,'=');
		if (eq){
			*eq=0;
			char *a=copy_string(cur);
			char *b=a+strlen(a)-1;
			while (isspace(*b) && b>a){
				*b=0;
				b--;
			};
			b=skip_spaces(eq+1);
			char *c=copy_string(b);
			b=c+strlen(c)-1;
			while (isspace(*b) && b>c){
				*b=0;
				b--;
			};
			if (name.get()==NULL){
				name.set(a);
				value.set(c);
			};
			if (equal_uncase(a,"expires")){
				time_of_life=ctime_to_time(c);
			};
			if (equal_uncase(a,"path")){
				path.set(c);
			};
			if (equal_uncase(a,"domain")){
				host.set(c);
			};
			delete[] a;
			delete[] c;
			*eq='=';
		};
		if (n){
			*n=';';
			n+=1;
		};
		cur=n;
	};
	if (time_of_life==0) time_of_life=time(NULL)+30*3600;
	if (srchost && host.get()==NULL) host.set(srchost);
	if (srcpath && path.get()==NULL){
		char *p=sum_strings("/",srcpath,NULL);
		path.set(p);
		delete[] p;
	};
	if (name.get() && value.get() && path.get() && host.get())
		return(0);
	return(1);
};

int cmp_back(const char *a,const char *b){
	const char *bb=b+strlen(b)-1;
	const char *aa=a+strlen(a)-1;
	while(bb>b && aa>a){
		if (*bb!=*aa) return(*bb-*aa);
		bb--;aa--;
	};
	if (bb>b) return(-1);
	if (aa>a) return(1);
	return(0);
};

int tCookie::cmp(tAbstractSortNode *b){
	DBC_RETVAL_IF_FAIL(b!=NULL,0);
//	printf("%s %s %i\n",((tCookie *)b)->host.get(),host.get(),cmp_back(((tCookie *)b)->host.get(),host.get()));
	return(cmp_back(((tCookie *)b)->host.get(),host.get()));
};

tCookie::~tCookie(){
};

/*
 */

tCookie *tCookiesTree::find_exact(tCookie *cookie){
	tCookie *temp=find(cookie->host.get());
	while (temp){
		if (equal(cookie->path.get(),temp->path.get()) &&
		    equal(cookie->name.get(),temp->name.get())){
			return temp;
		};
		temp=find(temp,cookie->host.get());
	};
	return(NULL);
};

tCookie *tCookiesTree::find(const char *what) {
	DBC_RETVAL_IF_FAIL(what!=NULL,NULL);
	if (Top && string_ended(((tCookie*)Top)->host.get(),what)==0)
		return((tCookie*)Top);
//	if (Top) printf("--- find from %s ---\n",((tCookie *)Top)->host.get());
	return find((tCookie *)Top,what);
};

tCookie *tCookiesTree::find(tCookie *begin,const char *what) {
	tCookie *temp=begin;
	while (temp) {
//		printf("%s %s ",temp->host.get(),what);
		int a=cmp_back(what,temp->host.get());
		if (a<0){
			temp=(tCookie *)(temp->more);
		}else {
			temp=(tCookie *)(temp->less);
		};
		if (temp && string_ended(temp->host.get(),what)==0){
			return temp;
		};
	};
	return NULL;
};

char *D4X_COOKIES_FILE="cookies.txt";

void tCookiesTree::load_from_file(int fd,int myown){
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
			cookie->myown=myown;
			if (cookie->path.get()==NULL || cookie->path.get()[0]!='/' ||
			    find_exact(cookie)!=NULL)
				delete(cookie);
			else{
				add(cookie);
			};
			delete[] data;
		};
	};
};


void tCookiesTree::save_cookie(int fd,tCookie *what){
	if (what->myown){
		f_wstr(fd,what->host.get());
		f_wstr(fd,"\tFALSE\t");
		char *path=unparse_percents(what->path.get());
		f_wstr(fd,path);
		delete[] path;
		f_wstr(fd,"\tFALSE\t");
		static char str[MAX_LEN];
		sprintf(str,"%ld",(long int)(what->get_time()));
		f_wstr(fd,str);
		f_wchar(fd,'\t');
		path=unparse_percents(what->name.get());
		f_wstr(fd,path);
		delete[] path;
		f_wchar(fd,'\t');
		path=unparse_percents(what->value.get());
		f_wstr(fd,path);
		delete[] path;
		f_wchar(fd,'\n');
	};
	if (what->less) save_cookie(fd,(tCookie *)(what->less));
	if (what->more) save_cookie(fd,(tCookie *)(what->more));
};

void tCookiesTree::save_cookies(){
	char *path=sum_strings(HOME_VARIABLE,"/",CFG_DIR,"/",D4X_COOKIES_FILE,NULL);
	int fd=open(path,O_TRUNC | O_CREAT |O_RDWR,S_IRUSR | S_IWUSR);
	delete[] path;
	if (fd>=0){
		if (Top) save_cookie(fd,(tCookie*)Top);
		close(fd);
	};
};


void tCookiesTree::load_cookies(){
	char *path=d4x_get_best_cookies();
	MainLog->add(_("Loading cookies"),LOG_WARNING | LOG_DETAILED);
	if (path!=NULL){
		int fd=open(path,O_RDONLY);
		if (fd>=0){
			load_from_file(fd);
			close(fd);
			MainLog->myprintf(LOG_OK|LOG_DETAILED,_("%i cookies loaded from browsers' files"),NUM);
		}else{
			MainLog->myprintf(LOG_ERROR|LOG_DETAILED,_("Can't open cookies file %s!"),path);
		};
		delete[] path;
	};
	path=sum_strings(HOME_VARIABLE,"/",CFG_DIR,"/",D4X_COOKIES_FILE,NULL);
	int fd=open(path,O_RDONLY);
	delete[] path;
	if (fd>=0){
		load_from_file(fd,1);
		close(fd);
		MainLog->myprintf(LOG_OK|LOG_DETAILED,_("%i cookies loaded"),NUM);
	};
};

tCookiesTree::~tCookiesTree(){
	while (Top){
		tCookie *temp=(tCookie *)Top;
		del((tCookie*)Top);
		delete(temp);
	};
};
