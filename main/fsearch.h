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
#ifndef __MY_FTP_SEARCH_HEADER__
#define __MY_FTP_SEARCH_HEADER__

#include <gtk/gtk.h>
#include "dlist.h"
#include "mainlog.h"
#include <regex.h>

enum FSEARCH_QUEUES_ENUM{
	DL_FS_STOP=0,
	DL_FS_WAIT,
	DL_FS_RUN,
	DL_FS_LAST
};

struct d4xSearchEngine:public tNode{
	tPStr name,match;
	tPStr urlsize,urlnosize;
	int used;
	d4xSearchEngine():tNode(){used=0;};
	void print(){printf("%s\n%s\n",name.get(),match.get());};
	void prepare_url(tAddr *adr,int size,const char *file,int num);
};

struct d4xFtpRegex{
	char *left,*center,*right;
	d4xFtpRegex();
	int compile(const char *str,const char *file);
	int compile_regexes(regex_t *regs);
	char *cut(const char *str,regex_t *regs);
	void free();
	void print();
	~d4xFtpRegex();
};


class d4xEnginesList:public tQueue{
public:
	d4xSearchEngine *first();
	d4xSearchEngine *next();
	d4xSearchEngine *prev();
	d4xSearchEngine *get_by_num(int num);
	void names2array(char **arr);
	d4xSearchEngine *get_next_used_engine(d4xSearchEngine *cur);
	int load();
};

class tMain;

class tFtpSearchCtrl{
	tDList *queues[DL_FS_LAST];
	GtkTreeView *view;
	tMain *parent;
	tMLog *log;
	void stop(tDownload *what);
	void remove_from_clist(tDownload *what);
 public:
	tFtpSearchCtrl();
	void init(GtkTreeView *widget,tMain *papa,tMLog *mylog);
	void cycle();
	void add(tDownload *what);
	void remove(tDownload *what);
	void reping(tDownload *what);
	void stop_all_offline();
	~tFtpSearchCtrl();
};

extern d4xEnginesList D4X_SEARCH_ENGINES;

#endif
