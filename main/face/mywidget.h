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
#ifndef __MY_GTK_WIDGETS__
#define __MY_GTK_WIDGETS__

#include <gtk/gtk.h>
#include "../history.h"

struct MyGtkFilesel{
	GtkHBox box;
	gint only_dirs;
	GtkWidget *browser;
	GtkWidget *combo;
	GtkWindow *modal;
};

struct MyGtkFileselClass{
	GtkHBoxClass parent_class;
};
#define MY_GTK_FILESEL(filesel) ((MyGtkFilesel *)(filesel)) 

GtkWidget *my_gtk_filesel_new(tHistory *history);

struct MyGtkColorsel{
	GtkHBox box;
	GtkWidget *preview;
	GtkWidget *browser;
	GtkWindow *modal;
	gdouble color[4];
};

struct MyGtkColorselClass{
	GtkHBoxClass parent_class;
};

#define MY_GTK_COLORSEL(colsel) ((MyGtkColorsel *)(colsel)) 
GtkWidget *my_gtk_colorsel_new(gint color,gchar *title);
gint my_gtk_colorsel_get_color(MyGtkColorsel *colsel);
void my_gtk_colorsel_set_color(MyGtkColorsel *colsel, gint color);


#endif
