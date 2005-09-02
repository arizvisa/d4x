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
#ifndef _D4X_QUEUE_TREE_HEADER_
#define _D4X_QUEUE_TREE_HEADER_

#include <gtk/gtk.h>
#include "../dqueue.h"

struct d4xQsTree{
	GtkTreeIter *row_to_color,*drop_to_row;
	GtkTreeView *view;
	GtkTreeStore *store;
	GtkWidget *menu1,*menu2;
	GtkWidget *dialog,*dialog_entry;
	GtkWidget *prefs;
	GtkWidget *columns_nums1,*columns_nums2,*columns_nums3,*columns_nums4;
	GtkWidget *columns_time1,*columns_time2;
	GtkWidget *columns_speed1,*columns_speed2;
	GtkWidget *del_completed,*del_fataled,*max_threads,*speed_limit;
	GtkWidget *name,*path_entry;
	int create_mode;
	d4xQsTree():store(NULL){};
	void init();
	void init_menus();
	void add(d4xDownloadQueue *what,d4xDownloadQueue *papa=(d4xDownloadQueue *)NULL);
	void del(d4xDownloadQueue *what);
	void update(d4xDownloadQueue *what);
	void update_speed(d4xDownloadQueue *what);
	void switch_remote(d4xDownloadQueue *what);
	void switch_to(d4xDownloadQueue *what);
	void select_row(GtkTreeIter *iter);
	void popup_menu(GdkEvent *event,int selected);
	void delete_queue();
	void create_init(int mode=0);
	void create_ok();
	void create_cancel();
	void prefs_init();
	void prefs_ok();
	void prefs_cancel();
	void drop_from(GtkTreeView *view);
	tDownload *get_download(GtkTreeView *view,GtkTreeIter *iter);
	void drag_motion(GtkTreeIter *iter);
	void move_to(tDownload *where);
	d4xDownloadQueue *selected();
};

#endif //_D4X_QUEUE_TREE_HEADER_
