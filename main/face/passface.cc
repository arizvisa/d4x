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
#include <stdio.h>
#include "passface.h"
#include "misc.h"
#include "../addr.h"
#include "../ntlocale.h"
#include "../var.h"
#include <gdk/gdkkeysyms.h>

static gint dialog_delete(GtkWidget *widget, GdkEvent *event,tPassDialog *parent) {
	parent->done();
	return TRUE;
};

static void dialog_delete2(GtkWidget *widget,tPassDialog *parent) {
	parent->done();
};

/* tPassDialog functions
 */

tPassDialog::tPassDialog() {
};

int tPassDialog::init(){
	if (window) {
		gdk_window_show(window->window);
		return 0;
	};
	window = gtk_window_new(GTK_WINDOW_DIALOG);
	gtk_window_set_title(GTK_WINDOW (window),_("Password and username"));
	gtk_window_set_position(GTK_WINDOW(window),GTK_WIN_POS_CENTER);
	gtk_container_border_width(GTK_CONTAINER(window),5);
	host_entry=gtk_entry_new();
	user_entry=gtk_entry_new();
	pass_entry=gtk_entry_new();
	gtk_entry_set_visibility(GTK_ENTRY(pass_entry),FALSE);
	proto_select=gtk_combo_new();
	GList *list=NULL;
	for (int i=D_PROTO_FTP;i<D_PROTO_LAST;i++)
		list = g_list_append (list, get_name_by_proto(i));
	gtk_combo_set_popdown_strings (GTK_COMBO (proto_select), list);
	gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(proto_select)->entry),FALSE);

	GtkWidget *proto_label=gtk_label_new(_("protocol"));
	GtkWidget *host_label=gtk_label_new(_("Host"));
	GtkWidget *hbox0=gtk_hbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(hbox0),5);
	gtk_box_pack_start(GTK_BOX(hbox0),proto_label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(hbox0),proto_select,FALSE,FALSE,0);
	GtkWidget *hbox1=gtk_hbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(hbox1),5);
	gtk_box_pack_start(GTK_BOX(hbox1),host_label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(hbox1),host_entry,TRUE,TRUE,0);

	GtkWidget *user_label=gtk_label_new(_("username"));
	GtkWidget *pass_label=gtk_label_new(_("password"));
	GtkWidget *hbox2=gtk_hbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(hbox2),5);
	gtk_box_pack_start(GTK_BOX(hbox2),user_entry,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(hbox2),user_label,FALSE,FALSE,0);
	GtkWidget *hbox3=gtk_hbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(hbox3),5);
	gtk_box_pack_start(GTK_BOX(hbox3),pass_entry,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(hbox3),pass_label,FALSE,FALSE,0);

	ok_button=gtk_button_new_with_label(_("Ok"));
	cancel_button=gtk_button_new_with_label(_("Cancel"));
	GTK_WIDGET_SET_FLAGS(ok_button,GTK_CAN_DEFAULT);
	GTK_WIDGET_SET_FLAGS(cancel_button,GTK_CAN_DEFAULT);
	GtkWidget *vbox=gtk_vbox_new(FALSE,0);
	GtkWidget *hbox=gtk_hbutton_box_new();
	gtk_box_set_spacing(GTK_BOX(vbox),5);
	gtk_box_set_spacing(GTK_BOX(hbox),5);
	gtk_box_pack_start(GTK_BOX(vbox),hbox0,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),hbox1,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),hbox2,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),hbox3,FALSE,FALSE,0);
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

void tPassDialog::done(){
	tDialog::done();
};

tPassDialog::~tPassDialog() {
};

/*
 */
static void face_pass_ok(GtkWidget *widget, tFacePass *parent) {
	parent->close();
};


static void face_pass_add(GtkWidget *widget, tFacePass *parent) {
	parent->open_dialog();
};

static gint face_pass_delete(GtkWidget *widget,GdkEvent *event, tFacePass *parent) {
	parent->close();
	return TRUE;
};

static void face_pass_del(GtkWidget *widget, tFacePass *parent) {
	parent->delete_rows();
};


