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
#include "gtk/gtk.h"
#include "list.h"
#include "edit.h"
#include "misc.h"
#include "mywidget.h"
#include "../savelog.h"
#include "../var.h"
#include "../ntlocale.h"
#include "../eff.h"
#include "../main.h"

extern tMain aa;

GtkWidget *LoadSaveWindow=(GtkWidget *)NULL;
GtkWidget *load_save_entry;

gint load_save_list_cancel() {
	if (LoadSaveWindow) gtk_widget_destroy(LoadSaveWindow);
	LoadSaveWindow=(GtkWidget *)NULL;
	return TRUE;
};

void load_list_ok(GtkWidget *parent,GtkWidget *who) {
	read_list_from_file(text_from_combo(MY_GTK_FILESEL(load_save_entry)->combo));
	ALL_HISTORIES[LOAD_SAVE_HISTORY]->add(text_from_combo(MY_GTK_FILESEL(load_save_entry)->combo));
	load_save_list_cancel();
};

void load_txt_list_ok(GtkWidget *parent,GtkWidget *who) {
	tUrlParser *parser=new tUrlParser(text_from_combo(MY_GTK_FILESEL(load_save_entry)->combo));
	tStringList *list=parser->parse();
	tString *tmp=list->last();
	while (tmp){
		aa.add_downloading(tmp->body,NULL,NULL);
		tmp=list->next();
	};
	delete(parser);
	delete(list);
	ALL_HISTORIES[LOAD_SAVE_HISTORY]->add(text_from_combo(MY_GTK_FILESEL(load_save_entry)->combo));
	load_save_list_cancel();
};


