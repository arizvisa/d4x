/*	WebDownloader for X-Window
 *	Copyright (C) 1999 Koshelev Maxim
 *	This Program is free but not GPL!!! You can't modify it
 *	without agreement with autor. You can't distribute modified
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

struct tTwoStrings{
    char *one;
    char *two;
    tTwoStrings();
    void zero();
    ~tTwoStrings();
};

class tMain{
    unsigned int LastTime;
    int MsgQueue;
    int LastReadedBytes;
    void split_string(char *what,char *delim,tTwoStrings *out);
    void case_download_completed(tDownload *what);
    void case_download_failed(tDownload *what);
    int run_new_thread(tDownload *what);
    int complete();
    void add_dir(tDownload *parent);
    void make_file_visible(tDownload *what);
    void print_info(tDownload *what);
    void redirect(tDownload *what);
    void prepare_for_stoping(tDownload *what,tDList *list);
    void absolute_delete_download(tDList *where,tDownload *what);
    unsigned int get_precise_time();
    tSpeedQueue *SpeedScheduler;
    public:
    	void init();
    	void init_main_log();
    	void speed();
    	tAddr *analize(char *what);
        void main_circle();
        void del_completed();
	    void del_fataled();
        void del_all();
        void load_defaults();
        void append_list(tStringList *what);
        void redraw_logs();
        void reinit_main_log();
        void stop_download(tDownload *what);
        int delete_download(tDownload *what);
        void continue_download(tDownload *what);
    	int add_downloading(char *adr,char *where,char *name);
    	void add_download_message(tDownload *what);
    	void run(int argv, char **argc);
		void done();
};

void *download_last(void *);
int get_port_by_proto(char *proto);

extern tMLog *MainLog;
extern tMeter *GlobalMeter;
extern tMeter *LocalMeter;

extern tDList *RunList;
extern tDList *StopList;
extern tDList *CompleteList;
extern tDList *PausedList;
extern tDList *WaitList;
extern tDList *WaitStopList;
//************************************************/
#endif