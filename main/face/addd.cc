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

#include <gtk/gtk.h>
#include <stdio.h>
#include "../main.h"
#include "../var.h"
#include "../locstr.h"
#include "edit.h"
#include "list.h"
#include "../ntlocale.h"

extern tMain aa;

tDownload *OneDownload=NULL;

void add_window_cancel() {
	if (OneDownload){
		delete OneDownload;
		OneDownload=NULL;
	};
};

void add_window_ok() {
	if (OneDownload->editor->apply_changes()) return;
	OneDownload->delete_editor();
	if (ALL_DOWNLOADS->find(OneDownload))
		delete(OneDownload);
	else {
		list_of_downloads_add(OneDownload);
		WaitList->insert(OneDownload);
		aa.add_download_message(OneDownload);
		ALL_DOWNLOADS->insert(OneDownload);
	};
	OneDownload=NULL;
};


void init_add_window(...) {
	if (OneDownload) {
		OneDownload->editor->popup();
		return;
	};
	tDownload *what=OneDownload=new tDownload;
	char *temp=copy_string("ftp://somesite");
	tAddr *info=aa.analize(temp);
	what->info=info;
	what->set_SavePath(CFG.GLOBAL_SAVE_PATH);
	what->set_default_cfg();

	if (CFG.USE_PROXY_FOR_FTP) {
		what->config.set_proxy_host(CFG.FTP_PROXY_HOST);
		what->config.proxy_port=CFG.FTP_PROXY_PORT;
		if (CFG.NEED_PASS_FTP_PROXY) {
			what->config.set_proxy_user(CFG.FTP_PROXY_USER);
			what->config.set_proxy_pass(CFG.FTP_PROXY_PASS);
		};
	};
	what->config.proxy_type=CFG.FTP_PROXY_TYPE;

	what->editor=new tDEdit;
	what->editor->init(what);
	gtk_window_set_title(GTK_WINDOW(what->editor->window),_("Add new download"));
	gtk_signal_connect(GTK_OBJECT(what->editor->cancel_button),"clicked",GTK_SIGNAL_FUNC(add_window_cancel), NULL);
	gtk_signal_connect(GTK_OBJECT(what->editor->ok_button),"clicked",GTK_SIGNAL_FUNC(add_window_ok),NULL);
	gtk_signal_connect(GTK_OBJECT(what->editor->window),"delete_event",GTK_SIGNAL_FUNC(add_window_cancel), NULL);
	what->editor->clear_url();
};

void init_add_clipboard_window(...) {
	if (OneDownload) return;
	init_add_window();
	tDownload *what=OneDownload;
	what->editor->paste_url();
	gtk_widget_grab_focus(what->editor->ok_button);
};

void init_add_dnd_window(char *url) {
	if (OneDownload || !url) return;
	init_add_window();
	tDownload *what=OneDownload;
	what->editor->set_url(url);
	gtk_widget_grab_focus(what->editor->ok_button);
};
