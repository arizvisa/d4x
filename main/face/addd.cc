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

#include <gtk/gtk.h>
#include <stdio.h>
#include "../main.h"
#include "../var.h"
#include "../locstr.h"
#include "edit.h"
#include "list.h"
#include "../ntlocale.h"
#include <gdk/gdkkeysyms.h>

extern tMain aa;

tDownload *OneDownload=(tDownload *)NULL;
tDList *list_for_adding=(tDList *)NULL;

void add_window_cancel(GtkWidget *widget, tDownload *what) {
	if (what){
		list_for_adding->del(what);
		delete(what);
	};
};

void add_window_delete(GtkWidget *widget,GdkEvent *event,tDownload *what) {
	if (what){
		list_for_adding->del(what);
		delete(what);
	};
};


void add_window_ok(GtkWidget *widget, tDownload *what) {
	if (what->editor->apply_changes()) return;
	list_for_adding->del(what);
	int tmp=what->editor->get_pause_check();
	what->delete_editor();
	if (tmp){
		what->owner=DL_PAUSE;
		aa.add_downloading_to(what);
	}else{
		if (aa.add_downloading(what)){
			delete(what);
		}else{
			aa.add_download_message(what);
		};
	};
};


static gint _add_window_event_handler(GtkWidget *window,GdkEvent *event,tDownload *what){
	if (event && event->type == GDK_KEY_PRESS) {
		GdkEventKey *kevent=(GdkEventKey *)event;
		switch(kevent->keyval) {
		case GDK_Escape:{
			if (what){
				list_for_adding->del(what);
				delete(what);
			};
			return TRUE;
			break;
		};
		};
	};
	return FALSE;
};

void init_add_window(...) {
	if (list_for_adding==NULL) {
		list_for_adding=new tDList(DL_TEMP);
		list_for_adding->init(0);
	};
	tDownload *what=new tDownload;
	tAddr *info=new tAddr("ftp://somesite.org");
	what->info=info;
	what->config.save_path.set(CFG.GLOBAL_SAVE_PATH);
	what->set_default_cfg();

	if (CFG.USE_PROXY_FOR_FTP) {
		what->config.proxy_host.set(CFG.FTP_PROXY_HOST);
		what->config.proxy_port=CFG.FTP_PROXY_PORT;
		if (CFG.NEED_PASS_FTP_PROXY) {
			what->config.proxy_user.set(CFG.FTP_PROXY_USER);
			what->config.proxy_pass.set(CFG.FTP_PROXY_PASS);
		};
	};
	what->config.proxy_type=CFG.FTP_PROXY_TYPE;

	what->editor=new tDEdit;
	what->editor->init(what);
	gtk_window_set_title(GTK_WINDOW(what->editor->window),_("Add new download"));
	gtk_signal_connect(GTK_OBJECT(what->editor->cancel_button),"clicked",GTK_SIGNAL_FUNC(add_window_cancel), what);
	gtk_signal_connect(GTK_OBJECT(what->editor->ok_button),"clicked",GTK_SIGNAL_FUNC(add_window_ok),what);
	gtk_signal_connect(GTK_OBJECT(what->editor->window),"delete_event",GTK_SIGNAL_FUNC(add_window_delete), what);
	gtk_signal_connect(GTK_OBJECT(what->editor->window), "key_press_event",
			   (GtkSignalFunc)_add_window_event_handler, what);
	what->editor->clear_url();
	list_for_adding->insert(what);
};

void init_add_clipboard_window(...) {
	init_add_window();
	tDownload *what=list_for_adding->last();
	what->editor->paste_url();
	gtk_widget_grab_focus(what->editor->ok_button);
};

void init_add_dnd_window(char *url) {
	if (!url) return;
	init_add_window();
	tDownload *what=list_for_adding->last();
	what->editor->set_url(url);
	gtk_widget_grab_focus(what->editor->ok_button);
};
