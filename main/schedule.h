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
#ifndef _D4X_SCHEDULER_HEADER_
#define _D4X_SCHEDULER_HEADER_

#include <time.h>
#include "dlist.h"

enum D4X_SCHEDULED_ACTION{
	SACT_SET_SPEED,
	SACT_POPUP_WINDOW,
	SACT_EXIT,
	SACT_DEL_COMPLETED,
	SACT_DEL_FAILED,
	SACT_RUN_DOWNLOAD,
	SACT_PAUSE_DOWNLOAD,
	SACT_DELETE_DOWNLOAD,
	SACT_DEL_IF_COMPLETED,
	SACT_ADD_DOWNLOAD,
	SACT_SAVE_LIST,
	SACT_LAST
};

class tMain;

/* actions */

struct d4xSchedAction{
	time_t start_time,period;
	int retries; // -1 - unlimited, 0 - no retries
	d4xSchedAction *next,*prev;
	int lock; //locked if editor opened
	/* methods */
	virtual int type()=0; //type of action
	virtual int load(int fd);
	virtual int save(int fd);
	virtual void run(tMain *papa)=0;
	virtual ~d4xSchedAction();
};

struct d4xSASpeed:public d4xSchedAction{
	int speed; // 1,2,3 speed level
	int type();
	int load(int fd);
	int save(int fd);
	void run(tMain *papa);
};


struct d4xSAPopup:public d4xSchedAction{
	int type();
	int save(int fd);
	void run(tMain *papa);
};

struct d4xSAExit:public d4xSchedAction{
	int type();
	int save(int fd);
	void run(tMain *papa);
};

struct d4xSADelCompleted:public d4xSchedAction{
	int type();
	int save(int fd);
	void run(tMain *papa);
};

struct d4xSADelFailed:public d4xSchedAction{
	int type();
	int save(int fd);
	void run(tMain *papa);
};

struct d4xSAUrl:public d4xSchedAction{
	tAddr *url;
	d4xSAUrl(){url=(tAddr *)NULL;};
	int load(int fd);
	int save(int fd);
	~d4xSAUrl(){if (url) delete(url);};
};

struct d4xSADelDownload:public d4xSAUrl{
	d4xSADelDownload();
	int type();
	int save(int fd);
	void run(tMain *papa);
};

struct d4xSADelIfCompleted:public d4xSAUrl{
	d4xSADelIfCompleted();
	int type();
	int save(int fd);
	void run(tMain *papa);
};

struct d4xSARunDownload:public d4xSAUrl{
	d4xSARunDownload();
	int type();
	int save(int fd);
	void run(tMain *papa);
};

struct d4xSAStopDownload:public d4xSAUrl{
	d4xSAStopDownload();
	int type();
	int save(int fd);
	void run(tMain *papa);
};

struct d4xSAAddDownload:public d4xSchedAction{
	tDownload *dwn;
	d4xSAAddDownload();
	int type();
	int load(int fd);
	int save(int fd);
	void run(tMain *papa);
	~d4xSAAddDownload();
};

struct d4xSASaveList:public d4xSchedAction{
	tPStr path;
	int type();
	int load(int fd);
	int save(int fd);
	void run(tMain *papa);
};
/* scheduler */

class d4xScheduler{
	d4xSchedAction *FIRST; //sorted list of actions
 public:
	d4xScheduler();
	void run(tMain *papa);
	void add_scheduled(tDownload *what);
	void add_action(d4xSchedAction *act);
	void del_action(d4xSchedAction *act);
	void redraw();
	int load(int fd);
	int save(int fd);
	void load();
	void save();
	~d4xScheduler();
};

extern d4xScheduler *MainScheduler;

#endif
