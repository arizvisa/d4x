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

struct d4xRule;

struct d4xRuleEdit{
	GtkWindow window;
	GtkWidget *host,*proto,*path,*file,*params,*tag;
	GtkWidget *include,*exclude;
	GtkWidget *vbox;
	GtkWidget *ok_button,*cancel_button;
	GtkWidget *filter_edit;
	d4xRule *rule;
};

struct d4xRuleEditClass{
	GtkWindowClass parent_class;
};

void d4x_rule_edit_apply(d4xRuleEdit *rule);
GtkWidget *d4x_rule_edit_new(d4xRule *rule);
GtkWidget *d4x_rule_edit_new_full(d4xRule *rule);

struct d4xFNode;

struct d4xFilterEdit{
	GtkWindow window;
	GtkWidget *vbox;
	GtkWidget *clist;  //list of rules
	GtkWidget *include,*exclude; //default action
	GtkWidget *name;   //name entry
	GtkWidget *ok,*edit;   //buttons
	d4xFNode *node;
};

struct d4xFilterEditClass{
	GtkWindowClass parent_class;
};

GtkWidget *d4x_filter_edit_new(d4xFNode *node);
void d4x_filter_edit_add_rule(d4xFilterEdit *edit,d4xRule *rule);

/* filter selector is used for select filter in properties of
   a download and in "Common properties"/HTTP
*/

struct d4xFilterSel{
	GtkWindow window;
	GtkWidget *clist,*ok,*cancel;
};
struct d4xFilterSelClass{
	GtkWindowClass parent_class;
};

struct d4xFNode;

GtkWidget *d4x_filter_sel_new();
void d4x_filter_sel_add(d4xFilterSel *sel,d4xFNode *node);

/* next widget is used for list of links which was found in
   legacy file via "Search links in file"
 */

struct d4xLinksSel{
	GtkWindow window;
	GtkCList *clist;
	GtkWidget *hbbox;
	GtkWidget *ok,*cancel,*remove,*find;
};

struct d4xLinksSelClass{
	GtkWindowClass parent_class;
};

GtkWidget *d4x_links_sel_new();
void d4x_links_sel_add(d4xLinksSel *sel,char *url,gpointer data);
GtkWidget *d4x_links_sel_new_with_add();

struct d4xStringEdit{
	GtkWindow window;
	GtkEntry *entry;
	GtkWidget *ok,*cancel;
};

struct d4xStringEditClass{
	GtkWindowClass parent_class;
};

GtkWidget *d4x_string_edit_new();

#endif
