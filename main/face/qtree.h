#ifndef _D4X_QUEUE_TREE_HEADER_
#define _D4X_QUEUE_TREE_HEADER_

#include <gtk/gtk.h>
#include "columns.h"
#include "../dqueue.h"

struct d4xQsTree{
	gint drop_to_row,row_to_color;
	GtkCTree *tree;
	GtkWidget *menu1,*menu2;
	GtkWidget *dialog,*dialog_entry;
	GtkWidget *prefs;
	GtkWidget *columns_nums1,*columns_nums2,*columns_nums3,*columns_nums4;
	GtkWidget *columns_time1,*columns_time2;
	GtkWidget *del_completed,*del_fataled,*max_threads;
	GtkWidget *name,*path_entry;
	tColumnsPrefs columns_order;
	int create_mode;
	void init();
	void init_menus();
	void add(d4xDownloadQueue *what,d4xDownloadQueue *papa=NULL);
	void del(d4xDownloadQueue *what);
	void update(d4xDownloadQueue *what);
	void switch_to(d4xDownloadQueue *what);
	void select_row(int row);
	void popup_menu(GdkEvent *event);
	void delete_queue();
	void create_init(int mode=0);
	void create_ok();
	void create_cancel();
	void prefs_init();
	void prefs_ok();
	void prefs_cancel();
	void drop_from(GtkWidget *clist);
	void drag_motion(int row);
	void move_to(tDownload *where);
	d4xDownloadQueue *selected();
};

#endif //_D4X_QUEUE_TREE_HEADER_
