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
#ifndef DOWNLOADER_COOKIES
#define DOWNLOADER_COOKIES

#include "sort.h"
#include <time.h>
#include "locstr.h"
#include "mutex.h"

struct tCookie:public tAbstractSortNode{
	protected:
	time_t time_of_life;
	public:
	int myown;
	tPStr host,path,name,value;
	tCookie();
	void set_time(char *what);
	void set_time(time_t t);
	void init(char *a,char *b,char *c,char *d);
	time_t get_time();
	int cmp(tAbstractSortNode *a);
	int parse(char *str,char *srchost,char *srcpath);
	void print();
	~tCookie();
};

class tCookiesTree:public tAbstractSortTree{
	void load_from_file(int fd,int myown=0);
	void save_cookie(int fd,tCookie *what);
 public:
	d4xMutex lock;
	tCookie *find(const char *path);
	tCookie *find(tCookie *begin,const char *path);
	tCookie *find_exact(tCookie *cookie);
	void load_cookies();
	void save_cookies();
	~tCookiesTree();
};
#endif
