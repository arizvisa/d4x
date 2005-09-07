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
#include <stdio.h>
#include "../main.h"
#include "../var.h"
#include "../locstr.h"
#include "edit.h"
#include "list.h"
#include "../ntlocale.h"
#include <gdk/gdkkeysyms.h>
#include "../sndserv.h"
#include "../autoadd.h"
#include "misc.h"
#include "dndtrash.h"

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
	int to_top=what->editor->get_to_top_check();
	d4xDownloadQueue *tmpq=D4X_QUEUE;
	if (what->editor->dnd && dnd_trash_target_queue)
		D4X_QUEUE=dnd_trash_target_queue;
	what->delete_editor();
	if (what->config->isdefault){
		delete(what->config);
		what->config=NULL;
	};
	if (tmp){
		what->status=DL_PAUSE;
		tDownload *dwn=_aa_.add_downloading_to(what,to_top);
		SOUND_SERVER->add_event(SND_ADD);
		D4X_QUEUE=tmpq;
		if (dwn)
			D4X_QVT->move_to(dwn);
	}else{
		if (_aa_.add_downloading(what,to_top)){
			D4X_QUEUE=tmpq;
			tDownload *dwn=ALL_DOWNLOADS->find(what);
			delete(what);
			if (dwn)
				D4X_QVT->move_to(dwn);
		}else{
			D4X_QUEUE=tmpq;
			_aa_.add_download_message(what);
			SOUND_SERVER->add_event(SND_ADD);
		};
	};
};

void init_add_window(...) {
	if (list_for_adding==NULL) {
		list_for_adding=new tDList(DL_TEMP);
		list_for_adding->init(0);
	};
	tDownload *what=new tDownload;
	tAddr *info=new tAddr("ftp://somesite.org");
	what->info=info;
	what->config=new tCfg;
	what->set_default_cfg();
	what->config->isdefault=CFG.USE_DEFAULT_CFG;
	what->config->save_path.set(D4X_QUEUE->save_path.get());

	what->editor=new tDEdit;
	what->editor->add_or_edit=1;
	what->editor->init(what);
	gtk_window_set_title(GTK_WINDOW(what->editor->window),_("Add new download"));
	g_signal_connect(G_OBJECT(what->editor->cancel_button),"clicked",G_CALLBACK(add_window_cancel), what);
	g_signal_connect(G_OBJECT(what->editor->ok_button),"clicked",G_CALLBACK(add_window_ok),what);
	g_signal_connect(G_OBJECT(what->editor->window),"delete_event",G_CALLBACK(add_window_delete), what);
	d4x_eschandler_init(what->editor->window,what);
	what->editor->clear_url();
	list_for_adding->insert(what);
};

void init_add_clipboard_window(...) {
	init_add_window();
	tDownload *what=list_for_adding->last();
	what->editor->paste_url();
	gtk_widget_grab_focus(what->editor->ok_button);
};

void init_add_dnd_window(const char *url,const char *desc,const char *ref) {
	if (!url) return;
	init_add_window();
	tDownload *what=list_for_adding->last();
	what->editor->dnd=1;
	what->editor->set_url(url);
	what->editor->set_description(desc);
	what->editor->set_referer(ref);
	gtk_widget_grab_focus(what->editor->ok_button);
};

/* automated adding */

void d4x_automated_ok(GtkWidget *widget, tDownload *what) {
	if (what->editor->apply_changes()) return;
	int to_top=what->editor->get_to_top_check();
	int to_pause=what->editor->get_pause_check();
	list_for_adding->del(what);

	d4xAutoGenerator *gen=new d4xAutoGenerator;
	if (gen->init(what->editor->get_url())){
		delete(gen);
		delete(what);
		return;
	};
	int i=0;
	char *tmp=gen->first();
	while(tmp){
		tDownload *dwn=new tDownload;
		dwn->info=new tAddr(tmp);
		dwn->info->username.set(what->info->username.get());
		dwn->info->pass.set(what->info->pass.get());
		if (what->config->isdefault==0){
			dwn->config=new tCfg;
			dwn->config->copy(what->config);
			dwn->restart_from_begin=what->restart_from_begin;
			dwn->config->referer.set(what->config->referer.get());
			dwn->Name2Save.set(what->Name2Save.get());
			dwn->config->save_path.set(what->config->save_path.get());
			dwn->config->log_save_path.set(what->config->log_save_path.get());
		};
		if (to_pause){
			dwn->status=DL_PAUSE;
			_aa_.add_downloading_to(dwn,to_top);
		}else
			_aa_.add_downloading(dwn,to_top);
		delete[] tmp;
		i+=1;
		tmp=gen->next();
		if (i>1000){
			delete[] tmp;
			break;
		};
	};
	delete(gen);
	delete(what);
};

