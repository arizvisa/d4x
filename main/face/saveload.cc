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

#include <gtk/gtk.h>
#include "list.h"
#include "edit.h"
#include "misc.h"
#include "mywidget.h"
#include "../savelog.h"
#include "../var.h"
#include "../ntlocale.h"
#include "../eff.h"
#include "../main.h"


GtkWidget *LoadSaveWindow=(GtkWidget *)NULL;
GtkWidget *load_save_entry;
GtkWidget *LoadingStatusWindow=(GtkWidget *)NULL;

gint load_save_list_cancel() {
	if (LoadSaveWindow) gtk_widget_destroy(LoadSaveWindow);
	LoadSaveWindow=(GtkWidget *)NULL;
	return TRUE;
};

void load_list_ok(GtkWidget *parent,GtkWidget *who) {
	read_list_from_file_current(text_from_combo(MY_GTK_FILESEL(load_save_entry)->combo));
	ALL_HISTORIES[LOAD_SAVE_HISTORY]->add(text_from_combo(MY_GTK_FILESEL(load_save_entry)->combo));
	load_save_list_cancel();
};

static void _tmp_foreach_(d4xLinksSel *sel,GtkTreeIter *iter,const gchar *s,gpointer rd,gpointer ud){
	_aa_.add_downloading((char*)s);
};

static void d4x_links_sel_ok(GtkWidget *button, d4xLinksSel *sel){
	d4x_links_sel_foreach(sel,_tmp_foreach_,NULL);
	gtk_widget_destroy(GTK_WIDGET(sel));
};

static gint time_for_load_refresh(GtkWidget *pbar){
	if (thread_for_parse_txt_status()==1){
		char text[100];
		float p=thread_for_parse_percent();
		sprintf(text,"%p%%",p);
		gtk_progress_bar_set_text (GTK_PROGRESS_BAR(pbar),text);
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(pbar),p);
		return 1;
	};
	if (thread_for_parse_full()){
		d4xLinksSel *sel=(d4xLinksSel *)d4x_links_sel_new();
		g_signal_connect(G_OBJECT(sel->ok),"clicked",
				   G_CALLBACK(d4x_links_sel_ok),
				   sel);
		thread_for_parse_add(sel);
	};
	gtk_widget_destroy(LoadingStatusWindow);
	LoadingStatusWindow=(GtkWidget *)NULL;
	return 0;
};

static gint try_to_stop_load_thread(GtkWindow *window,GdkEvent *event,gpointer data){
	thread_for_parse_stop();
	return(TRUE);
};

void load_txt_list_ok(GtkWidget *parent,GtkWidget *who) {
	tUrlParser *parser=new tUrlParser(text_from_combo(MY_GTK_FILESEL(load_save_entry)->combo));
	ALL_HISTORIES[LOAD_SAVE_HISTORY]->add(text_from_combo(MY_GTK_FILESEL(load_save_entry)->combo));
	load_save_list_cancel();

        LoadingStatusWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_wmclass(GTK_WINDOW(LoadingStatusWindow),
			       "D4X_LoadStatus","D4X");
	g_signal_connect(G_OBJECT(LoadingStatusWindow),
			   "delete_event",
			   G_CALLBACK(try_to_stop_load_thread), NULL);
	gtk_window_set_resizable(GTK_WINDOW(LoadingStatusWindow), FALSE);
	gtk_window_set_position(GTK_WINDOW(LoadingStatusWindow),GTK_WIN_POS_CENTER);
	gtk_window_set_title(GTK_WINDOW (LoadingStatusWindow), _("Loading"));
	gtk_container_set_border_width(GTK_CONTAINER(LoadingStatusWindow),5);
	GtkWidget *pbar = gtk_progress_bar_new();
	gtk_widget_set_size_request(pbar,200,-1);
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(pbar),0);
	gtk_container_add(GTK_CONTAINER(LoadingStatusWindow),pbar);

	gtk_widget_show_all(LoadingStatusWindow);
	gtk_window_set_modal (GTK_WINDOW(LoadingStatusWindow),TRUE);
	gtk_window_set_transient_for (GTK_WINDOW (LoadingStatusWindow), GTK_WINDOW (MainWindow));

	gint timeout=gtk_timeout_add (100, GtkFunction(time_for_load_refresh) , pbar);
	if (thread_for_parse_txt(parser)){
		gtk_timeout_remove(timeout);
		gtk_widget_destroy(LoadingStatusWindow);
		LoadingStatusWindow=(GtkWidget *)NULL;
	};
/*
	tStringList *list=parser->parse();
	tString *tmp=list->last();
	while (tmp){
		_aa_.add_downloading(tmp->body);
		tmp=list->next();
	};
	delete(parser);
	delete(list);
*/
};


