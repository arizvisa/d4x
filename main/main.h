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
#include "download.h"
#include "liststr.h"
#include "dlist.h"
#include "mdlist.h"
#include "meter.h"
#include "mainlog.h"
#include "speed.h"
#include "srvclt.h"

class tMain{
	unsigned int LastTime;
	GList *list_to_delete;
	int MsgQueue;
	tMsgServer *server;
	pthread_t server_thread_id;
	tSpeedQueue *SpeedScheduler;
	int LastReadedBytes;
	void case_download_completed(tDownload *what);
	void case_download_failed(tDownload *what);
	int run_new_thread(tDownload *what);
	int try_to_run_download(tDownload *what);
	void add_dir(tDownload *parent);
	void print_info(tDownload *what);
	void redirect(tDownload *what);
	void prepare_for_stoping(tDownload *what,tDList *list);
	void absolute_delete_download(tDList *where,tDownload *what);
	void del_all_from_list(tDList *list);
	unsigned int get_precise_time();
	void run_msg_server();
 public:
    	void init();
    	void init_main_log();
    	void speed();
	int complete();
        void main_circle();
        void del_completed();
	void del_fataled();
        void del_all();
        void load_defaults();
	void check_for_remote_commands();
        void append_list(tStringList *what);
        void redraw_logs();
        void reinit_main_log();
        void stop_download(tDownload *what);
        int delete_download(tDownload *what);
        void continue_download(tDownload *what);
    	int add_downloading(char *adr,char *where,char *name);
	int add_downloading(tDownload *what);
	void add_downloading_to(tDownload *what);
    	void add_download_message(tDownload *what);
    	void run(int argv, char **argc);
	void run_after_quit();
	void go_to_delete();
	void done();
};

void *download_last(void *);
int get_port_by_proto(char *proto);
int amount_of_downloads_in_queues();

extern tMLog *MainLog;
extern tMeter *GlobalMeter;
extern tMeter *LocalMeter;

extern tDList *DOWNLOAD_QUEUES[DL_TEMP];
//************************************************/
#endif
