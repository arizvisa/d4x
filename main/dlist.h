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
#ifndef LIST_DONWLOAD
#define LIST_DONWLOAD

#include "queue.h"
#include "sort.h"
#include "liststr.h"
#include "log.h"
#include "download.h"
#include "speed.h"
#include <time.h>
#include <pthread.h>


struct tAddr{
    char *protocol,*host,*username,*pass,*path,*file;
    int port;
    int mask;
    tAddr();
    void print();
    ~tAddr();
};

class tDEdit;

struct tTriger{
	int curent,old;
	void reset();
	void set(int a);
	void clear();
	void update();
	int change();
};

class tDList;

struct tDownload:public tAbstractSortNode{
    tCfg config;
    tFileInfo finfo;
    tAddr *info;
    tDownloader *who;
    tLog *LOG;
    tDEdit *editor;
    //------------------------------------
    time_t Start,Pause;
    pthread_t thread_id;
    int status,owner,action;
    int NanoSpeed;
    int GTKCListRow;
    tTriger Percent,Size,Attempt,Status,Speed;
	//------------------------------------
private:
    char *SavePath;
    char *SaveName;
public:
	char *get_SavePath(){return SavePath;};
	char *get_SaveName(){return SaveName;};
	void set_SavePath(char *what);
	void set_SaveName(char *what);
    //------------------------------------
    tDList *DIR;
    tSpeed *SpeedLimit;
    time_t ScheduleTime;
    //------------------------------------
    tDownload();
    void clear();
    void delete_editor();
    void print();
    void convert_list_to_dir();
    void convert_list_to_dir2();
    void make_file_visible();
    int create_file();
    void set_date_file();
    void update_trigers();
    ~tDownload();
};

class tDList:public tQueue{
	int OwnerKey;
	int Pixmap;
    public:
		tDList();
		tDList(int key);
		void insert(tDownload *what);
		void init_pixmap(int a);
		void insert_before(tDownload *what,tDownload *where);
		void del(tDownload *what);
		void forward(tDownload *what);
		void backward(tDownload *what);
		tDownload *last();
		tDownload *next();
		tDownload *prev();
		tDownload *first();
		int owner(tDownload *which);
		~tDList();
};

void make_url_from_download(tDownload *what,char *where);
char * make_simply_url(tDownload *what);
void make_dir_hier(char *path);

enum {
    DL_ALONE,
    DL_RUN,
    DL_STOP,
    DL_WAIT,
    DL_PAUSE,
    DL_COMPLETE,
    DL_STOPWAIT,
    DL_TEMP
};

enum {
	ACTION_NONE,
	ACTION_DELETE,
	ACTION_CONTINUE,
	ACTION_STOP
};
#endif