#ifndef DOWNLOADER_CONDITIONS
#define DOWNLOADER_CONDITIONS

#include "queue.h"
#include "dlist.h"
#include <time.h>

class tAbstractCondition:public tNode{
 protected:
	tDownload *parent;
	void save_begin(int fd);
	void save_end(int fd);
	virtual void save_data(int fd)=0;
 public:
	tAbstractCondition();
	virtual void set(tDownload *where);
	virtual int check()=0; //return zero if failed
	virtual int load(int fd)=0; // loading from config
	virtual void save(int fd); // saving to config
	virtual int  type()=0;
	virtual ~tAbstractCondition();
};

class tTimeCond:public tAbstractCondition{
 protected:
	time_t date;
	void save_data(int fd);
 public:
	tTimeCond();
	void set_time(time_t when);
	void set(tDownload *where);
	int type();
	int load(int fd);
	int check();
};

void cond_load_from_config(int fd,tDownload *where);

enum CONDITIONS_ENUM{
	COND_TIME,
	COND_LAST
};

#endif
