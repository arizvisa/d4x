/*	WebDownloader for X-Window
 *	Copyright (C) 1999 Koshelev Maxim
 *	This Program is free but not GPL!!! You can't modify it
 *	without agreement with author. You can't distribute modified
 *	program but you can distribute unmodified program.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <gtk/gtk.h>
#include <stdio.h>
#include "list.h"
#include "../var.h"
#include "about.h"
#include "../ntlocale.h"

GtkWidget *AboutWindow=NULL;

void destroy_about_window() {
	if (AboutWindow){
		gtk_widget_destroy(AboutWindow);
		AboutWindow=NULL;
		gtk_widget_set_sensitive(MainWindow,TRUE);
	};
};

void init_about_window(...) {
	if (AboutWindow) {
		gdk_window_show(AboutWindow->window);
		return;
	};
	AboutWindow = gtk_window_new(GTK_WINDOW_DIALOG);
	//    gtk_widget_set_usize( GTK_WIDGET (AboutWindow), 400, 105);
	gtk_window_set_policy (GTK_WINDOW(AboutWindow), FALSE,FALSE,FALSE);
	gtk_window_set_position(GTK_WINDOW(AboutWindow),GTK_WIN_POS_CENTER);
	gtk_window_set_title(GTK_WINDOW (AboutWindow), _("About"));
	gtk_container_border_width(GTK_CONTAINER(AboutWindow),5);
	GtkWidget *box=gtk_vbox_new(FALSE,0);
	GtkWidget *label1=gtk_label_new(VERSION_NAME);
	GtkWidget *label2=gtk_label_new(HOME_PAGE);
	GtkWidget *label3=gtk_label_new(_("Author: Koshelev Maxim"));
	GtkWidget *label4=gtk_label_new("e-mail: mdem@chat.ru");
	GtkWidget *frame=gtk_frame_new(_("Translators team"));
	gtk_frame_set_shadow_type(GTK_FRAME(frame),GTK_SHADOW_IN);
	GtkWidget *box1=gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(frame),box1);
	GtkWidget *label=gtk_label_new("Vicente Aguilar");
	gtk_box_pack_start(GTK_BOX(box1),label,FALSE,FALSE,0);
	label=gtk_label_new("Vittorio Rebecchi");
	gtk_box_pack_start(GTK_BOX(box1),label,FALSE,FALSE,0);
	label=gtk_label_new("Paulo Henrique");
	gtk_box_pack_start(GTK_BOX(box1),label,FALSE,FALSE,0);
	label=gtk_label_new("A.J.");
	gtk_box_pack_start(GTK_BOX(box1),label,FALSE,FALSE,0);
	label=gtk_label_new("Josef Jahn");
	gtk_box_pack_start(GTK_BOX(box1),label,FALSE,FALSE,0);
	label=gtk_label_new("Marlin [TLC-ML]");
	gtk_box_pack_start(GTK_BOX(box1),label,FALSE,FALSE,0);
	label=gtk_label_new("Philippe Rigaux");
	gtk_box_pack_start(GTK_BOX(box1),label,FALSE,FALSE,0);
	label=gtk_label_new("Jerome Couderc");
	gtk_box_pack_start(GTK_BOX(box1),label,FALSE,FALSE,0);
	label=gtk_label_new("Eric Seigne");
	gtk_box_pack_start(GTK_BOX(box1),label,FALSE,FALSE,0);
	label=gtk_label_new("Robin Verduijn");
	gtk_box_pack_start(GTK_BOX(box1),label,FALSE,FALSE,0);
	label=gtk_label_new("Priyadi Iman Nurcahyo");
	gtk_box_pack_start(GTK_BOX(box1),label,FALSE,FALSE,0);
	label=gtk_label_new("Kei Kodera");
	gtk_box_pack_start(GTK_BOX(box1),label,FALSE,FALSE,0);
	label=gtk_label_new("Guiliano Rangel Alves");
	gtk_box_pack_start(GTK_BOX(box1),label,FALSE,FALSE,0);
	label=gtk_label_new("Pavel Janousek");
	gtk_box_pack_start(GTK_BOX(box1),label,FALSE,FALSE,0);
	GtkWidget *Button=gtk_button_new_with_label(_("Ok"));
	gtk_box_pack_start(GTK_BOX(box),label1,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(box),label2,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(box),label3,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(box),label4,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(box),frame,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(box),Button,FALSE,FALSE,0);
	gtk_container_add(GTK_CONTAINER(AboutWindow),box);
	gtk_signal_connect(GTK_OBJECT(Button),"clicked",
	                   (GtkSignalFunc)destroy_about_window,NULL);
	gtk_signal_connect(GTK_OBJECT(AboutWindow), "delete_event",
	                   (GtkSignalFunc)destroy_about_window,NULL);
	GTK_WIDGET_SET_FLAGS(Button,GTK_CAN_DEFAULT);
	gtk_window_set_default(GTK_WINDOW(AboutWindow),Button);
	gtk_widget_show_all(AboutWindow);
//	gtk_widget_show(AboutWindow);
	gtk_widget_set_sensitive(MainWindow,FALSE);
};

/* ------------------------------------------------------------
 * Dialogs...
 * ------------------------------------------------------------
 */