static void face_pass_clist_handler(GtkWidget *clist, gint row, gint column,
                                      GdkEventButton *event,tFacePass *parent) {
	if (event && event->type==GDK_2BUTTON_PRESS && event->button==1)
		parent->edit_row(row);
};

static void face_pass_dialog_ok(GtkWidget *widget,tFacePass *parent) {
	parent->apply_dialog();
};

static int face_pass_list_event_callback(GtkWidget *widget,GdkEvent *event,tFacePass *parent) {
	if (event->type == GDK_KEY_PRESS) {
		GdkEventKey *kevent=(GdkEventKey *)event;
		switch(kevent->keyval) {
			case GDK_Delete:
			case GDK_KP_Delete:
				{
					parent->delete_rows();
					return TRUE;
				};
		};
	};
	return FALSE;
};


tFacePass::tFacePass(){
	window=NULL;
	dialog=NULL;
};

tFacePass::~tFacePass(){
	if (dialog)
		delete(dialog);
	close();
};

void tFacePass::open_dialog() {
	if (!dialog) dialog=new tPassDialog;
	if (dialog->init()){
		gtk_signal_connect(GTK_OBJECT(dialog->ok_button),"clicked",GTK_SIGNAL_FUNC(face_pass_dialog_ok),this);
		dialog->set_modal(window);
	};
	dialog->data=NULL;
	dialog->row=-1;
};

void tFacePass::delete_rows() {
	GList *select=((GtkCList *)clist)->selection;
	if (select) {
		int row=GPOINTER_TO_INT(select->data);
		char *host;
		gtk_clist_get_text(GTK_CLIST(clist),row,0,&host);
		int proto=get_proto_by_name(host);
		gtk_clist_get_text(GTK_CLIST(clist),row,1,&host);
		tUserPass *tmp=PasswordsForHosts->find(proto,host);
		if (tmp)
			PasswordsForHosts->del(tmp);
		gtk_clist_remove(GTK_CLIST(clist),row);
	};
};

void tFacePass::edit_row(int row) {
	char *host;
	gtk_clist_get_text(GTK_CLIST(clist),row,0,&host);
	int proto=get_proto_by_name(host);
	gtk_clist_get_text(GTK_CLIST(clist),row,1,&host);
	tUserPass *tmp=PasswordsForHosts->find(proto,host);
	if (tmp) {
		open_dialog();
		dialog->data=tmp;
		dialog->row=row;
		gtk_entry_set_text(GTK_ENTRY(dialog->host_entry),tmp->get_host());
		gtk_entry_set_text(GTK_ENTRY(dialog->user_entry),tmp->get_user());
		gtk_entry_set_text(GTK_ENTRY(dialog->pass_entry),tmp->get_pass());
		text_to_combo(dialog->proto_select,get_name_by_proto(proto));
	};
};

void tFacePass::apply_dialog() {
	if (!dialog) return;
	if (dialog->data)
		PasswordsForHosts->del(dialog->data);
	dialog->data=NULL;
	tUserPass *a=new tUserPass;
	a->proto=get_proto_by_name(text_from_combo(dialog->proto_select));
	a->set_host(gtk_entry_get_text(GTK_ENTRY(dialog->host_entry)));
	a->set_user(gtk_entry_get_text(GTK_ENTRY(dialog->user_entry)));
	a->set_pass(gtk_entry_get_text(GTK_ENTRY(dialog->pass_entry)));
	tUserPass *tmp=PasswordsForHosts->find(a->proto,a->get_host());
	if (tmp){
		PasswordsForHosts->del(tmp);
		PasswordsForHosts->add(a);
		gtk_clist_clear(GTK_CLIST(clist));
		PasswordsForHosts->fill_face(this);
	}else{
		PasswordsForHosts->add(a);
		if (dialog->row>=0){
			gtk_clist_set_text(GTK_CLIST(clist),dialog->row,0,get_name_by_proto(a->proto));
			gtk_clist_set_text(GTK_CLIST(clist),dialog->row,1,a->get_host());
			gtk_clist_set_text(GTK_CLIST(clist),dialog->row,2,a->get_user());
		}else
			add(a);
	};
	dialog->done();
};

