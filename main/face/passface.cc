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

#include <stdio.h>
#include <sys/stat.h>
#include "passface.h"
#include "misc.h"
#include "edit.h"
#include "../addr.h"
#include "../ntlocale.h"
#include "../var.h"
#include "../main.h"
#include "list.h"
#include <gdk/gdkkeysyms.h>

tFacePass *FaceForPasswords=(tFacePass *)NULL;

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

static void add_url_cancel(GtkWidget *widget, tDownload *dwn){
	FaceForPasswords->addlist_del(dwn);
};
static void add_url_ok(GtkWidget *widget, tLimitDownload *dwn){
	FaceForPasswords->addlist_add(dwn);
};
static void add_url_delete(GtkWidget *widget,GdkEvent *event, tDownload *dwn){
	FaceForPasswords->addlist_del(dwn);
};

static void edit_url_cancel(GtkWidget *widget, tDownload *dwn){
	dwn->delete_editor();
};
static void edit_url_ok(GtkWidget *widget, tLimitDownload *dwn){
	if (dwn->editor->apply_changes()) return;
	dwn->Name2Save.set(dwn->editor->get_url());
	FaceForPasswords->recalc_run(dwn);
	dwn->delete_editor();
	FaceForPasswords->redraw_url(dwn);
};
static void edit_url_delete(GtkWidget *widget,GdkEvent *event, tDownload *dwn){
	dwn->delete_editor();
};

tFacePass::tFacePass(){
	do_not_run=0;
	window=NULL;
};

tFacePass::~tFacePass(){
	do_not_run=1;
	tDownload *dwn=dlist.first();
	while(dwn){
		free_matched((tLimitDownload*)dwn);
		dwn=dlist.prev();
	};
	close();
};

void tFacePass::set_do_not_run(int a){do_not_run=1;};

void tFacePass::addlist_del(tDownload *dwn){
	addlist.del(dwn);
	delete(dwn);
};

void tFacePass::show_url(tLimitDownload *dwn){
	char *URL=dwn->Name2Save.get();
	char data1[100];
	char data2[100];
	char *data[]={URL,data1,data2};
	sprintf(data1,"%i",dwn->config->con_limit);
	sprintf(data2,"%i",dwn->cur_limit);
	gint row=gtk_clist_append(GTK_CLIST(clist),data);
	gtk_clist_set_row_data(GTK_CLIST(clist),row,gpointer(dwn));
};

void tFacePass::redraw_url(tLimitDownload *dwn){
	if (!window) return;
	gint row=gtk_clist_find_row_from_data(GTK_CLIST(clist),dwn);
	if (row>=0){
		char data1[100];
		sprintf(data1,"%i",dwn->config->con_limit);
		gtk_clist_set_text(GTK_CLIST(clist),row,0,dwn->Name2Save.get());
		gtk_clist_set_text(GTK_CLIST(clist),row,1,data1);
		sprintf(data1,"%i",dwn->cur_limit);
		gtk_clist_set_text(GTK_CLIST(clist),row,2,data1);
	};
};

void tFacePass::calc_matched_run_rec(tQueue *q,tLimitDownload *dwn,regex_t *reg){
	d4xDownloadQueue *dq=(d4xDownloadQueue*)(q->first());
	while (dq){
		tDownload *what=dq->first(DL_RUN);
		while (what){
			if (what->regex_match==NULL){
				char *url=what->info->url();
				if (regexec(reg,url,0,NULL,0)==0){
					what->regex_match=new d4xDwnLink(what,dwn);  //awful lists for fast access
					what->regex_match->q=&(dwn->lim_run);
					dwn->lim_run.insert(what->regex_match);  //on REAL LARGE queues :-)
					if (what->split){
						dwn->cur_limit+=what->split->runcount - what->split->stopcount;
					}else
						dwn->cur_limit+=1;
				};
				delete[] url;
			};
			what=(tDownload *)(what->prev);
		};
		calc_matched_run_rec(&(dq->child),dwn,reg);
		dq=(d4xDownloadQueue *)(dq->prev);
	};
};

void tFacePass::calc_matched_run(tLimitDownload *dwn){
	// walk over all run queues to match limit
	regex_t reg;
	if (regcomp(&reg,dwn->Name2Save.get(),REG_NOSUB)==0){
		calc_matched_run_rec(&D4X_QTREE,dwn,&reg);
		regfree(&reg);
	};
};

