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

struct tCookie:public tAbstractSortNode{
	protected:
	char *host,*path,*name,*value;
	time_t time_of_life;
	public:
	tCookie();
	void set_time(char *what);
	void set_host(char *what);
	void set_path(char *what);
	void set_name(char *what);
	void set_value(char *what);
	void init(char *a,char *b,char *c,char *d);
	time_t get_time();
	char *get_host();
	char *get_name();
	char *get_path();
	char *get_value();
	void print();
	~tCookie();
};

class tCookiesTree:public tAbstractSortTree{
 protected:
	int compare_nodes(tAbstractSortNode *a,tAbstractSortNode *b);
 public:
	tCookie *find(char *path);
	tCookie *find(tCookie **begin,char *path);
	void load_cookies();
	~tCookiesTree();
};
#endif
