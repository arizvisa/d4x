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
#ifndef MY_LIST_OF_DOWNLOADS
#define MY_LIST_OF_DOWNLOADS

#include <gtk/gtk.h>
#include "../dlist.h"

struct d4xWFNode:public tNode{
	tDownload *dwn;
	void print(){};
};

struct tColumn{
	int type;
	int visible;
	int size;
};

enum {
	STATUS_COL=0,
	FILE_COL,
	FILE_TYPE_COL,
	FULL_SIZE_COL,
	DOWNLOADED_SIZE_COL,
	REMAIN_SIZE_COL,
	PERCENT_COL,
	SPEED_COL,
	TIME_COL,
	ELAPSED_TIME_COL,
	PAUSE_COL,
	TREAT_COL,
	DESCRIPTION_COL,
	URL_COL,
	NOTHING_COL
};

struct d4xQVPrefs{
	tTriger dformat;
	int tformat;
	tColumn cols[NOTHING_COL+1];
	d4xQVPrefs();
};

enum {
	TARGET_URL,
	TARGET_DND_TEXT
};

enum STATUS_PIXMAPS_ENUM{
	PIX_WAIT=0,
	PIX_STOP,
	PIX_STOP_WAIT,
	PIX_RUN,
	PIX_RUN1,
	PIX_RUN2,
	PIX_RUN3,
	PIX_RUN4,
	PIX_RUN5,
	PIX_RUN6,
	PIX_RUN7,
	PIX_RUN8,
	PIX_RUN_BAD,
	PIX_RUN_BAD1,
	PIX_RUN_BAD2,
	PIX_RUN_BAD3,
	PIX_RUN_BAD4,
	PIX_RUN_BAD5,
	PIX_RUN_BAD6,
	PIX_RUN_BAD7,
	PIX_RUN_BAD8,
	PIX_RUN_PART,
	PIX_RUN_PART1,
	PIX_RUN_PART2,
	PIX_RUN_PART3,
	PIX_RUN_PART4,
	PIX_RUN_PART5,
	PIX_RUN_PART6,
	PIX_RUN_PART7,
	PIX_RUN_PART8,
	PIX_COMPLETE,
	PIX_PAUSE,
	PIX_UNKNOWN
};

struct d4xQueueView{
private:
	int move_success;
	char *wildcard;
	void remove_wf(tDownload *what);
	void add_wf(tDownload *what);
//	void init_sort_buttons();
//	void set_column_justification (int col, GtkJustification justify);
//	GtkWidget *d4xQueueView::get_column_widget(int col);
public:
	tDownload *last_selected;
	GtkListStore *list_store;
	GtkWidget *ListOfDownloads;
	tQueue ListOfDownloadsWF;
	float current_shift;
	d4xQVPrefs prefs;
	int LoDSortFlag;
	d4xQueueView();
	~d4xQueueView();
	void toggle_column_visibility(int a);
	void popup_columns_visibility_menu(GdkEventButton *event);
	void get_sizes();
	void init();
	void add(tDownload *what);
	void add_first(tDownload *what);
	void remove(tDownload *what);
	void update(tDownload *what);
	void change_data(GtkTreeIter *iter,int column,gchar *data);
	void set_percent(GtkTreeIter *iter,float percent);
	void set_desc(tDownload *what);
	void set_color(tDownload *what);
	void set_filename(tDownload *what);

	gint get_height();
	void set_height();
	void print_size(tDownload *what);

	void set_pixmap(tDownload *what,int type);
	void set_pixmap(GtkTreeIter *iter,int type);
	void set_pixmap(tDownload *what);
	void redraw_pixmap(GtkTreeIter *iter);

	void set_run_icon(tDownload *what);
	int rows();

	void move_up();
	void move_down();
	void move_download_up(GtkTreeIter *iter);
	void move_download_down(GtkTreeIter *iter);
	int move_selected_up();
	int move_selected_down();
	void move_selected_home();
	void move_selected_end();
	void unselect_all();
	void select_all();
	void invert_selection();
	void select(tDownload *dwn);
	void real_select(int type,char *wildcard);
	int get_row_num(tDownload *dwn);

	void init_select_window(int type=0);
//	void swap(tDownload *a,tDownload *b);
//	void rebuild_wait();
//	void sort(int how);

	tDownload *get_download(GtkTreeIter *iter);

	void open_logs();
	void continue_opening_logs();
	void set_shift(float shift);
	void get_adj();
	void move_to(tDownload *dwn);

	void stop_downloads();
	void delete_downloads(int flag=0);
	void continue_downloads(int from_begin=0);
	void inv_protect_flag();
	void save_to_config(int fd);
	int load_from_config(int fd);
	void inherit_settings(d4xQueueView *papa);
	void redraw_icons();
	/* may be better describe friends for next funcs? */
	void select_by_wildcard(GtkTreeIter *iter);
	void unselect_by_wildcard(GtkTreeIter *iter);
	void invert_sel(GtkTreeIter *iter);
};

extern GdkPixbuf *list_of_downloads_pixbufs[];

void lod_init_pixmaps();
gint lod_get_height();
void lod_set_height();
void lod_theme_changed();

#endif
