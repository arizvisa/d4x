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
#ifndef MY_GTK_GRAPH
#define MY_GTK_GRAPH

struct MyGtkGraph{
	GtkWidget widget;
	GdkRgbCmap *cmap;
	guchar *rgb_data;
};

struct MyGtkGraphClass{
	GtkWidgetClass parent_class;
};

extern MyGtkGraph *GLOBAL_GRAPH;

GtkWidget *my_gtk_graph_new();
void my_gtk_graph_recalc(MyGtkGraph *graph);
void my_gtk_graph_cmap_reinit(MyGtkGraph *graph);

#endif