void tFacePass::free_matched(tLimitDownload *dwn){
	d4xDwnLink *lnk=(d4xDwnLink *)dwn->lim_run.first();
	while(lnk){
		lnk->dwn->regex_match=NULL;
		dwn->lim_run.dispose();
		lnk=(d4xDwnLink *)dwn->lim_run.first();
	};
	lnk=(d4xDwnLink *)dwn->limited.first();
	while(lnk){
		lnk->dwn->regex_match=NULL;
		d4xDownloadQueue *dq=lnk->dwn->myowner->PAPA;
		dq->del(lnk->dwn);
		aa.insert_into_wait_list(lnk->dwn,dq);
		if (!do_not_run)
			aa.try_to_run_wait(dq);
		dwn->limited.dispose();
		lnk=(d4xDwnLink *)dwn->limited.first();
	};
};

void tFacePass::rerun_wait_queues(tQueue *q){
	d4xDownloadQueue *dq=(d4xDownloadQueue*)(q->first());
	while (dq){
		aa.try_to_run_wait(dq);
		rerun_wait_queues(&(dq->child));
		dq=(d4xDownloadQueue *)(dq->prev);
	};
};

void tFacePass::recalc_run(tLimitDownload *dwn){
	int need2rw=dwn->limited.first()?1:0;
	do_not_run=1;
	free_matched(dwn);
	do_not_run=0;
	dwn->cur_limit=0;
	if (dwn->config->con_limit>0)
		calc_matched_run(dwn);
	if (need2rw)
		rerun_wait_queues(&D4X_QTREE);
};

void tFacePass::stop_matched(tDownload *dwn){
	if (dwn->regex_match==NULL) return;
	dwn->regex_match->q->del(dwn->regex_match);
	delete(dwn->regex_match);
	dwn->regex_match=NULL;
};

void tFacePass::limit_dec(tDownload *what){
	if (what->regex_match==NULL) return;
	tLimitDownload *dwn=(tLimitDownload *)(what->regex_match->papa);
	dwn->cur_limit--;
	redraw_url(dwn);
	if (dwn->cur_limit<dwn->config->con_limit){
		d4xDwnLink *lnk=(d4xDwnLink *)dwn->limited.first();
		if (lnk){
			lnk->dwn->regex_match=NULL;
			d4xDownloadQueue *dq=lnk->dwn->myowner->PAPA;
			dq->del(lnk->dwn);
			aa.insert_into_wait_list(lnk->dwn,dq);
			if (!do_not_run)
				aa.try_to_run_wait(dq);
			dwn->limited.dispose();
		};
	};
};

void tFacePass::limit_inc(tDownload *what){
	if (what->regex_match==NULL) return;
	tLimitDownload *dwn=(tLimitDownload *)(what->regex_match->papa);
	dwn->cur_limit++;
	redraw_url(dwn);
};

void tFacePass::limit_to_run(tDownload *what){
	if (what->regex_match==NULL) return;
	tLimitDownload *dwn=(tLimitDownload *)(what->regex_match->papa);
	dwn->limited.del(what->regex_match);
	dwn->lim_run.insert(what->regex_match);
	what->regex_match->q=&(dwn->lim_run);
	dwn->cur_limit++;
	redraw_url(dwn);
};

int tFacePass::limit_check(tDownload *what){
	if (what->regex_match){
		tLimitDownload *dwn=(tLimitDownload *)(what->regex_match->papa);
		return(dwn->cur_limit>=dwn->config->con_limit);
	};
	return(0);
};


int tFacePass::match_and_check(tDownload *what,int move){
	tLimitDownload *dwn=find_match(what);
	if (dwn && dwn->config->con_limit>0 && what->regex_match==NULL){
		if (!move){
			d4xDownloadQueue *dq=what->myowner->PAPA;
			dq->del(what);
			dq->add(what,DL_LIMIT);
		};
		what->regex_match=new d4xDwnLink(what,dwn);
		what->regex_match->q=&(dwn->limited);
		dwn->limited.insert(what->regex_match);
	};
	return(limit_check(what));
};

