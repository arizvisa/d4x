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
#ifndef LIST_DONWLOAD
#define LIST_DONWLOAD

#include "download.h"
#include "queue.h"
#include "sort.h"
#include "liststr.h"
#include "log.h"
#include "speed.h"
#include <time.h>
#include <pthread.h>
#include "addr.h"

#define MINIMUM_SIZE_TO_SPLIT 102400

class tDEdit;

struct tTriger{
	int curent,old;
	void reset();
	void set(int a);
	void clear();
	void update();
	int change();
};

class tDefaultWL:public tWriterLoger{
	int fd;
	tLog *LOG;
 public:
	tDefaultWL();
	void set_fd(int newfd);
	int get_fd();
	void set_log(tLog *log);
	int write(const void *buff, int len);
	int shift(int shift);
	char *cookie(const char *host, const char *path);
	void log(int type, const char *str);
	~tDefaultWL();
};

class tDList;
struct tDownload;

struct tSplitInfo{
	int NumOfParts,FirstByte,LastByte;
	int status;
	tDownload *next_part,*parent;
	tSplitInfo();
	~tSplitInfo();
};

struct tDownload:public tAbstractSortNode{
	tCfg config;
	tFileInfo finfo;
	tAddr *info;
	tDownloader *who;
	tLog *LOG;
	tWriterLoger *WL;
	tDEdit *editor;
	//------Split information-------------
	tSplitInfo *split;
	//------------------------------------
	time_t Start,Pause;
	pthread_t thread_id;
	int status,owner,action;
	int NanoSpeed;
	int GTKCListRow;
	tTriger Percent,Size,Attempt,Status,Speed,Remain;
	//------------------------------------
//	tQueue *conditions;
	private:
	char *create_new_file_path();
	char *create_new_save_path();
	public:
	//------------------------------------
	tDList *DIR;
	tSpeed *SpeedLimit;
	time_t ScheduleTime;
	//------------------------------------
	tDownload();
	void clear();
	void delete_editor();
	void set_default_cfg();
	void print();
	void convert_list_to_dir();
	void convert_list_to_dir2(tStringList *dir);
	/* downloading functions*/
	int http_check_settings(tAddr *what);
	void delete_who();
	void download_completed(int type);
	void download_failed();
	void recurse_http();
	void download_ftp();
	void download_http();
	/*file manipulations*/
	void make_file_visible();
	void set_date_file();
	int create_file();
	int delete_file();
	int file_type();

	void prepare_next_split();
	void save_to_config(int fd);
	int load_from_config(int fd);
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
		void dispose();
		tDownload *last();
		tDownload *next();
		tDownload *prev();
		tDownload *first();
		int owner(tDownload *which);
		~tDList();
};

void make_dir_hier(char *path);
void make_dir_hier_without_last(char *path);

enum {
    DL_ALONE=0,
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
	ACTION_STOP,
	ACTION_FAILED
};
#endif
