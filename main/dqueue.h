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
#ifndef _D4X_DOWNLOAD_QUEUE_HEADER_
#define _D4X_DOWNLOAD_QUEUE_HEADER_

#include "dlist.h"
#include "face/lod.h"
#include "locstr.h"
#include <pthread.h>

class d4xDownloadQueue:public tNode{
	tDList *queues[DL_TEMP];
public:
	int MAX_ACTIVE,TIME_FORMAT,SPEED_FORMAT,NICE_DEC_DIGITALS;
	int AUTODEL_COMPLETED,AUTODEL_FAILED;
	int IamDefault;
	int inserted;
	GtkTreeIter tree_iter;
	tQueue child;
	d4xSpeedCalc speed;
	d4xDownloadQueue *parent;
	d4xQueueView qv;
	tPStr save_path;
	tPStr name;
	d4xDownloadQueue();
	~d4xDownloadQueue();
	void done();
	void print(){};
	void reset_empty_func();
	void set_defaults();
	void init_pixmaps();
	int count(int q=DL_ALONE);
	int current_run(char *host,int port);
	tDownload *first(int q);
	tDownload *last(int q);
	int is_first(int q,tDownload *f);
	void forward(tDownload *what);
	void backward(tDownload *what);
	void insert_before(tDownload *what,tDownload *where);
	void replace_list(tDList *list,int q);
	void add(tDownload *what,int where=DL_WAIT);
	void del(tDownload *what);
	tDList *get_queue(int q);
	void subq_add(d4xDownloadQueue *what);
	void subq_del(d4xDownloadQueue *what);
	void update();
	void save_to_config(int fd);
	void save_to_config_list(int fd);
	int load_from_config(int fd);
	int load_from_config_list(int fd);
	void inherit_settings(d4xDownloadQueue *papa,const char *path=NULL);
};

class d4xDUpdate{
	pthread_mutex_t mylock,mylock_s;
	void add_without_lock(tDownload *dwn);
public:
	tDownload *first,*last;
	tDownload *first_s,*last_s;
	d4xDUpdate();
	void add(tDownload *dwn,int status);
	void add(tDownload *dwn);
	void del();
	void del(tDownload *dwn);
	void del_s();
	void lock();
	void unlock();
	void lock_s();
	void unlock_s();
	void update(tDList *dl);
	~d4xDUpdate();
};

int d4x_run_or_wait_downloads();
#endif // define _D4X_DOWNLOAD_QUEUE_HEADER_
