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
#ifndef T_MAIN
#define T_MAIN
#include <pthread.h>
#include <time.h>
#include "dlist.h"
#include "meter.h"
#include "mainlog.h"
#include "srvclt.h"
#include "fsearch.h"
#include "msgqueue.h"
#include "dqueue.h"

class tMain{
	int prev_speed_limit;
	unsigned int LastTime;
	GList *list_to_delete;
	tMsgServer *server;
	tMsgQueue *MsgQueue;
	tFtpSearchCtrl *ftpsearch;
	tMeter::BSize LastReadedBytes;
	int TO_WAIT_IF_HERE,DONTRY2RUN;
	void case_download_completed(tDownload *what);
	void case_download_failed(tDownload *what);

	void try_to_run_split(tDownload *what);
	void stop_split(tDownload *what);
	void check_split(tDownload *dwn);
	int try_to_run_download(tDownload *what);
	void absolute_delete_download(tDownload *what);

	void add_dir(tDownload *parent,int http=0);
	void print_info(tDownload *what);
	void redirect(tDownload *what,d4xDownloadQueue *dq);
	void del_all_from_list(int list,d4xDownloadQueue *queue=(d4xDownloadQueue *)NULL);
	unsigned int get_precise_time();
	void run_msg_server();
	void speed_calculation(tDownload *what);
	void run_without_face();
	void main_circle_first(tDownload *dw);
	void main_circle_second(tDownload *dwn);
	void init_qtree(tQueue *list,d4xDownloadQueue *papa=(d4xDownloadQueue *)NULL);
	void stop_all(tQueue *q);
	void stop_all_offline(tQueue *q);
	int try_to_switch_split(tDownload *dwn,tDownload *gp);
	int try_to_switch(tDownload *dwn);
	void prepare_for_stoping_pre(tDownload *what);
	void sizequery_run_first(d4xDownloadQueue *q);
 public:
	d4x::SpeedQueue *SpeedScheduler;
    	int init();
    	void init_main_log();
    	void speed();
        void main_circle();
	void main_circle_nano1();
	void main_circle_nano2();
        void del_completed(d4xDownloadQueue *queue=(d4xDownloadQueue *)NULL);
	void del_fataled(d4xDownloadQueue *queue=(d4xDownloadQueue *)NULL);
        void del_all();
	void rerun_failed();
        void load_defaults();
	void set_speed(int speed);
	void check_for_remote_commands();
        void redraw_logs();
        void reinit_main_log();
	void schedule_download(tDownload *what);
	/* manipulation by url */
	tDownload *find_url(const d4x::URL &adr);
	void move_to_sizequery(tDownload *what);
	void stop_download_url(const d4x::URL &adr);
	void delete_download_url(const d4x::URL &adr);
        void continue_download_url(const d4x::URL &adr);
	/* manipulations with downloads */
        void stop_download(tDownload *what);
        int delete_download(tDownload *what,int flag=0);
        void continue_download(tDownload *what);
    	int add_downloading(const char *adr,char *where=0,char *name=0,char *desc=0,const char *ref=0);
	tDownload *add_downloading(tDownload *what,int to_top=0);
	tDownload *add_downloading_to(tDownload *what,int to_top=0);
	void ftp_search(tDownload *what,int type=0);
	void ftp_search_name(char *name);
    	void add_download_message(tDownload *what);
    	void run(int argv, char **argc);
	int set_auto_run(int a);
	void run_after_quit();
	void done();
	void try_to_run_run(d4xDownloadQueue *papa);
	/* next method used by URL-manager too */
	void try_to_run_wait(d4xDownloadQueue *papa);
	void insert_into_wait_list(tDownload *what,d4xDownloadQueue *dq);
	/* next methods are public especialy for tFtpSearchCtrl */
	void post_stopping(tDownload *what);
	void prepare_for_stoping(tDownload *what);
	int run_new_thread(tDownload *what);
	void ftp_search_remove(tDownload *what);
	void ftp_search_reping(tDownload *what);
	void quit();
	void switch_offline_mode();
};

typedef void (*d4xQTreeFunc) (d4xDownloadQueue *dq,void *p);
void d4x_qtree_for_each(d4xQTreeFunc dothis,void *a);

void *download_last(void *);
int get_port_by_proto(char *proto);
int calc_curent_run(char *host,int port);
void create_new_queue(char *name,d4xDownloadQueue *papa=(d4xDownloadQueue *)NULL);
int d4x_only_one_queue();

extern tMLog *MainLog;
extern tMeter *GlobalMeter;
extern tMeter *LocalMeter;
extern tMeter *GraphMeter;
extern tMeter *GraphLMeter;

extern d4xDownloadQueue *D4X_QUEUE;
extern tQueue D4X_QTREE;

extern tMain _aa_;

//************************************************/
#endif
