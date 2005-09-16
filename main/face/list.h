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
#ifndef  T_GTK_DOWNLOAD_LIST
#define  T_GTK_DOWNLOAD_LIST

#include <gtk/gtk.h>
#include <pthread.h>
#include "../dlist.h"
#include "../main.h"
#include "lod.h"
#include "graph.h"
#include "qtree.h"

extern GtkWidget *MainMenu;
extern GtkWidget *MainLogList,*MAIN_PANED,*MAIN_PANED2,*MAIN_PANED1;
extern GtkAdjustment *ProgressBarValues;
extern GtkWidget *ProgressOfDownload;
extern GtkWidget *MainStatusBar;
extern GtkWidget *MainWindow;
extern GdkGC *MainWindowGC;
extern GtkWidget *ContainerForCList;
extern gint StatusBarContext;
extern GtkTreeView *FSearchView;
extern GtkTreeView *FSearchView2;
extern d4xQsTree *D4X_QVT;

struct d4xDisplayLogInfo{
	GtkTreeView *view;
	tLog *log;
	tDownload *papa;
	GtkWidget *buttonsbar;
	GtkWidget *buttons[10];
	int curbutton;
};

extern  d4xDisplayLogInfo D4X_LOG_DISPLAY;


void main_menu_speed_prepare();
void main_menu_completed_empty();
void main_menu_completed_nonempty();
void main_menu_failed_empty();
void main_menu_failed_nonempty();
void init_status_bar();
void init_main_window();
void init_load_accelerators();
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
gint main_menu_prepare();

void main_window_iconify();
void main_window_toggle();
void main_window_popup();
void main_window_resize(int w,int h);
void main_window_move(int x,int y);

void update_progress_bar();
void update_mainwin_title();
void open_edit_for_selected(...);
void del_completed_downloads(...);
void del_fataled_downloads(...);
void stop_downloads(...);
void delete_downloads(gint flag);
void continue_downloads(...);
void my_main_quit(...);
char *old_clipboard_content();

void d4x_mw_clipboard_set(const char *str);
char *d4x_mw_clipboard_get();
void d4x_normalize_coords(gint *x,gint *y,gint width=0,gint heigh=0);
void d4x_main_switch_log(tDownload *dwn);
void d4x_main_log_del_string();
void d4x_main_log_add_string(tLogString *str);
void d4x_main_offline_mode();
void d4x_vertical_toolbar_change_theme();

void list_dnd_drop_internal(GtkWidget *widget,
			    GdkDragContext *context,
			    gint x, gint y,
			    GtkSelectionData *selection_data,
			    guint info,
			    guint time);

#endif
