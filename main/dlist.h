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

namespace d4x{
	template<typename T>
	class Triger{
		T current,old;
	public:
		Triger():current(0),old(0){};
		Triger(T &val):current(val),old(val){
		};
		operator T(){
			return current;
		};
		Triger &operator=(const Triger &tr){
			current=tr.current;
			old=tr.old;
			return *this;
		};
		Triger &operator=(const T &val){
			old=current;
			current=val;
			return *this;
		};
		bool operator!(){
			return(current!=old);
		};
		void reset(){
			old=current;
		};
		void clear(){
			old=-1;
			current=0;
		};
		void update(){
			old=current-1;
		};
	};
};

struct tTriger{
	fsize_t curent,old;
	void reset();
	void set(fsize_t a);
	void clear();
	void update();
	int change();
};



class tDefaultWL:public tWriterLoger{
	int fd;
	int fdlock;
	int overlap_flag;
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
	int is_overlaped();
	void set_log(tLog *log);
	fsize_t write(const void *buff, fsize_t len);
	fsize_t read(void *dst,fsize_t len);
	fsize_t shift(fsize_t shift,int mode);
	void truncate();
	std::string cookie(const char *host, const char *path);
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

struct d4xDwnLink:public tNode{
	tDownload *dwn,*papa;
	tQueue *q;
	d4xDwnLink(){dwn=NULL;papa=NULL;q=NULL;};
	d4xDwnLink(tDownload *a,tDownload *p){dwn=a;papa=p;};
	void print(){};
};

struct d4xSearchEngine;

struct tDownload:public tAbstractSortNode{
	GtkTreeIter *list_iter;
	tCfg *config;
	std::string Name2Save;
	int restart_from_begin;
	char fsearch; // 0 - just search, not 0 - arrange alternates
	tFileInfo finfo;
	d4x::URL info;
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
	time_t Start,Pause,Difference,HistoryTime;
	pthread_t thread_id;
	tDList *myowner;
	int status,action,status_cp;
	bool STOPPED_BY_USER;
	int protect;
	int sizequery;
	float Percent;
	d4x::Triger<fsize_t> Size,Attempt,ActStatus,Speed,Remain;
	fsize_t StartSize;
	int GTKCListRow;
	// to realise stack of items which is needed to be updated
	tDownload *next2update,*prev2update,*next2stop;
	//------------------------------------
private:
	int need_to_rename,im_first,im_last;
	d4x::URL RedirectURL;
	d4x::Path create_new_file_path();
	d4x::Path create_new_save_path();
	void make_file_names(char **name, char **guess);
	void check_local_file_time();
	void print_error(int err);
	void prepare_splits();
	d4x::Path make_path_to_file();
	void remove_links(d4xSearchEngine *engine);
	void sort_links();
	void http_check_redirect(bool removefiles);
	void ftp_search_sizes();
	int try_to_lock_fdesc();
	fsize_t init_segmentator(int fdesc,fsize_t cursize,char *name);
	void export_socket(tDownloader *what);
	void download_http_size();
	void download_ftp_size();
public:
	//------------------------------------
	d4xDwnLink *regex_match; //is neded for URL-manager's limits
	tDList *DIR;
	d4x::Speed *SpeedLimit;
	d4xSpeedCalc SpeedCalc;
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
	int http_check_settings(const d4x::URL &what);
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
	fsize_t filesize();

	int find_best_split();
	void set_initial_speedlimit();

	void save_to_config(int fd);
	int load_from_config(int fd);
	int owner();
	d4x::URL redirect_url();
	void update_trigers();
	void set_split_count(int num);
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
	tDownload *find(const d4x::URL &addr);
	void insert(tDownload *what);
	void init_pixmap(int a);
	void insert_before(tDownload *what,tDownload *where);
	void insert_if_absent(tDownload *what);
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
    DL_SIZEQUERY,
    DL_LIMIT,
    DL_TEMP
};

enum {
	ACTION_NONE,
	ACTION_DELETE,
	ACTION_CONTINUE,
	ACTION_STOP,
	ACTION_FAILED,
	ACTION_REAL_DELETE,
	ACTION_SIZEQUERY,
	ACTION_REPING
};
#endif