void save_list_ok(GtkWidget *parent,GtkWidget *who) {
	save_list_to_file(text_from_combo(MY_GTK_FILESEL(load_save_entry)->combo));
	ALL_HISTORIES[LOAD_SAVE_HISTORY]->add(text_from_combo(MY_GTK_FILESEL(load_save_entry)->combo));
	load_save_list_cancel();
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
	LoadSaveWindow=gtk_window_new(GTK_WINDOW_DIALOG);
	gtk_window_set_title(GTK_WINDOW(LoadSaveWindow),_("Save list"));
	gtk_window_set_position(GTK_WINDOW(LoadSaveWindow),GTK_WIN_POS_CENTER);
	gtk_window_set_policy (GTK_WINDOW(LoadSaveWindow), FALSE,FALSE,FALSE);
	gtk_container_border_width(GTK_CONTAINER(LoadSaveWindow),5);
	gtk_signal_connect(GTK_OBJECT(LoadSaveWindow),"delete_event",GTK_SIGNAL_FUNC(load_save_list_cancel), NULL);

	load_save_entry=my_gtk_filesel_new(ALL_HISTORIES[LOAD_SAVE_HISTORY]);
	gtk_widget_set_usize(GTK_COMBO(MY_GTK_FILESEL(load_save_entry)->combo)->entry,400,-1);
	MY_GTK_FILESEL(load_save_entry)->modal=GTK_WINDOW(LoadSaveWindow);

	GtkWidget *vbox=gtk_vbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(vbox),5);
	gtk_box_pack_start(GTK_BOX(vbox),load_save_entry,FALSE,FALSE,0);

	GtkWidget *hbox=gtk_hbutton_box_new();
	gtk_button_box_set_layout(GTK_BUTTON_BOX(hbox),GTK_BUTTONBOX_END);
	gtk_button_box_set_spacing(GTK_BUTTON_BOX(hbox),5);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);
	GtkWidget *button_ok=gtk_button_new_with_label(_("Ok"));
	gtk_signal_connect(GTK_OBJECT(button_ok),"clicked",GTK_SIGNAL_FUNC(save_list_ok),NULL);
	GtkWidget *button_cancel=gtk_button_new_with_label(_("Cancel"));
	gtk_signal_connect(GTK_OBJECT(button_cancel),"clicked",GTK_SIGNAL_FUNC(load_save_list_cancel),NULL);
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
	LoadSaveWindow=gtk_window_new(GTK_WINDOW_DIALOG);
	gtk_window_set_title(GTK_WINDOW(LoadSaveWindow),_("Load list"));
	gtk_window_set_position(GTK_WINDOW(LoadSaveWindow),GTK_WIN_POS_CENTER);
	gtk_window_set_policy (GTK_WINDOW(LoadSaveWindow), FALSE,FALSE,FALSE);
	gtk_container_border_width(GTK_CONTAINER(LoadSaveWindow),5);
	gtk_signal_connect(GTK_OBJECT(LoadSaveWindow),"delete_event",GTK_SIGNAL_FUNC(load_save_list_cancel), NULL);

	load_save_entry=my_gtk_filesel_new(ALL_HISTORIES[LOAD_SAVE_HISTORY]);
	gtk_widget_set_usize(GTK_COMBO(MY_GTK_FILESEL(load_save_entry)->combo)->entry,400,-1);

	GtkWidget *vbox=gtk_vbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(vbox),5);
	gtk_box_pack_start(GTK_BOX(vbox),load_save_entry,FALSE,FALSE,0);
	MY_GTK_FILESEL(load_save_entry)->modal=GTK_WINDOW(LoadSaveWindow);

	GtkWidget *hbox=gtk_hbutton_box_new();
	gtk_button_box_set_layout(GTK_BUTTON_BOX(hbox),GTK_BUTTONBOX_END);
	gtk_button_box_set_spacing(GTK_BUTTON_BOX(hbox),5);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);
	GtkWidget *button_ok=gtk_button_new_with_label(_("Ok"));
	gtk_signal_connect(GTK_OBJECT(button_ok),"clicked",GTK_SIGNAL_FUNC(load_list_ok),NULL);
	GtkWidget *button_cancel=gtk_button_new_with_label(_("Cancel"));
	gtk_signal_connect(GTK_OBJECT(button_cancel),"clicked",GTK_SIGNAL_FUNC(load_save_list_cancel),NULL);
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
	LoadSaveWindow=gtk_window_new(GTK_WINDOW_DIALOG);
	gtk_window_set_title(GTK_WINDOW(LoadSaveWindow),_("Find links in txt file"));
	gtk_window_set_position(GTK_WINDOW(LoadSaveWindow),GTK_WIN_POS_CENTER);
	gtk_window_set_policy (GTK_WINDOW(LoadSaveWindow), FALSE,FALSE,FALSE);
	gtk_container_border_width(GTK_CONTAINER(LoadSaveWindow),5);
	gtk_signal_connect(GTK_OBJECT(LoadSaveWindow),"delete_event",GTK_SIGNAL_FUNC(load_save_list_cancel), NULL);

	load_save_entry=my_gtk_filesel_new(ALL_HISTORIES[LOAD_SAVE_HISTORY]);
	gtk_widget_set_usize(GTK_COMBO(MY_GTK_FILESEL(load_save_entry)->combo)->entry,400,-1);

	GtkWidget *vbox=gtk_vbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(vbox),5);
	gtk_box_pack_start(GTK_BOX(vbox),load_save_entry,FALSE,FALSE,0);
	MY_GTK_FILESEL(load_save_entry)->modal=GTK_WINDOW(LoadSaveWindow);

	GtkWidget *hbox=gtk_hbutton_box_new();
	gtk_button_box_set_layout(GTK_BUTTON_BOX(hbox),GTK_BUTTONBOX_END);
	gtk_button_box_set_spacing(GTK_BUTTON_BOX(hbox),5);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);
	GtkWidget *button_ok=gtk_button_new_with_label(_("Ok"));
	gtk_signal_connect(GTK_OBJECT(button_ok),"clicked",GTK_SIGNAL_FUNC(load_txt_list_ok),NULL);
	GtkWidget *button_cancel=gtk_button_new_with_label(_("Cancel"));
	gtk_signal_connect(GTK_OBJECT(button_cancel),"clicked",GTK_SIGNAL_FUNC(load_save_list_cancel),NULL);
	GTK_WIDGET_SET_FLAGS(button_cancel,GTK_CAN_DEFAULT);
	GTK_WIDGET_SET_FLAGS(button_ok,GTK_CAN_DEFAULT);
	gtk_box_pack_start(GTK_BOX(hbox),button_ok,TRUE,TRUE,0);
	gtk_box_pack_start(GTK_BOX(hbox),button_cancel,TRUE,TRUE,0);

	gtk_container_add(GTK_CONTAINER(LoadSaveWindow),vbox);
	gtk_window_set_default(GTK_WINDOW(LoadSaveWindow),button_ok);
	gtk_widget_show_all(LoadSaveWindow);
	_sl_set_modal();
};
