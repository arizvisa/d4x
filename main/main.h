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

class tMain{
	int prev_speed_limit;
	unsigned int LastTime;
	GList *list_to_delete;
	tMsgServer *server;
	tMsgQueue *MsgQueue;
	tFtpSearchCtrl *ftpsearch;
	pthread_t server_thread_id;
	tSpeedQueue *SpeedScheduler;
	int LastReadedBytes;
	void case_download_completed(tDownload *what);
	void case_download_failed(tDownload *what);

	void try_to_run_split(tDownload *what);
	void stop_split(tDownload *what);
	int get_status_split(tDownload *what);
	int get_split_loaded(tDownload *what);
	int try_to_run_download(tDownload *what);
	void absolute_delete_download(tDList *where,tDownload *what);

	void add_dir(tDownload *parent);
	void print_info(tDownload *what);
	void redirect(tDownload *what);
	void del_all_from_list(tDList *list);
	unsigned int get_precise_time();
	void run_msg_server();
	void speed_calculation(tDownload *what);
	void run_without_face();
	void main_circle_first();
	void main_circle_second();
	void insert_into_wait_list(tDownload *what);
	void append_list(tStringList *what);
 public:
    	void init();
    	void init_main_log();
    	void speed();
	int complete();
        void main_circle();
        void del_completed();
	void del_fataled();
        void del_all();
	void rerun_failed();
        void load_defaults();
	void check_for_remote_commands();
        void redraw_logs();
        void reinit_main_log();
        void stop_download(tDownload *what);
        int delete_download(tDownload *what,int flag);
        void continue_download(tDownload *what);
    	int add_downloading(char *adr,char *where,char *name,char *desc=(char *)NULL);
	int add_downloading(tDownload *what);
	void ftp_search(tDownload *what);
	void add_downloading_to(tDownload *what);
    	void add_download_message(tDownload *what);
    	void run(int argv, char **argc);
	void run_after_quit();
	void go_to_delete();
	void done();
	/* next methods are public especialy for tFtpSearchCtrl */
	void prepare_for_stoping(tDownload *what,tDList *list);
	int run_new_thread(tDownload *what);
	void ftp_search_remove(tDownload *what);
};

void *download_last(void *);
int get_port_by_proto(char *proto);
int amount_of_downloads_in_queues();
int calc_curent_run(char *host,int port);

extern tMLog *MainLog;
extern tMeter *GlobalMeter;
extern tMeter *LocalMeter;

extern tDList *DOWNLOAD_QUEUES[DL_TEMP];

//************************************************/
#endif