tDialog::tDialog() {
	window=NULL;
};

void tDialog::done() {
	if (!window) return;
	gtk_widget_destroy(window);
	window=NULL;
};

tDialog::~tDialog() {
	done();
};


static void dialog_delete(GtkWidget *widget, GdkEvent *event,tDialogWidget *parent) {
	parent->done();
};

static void dialog_delete2(GtkWidget *widget,tDialogWidget *parent) {
	parent->done();
};

tDialogWidget::tDialogWidget() {
};

int tDialogWidget::init(char *ask,char *title) {
	if (window) {
		gdk_window_show(window->window);
		return 0;
	};
	window = gtk_window_new(GTK_WINDOW_DIALOG);
	gtk_window_set_title(GTK_WINDOW (window),title);
	gtk_window_set_position(GTK_WINDOW(window),GTK_WIN_POS_CENTER);
	gtk_container_border_width(GTK_CONTAINER(window),5);
	label=gtk_label_new(ask);
	ok_button=gtk_button_new_with_label(_("Ok"));
	cancel_button=gtk_button_new_with_label(_("Cancel"));
	GTK_WIDGET_SET_FLAGS(ok_button,GTK_CAN_DEFAULT);
	GTK_WIDGET_SET_FLAGS(cancel_button,GTK_CAN_DEFAULT);
	GtkWidget *vbox=gtk_vbox_new(FALSE,0);
	GtkWidget *hbox=gtk_hbutton_box_new();
	gtk_box_set_spacing(GTK_BOX(vbox),5);
	gtk_box_set_spacing(GTK_BOX(hbox),5);
	gtk_box_pack_start(GTK_BOX(vbox),label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);
	gtk_box_pack_end(GTK_BOX(hbox),ok_button,FALSE,FALSE,0);
	gtk_box_pack_end(GTK_BOX(hbox),cancel_button,FALSE,FALSE,0);
	gtk_container_add(GTK_CONTAINER(window),vbox);
	gtk_window_set_default(GTK_WINDOW(window),cancel_button);
	gtk_widget_show_all(window);
	gtk_signal_connect(GTK_OBJECT(cancel_button),"clicked",GTK_SIGNAL_FUNC(dialog_delete2),this);
	gtk_signal_connect(GTK_OBJECT(window),"delete_event",GTK_SIGNAL_FUNC(dialog_delete), this);
	return 1;
};


tDialogWidget::~tDialogWidget() {
};
/* -----------------------------------------------
 */

static gint string_dialog_delete_event(GtkWidget *widget, GdkEvent *event,tStringDialog *parent) {
	parent->done();
	return TRUE;
};

static void string_dialog_ok_clicked(GtkWidget *widget,tStringDialog *parent) {
	parent->done();
};

tStringDialog::tStringDialog() {
};

int tStringDialog::init(char *str,char *title) {
	if (window) {
		gtk_entry_set_text(GTK_ENTRY(entry),str);
		gtk_window_set_title(GTK_WINDOW (window), title);
		gtk_window_set_default(GTK_WINDOW(window),ok_button);
		gdk_window_show(window->window);
		return 0;
	};
	window=gtk_window_new(GTK_WINDOW_DIALOG);
	gtk_signal_connect(GTK_OBJECT(window),"delete_event",GTK_SIGNAL_FUNC(string_dialog_delete_event), this);
	gtk_window_set_title(GTK_WINDOW (window), title);
	gtk_container_border_width(GTK_CONTAINER(window),1);
	gtk_window_set_policy (GTK_WINDOW(window), FALSE,FALSE,FALSE);
	GtkWidget *hbox=gtk_hbutton_box_new();
	GtkWidget *vbox=gtk_vbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(vbox),5);
	gtk_button_box_set_layout(GTK_BUTTON_BOX(hbox),GTK_BUTTONBOX_END);
	ok_button=gtk_button_new_with_label(_("Ok"));
	gtk_signal_connect(GTK_OBJECT(ok_button),"clicked",GTK_SIGNAL_FUNC(string_dialog_ok_clicked),this);
	GTK_WIDGET_SET_FLAGS(ok_button,GTK_CAN_DEFAULT);
	entry=gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entry),str);
	gtk_entry_set_editable(GTK_ENTRY(entry),FALSE);
	gtk_widget_set_usize(GTK_WIDGET (window), 500,-1);
	gtk_box_pack_start(GTK_BOX(vbox),entry,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);
	gtk_box_pack_end(GTK_BOX(hbox),ok_button,FALSE,FALSE,0);
	GtkWidget *frame=gtk_frame_new("");
	gtk_frame_set_shadow_type(GTK_FRAME(frame),GTK_SHADOW_OUT);
	//	    gtk_container_border_width(GTK_CONTAINER(frame),5);
	gtk_container_add(GTK_CONTAINER(frame),vbox);
	gtk_container_add(GTK_CONTAINER(window),frame);
	gtk_widget_show_all(window);
	gtk_window_set_default(GTK_WINDOW(window),ok_button);
	return 1;
};

tStringDialog::~tStringDialog() {
};
