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
#ifndef  T_GTK_DOWNLOAD_LIST
#define  T_GTK_DOWNLOAD_LIST

#include <gtk/gtk.h>
#include <pthread.h>
#include "../dlist.h"
#include "../main.h"
#include "limface.h"
#include "lod.h"

extern tMain aa;

extern GtkWidget *MainMenu;
extern GtkWidget *MainLogList;
extern GtkAdjustment *ProgressBarValues;
extern GtkWidget *ProgressOfDownload;
extern GtkWidget *MainStatusBar;
extern GtkWidget *MainWindow;
extern GdkGC *MainWindowGC;
extern tFaceLimits *FaceForLimits;
extern GtkWidget *ContainerForCList;
extern gint StatusBarContext;

void main_menu_speed_prepare();
void init_status_bar();
void init_main_window();
void init_face(int argc, char *argv[]);
void init_timeouts();
gint get_mainwin_sizes(GtkWidget *window);

void set_limit_to_download();
/* Asking functions
 */
void ask_delete_fataled_downloads(...);
void ask_delete_completed_downloads(...);
void ask_delete_download(...);
void ask_exit(...);
gint ask_exit2();
/* Other functions
 */
void main_menu_del_completed_set_state(gint a);
void main_menu_del_failed_set_state(gint a);
void main_menu_prepare();

void main_window_iconify();
void main_window_popup();
void update_progress_bar();
void update_mainwin_title();
void open_edit_for_selected(...);
void del_completed_downloads(...);
void del_fataled_downloads(...);
void stop_downloads(...);
void delete_downloads(...);
void continue_downloads(...);
void my_main_quit(...);
#endif