void tFacePass::addlist_add(tLimitDownload *dwn){
	if (dwn->editor->apply_changes()) return;
	addlist.del(dwn);
	dwn->Name2Save.set(dwn->editor->get_url());
	dwn->delete_editor();
	dlist.insert(dwn);
	if (dwn->config->con_limit>0)
		calc_matched_run(dwn);
	show_url(dwn);
};

void tFacePass::open_dialog() {
	tLimitDownload *dwn=new tLimitDownload;
	dwn->config=new tCfg;
	dwn->config->isdefault=0;
	dwn->set_default_cfg();
	addlist.insert(dwn);
	dwn->info=new tAddr("ftp://somesite.org");
	dwn->editor=new tDEdit;
	dwn->editor->add_or_edit=1;
	dwn->editor->limit=1;
	dwn->editor->init(dwn);
	gtk_window_set_title(GTK_WINDOW(dwn->editor->window),_("Add new URL to URL-manager"));
	gtk_signal_connect(GTK_OBJECT(dwn->editor->cancel_button),"clicked",GTK_SIGNAL_FUNC(add_url_cancel), dwn);
	gtk_signal_connect(GTK_OBJECT(dwn->editor->ok_button),"clicked",GTK_SIGNAL_FUNC(add_url_ok),dwn);
	gtk_signal_connect(GTK_OBJECT(dwn->editor->window),"delete_event",GTK_SIGNAL_FUNC(add_url_delete), dwn);
	d4x_eschandler_init(dwn->editor->window,dwn);
	gtk_widget_set_sensitive(dwn->editor->isdefault_check,FALSE);
	dwn->editor->clear_save_name();
	dwn->editor->disable_time();
	dwn->editor->disable_save_name();
	dwn->editor->clear_url();
};

void tFacePass::delete_rows() {
        GList *select=GTK_CLIST(clist)->selection;
	if (select) {
		int row=GPOINTER_TO_INT(select->data);
		tLimitDownload *dwn=(tLimitDownload *)gtk_clist_get_row_data(GTK_CLIST(clist),row);
		dlist.del(dwn);
		free_matched(dwn);
		delete(dwn);
		gtk_clist_remove(GTK_CLIST(clist),row);
		select=GTK_CLIST(clist)->selection;
	};
};

void tFacePass::edit_row(int row) {
	tDownload *dwn=(tDownload *)gtk_clist_get_row_data(GTK_CLIST(clist),row);
	if (!dwn) return;
	if (dwn->editor) return;
	dwn->editor=new tDEdit;
	dwn->editor->add_or_edit=1;
	dwn->editor->limit=1;
	dwn->editor->init(dwn);
	gtk_window_set_title(GTK_WINDOW(dwn->editor->window),_("Edit default preferences"));
	gtk_signal_connect(GTK_OBJECT(dwn->editor->cancel_button),"clicked",GTK_SIGNAL_FUNC(edit_url_cancel), dwn);
	gtk_signal_connect(GTK_OBJECT(dwn->editor->ok_button),"clicked",GTK_SIGNAL_FUNC(edit_url_ok),dwn);
	gtk_signal_connect(GTK_OBJECT(dwn->editor->window),"delete_event",GTK_SIGNAL_FUNC(edit_url_delete), dwn);
	d4x_eschandler_init(dwn->editor->window,dwn);
	gtk_widget_set_sensitive(dwn->editor->isdefault_check,FALSE);
	dwn->editor->clear_save_name();
	dwn->editor->disable_time();
	dwn->editor->disable_save_name();
	dwn->editor->set_url(dwn->Name2Save.get());
};

void tFacePass::apply_dialog() {
};

