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

enum FSEARCH_QUEUES_ENUM{
	DL_FS_STOP=0,
	DL_FS_WAIT,
	DL_FS_RUN,
	DL_FS_LAST
};

class tMain;

class tFtpSearchCtrl{
	tDList *queues[DL_FS_LAST];
	GtkCList *clist;
	tMain *parent;
	tMLog *log;
	void stop(tDownload *what);
	void remove_from_clist(tDownload *what);
 public:
	tFtpSearchCtrl();
	void init(GtkCList *widget,tMain *papa,tMLog *mylog);
	void cycle();
	void add(tDownload *what);
	void remove(tDownload *what);
	void reping(tDownload *what);
	~tFtpSearchCtrl();
};

#endif
