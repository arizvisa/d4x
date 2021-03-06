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
#include "../addr.h"
#include "../filter.h"

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
	GdkColor color;
};

struct MyGtkColorselClass{
	GtkHBoxClass parent_class;
};

#define MY_GTK_COLORSEL(colsel) ((MyGtkColorsel *)(colsel)) 
GtkWidget *my_gtk_colorsel_new(gint color,gchar *title);
gint my_gtk_colorsel_get_color(MyGtkColorsel *colsel);
void my_gtk_colorsel_set_color(MyGtkColorsel *colsel, gint color);


struct d4xRuleEdit{
	GtkWindow window;
	GtkWidget *host,*proto,*path,*file,*params,*tag;
	GtkWidget *include,*exclude;
	GtkWidget *vbox;
	GtkWidget *ok_button,*cancel_button;
	GtkWidget *filter_edit;
	d4x::Filter::Rule *rule;
	GtkTreeIter *iter;
};

struct d4xRuleEditClass{
	GtkWindowClass parent_class;
};

void d4x_rule_edit_apply(d4xRuleEdit *rule);
GtkWidget *d4x_rule_edit_new(const d4x::Filter::Rule &rule);

struct d4xFNode;

struct d4xFilterEdit{
	GtkWindow window;
	GtkWidget *vbox;
	GtkTreeView *view;  //list of rules
	GtkListStore *store;
	GtkWidget *include,*exclude; //default action
	GtkWidget *name;   //name entry
	GtkWidget *ok,*edit;   //buttons
	d4x::Filter *filter;
	GtkTreeIter *iter;
};

struct d4xFilterEditClass{
	GtkWindowClass parent_class;
};

GtkWidget *d4x_filter_edit_new(const d4x::Filter &filter);
void d4x_filter_edit_add_rule(d4xFilterEdit *edit,const d4x::Filter::Rule &rule,d4x::Filter::iterator *it);

/* filter selector is used for select filter in properties of
   a download and in "Common properties"/HTTP
*/

struct PopulateFilters{
	GtkListStore *store;
	PopulateFilters(GtkListStore *s):store(s){};
	void operator()(const std::string &name){
		GtkTreeIter iter;
		gtk_list_store_append(store,&(iter));
		gtk_list_store_set(store,&(iter),
				   0,name.c_str(),
				   -1);
	};
};

struct d4xFilterSel{
	GtkWindow window;
	GtkTreeView *view;
	GtkWidget *ok,*cancel;
};
struct d4xFilterSelClass{
	GtkWindowClass parent_class;
};

struct d4xFNode;

GtkWidget *d4x_filter_sel_new();
void d4x_filter_sel_to_combo(d4xFilterSel *sel,GtkWidget *combo);

/* next widget is used for list of links which was found in
   legacy file via "Search links in file"
 */

struct d4xLinksSel{
	GtkWindow window;
	GtkTreeView *view;
	GtkWidget *hbbox,*vbox;
	GtkWidget *referer,*directory;
	GtkWidget *ok,*cancel,*remove,*find;
};

struct d4xLinksSelClass{
	GtkWindowClass parent_class;
};

typedef void (* d4xLinksSelForeachFunc) (d4xLinksSel *sel, GtkTreeIter *iter,const gchar *text, gpointer rowdata,gpointer userdata);

GtkWidget *d4x_links_sel_new();
void d4x_links_sel_add(d4xLinksSel *sel,const char *url,gpointer data);
void d4x_links_sel_foreach(d4xLinksSel *sel,d4xLinksSelForeachFunc func,gpointer data);
void d4x_links_sel_selected_foreach(d4xLinksSel *sel,d4xLinksSelForeachFunc func,gpointer data);
void d4x_links_sel_del(d4xLinksSel *sel,GtkTreeIter *iter);
void d4x_links_sel_clear(d4xLinksSel *sel);
gpointer d4x_links_sel_get_data(d4xLinksSel *sel,GtkTreeIter *iter);
void d4x_links_sel_set(d4xLinksSel *sel,GtkTreeIter *iter,const char *url,gpointer p);

GtkWidget *d4x_links_sel_new_with_add();
GtkWidget *d4x_links_sel_new_with_referer(const char*);

struct d4xStringEdit{
	GtkWindow window;
	GtkEntry *entry;
	GtkWidget *ok,*cancel;
};

struct d4xStringEditClass{
	GtkWindowClass parent_class;
};

GtkWidget *d4x_string_edit_new();


struct d4xAltEdit{
	GtkWindow window;
	GtkEntry *entry;
	GtkWidget *proxy_use_check;
	GtkWidget *proxy_type_ftp;
	GtkWidget *proxy_type_http;
	GtkWidget *proxy_host;
	GtkWidget *proxy_port;
	GtkWidget *proxy_user;
	GtkWidget *proxy_pass;
	GtkWidget *proxy_user_check;
	GtkWidget *proxy_view;
	GtkWidget *ok,*cancel;
};

struct d4xAltEditClass {
	GtkWindowClass parent_class;
};

GtkWidget *d4x_alt_edit_new();
void d4x_alt_edit_set(d4xAltEdit *sel,const d4x::URL &info);
void d4x_alt_edit_get(d4xAltEdit *sel,d4x::URL &info);

struct MyGtkVbookmark{
	GtkRadioButton button;
};
struct MyGtkVbookmarkClass{
	GtkRadioButtonClass parent_class;
};

GtkWidget *my_gtk_vbookmark_new(GSList *group);
GtkWidget *my_gtk_vbookmark_new_with_label(GSList *group,const gchar *label);


#endif
