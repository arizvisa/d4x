/*	WebDownloader for X-Window
 *	Copyright (C) 1999-2001 Koshelev Maxim
 *	This Program is free but not GPL!!! You can't modify it
 *	without agreement with author. You can't distribute modified
 *	program but you can distribute unmodified program.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */
#ifndef _D4X_SCHEDULER_FACE_HEADER_
#define _D4X_SCHEDULER_FACE_HEADER_

#include <gtk/gtk.h>
#include "../dlist.h"
#include "../schedule.h"

struct MyGtkAEditor{
	GtkWindow window;
	GtkWidget *frame; //properties frame
	GtkWidget *frame_child; //child of frame
	GtkWidget *hbox;      //hbox for various stuff
	GtkWidget *omenu;     //actions
	GtkWidget *calendar;  //date
	GtkWidget *hour,*min,*sec; //time entries
	GtkWidget *period_days,*period_hours,*period_mins; //period entries
	GtkWidget *retry;     //retry check
	GtkWidget *retry_times; //retry entry
	int last_action;
	GtkWidget *sb_low,*sb_middle,*sb_high;
	GtkWidget *url_entry; //url entry for actions with downloads
	GtkWidget *path_entry; //path for saving list
	GtkWidget *browser;
	tDownload *dwn; // download for temporary editing when added
	d4xSchedAction *action; //edited action
};

struct MyGtkAEditorClass{
	GtkWindowClass parent_class;
};

#define MY_GTK_AEDITOR(editor) ((MyGtkAEditor *)(editor)) 


GtkWidget *my_gtk_aeditor_new(d4xSchedAction *action=(d4xSchedAction *)NULL);
guint my_gtk_aeditor_get_type();

void d4x_scheduler_init();
void d4x_scheduler_insert(d4xSchedAction *act,d4xSchedAction *prev);
void d4x_scheduler_remove(d4xSchedAction *act);
gint d4x_scheduler_close();

#endif

