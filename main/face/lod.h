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
	int enum_index; // it is an index in array of tColumn of strings
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

extern GdkPixmap *list_of_downloads_pixmaps[PIX_UNKNOWN];
extern GdkBitmap *list_of_downloads_bitmaps[PIX_UNKNOWN];

struct d4xQueueView{
private:
	void remove_wf(tDownload *what);
	void add_wf(tDownload *what);
	void init_sort_buttons();
	void set_column_justification (int col, GtkJustification justify);
	GtkWidget *d4xQueueView::get_column_widget(int col);
public:
	GtkWidget *ListOfDownloads;
	tQueue ListOfDownloadsWF;
	d4xQVPrefs prefs;
	int LoDSortFlag;
	d4xQueueView();
	~d4xQueueView();
	void get_sizes();
	void init();
	void add(tDownload *what);
	void add(tDownload *what,int row);
	void remove(tDownload *what);
	void update(tDownload *what);
	void change_data(int row,int column,gchar *data);
	void set_percent(int row,int col,float percent);
	void set_desc(gint row,tDownload *what);
	void set_color(tDownload *what,int row);
	void set_filename(gint row,tDownload *what);

	void freeze();
	void unfreeze();
	gint get_height();
	void set_height();
	void print_size(gint row,tDownload *what);

	void set_pixmap(tDownload *what,int type);
	void set_pixmap(gint row,int type);
	void set_pixmap(gint row, tDownload *what);

	void set_run_icon(tDownload *what);
	int sel();
	int rows();

	void swap(tDownload *a,tDownload *b);
	void move_up();
	void move_down();
	void move_download_up(int row);
	void move_download_down(int row);
	int move_selected_up();
	int move_selected_down();
	void move_selected_home();
	void move_selected_end();
	void unselect_all();
	void select_all();
	void invert_selection();
	void select(tDownload *dwn);
	void real_select(int type,char *wildcard);

	void init_select_window(int type=0);
	void rebuild_wait();
	void sort(int how);

	tDownload *get_download(int row);
	gint get_row(tDownload *what);
	tDownload *last_selected();

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
};

void lod_init_pixmaps();
gint lod_get_height();
void lod_set_height();
void lod_theme_changed();

#endif