void tFacePass::init(){
	if (window) {
		gdk_window_show(window->window);
		return;
	};
	window = gtk_window_new(GTK_WINDOW_DIALOG);
	gtk_window_set_title(GTK_WINDOW (window),_("Default passwords and usernames"));
	gtk_window_set_position(GTK_WINDOW(window),GTK_WIN_POS_CENTER);
	gtk_widget_set_usize(window,-1,400);
	gtk_container_border_width(GTK_CONTAINER(window),5);
	gchar *titles[]={_("#"),_("Host"),_("username")};
	clist = gtk_clist_new_with_titles(3, titles);
	gtk_signal_connect(GTK_OBJECT(clist),"select_row",GTK_SIGNAL_FUNC(face_pass_clist_handler),this);
	gtk_clist_set_shadow_type (GTK_CLIST(clist), GTK_SHADOW_IN);
	gtk_clist_set_column_width (GTK_CLIST(clist), 0 , 50);
	gtk_clist_set_column_width (GTK_CLIST(clist), 1 , 180);
	gtk_clist_set_column_width (GTK_CLIST(clist), 2 , 50);
	gtk_clist_set_column_auto_resize(GTK_CLIST(clist),0,TRUE);
	gtk_clist_set_column_auto_resize(GTK_CLIST(clist),2,TRUE);
//	gtk_clist_set_selection_mode(GTK_CLIST(clist),GTK_SELECTION_EXTENDED);
	GtkWidget *scroll_window=gtk_scrolled_window_new(NULL,NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll_window),
	                                GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(scroll_window),clist);
	button=gtk_button_new_with_label(_("Ok"));
	add_button=gtk_button_new_with_label(_("Add new"));
	del_button=gtk_button_new_with_label(_("Delete"));
	GTK_WIDGET_SET_FLAGS(button,GTK_CAN_DEFAULT);
	GTK_WIDGET_SET_FLAGS(add_button,GTK_CAN_DEFAULT);
	GTK_WIDGET_SET_FLAGS(del_button,GTK_CAN_DEFAULT);
	GtkWidget *vbox=gtk_vbox_new(FALSE,0);
	GtkWidget *hbox=gtk_hbutton_box_new();
	gtk_box_set_spacing(GTK_BOX(vbox),5);
	gtk_box_set_spacing(GTK_BOX(hbox),5);
	gtk_box_pack_start(GTK_BOX(vbox),scroll_window,TRUE,TRUE,0);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);
	gtk_box_pack_end(GTK_BOX(hbox),add_button,FALSE,FALSE,0);
	gtk_box_pack_end(GTK_BOX(hbox),del_button,FALSE,FALSE,0);
	gtk_box_pack_end(GTK_BOX(hbox),button,FALSE,FALSE,0);
	gtk_container_add(GTK_CONTAINER(window),vbox);
	gtk_window_set_default(GTK_WINDOW(window),button);
	PasswordsForHosts->fill_face(this);
	gtk_widget_show_all(window);
	gtk_signal_connect(GTK_OBJECT(clist),"event",GTK_SIGNAL_FUNC(face_pass_list_event_callback),this);
	gtk_signal_connect(GTK_OBJECT(button),"clicked",GTK_SIGNAL_FUNC(face_pass_ok),this);
	gtk_signal_connect(GTK_OBJECT(del_button),"clicked",GTK_SIGNAL_FUNC(face_pass_del),this);
	gtk_signal_connect(GTK_OBJECT(add_button),"clicked",GTK_SIGNAL_FUNC(face_pass_add),this);
	gtk_signal_connect(GTK_OBJECT(window),"delete_event",GTK_SIGNAL_FUNC(face_pass_delete), this);
};

void tFacePass::close() {
	if (window) {
		gtk_widget_destroy(window);
		window=NULL;
	};
};

void tFacePass::add(tUserPass *a){
	char *row[]={get_name_by_proto(a->proto),a->get_host(),a->get_user()};
	gtk_clist_append(GTK_CLIST(clist),row);
};
