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
#include <stdio.h>
#include <gdk/gdkkeysyms.h>
#include "limface.h"
#include "../var.h"
#include "../dlist.h"
#include "../locstr.h"
#include "../ntlocale.h"
extern tDList *RunList;

static gint dialog_delete(GtkWidget *widget, GdkEvent *event,tLimitDialog *parent) {
	parent->done();
	return TRUE;
};

static void dialog_delete2(GtkWidget *widget,tLimitDialog *parent) {
	parent->done();
};

tLimitDialog::tLimitDialog() {
	oldhost=NULL;
	oldport=0;
};

void tLimitDialog::set_old(char *host,int port) {
	if (oldhost) delete oldhost;
	oldhost=copy_string(host);
	oldport=port;
};

void tLimitDialog::reset_old() {
	if (oldhost) {
		delete oldhost;
		oldhost=NULL;
		oldport=0;
	};
};

int tLimitDialog::init() {
	if (window) {
		gdk_window_show(window->window);
		return 0;
	};
	window = gtk_window_new(GTK_WINDOW_DIALOG);
	gtk_window_set_title(GTK_WINDOW (window),_("Limitation for the host"));
	gtk_window_set_position(GTK_WINDOW(window),GTK_WIN_POS_CENTER);
	gtk_container_border_width(GTK_CONTAINER(window),5);
	host_entry=gtk_entry_new();
	port_entry=gtk_entry_new();
	limit_entry=gtk_entry_new();
	GtkWidget *host_label=gtk_label_new(_("Host:"));
	GtkWidget *port_label=gtk_label_new(_("Port:"));
	GtkWidget *limit_label=gtk_label_new(_("Maximum connections:"));
	GtkWidget *hbox1=gtk_hbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(hbox1),5);
	gtk_box_pack_start(GTK_BOX(hbox1),host_label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(hbox1),host_entry,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(hbox1),port_label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(hbox1),port_entry,FALSE,FALSE,0);
	gtk_widget_set_usize(port_entry,40,-1);
	gtk_widget_set_usize(limit_entry,40,-1);
	GtkWidget *hbox2=gtk_hbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(hbox2),5);
	gtk_box_pack_start(GTK_BOX(hbox2),limit_label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(hbox2),limit_entry,FALSE,FALSE,0);
	ok_button=gtk_button_new_with_label(_("Ok"));
	cancel_button=gtk_button_new_with_label(_("Cancel"));
	GTK_WIDGET_SET_FLAGS(ok_button,GTK_CAN_DEFAULT);
	GTK_WIDGET_SET_FLAGS(cancel_button,GTK_CAN_DEFAULT);
	GtkWidget *vbox=gtk_vbox_new(FALSE,0);
	GtkWidget *hbox=gtk_hbutton_box_new();
	gtk_box_set_spacing(GTK_BOX(vbox),5);
	gtk_box_set_spacing(GTK_BOX(hbox),5);
	gtk_box_pack_start(GTK_BOX(vbox),hbox1,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),hbox2,FALSE,FALSE,0);
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

tLimitDialog::~tLimitDialog() {
	reset_old();
};

//-------------------------------------------------------------------------------
static void face_limits_ok(GtkWidget *widget, tFaceLimits *parent) {
	parent->close();
};


static void face_limits_add(GtkWidget *widget, tFaceLimits *parent) {
	parent->open_dialog();
};

static gint face_limits_delete(GtkWidget *widget,GdkEvent *event, tFaceLimits *parent) {
	parent->close();
	return TRUE;
};

static void face_limits_del(GtkWidget *widget, tFaceLimits *parent) {
	parent->delete_rows();
};


static void face_limits_clist_handler(GtkWidget *clist, gint row, gint column,
                                      GdkEventButton *event,tFaceLimits *parent) {
	if (event && event->type==GDK_2BUTTON_PRESS && event->button==1)
		parent->open_row(row);
};

static void face_limits_dialog_ok(GtkWidget *widget,tFaceLimits *parent) {
	parent->apply_dialog();
};