void save_list_ok(GtkWidget *parent,GtkWidget *who) {
	save_list_to_file_current(text_from_combo(MY_GTK_FILESEL(load_save_entry)->combo));
	ALL_HISTORIES[LOAD_SAVE_HISTORY]->add(text_from_combo(MY_GTK_FILESEL(load_save_entry)->combo));
	ALL_HISTORIES[SAVE_HISTORY]->add(text_from_combo(MY_GTK_FILESEL(load_save_entry)->combo));
	load_save_list_cancel();
	init_load_accelerators();
};

static void _sl_set_modal(){
	gtk_window_set_modal (GTK_WINDOW(LoadSaveWindow),TRUE);
	gtk_window_set_transient_for (GTK_WINDOW (LoadSaveWindow), GTK_WINDOW (MainWindow));
};

void init_save_list(...) {
	if (LoadSaveWindow) {
		gdk_window_show(LoadSaveWindow->window);
		return;
	};
	LoadSaveWindow=gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_wmclass(GTK_WINDOW(LoadSaveWindow),
			       "D4X_Save","D4X");
	gtk_window_set_title(GTK_WINDOW(LoadSaveWindow),_("Save list"));
	gtk_window_set_position(GTK_WINDOW(LoadSaveWindow),GTK_WIN_POS_CENTER);
	gtk_window_set_resizable(GTK_WINDOW(LoadSaveWindow), FALSE);
	gtk_container_set_border_width(GTK_CONTAINER(LoadSaveWindow),5);
	g_signal_connect(G_OBJECT(LoadSaveWindow),"delete_event",G_CALLBACK(load_save_list_cancel), NULL);
	d4x_eschandler_init(LoadSaveWindow,NULL);

	load_save_entry=my_gtk_filesel_new(ALL_HISTORIES[LOAD_SAVE_HISTORY]);
	gtk_widget_set_size_request(GTK_COMBO(MY_GTK_FILESEL(load_save_entry)->combo)->entry,400,-1);
	MY_GTK_FILESEL(load_save_entry)->modal=GTK_WINDOW(LoadSaveWindow);

	GtkWidget *vbox=gtk_vbox_new(FALSE,5);
	gtk_box_pack_start(GTK_BOX(vbox),load_save_entry,FALSE,FALSE,0);

	GtkWidget *hbox=gtk_hbutton_box_new();
	gtk_box_set_spacing(GTK_BOX(hbox),5);
	gtk_button_box_set_layout(GTK_BUTTON_BOX(hbox),GTK_BUTTONBOX_END);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);
	GtkWidget *button_ok=gtk_button_new_from_stock(GTK_STOCK_OK);
	g_signal_connect(G_OBJECT(button_ok),"clicked",G_CALLBACK(save_list_ok),NULL);
	GtkWidget *button_cancel=gtk_button_new_from_stock(GTK_STOCK_CANCEL);
	g_signal_connect(G_OBJECT(button_cancel),"clicked",G_CALLBACK(load_save_list_cancel),NULL);
	GTK_WIDGET_SET_FLAGS(button_cancel,GTK_CAN_DEFAULT);
	GTK_WIDGET_SET_FLAGS(button_ok,GTK_CAN_DEFAULT);
	gtk_box_pack_start(GTK_BOX(hbox),button_ok,TRUE,TRUE,0);
	gtk_box_pack_start(GTK_BOX(hbox),button_cancel,TRUE,TRUE,0);

	gtk_container_add(GTK_CONTAINER(LoadSaveWindow),vbox);
	gtk_window_set_default(GTK_WINDOW(LoadSaveWindow),button_ok);
	gtk_widget_show_all(LoadSaveWindow);
	_sl_set_modal();
};


