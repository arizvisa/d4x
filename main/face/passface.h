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
#ifndef MY_FACE_FOR_PASS
#define MY_FACE_FOR_PASS

#include <gtk/gtk.h>
#include "../dlist.h"
#include <regex.h>


struct tLimitDownload:public tDownload{
	int cur_limit;
	tLimitDownload():tDownload(),cur_limit(0){};
	tQueue limited; // needed to store all downloads marked by this regex
	tQueue lim_run; // needed to recalc run downloads when regex is changed
};

class tFacePass{
protected:
	int do_not_run;
	GtkWidget *window;
	GtkTreeView *view;
	GtkListStore *list_store;
	GtkWidget *button;
	GtkWidget *add_button;
	GtkWidget *del_button;
	tDList dlist,addlist;
	void del_row(GtkTreeIter *inter);
	void show_url(tLimitDownload *dwn);
	void calc_matched_run(tLimitDownload *dwn);
	void calc_matched_run_rec(tQueue *q,tLimitDownload *dwn,regex_t *reg);
	void rerun_wait_queues(tQueue *q);
	void free_matched(tLimitDownload *dwn);
	tLimitDownload *find_match(tDownload *what);
public:
	tFacePass();
	void init();
	void edit_row(GtkTreeIter *iter);
	void apply_dialog();
	void addlist_del(tDownload *dwn);
	void addlist_add(tLimitDownload *dwn);
	void redraw_url(tLimitDownload *dwn);
	void delete_rows();
	void open_dialog();
	void limit_dec(tDownload *what);
	void limit_inc(tDownload *what);
	void limit_to_run(tDownload *what);
	int match_and_check(tDownload *what,int move=0);
	int limit_check(tDownload *what);
	void save();
	void load();
	void close();
	void set_cfg(tDownload *dwn);
	void recalc_run(tLimitDownload *dwn);
	void stop_matched(tDownload *dwn);
	void set_do_not_run(int a);
	~tFacePass();
};

extern tFacePass *FaceForPasswords;

#endif