void tFacePass::init(){
	if (window) {
		gdk_window_show(window->window);
		return;
	};
	window = gtk_window_new(GTK_WINDOW_DIALOG);
	gtk_window_set_wmclass(GTK_WINDOW(window),
			       "D4X_Passwords","D4X");
	gtk_window_set_title(GTK_WINDOW (window),_("URL-manager"));
	gtk_window_set_position(GTK_WINDOW(window),GTK_WIN_POS_CENTER);
	gtk_widget_set_usize(window,-1,400);
	gtk_container_border_width(GTK_CONTAINER(window),5);
	gchar *titles[]={"URL regexp",_("limit"),_("con.")};
	clist = gtk_clist_new_with_titles(3, titles);
	gtk_signal_connect(GTK_OBJECT(clist),"select_row",GTK_SIGNAL_FUNC(face_pass_clist_handler),this);
	gtk_clist_set_shadow_type (GTK_CLIST(clist), GTK_SHADOW_IN);
	gtk_clist_set_column_auto_resize(GTK_CLIST(clist),0,TRUE);
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
	gtk_widget_show_all(window);
	gtk_signal_connect(GTK_OBJECT(clist),"event",GTK_SIGNAL_FUNC(face_pass_list_event_callback),this);
	gtk_signal_connect(GTK_OBJECT(button),"clicked",GTK_SIGNAL_FUNC(face_pass_ok),this);
	gtk_signal_connect(GTK_OBJECT(del_button),"clicked",GTK_SIGNAL_FUNC(face_pass_del),this);
	gtk_signal_connect(GTK_OBJECT(add_button),"clicked",GTK_SIGNAL_FUNC(face_pass_add),this);
	gtk_signal_connect(GTK_OBJECT(window),"delete_event",GTK_SIGNAL_FUNC(face_pass_delete), this);
	tDownload *dwn=dlist.first();
	while(dwn){
		show_url((tLimitDownload*)dwn);
		dwn=dlist.prev();
	};
	d4x_eschandler_init(window,this);
};

void tFacePass::close() {
	if (window) {
		gtk_widget_destroy(window);
		window=NULL;
	};
	tDownload *dwn=addlist.first();
	while(dwn){
		addlist_del(dwn);
		dwn=addlist.first();
	};
};

static char *CFG_URLMANAGER="urlmanager";

void tFacePass::save(){
	if (!HOME_VARIABLE) return;
	char *cfgpath=sum_strings(HOME_VARIABLE,"/",CFG_DIR,"/",CFG_URLMANAGER,NULL);
	int fd=open(cfgpath,O_TRUNC | O_CREAT |O_RDWR,S_IRUSR | S_IWUSR);
	delete[] cfgpath;
	if (fd<0) return;
	tDownload *dwn=dlist.first();
	while(dwn){
		dwn->save_to_config(fd);
		dwn=dlist.prev();
	};
	::close(fd);
};

void tFacePass::load(){
	if (!HOME_VARIABLE) return;
	char *cfgpath=sum_strings(HOME_VARIABLE,"/",CFG_DIR,"/",CFG_URLMANAGER,NULL);
	int fd=open(cfgpath,O_RDONLY);
	delete[] cfgpath;
	if (fd<0) return;
	char buf[MAX_LEN];
	while(f_rstr(fd,buf,MAX_LEN)>0){
		if (equal_uncase(buf,"Download:")){
			tLimitDownload *tmp=new tLimitDownload;
			if (tmp->load_from_config(fd))
				delete(tmp);
			else
				dlist.insert(tmp);
		};
	};
	::close(fd);
};

tLimitDownload *tFacePass::find_match(tDownload *what){
	char *url=what->info->url();
	tDownload *dwn=dlist.first();
	while(dwn){
		regex_t reg;
		if (regcomp(&reg,dwn->Name2Save.get(),REG_NOSUB)==0){
			if (regexec(&reg,url,0,NULL,0)==0){
				regfree(&reg);
				break;
			};
			regfree(&reg);
		};
		dwn=dlist.prev();
	};
	delete[] url;
	return((tLimitDownload *)(dwn));
};

void tFacePass::set_cfg(tDownload *what){
	tLimitDownload *dwn=find_match(what);
	if (dwn){
		if (dwn->config){
			if (what->config==NULL) what->config=new tCfg;
			what->config->copy(dwn->config);
			what->restart_from_begin=dwn->restart_from_begin;
			what->config->referer.set(dwn->config->referer.get());
			what->config->save_path.set(dwn->config->save_path.get());
			what->config->log_save_path.set(dwn->config->log_save_path.get());
			what->config->isdefault=0;
		};
		what->info->pass.set(dwn->info->pass.get());
		what->info->username.set(dwn->info->username.get());
		if (dwn->split==NULL && what->split)
			delete(what->split);
		if (dwn->split){
			if (what->split==NULL) what->split=new tSplitInfo;
			what->split->NumOfParts=dwn->split->NumOfParts;
		};
	};
};