void init_load_list(...) {
	if (LoadSaveWindow) {
		gdk_window_show(LoadSaveWindow->window);
		return;
	};
	LoadSaveWindow=gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_wmclass(GTK_WINDOW(LoadSaveWindow),
			       "D4X_Load","D4X");
	gtk_window_set_title(GTK_WINDOW(LoadSaveWindow),_("Load list"));
	gtk_window_set_position(GTK_WINDOW(LoadSaveWindow),GTK_WIN_POS_CENTER);
	gtk_window_set_resizable(GTK_WINDOW(LoadSaveWindow), FALSE);
	gtk_container_set_border_width(GTK_CONTAINER(LoadSaveWindow),5);
	g_signal_connect(G_OBJECT(LoadSaveWindow),"delete_event",G_CALLBACK(load_save_list_cancel), NULL);
	d4x_eschandler_init(LoadSaveWindow,NULL);

	load_save_entry=my_gtk_filesel_new(ALL_HISTORIES[LOAD_SAVE_HISTORY]);
	gtk_widget_set_size_request(GTK_COMBO(MY_GTK_FILESEL(load_save_entry)->combo)->entry,400,-1);

	GtkWidget *vbox=gtk_vbox_new(FALSE,5);
	gtk_box_pack_start(GTK_BOX(vbox),load_save_entry,FALSE,FALSE,0);
	MY_GTK_FILESEL(load_save_entry)->modal=GTK_WINDOW(LoadSaveWindow);

	GtkWidget *hbox=gtk_hbutton_box_new();
	gtk_box_set_spacing(GTK_BOX(hbox),5);
	gtk_button_box_set_layout(GTK_BUTTON_BOX(hbox),GTK_BUTTONBOX_END);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);
	GtkWidget *button_ok=gtk_button_new_from_stock(GTK_STOCK_OK);
	g_signal_connect(G_OBJECT(button_ok),"clicked",G_CALLBACK(load_list_ok),NULL);
	GtkWidget *button_cancel=gtk_button_new_from_stock(GTK_STOCK_CANCEL);
	g_signal_connect(G_OBJECT(button_cancel),"clicked",G_CALLBACK(load_save_list_cancel),NULL);
	GTK_WIDGET_SET_FLAGS(button_cancel,GTK_CAN_DEFAULT);
	GTK_WIDGET_SET_FLAGS(button_ok,GTK_CAN_DEFAULT);
	gtk_box_pack_start(GTK_BOX(hbox),button_ok,TRUE,TRUE,0);
	gtk_box_pack_start(GTK_BOX(hbox),button_cancel,TRUE,TRUE,0);

	gtk_container_add(GTK_CONTAINER(LoadSaveWindow),vbox);
	gtk_window_set_default(GTK_WINDOW(LoadSaveWindow),button_ok);
	gtk_widget_show_all(LoadSaveWindow);
	_sl_set_modal();
};

void init_load_txt_list(...) {
	if (LoadSaveWindow) {
		gdk_window_show(LoadSaveWindow->window);
		return;
	};
	LoadSaveWindow=gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_wmclass(GTK_WINDOW(LoadSaveWindow),
			       "D4X_ParseTxt","D4X");
	gtk_window_set_title(GTK_WINDOW(LoadSaveWindow),_("Find links in txt file"));
	gtk_window_set_position(GTK_WINDOW(LoadSaveWindow),GTK_WIN_POS_CENTER);
	gtk_window_set_resizable(GTK_WINDOW(LoadSaveWindow), FALSE);
	gtk_container_set_border_width(GTK_CONTAINER(LoadSaveWindow),5);
	g_signal_connect(G_OBJECT(LoadSaveWindow),"delete_event",G_CALLBACK(load_save_list_cancel), NULL);
	d4x_eschandler_init(LoadSaveWindow,NULL);

	load_save_entry=my_gtk_filesel_new(ALL_HISTORIES[LOAD_SAVE_HISTORY]);
	gtk_widget_set_size_request(GTK_COMBO(MY_GTK_FILESEL(load_save_entry)->combo)->entry,400,-1);

	GtkWidget *vbox=gtk_vbox_new(FALSE,5);
	gtk_box_pack_start(GTK_BOX(vbox),load_save_entry,FALSE,FALSE,0);
	MY_GTK_FILESEL(load_save_entry)->modal=GTK_WINDOW(LoadSaveWindow);

	GtkWidget *hbox=gtk_hbutton_box_new();
	gtk_box_set_spacing(GTK_BOX(hbox),5);
	gtk_button_box_set_layout(GTK_BUTTON_BOX(hbox),GTK_BUTTONBOX_END);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);
	GtkWidget *button_ok=gtk_button_new_from_stock(GTK_STOCK_OK);
	g_signal_connect(G_OBJECT(button_ok),"clicked",G_CALLBACK(load_txt_list_ok),NULL);
	GtkWidget *button_cancel=gtk_button_new_from_stock(GTK_STOCK_CANCEL);
	g_signal_connect(G_OBJECT(button_cancel),"clicked",G_CALLBACK(load_save_list_cancel),NULL);
	GTK_WIDGET_SET_FLAGS(button_cancel,GTK_CAN_DEFAULT);
	GTK_WIDGET_SET_FLAGS(button_ok,GTK_CAN_DEFAULT);
	gtk_box_pack_start(GTK_BOX(hbox),button_ok,TRUE,TRUE,0);
	gtk_box_pack_start(GTK_BOX(hbox),button_cancel,TRUE,TRUE,0);

	gtk_container_add(GTK_CONTAINER(LoadSaveWindow),vbox);
	gtk_window_set_default(GTK_WINDOW(LoadSaveWindow),button_ok);
	gtk_widget_show_all(LoadSaveWindow);
	_sl_set_modal();
};
