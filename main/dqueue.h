#ifndef _D4X_DOWNLOAD_QUEUE_HEADER_
#define _D4X_DOWNLOAD_QUEUE_HEADER_

#include "dlist.h"
#include "face/lod.h"
#include "locstr.h"
#include <pthread.h>

class d4xDownloadQueue:public tNode{
	tDList *queues[DL_TEMP];
public:
	int MAX_ACTIVE,TIME_FORMAT,NICE_DEC_DIGITALS;
	int AUTODEL_COMPLETED,AUTODEL_FAILED;
	int IamDefault;
	tQueue child;
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
	void inherit_settings(d4xDownloadQueue *papa);
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
	void del_s();
	void lock();
	void unlock();
	void lock_s();
	void unlock_s();
	void update(tDList *dl);
	~d4xDUpdate();
};

#endif // define _D4X_DOWNLOAD_QUEUE_HEADER_
