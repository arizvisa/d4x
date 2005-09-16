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
#ifndef T_GTK_LOG
#define T_GTK_LOG
#include <glib.h>
GtkTreeView *log_model_view_init();
void log_window_init(tDownload *what);
void log_model_view_add_string(GtkTreeView *view,tLogString *str);
void log_window_add_string(tLog *log,tLogString *str);
int  log_window_destroy(GtkWidget *window,GdkEvent *event, tLog *log);
void log_window_destroy_by_log(void *a);
void del_first_from_log(tLog *what);
void log_window_set_title(tDownload *what,char *title);
void log_window_set_split_info(tDownload *what);
void log_print_to_view(tLog *log,GtkTreeView *view);

GList *log_window_unfreeze(GList *list);
GList *log_window_freeze(GList *list,tLog *what);

#endif