void d4x_automated_add(){
	if (list_for_adding==NULL) {
		list_for_adding=new tDList(DL_TEMP);
		list_for_adding->init(0);
	};
	tDownload *what=new tDownload;
	tAddr *info=new tAddr("ftp://somesite.org");
	what->info=info;
	what->config=new tCfg;
	what->set_default_cfg();
	what->config->save_path.set(D4X_QUEUE->save_path.get());

	what->editor=new tDEdit;
	what->editor->add_or_edit=1;
	what->editor->not_url_history=1;
	what->editor->init(what);
	gtk_window_set_title(GTK_WINDOW(what->editor->window),_("Automated adding"));
	g_signal_connect(G_OBJECT(what->editor->cancel_button),"clicked",G_CALLBACK(add_window_cancel), what);
	g_signal_connect(G_OBJECT(what->editor->ok_button),"clicked",G_CALLBACK(d4x_automated_ok),what);
	g_signal_connect(G_OBJECT(what->editor->window),"delete_event",G_CALLBACK(add_window_delete), what);
	d4x_eschandler_init(what->editor->window,what);
	what->editor->clear_url();
	what->editor->paste_url();
	list_for_adding->insert(what);
};

/*******************************************/
static gint _tmp_compare_(gconstpointer a,gconstpointer b){
	gint aa=GPOINTER_TO_INT(a);
	gint bb=GPOINTER_TO_INT(b);
	return(bb-aa);
};

static void _apply_common_properties_(GtkTreeModel *model,GtkTreePath *path,
				      GtkTreeIter *iter,gpointer data){
	GValue val={0,};
	gtk_tree_model_get_value(model,iter,NOTHING_COL,&val);
	tDownload *tmp=(tDownload *)g_value_peek_pointer(&val);
	g_value_unset(&val);
	tDownload *what=(tDownload *)data;
	if (tmp && tmp->owner()!=DL_RUN && tmp->owner()!=DL_STOPWAIT){
		if (tmp->config==NULL){
			tmp->config=new tCfg;
			tmp->set_default_cfg();
			tmp->config->isdefault=0;
		};
		what->editor->set_parent(tmp);
		tmp->editor->apply_enabled_changes();
		tmp->editor->set_parent(what);
	};
};

void edit_common_properties_ok(GtkWidget *widget, tDownload *what){
	/* FIXME: too deep access via 'D4X_QUEUE->qv.' */
	GtkTreeSelection *sel=gtk_tree_view_get_selection(GTK_TREE_VIEW(D4X_QUEUE->qv.ListOfDownloads));
	gtk_tree_selection_selected_foreach(sel,_apply_common_properties_,what);
	what->editor->set_parent(what);
	what->delete_editor();
	list_for_adding->del(what);
	delete(what);
};

void init_edit_common_properties_window(int *array) {
	if (list_for_adding==NULL) {
		list_for_adding=new tDList(DL_TEMP);
		list_for_adding->init(0);
	};
	tDownload *what=new tDownload;
	tAddr *info=new tAddr("ftp://somesite.org");
	what->info=info;
	what->config=new tCfg;
	what->config->isdefault=0;
	what->config->save_path.set(CFG.GLOBAL_SAVE_PATH);
	what->set_default_cfg();

	what->editor=new tDEdit;
	what->editor->init(what);
	gtk_widget_hide(what->editor->isdefault_check);
	what->editor->disable_items(array);
	gtk_window_set_title(GTK_WINDOW(what->editor->window),_("Add new download"));
	g_signal_connect(G_OBJECT(what->editor->cancel_button),"clicked",G_CALLBACK(add_window_cancel), what);
	g_signal_connect(G_OBJECT(what->editor->ok_button),"clicked",G_CALLBACK(edit_common_properties_ok),what);
	g_signal_connect(G_OBJECT(what->editor->window),"delete_event",G_CALLBACK(add_window_delete), what);
	d4x_eschandler_init(what->editor->window,what);
	what->editor->clear_url();
	list_for_adding->insert(what);
	
	gtk_window_set_transient_for (GTK_WINDOW (what->editor->window), GTK_WINDOW (MainWindow));
	gtk_window_set_modal (GTK_WINDOW(what->editor->window),TRUE);
};
