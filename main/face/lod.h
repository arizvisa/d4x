/*	WebDownloader for X-Window
 *	Copyright (C) 1999 Koshelev Maxim
 *	This Program is free but not GPL!!! You can't modify it
 *	without agreement with autor. You can't distribute modified
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

extern GtkWidget *ListOfDownloads;

struct tColumn{
	int type;
	int enum_index; // it is an index in array of tColumn of strings
	char *name;
	int size;
};

enum {
    STATUS_COL=0,
    FILE_COL,
    FILE_TYPE_COL,
    FULL_SIZE_COL,
    DOWNLOADED_SIZE_COL,
    PERCENT_COL,
    SPEED_COL,
    TIME_COL,
    ELAPSED_TIME_COL,
    PAUSE_COL,
    TREAT_COL,
    URL_COL,
    NOTHING_COL
};

enum{
	TARGET_URL
};

enum {
	PIX_WAIT=0,
	PIX_STOP,
	PIX_STOP_WAIT,
	PIX_RUN,
	PIX_RUN_BAD,
	PIX_RUN_PART,
	PIX_COMPLETE,
	PIX_PAUSE,
	PIX_UNKNOWN
};


extern tColumn ListColumns[NOTHING_COL+1];

void list_dnd_drop_internal(GtkWidget *widget,GdkDragContext *context,gint x, gint y,GtkSelectionData *selection_data,guint info, guint time);

void list_of_downloads_get_sizes();
void list_of_downloads_init();
void list_of_downloads_init_pixmaps();
void list_of_downloads_add(tDownload *what);
void list_of_downloads_add(tDownload *what,int row);
void list_of_downloads_change_data(int row,int column,gchar *data);

void list_of_downloads_del(tDownload *what);
void list_of_downloads_freeze();
void list_of_downloads_unfreeze();
void list_of_downloads_get_height();
void list_of_downloads_set_height();
void init_columns_info();

void list_of_downloads_set_pixmap(int row,int type);

void move_download_up();
void move_download_down();

tDownload *get_download_from_clist(int row);
tDownload *list_of_downloads_last_selected();

#endif