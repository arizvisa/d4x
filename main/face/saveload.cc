/*	WebDownloader for X-Window
 *	Copyright (C) 1999 Koshelev Maxim
 *	This Program is free but not GPL!!! You can't modify it
 *	without agreement with autor. You can't distribute modified
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
#include "../savelog.h"
#include "../var.h"
#include "../ntlocale.h"
GtkWidget *LoadSaveBrowser=NULL;
GtkWidget *LoadSaveWindow=NULL;
GtkWidget *load_save_entry;
/*
 */
gint load_save_browser_cancel() {
	gtk_widget_destroy(LoadSaveBrowser);
	LoadSaveBrowser=NULL;
	return TRUE;
};

void load_save_browser_ok() {
	text_to_combo(load_save_entry,gtk_file_selection_get_filename(GTK_FILE_SELECTION(LoadSaveBrowser)));
	load_save_browser_cancel();
};

void load_save_open_browser() {
	if (LoadSaveBrowser) {
		gdk_window_show(LoadSaveBrowser->window);
		return;
	};
	LoadSaveBrowser=gtk_file_selection_new(_("Select file"));
	gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(LoadSaveBrowser)->ok_button),
	                   "clicked",GTK_SIGNAL_FUNC(load_save_browser_ok),LoadSaveBrowser);
	gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(LoadSaveBrowser)->cancel_button),
	                   "clicked",GTK_SIGNAL_FUNC(load_save_browser_cancel),LoadSaveBrowser);
	gtk_signal_connect(GTK_OBJECT(&(GTK_FILE_SELECTION(LoadSaveBrowser)->window)),
	                   "delete_event",GTK_SIGNAL_FUNC(load_save_browser_cancel),LoadSaveBrowser);
	gtk_widget_show(LoadSaveBrowser);
};

gint load_save_list_cancel() {
	gtk_widget_destroy(LoadSaveWindow);
	if (LoadSaveBrowser) load_save_browser_cancel();
	LoadSaveWindow=NULL;
	return TRUE;
};

void load_list_ok(GtkWidget *parent,GtkWidget *who) {
	tStringList *temp=new tStringList;
	temp->init(0);
	read_list_from_file(text_from_combo(load_save_entry),temp);
	aa.append_list(temp);
	delete(temp);
	LoadSaveHistory->add(text_from_combo(load_save_entry));
	load_save_list_cancel();
};

void save_list_ok(GtkWidget *parent,GtkWidget *who) {
	save_list_to_file(text_from_combo(load_save_entry));
	LoadSaveHistory->add(text_from_combo(load_save_entry));
	load_save_list_cancel();
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

	load_save_entry=gtk_combo_new();
	gtk_widget_set_usize(GTK_COMBO(load_save_entry)->entry,400,-1);
	GList *List=make_glist_from_mylist(LoadSaveHistory);
	if (List)
		gtk_combo_set_popdown_strings (GTK_COMBO (load_save_entry), List);
	GtkWidget *hbox=gtk_hbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(hbox),5);
	gtk_box_pack_start(GTK_BOX(hbox),load_save_entry,FALSE,FALSE,0);
	GtkWidget *button=gtk_button_new_with_label(_("Browse"));
	gtk_signal_connect(GTK_OBJECT(button),"clicked",GTK_SIGNAL_FUNC(load_save_open_browser),NULL);
	gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,FALSE,0);

	GtkWidget *vbox=gtk_vbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(vbox),5);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);

	hbox=gtk_hbutton_box_new();
	gtk_button_box_set_layout(GTK_BUTTON_BOX(hbox),GTK_BUTTONBOX_END);
	gtk_button_box_set_spacing(GTK_BUTTON_BOX(hbox),5);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);
	GtkWidget *button_ok=gtk_button_new_with_label("Ok");
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

	load_save_entry=gtk_combo_new();
	gtk_widget_set_usize(GTK_COMBO(load_save_entry)->entry,400,-1);
	GList *List=make_glist_from_mylist(LoadSaveHistory);
	if (List)
		gtk_combo_set_popdown_strings (GTK_COMBO (load_save_entry), List);
	GtkWidget *hbox=gtk_hbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(hbox),5);
	gtk_box_pack_start(GTK_BOX(hbox),load_save_entry,FALSE,FALSE,0);
	GtkWidget *button=gtk_button_new_with_label(_("Browse"));
	gtk_signal_connect(GTK_OBJECT(button),"clicked",GTK_SIGNAL_FUNC(load_save_open_browser),NULL);
	gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,FALSE,0);

	GtkWidget *vbox=gtk_vbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(vbox),5);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);

	hbox=gtk_hbutton_box_new();
	gtk_button_box_set_layout(GTK_BUTTON_BOX(hbox),GTK_BUTTONBOX_END);
	gtk_button_box_set_spacing(GTK_BUTTON_BOX(hbox),5);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);
	GtkWidget *button_ok=gtk_button_new_with_label("Ok");
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
};
