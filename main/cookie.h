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
#ifndef DOWNLOADER_COOKIES
#define DOWNLOADER_COOKIES

#include "sort.h"
#include <time.h>
#include "locstr.h"

struct tCookie:public tAbstractSortNode{
	protected:
	time_t time_of_life;
	public:
	tPStr host,path,name,value;
	tCookie();
	void set_time(char *what);
	void init(char *a,char *b,char *c,char *d);
	time_t get_time();
	int cmp(tAbstractSortNode *a);
	void print();
	~tCookie();
};

class tCookiesTree:public tAbstractSortTree{
 public:
	tCookie *find(const char *path);
	tCookie *find(tCookie **begin,const char *path);
	void add(tCookie *what);
	void del(tCookie *what);
	void load_cookies();
	~tCookiesTree();
};
#endif
