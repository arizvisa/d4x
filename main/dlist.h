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
#include "segments.h"
#include "alt.h"

#define MINIMUM_SIZE_TO_SPLIT 102400

class tDEdit;

struct tTriger{
	fsize_t curent,old;
	void reset();
	void set(int a);
	void clear();
	void update();
	int change();
};



class tDefaultWL:public tWriterLoger{
	int fd;
	int fdlock;
	tLog *LOG;
	tSegmentator *segments;
	void fd_close();
 public:
	tDefaultWL();
	int lock_fd();
	void unlock_fd();
	void set_fd(int newfd,int lockstate=0);
	void set_segments(tSegmentator *newseg);
	int get_fd();
	void set_log(tLog *log);
	fsize_t write(const void *buff, fsize_t len);
	fsize_t read(void *dst,fsize_t len);
	fsize_t shift(fsize_t shift,int mode);
	void truncate();
	char *cookie(const char *host, const char *path);
	void cookie_set(tCookie *cookie);
	void log(int type, const char *str);
	~tDefaultWL();
};

class tDList;
struct tDownload;

struct d4xCondition{
	pthread_mutex_t my_mutex;
	int value;
	d4xCondition();
	void set_value(int val);
	int get_value();
	int dec();
	int inc();
	~d4xCondition();
};

struct tSplitInfo{
	int NumOfParts,thread_num;
	fsize_t FirstByte,LastByte;
	char failed,prepared,run;
	int stopcount,runcount;
	int alt;
	d4xCondition *cond;
	tDownload *next_part,*parent,*grandparent;
	tSplitInfo();
	void reset();
	~tSplitInfo();
};

struct tDownload:public tAbstractSortNode{
	tCfg *config;
	tPStr Name2Save;
	tFileInfo finfo;
	tAddr *info;
	tDownloader *who;
	d4xAltList *ALTS;
	tLog *LOG,*CurrentLog;// CurrentLog is used for splited downloads
	tWriterLoger *WL;
	tDEdit *editor;
	tNode *WFP; // used when run without interface
	tSegmentator *segments;
	tPStr Description;
	//------Split information-------------
	tSplitInfo *split;
	//------------------------------------
	time_t Start,Pause,Difference;
	pthread_t thread_id;
	tDList *myowner;
	int status,action,status_cp;
	int BLOCKED;
	int protect;
	float Percent;
	tTriger Size,Attempt,Status,Speed,Remain;
	fsize_t StartSize;
	int GTKCListRow;
	// to realise stack of items which is needed to be updated
	tDownload *next2update,*next2stop;
	//------------------------------------
	private:
	int need_to_rename,im_first,im_last;
	char *create_new_file_path();
	char *create_new_save_path();
	void make_file_names(char **name, char **guess);
	void check_local_file_time();
	void print_error(int err);
	void prepare_splits();
	char *make_path_to_file();
	void remove_links();
	void sort_links();
	void http_check_redirect();
	void ftp_search_sizes();
	int try_to_lock_fdesc();
	fsize_t init_segmentator(int fdesc,fsize_t cursize,char *name);
	void export_socket(tDownloader *what);
	public:
	//------------------------------------
	tDList *DIR;
	tSpeed *SpeedLimit;
	time_t ScheduleTime;
	//------------------------------------
	tDownload();
	int cmp(tAbstractSortNode *b);
	void delete_editor();
	void set_default_cfg();
	void copy(tDownload *what);
	void print();
	void convert_list_to_dir();
	void convert_list_to_dir2(tQueue *dir);
	/* downloading functions*/
	int http_check_settings(tAddr *what);
	void delete_who();
	void download_completed(int type);
	void download_failed();
	void http_recurse();
	void http_postload();
	void download_ftp();
	void download_http();
	void ftp_search();
	/*file manipulations*/
	void make_file_visible();
	void set_date_file();
	fsize_t create_file();
	int delete_file();
	void remove_tmp_files();
	int file_type();
	fsize_t get_loaded();
	fsize_t start_size();
	fsize_t filesize();
	
	void save_to_config(int fd);
	int load_from_config(int fd);
	int owner();
	tAddr *redirect_url();
	void update_trigers();
	~tDownload();
};


class d4xDownloadQueue;

class tDList:public tQueue{
	int OwnerKey;
	int Pixmap;
	void (*empty)();
	void (*non_empty)();
public:
	d4xDownloadQueue *PAPA;
	tDList();
	tDList(int key);
	int get_key();
	void insert(tDownload *what);
	void init_pixmap(int a);
	void insert_before(tDownload *what,tDownload *where);
	void del(tDownload *what);
	void forward(tDownload *what);
	void backward(tDownload *what);
	void dispose();
	void set_empty_func(void (*emp)(),void (*nonemp)());
	tDownload *last();
	tDownload *next();
	tDownload *prev();
	tDownload *first();
	~tDList();
};

#define DQV(arg) arg->myowner->PAPA->qv

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
	ACTION_FAILED,
	ACTION_REAL_DELETE,
	ACTION_REPING
};
#endif
