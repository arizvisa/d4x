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
#ifndef _MY_CLIST_HEADER_
#define _MY_CLIST_HEADER_

#include <gtk/gtk.h>

struct MyGtkCList{
	GtkCList clist;
};

struct MyGtkCListClass{
	GtkCListClass parent_klass;
};

enum MyGtkCellTypes{
	GTK_CELL_PROGRESS=GTK_CELL_WIDGET+1
};

#define MY_GTK_TYPE_CLIST            (my_gtk_clist_get_type ())
#define GTK_CELL_PROGRESS(cell)   (((GtkCellProgress *) &(cell)))

struct GtkCellProgress{
	GtkCellType type;
  
	gint16 vertical;
	gint16 horizontal;
  
	GtkStyle *style;
	
	float value; //value of progress from 0 to 100
};

GtkType my_gtk_clist_get_type (void);

GtkWidget* my_gtk_clist_new             (gint   columns);
GtkWidget* my_gtk_clist_new_with_titles (gint   columns,
				         gchar *titles[]);

void my_gtk_clist_set_progress (GtkCList  *clist,
				gint       row,
				gint       column,
				float value);

int my_gtk_clist_get_progress (GtkCList   *clist,
			       gint        row,
			       gint        column);

#endif
