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
#include <package_config.h>
#include <stdio.h>
#include <sys/stat.h>
#include "passface.h"
#include "misc.h"
#include "edit.h"
#include "../addr.h"
#include "../ntlocale.h"
#include "../var.h"
#include <gdk/gdkkeysyms.h>
#include <regex.h>

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
static void add_url_ok(GtkWidget *widget, tDownload *dwn){
	FaceForPasswords->addlist_add(dwn);
};
static void add_url_delete(GtkWidget *widget,GdkEvent *event, tDownload *dwn){
	FaceForPasswords->addlist_del(dwn);
};

static void edit_url_cancel(GtkWidget *widget, tDownload *dwn){
	dwn->delete_editor();
};
static void edit_url_ok(GtkWidget *widget, tDownload *dwn){
	if (dwn->editor->apply_changes()) return;
	dwn->Name2Save.set(dwn->editor->get_url());
	dwn->delete_editor();
	FaceForPasswords->redraw_url(dwn);
};
static void edit_url_delete(GtkWidget *widget,GdkEvent *event, tDownload *dwn){
	dwn->delete_editor();
};

tFacePass::tFacePass(){
	window=NULL;
};

tFacePass::~tFacePass(){
	close();
};

void tFacePass::addlist_del(tDownload *dwn){
	addlist.del(dwn);
	delete(dwn);
};

void tFacePass::show_url(tDownload *dwn){
	char *URL=dwn->Name2Save.get();
	char *data[]={URL};
	gint row=gtk_clist_append(GTK_CLIST(clist),data);
	gtk_clist_set_row_data(GTK_CLIST(clist),row,gpointer(dwn));
};

void tFacePass::redraw_url(tDownload *dwn){
	gint row=gtk_clist_find_row_from_data(GTK_CLIST(clist),dwn);
	if (row>=0){
		gtk_clist_set_text(GTK_CLIST(clist),row,0,dwn->Name2Save.get());
	};
};

void tFacePass::addlist_add(tDownload *dwn){
	if (dwn->editor->apply_changes()) return;
	addlist.del(dwn);
	dwn->Name2Save.set(dwn->editor->get_url());
	dwn->delete_editor();
	dlist.insert(dwn);
	show_url(dwn);
};

void tFacePass::open_dialog() {
	tDownload *dwn=new tDownload;
	dwn->config=new tCfg;
	dwn->config->isdefault=0;
	dwn->set_default_cfg();
	addlist.insert(dwn);
	dwn->info=new tAddr("ftp://somesite.org");
	dwn->editor=new tDEdit;
	dwn->editor->add_or_edit=1;
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
		tDownload *dwn=(tDownload *)gtk_clist_get_row_data(GTK_CLIST(clist),row);
		dlist.del(dwn);
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
	gtk_window_set_title(GTK_WINDOW (window),_("Default settings"));
	gtk_window_set_position(GTK_WINDOW(window),GTK_WIN_POS_CENTER);
	gtk_widget_set_usize(window,-1,400);
	gtk_container_border_width(GTK_CONTAINER(window),5);
	gchar *titles[]={"URL regexp"};
	clist = gtk_clist_new_with_titles(1, titles);
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
		show_url(dwn);
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
			tDownload *tmp=new tDownload;
			if (tmp->load_from_config(fd))
				delete(tmp);
			else
				dlist.insert(tmp);
		};
	};
	::close(fd);
};

void tFacePass::set_cfg(tDownload *what){
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
	if (dwn){
		if (dwn->config){
			if (what->config==NULL) what->config=new tCfg;
			what->config->copy(dwn->config);
			what->config->restart_from_begin=dwn->config->restart_from_begin;
			what->config->referer.set(dwn->config->referer.get());
			what->config->save_path.set(dwn->config->save_path.get());
			what->config->log_save_path.set(dwn->config->log_save_path.get());
			what->config->isdefault=0;
		};
		if (dwn->split==NULL && what->split)
			delete(what->split);
		if (dwn->split){
			if (what->split==NULL) what->split=new tSplitInfo;
			what->split->NumOfParts=dwn->split->NumOfParts;
		};
	};
};