static int face_limits_list_event_callback(GtkWidget *widget,GdkEvent *event,tFaceLimits *parent) {
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

int calc_curent_run(char *host,int port) {
	tDownload *temp=RunList->last();
	int count=0;
	while(temp) {
		if (strcmp(host,temp->info->host)==0 && port==temp->info->port)
			count+=1;
		temp=RunList->next();
	};
	return count;
};

tFaceLimits::tFaceLimits() {
	window=NULL;
	dialog=NULL;
	size1=CFG.FACE_LIMITS_SIZE1;
	size2=CFG.FACE_LIMITS_SIZE2;
};


void tFaceLimits::apply_dialog() {
	if (!dialog) return;
	int port=0,limit=0;
	sscanf(gtk_entry_get_text(GTK_ENTRY(dialog->port_entry)),"%i",&port);
	sscanf(gtk_entry_get_text(GTK_ENTRY(dialog->limit_entry)),"%i",&limit);
	if (port==0 || limit==0) {
		dialog->done();
		return;
	};
	char *host=gtk_entry_get_text(GTK_ENTRY(dialog->host_entry));
	tSortString *temp=LimitsForHosts->find(host,port);
	if (temp) {
		temp->upper=limit;
		if (dialog->oldhost!=NULL && dialog->oldport!=port && (temp=LimitsForHosts->find(dialog->oldhost,dialog->oldport))) {
			LimitsForHosts->del(temp);
			delete temp;
		};
	} else {
		if (dialog->oldhost!=NULL && (temp=LimitsForHosts->find(dialog->oldhost,dialog->oldport))) {
			LimitsForHosts->del(temp);
			delete temp;
		};
		int curent=calc_curent_run(host,port);
		LimitsForHosts->add(host,port,curent,limit);
	};
	dialog->reset_old();
	redraw();
	dialog->done();
};

void tFaceLimits::open_dialog() {
	if (!dialog) dialog=new tLimitDialog;
	if (dialog->init())
		gtk_signal_connect(GTK_OBJECT(dialog->ok_button),"clicked",GTK_SIGNAL_FUNC(face_limits_dialog_ok),this);
};
//tFaceLimits::
void tFaceLimits::add(char *host,int port) {
	open_dialog();
	tSortString *temp=LimitsForHosts->find(host,port);
	char data[MAX_LEN];
	sprintf(data,"%i",port);
	gtk_entry_set_text(GTK_ENTRY(dialog->host_entry),host);
	gtk_entry_set_text(GTK_ENTRY(dialog->port_entry),data);
	if (temp) {
		sprintf(data,"%i",temp->upper);
		gtk_entry_set_text(GTK_ENTRY(dialog->limit_entry),data);
	} else
		gtk_entry_set_text(GTK_ENTRY(dialog->limit_entry),"5");
};

void tFaceLimits::open_row(int row) {
	open_dialog();
	tSortString *temp=(tSortString *)gtk_clist_get_row_data(GTK_CLIST(clist),row);
	if (temp) {
		char data[MAX_LEN];
		sprintf(data,"%i",temp->key);
		gtk_entry_set_text(GTK_ENTRY(dialog->host_entry),temp->body);
		gtk_entry_set_text(GTK_ENTRY(dialog->port_entry),data);
		sprintf(data,"%i",temp->upper);
		gtk_entry_set_text(GTK_ENTRY(dialog->limit_entry),data);
		dialog->set_old(temp->body,temp->key);
	};
};

void tFaceLimits::close() {
	if (window) {
		GtkCListColumn *tmp=GTK_CLIST(clist)->column;
		size1=tmp->width;
		tmp++;
		size2=tmp->width;
		gtk_widget_destroy(window);
		window=NULL;
	};
};

void tFaceLimits::delete_rows() {
	GList *select=((GtkCList *)clist)->selection;
	while (select) {
		tSortString *temp=(tSortString *)gtk_clist_get_row_data(
		                      GTK_CLIST(clist),GPOINTER_TO_INT(select->data));
		LimitsForHosts->del(temp);
		delete temp;
		select=select->next;
	};
	redraw();
};

void tFaceLimits::redraw() {
	if (!window) return;
	gtk_clist_freeze(GTK_CLIST(clist));
	gtk_clist_clear(GTK_CLIST(clist));
	tSortString *temp=LimitsForHosts->last();
	while(temp) {
		char data1[MAX_LEN];
		char data2[MAX_LEN];
		sprintf(data1,"%s:%i",temp->body,temp->key);
		sprintf(data2,"%i",temp->upper);
		char *data[]={data1,data2};
		int row=gtk_clist_append(GTK_CLIST(clist),data);
		gtk_clist_set_row_data(GTK_CLIST(clist),row,temp);
		temp=LimitsForHosts->next();
	};
	gtk_clist_thaw(GTK_CLIST(clist));
	gtk_widget_show(clist);
};

void tFaceLimits::get_sizes() {
	CFG.FACE_LIMITS_SIZE1=size1;
	CFG.FACE_LIMITS_SIZE2=size2;
};

void tFaceLimits::init() {
	if (window) {
		gdk_window_show(window->window);
		return;
	};
	window = gtk_window_new(GTK_WINDOW_DIALOG);
	gtk_window_set_title(GTK_WINDOW (window),_("Limitations for the hosts"));
	gtk_window_set_position(GTK_WINDOW(window),GTK_WIN_POS_CENTER);
	gtk_widget_set_usize(window,-1,400);
	gtk_container_border_width(GTK_CONTAINER(window),5);
	gchar *titles[]={_("Host"),_("N")};
	clist = gtk_clist_new_with_titles(2, titles);
	gtk_signal_connect(GTK_OBJECT(clist),"select_row",GTK_SIGNAL_FUNC(face_limits_clist_handler),this);
	gtk_clist_set_shadow_type (GTK_CLIST(clist), GTK_SHADOW_IN);
	gtk_clist_set_column_width (GTK_CLIST(clist), 0 , size1);
	gtk_clist_set_column_width (GTK_CLIST(clist), 1 , size2);
	gtk_clist_set_column_auto_resize(GTK_CLIST(clist),2,TRUE);
	gtk_clist_set_column_auto_resize(GTK_CLIST(clist),3,TRUE);
	gtk_clist_set_selection_mode(GTK_CLIST(clist),GTK_SELECTION_EXTENDED);
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
	redraw();
	gtk_widget_show_all(window);
	gtk_signal_connect(GTK_OBJECT(clist),"event",GTK_SIGNAL_FUNC(face_limits_list_event_callback),this);
	gtk_signal_connect(GTK_OBJECT(button),"clicked",GTK_SIGNAL_FUNC(face_limits_ok),this);
	gtk_signal_connect(GTK_OBJECT(del_button),"clicked",GTK_SIGNAL_FUNC(face_limits_del),this);
	gtk_signal_connect(GTK_OBJECT(add_button),"clicked",GTK_SIGNAL_FUNC(face_limits_add),this);
	gtk_signal_connect(GTK_OBJECT(window),"delete_event",GTK_SIGNAL_FUNC(face_limits_delete), this);
};

tFaceLimits::~tFaceLimits() {
	close();
	if (dialog) delete dialog;
};
