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
#ifndef  T_GTK_DOWNLOAD_LIST
#define  T_GTK_DOWNLOAD_LIST

#include <gtk/gtk.h>
#include <pthread.h>
#include "../dlist.h"
#include "../main.h"
#include "limface.h"

extern tMain aa;

extern GtkWidget *MainMenu;
extern GtkWidget *ListOfDownloads;
extern GtkWidget *MainLogList;
extern GtkAdjustment *ProgressBarValues;
extern GtkWidget *ProgressOfDownload;
extern GtkWidget *MainStatusBar;
extern GtkWidget *MainWindow;
extern GdkGC *MainWindowGC;
extern tFaceLimits *FaceForLimits;

struct tColumn{
	int type;
	int status; // zero - off, nonzero - on
	char *name;
	int size;
};

extern tColumn ListColumns[12];

enum {
    STATUS_COL,
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
    URL_COL
};


void list_of_downloads_get_sizes();
void init_view_list();
void init_status_bar();
void init_main_window();
void init_face(int argc, char *argv[]);
void init_timeouts();
gint get_mainwin_sizes(GtkWidget *window);
/*  CList functions 
 */
void add_download_to_clist(tDownload *what);
void change_clist_data(int row,int column,gchar *data);
void del_download_from_clist(tDownload *what);
void freeze_clist();
void unfreeze_clist();
void set_pixmap_wait(int row);
void set_pixmap_stop(int row);
void set_pixmap_stop_wait(int row);
void set_pixmap_run(int row);
void set_pixmap_run_bad(int row);
void set_pixmap_part_run(int row);
void set_pixmap_complete(int row);
void set_pixmap_pause(int row);
void set_limit_to_download();
void move_download_up();
void move_download_down();
/* Asking functions
 */
void ask_delete_fataled_downloads(...);
void ask_delete_completed_downloads(...);
void ask_delete_download(...);
void ask_exit(...);
gint ask_exit2();
/* Other functions
 */
void update_progress_bar();
void update_mainwin_title();
void open_log_for_selected(...);
void open_edit_for_selected(...);
void del_completed_downloads(...);
void del_fataled_downloads(...);
void stop_downloads(...);
void delete_downloads(...);
void continue_downloads(...);
void my_main_quit(...);
tDownload *get_download_from_clist(int row);
tDownload *get_last_selected();
#endif