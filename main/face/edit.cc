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
#include <string.h>
#include <strings.h>
#include "edit.h"
#include "list.h"
#include "misc.h"
#include "mywidget.h"
#include "addd.h"
#include "../history.h"
#include "../var.h"
#include "../locstr.h"
#include "../main.h"
#include "../ntlocale.h"
#include "../filter.h"
#include <gdk/gdkkeysyms.h>

enum EDIT_OPTIONS_ENUM{
	EDIT_OPT_USERPASS=0,
	EDIT_OPT_SAVEPATH,
	EDIT_OPT_FROMBEGIN,
	EDIT_OPT_TIMEOUT,
	EDIT_OPT_SPEED,
	EDIT_OPT_ATTEMPTS,
	EDIT_OPT_SLEEPTIME,
	EDIT_OPT_ROLLBACK,
	EDIT_OPT_DATE,
	EDIT_OPT_IFNOREGET,
	EDIT_OPT_SPLIT,
	EDIT_OPT_PASSIVEFTP,
	EDIT_OPT_PERMISSIONS,
	EDIT_OPT_DONT_SEND_QUIT,
	EDIT_OPT_DIRONTOP,
	EDIT_OPT_CHECK_TIME,
	EDIT_OPT_SLEEP_BEFORE_COMPLETE,
	EDIT_OPT_FOLLOWLINK,
	EDIT_OPT_RECURSEDEPTHFTP,
	EDIT_OPT_RECURSEDEPTHHTTP,
	EDIT_OPT_LEAVEDIR,
	EDIT_OPT_LEAVESERVER,
	EDIT_OPT_CHANGE_LINKS,
	EDIT_OPT_IHATEETAG,
	EDIT_OPT_USERAGENT,
	EDIT_OPT_REFERER,
	EDIT_OPT_COOKIE,
	EDIT_OPT_PROXY,
	EDIT_OPT_TIME,
	EDIT_OPT_LASTOPTION
};

char *edit_fields_labels[]={
	N_("Use password for this site"),
	N_("Save download to folder"),
	N_("Restart this download from begining"),
	N_("Timeout for reading from socket"),
	N_("Speed limitation"),
	N_("Maximum attempts"),
	N_("Timeout before reconnection"),
	N_("Rollback after reconnecting"),
	N_("Get date from the server"),
	N_("Retry if resuming is not supported"),
	N_("Number of parts for spliting this download"),
	N_("Use passive mode for FTP"),
	N_("Get permissions of the file from server (FTP only)"),
	N_("Don't send QUIT command (FTP)"),
	N_("Put directories on the top of queue during recursion"),
	N_("Compare date/time of remote file with local one"),
	N_("Sleep before completing"),
	N_("Follow symbolic links"),
	N_("Depth of recursing for FTP"),
	N_("Depth of recursing for HTTP"),
	N_("Only subdirs"),
	N_("Allow leave this server while recursing via HTTP"),
	N_("Change links in HTML file to local"),
	N_("Ignore ETag field in reply"),
	N_("User-Agent"),
	N_("Referer"),
	N_("Cookie"),
	N_("Proxy"),
	N_("Time")
};

extern tMain aa;
void edit_window_cancel(GtkWidget *parent,tDEdit *where);
gint edit_window_delete(GtkObject *parent);
void edit_window_ok(GtkWidget *which,tDEdit *where);

GList *make_glist_from_mylist(tHistory *parent) {
	GList *rvalue=NULL;
	tString *tmp=parent->last();
	while (tmp) {
		rvalue = g_list_append (rvalue, tmp->body);
		tmp=parent->next();
	};
	return rvalue;
};

GtkWidget *my_gtk_combo_new(tHistory *history) {
	GtkWidget *combo=gtk_combo_new();
	GList *list=make_glist_from_mylist(history);
	if (list){
		gtk_combo_set_popdown_strings (GTK_COMBO (combo), list);
		g_list_free(list);
	};
	gtk_combo_set_case_sensitive(GTK_COMBO(combo),TRUE);
	return combo;
};

GtkWidget *my_gtk_combo_new(int from,int to) {
	GList *rvalue=NULL;
	for (int i=from;i<=to;i++) {
		char data[MAX_LEN];
		sprintf(data,"%i",i);
		char *tmp=copy_string(data);
		rvalue = g_list_append (rvalue,tmp);
	};
	GtkWidget *combo=gtk_combo_new();
	if (rvalue){
		gtk_combo_set_popdown_strings (GTK_COMBO (combo), rvalue);
		GList *tmplist=rvalue;
		while(tmplist){
			char *a=(char *)(tmplist->data);
			delete[] a;
			tmplist=tmplist->next;
		};
		g_list_free(rvalue);
	};
	gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(combo)->entry),FALSE);
	gtk_combo_set_case_sensitive(GTK_COMBO(combo),TRUE);
	return combo;
};

GtkWidget *my_gtk_combo_new_month() {
	GList *rvalue=NULL;
	char *tmp=copy_string("Jan");
	rvalue = g_list_append (rvalue,tmp);
	tmp=copy_string("Feb");
	rvalue = g_list_append (rvalue,tmp);
	tmp=copy_string("Mar");
	rvalue = g_list_append (rvalue,tmp);
	tmp=copy_string("Apr");
	rvalue = g_list_append (rvalue,tmp);
	tmp=copy_string("May");
	rvalue = g_list_append (rvalue,tmp);
	tmp=copy_string("Jun");
	rvalue = g_list_append (rvalue,tmp);
	tmp=copy_string("Jul");
	rvalue = g_list_append (rvalue,tmp);
	tmp=copy_string("Aug");
	rvalue = g_list_append (rvalue,tmp);
	tmp=copy_string("Sep");
	rvalue = g_list_append (rvalue,tmp);
	tmp=copy_string("Oct");
	rvalue = g_list_append (rvalue,tmp);
	tmp=copy_string("Nov");
	rvalue = g_list_append (rvalue,tmp);
	GtkWidget *combo=gtk_combo_new();
	if (rvalue)
		gtk_combo_set_popdown_strings (GTK_COMBO (combo), rvalue);
	gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(combo)->entry),FALSE);
	return combo;
};

/******************************************************/
void init_edit_window(tDownload *what) {
	if (!what) return;
	if (what->editor) {
		what->editor->popup();
		return;
	};
	if (what->config==NULL){
		what->config=new tCfg;
		what->set_default_cfg();
	};
	what->editor=new tDEdit;
	what->editor->init(what);
	what->editor->parent_in_db=1;
	if (what->owner()==DL_RUN ||
	    what->owner()==DL_STOPWAIT)
		what->editor->disable_ok_button();
	gtk_window_set_title(GTK_WINDOW(what->editor->window),_("Edit download"));
	gtk_signal_connect(GTK_OBJECT(what->editor->cancel_button),"clicked",GTK_SIGNAL_FUNC(edit_window_cancel),what->editor);
	gtk_signal_connect(GTK_OBJECT(what->editor->ok_button),"clicked",GTK_SIGNAL_FUNC(edit_window_ok),what->editor);
	gtk_signal_connect(GTK_OBJECT(what->editor->window),"delete_event",GTK_SIGNAL_FUNC(edit_window_delete), what->editor);
	d4x_eschandler_init(what->editor->window,what->editor);
};

void init_edit_window_without_ok(tDownload *what) {
	if (!what) return;
	if (what->editor) {
		what->editor->popup();
		return;
	};
	what->editor=new tDEdit;
	what->editor->init(what);
	if (what->owner()==DL_RUN ||
	    what->owner()==DL_STOPWAIT)
		what->editor->disable_ok_button();
	gtk_window_set_title(GTK_WINDOW(what->editor->window),_("Edit download"));
	gtk_signal_connect(GTK_OBJECT(what->editor->cancel_button),"clicked",GTK_SIGNAL_FUNC(edit_window_cancel),what->editor);
	gtk_signal_connect(GTK_OBJECT(what->editor->window),"delete_event",GTK_SIGNAL_FUNC(edit_window_delete), what->editor);
	d4x_eschandler_init(what->editor->window,what->editor);
};

void edit_window_cancel(GtkWidget *parent,tDEdit *where) {
	delete where;
};

gint edit_window_delete(GtkObject *parent) {
	tDEdit *tmp=(tDEdit *)gtk_object_get_user_data(parent);
	delete tmp;
	return TRUE;
};

void edit_window_url_activate(GtkWidget *which,tDEdit *where){
	if (GTK_WIDGET_SENSITIVE(where->ok_button))
		gtk_signal_emit_by_name(GTK_OBJECT(where->ok_button),
					"clicked",where);
};

void edit_window_ok(GtkWidget *which,tDEdit *where) {
	if (where->apply_changes())
		return;
	tDownload *dwn=where->get_parent();
	if (dwn->config->isdefault){
		delete(dwn->config);
		dwn->config=NULL;
	};
	if (where->parent_in_db){
		DQV(dwn).update(dwn);
		if (!where->get_pause_check())
			aa.continue_download(dwn);
	};
	delete where;
};

static void edit_browser_path_set_as_default(GtkWidget *parent,tDEdit *where){
	where->set_path_as_default();
};

static void edit_window_password(GtkWidget *parent,tDEdit *where) {
	where->setup_entries();
};

static void edit_isdefault_check_clicked(GtkWidget *parent,tDEdit *where) {
	where->toggle_isdefault();
};

static void edit_time_check_clicked(GtkWidget *parent,tDEdit *where) {
	where->toggle_time();
};

static void edit_auto_log_clicked(GtkWidget *parent,tDEdit *where){
	where->auto_fill_log();
};

static void edit_filter_sel_clicked(GtkWidget *parent,tDEdit *where){
	where->init_filter_sel();
};

static void edit_filter_sel_ok(GtkWidget *parent,tDEdit *where){
	where->filter_ok();
};
static void edit_filter_sel_select(GtkWidget *clist, gint row, gint column,
				  GdkEventButton *event,
				   tDEdit *where) {
	if (event && event->type==GDK_2BUTTON_PRESS && event->button==1)
		where->filter_ok();
};
static void edit_filter_sel_cancel(GtkWidget *parent,tDEdit *where){
	where->filter_cancel();
};
static void edit_filter_sel_delete(GtkWidget *parent,
				   GdkEvent *event,
				   tDEdit *where){
	where->filter_cancel();
};
/******************************************************/

tDEdit::tDEdit() {
	parent=NULL;
	window=NULL;
	proxy=NULL;
	filter_sel=NULL;
	parent_in_db=0;
	add_or_edit=dnd=0;
};

void tDEdit::popup() {
	if (window)
		gdk_window_show(window->window);
};

tDownload *tDEdit::get_parent(){
	return parent;
};

void tDEdit::set_parent(tDownload *what){
	if (parent)
		parent->editor=NULL;
	if (what)
		what->editor=this;
	parent=what;		
};

void tDEdit::set_path_as_default(){
	if (CFG.GLOBAL_SAVE_PATH)
		delete[] CFG.GLOBAL_SAVE_PATH;
	CFG.GLOBAL_SAVE_PATH=normalize_path_full(text_from_combo(MY_GTK_FILESEL(path_entry)->combo));
};

void tDEdit::file_from_url(){
	char *a=text_from_combo(url_entry);
	char *b=text_from_combo(MY_GTK_FILESEL(file_entry)->combo);
	if (a && *a && (b==NULL || *b==0)){
		tAddr *adr=new tAddr(a);
		text_to_combo(MY_GTK_FILESEL(file_entry)->combo,adr->file.get());
		delete(adr);
	};
};

void tDEdit::file_check(){
	char *a=text_from_combo(MY_GTK_FILESEL(file_entry)->combo);
	char *b;
	if (a){
		b=rindex(a,'/');
		if (b){
			a=copy_string(b+1);
			text_to_combo(MY_GTK_FILESEL(file_entry)->combo,a);
			delete[] a;
		};
	};
	b=text_from_combo(url_entry);
	a=text_from_combo(MY_GTK_FILESEL(file_entry)->combo);
	if (b && *b && a){
		tAddr *adr=new tAddr(b);
		if (equal(adr->file.get(),a))
			text_to_combo(MY_GTK_FILESEL(file_entry)->combo,"");
		delete(adr);		
	};
};

static void edit_browser_file_focus(GtkWidget *widget,
				    GdkEvent *event,
				    tDEdit *edit){
	edit->file_from_url();
};

static void edit_browser_file_un_focus(GtkWidget *widget,
				    GdkEvent *event,
				    tDEdit *edit){
	edit->file_check();
};

void tDEdit::init_main(tDownload *who) {
	/* initing entries
	 */
	user_entry=my_gtk_combo_new(ALL_HISTORIES[USER_HISTORY]);
	if (CFG.REMEMBER_PASS)
		pass_entry=my_gtk_combo_new(ALL_HISTORIES[PASS_HISTORY]);
	else{
		pass_entry=gtk_entry_new_with_max_length(MAX_LEN);
		gtk_entry_set_visibility(GTK_ENTRY(pass_entry),FALSE);
	};
	path_entry=my_gtk_filesel_new(ALL_HISTORIES[PATH_HISTORY]);//my_gtk_combo_new(ALL_HISTORIES[PATH_HISTORY]);
	file_entry=my_gtk_filesel_new(ALL_HISTORIES[FILE_HISTORY]);//my_gtk_combo_new(ALL_HISTORIES[FILE_HISTORY]);
	GtkWidget *tmp_object=GTK_COMBO(MY_GTK_FILESEL(file_entry)->combo)->entry;
	gtk_signal_connect(GTK_OBJECT(tmp_object),"focus_out_event",
			   GTK_SIGNAL_FUNC(edit_browser_file_un_focus),this);
	gtk_signal_connect(GTK_OBJECT(tmp_object),"focus_in_event",
			   GTK_SIGNAL_FUNC(edit_browser_file_focus),this);
	MY_GTK_FILESEL(path_entry)->modal=GTK_WINDOW(window);
	MY_GTK_FILESEL(file_entry)->modal=GTK_WINDOW(window);
	url_entry=my_gtk_combo_new(ALL_HISTORIES[URL_HISTORY]);
	gtk_signal_connect(GTK_OBJECT(GTK_COMBO(url_entry)->entry), "activate",
			   GTK_SIGNAL_FUNC (edit_window_url_activate), this);
	MY_GTK_FILESEL(path_entry)->only_dirs=TRUE;
	desc_entry=my_gtk_combo_new(ALL_HISTORIES[DESC_HISTORY]);
	if (who->Description.get())
		set_description(who->Description.get());
	else
		set_description("");
	
//	char temp[MAX_LEN];
//	make_url_from_download(who,temp);
//	text_to_combo(url_entry,temp);
	char *URL=who->info->url();
	text_to_combo(url_entry,URL);
	delete[] URL;

	text_to_combo(MY_GTK_FILESEL(path_entry)->combo,who->config->save_path.get());
	if (who->Name2Save.get())
		text_to_combo(MY_GTK_FILESEL(file_entry)->combo,who->Name2Save.get());
	else text_to_combo(MY_GTK_FILESEL(file_entry)->combo,"");
	if (who->info->pass.get())
		text_to_combo(pass_entry,who->info->pass.get());
	else
		text_to_combo(pass_entry,"");
	if (who->info->username.get())
		text_to_combo(user_entry,who->info->username.get());
	/* initing labels
	 */
	GtkWidget *url_label=gtk_label_new("URL:");
	GtkWidget *desc_label=gtk_label_new(_("Description"));
	GtkWidget *path_label=gtk_label_new(_("Save download to folder"));
	GtkWidget *file_label=gtk_label_new(_("Save download to file"));
	GtkWidget *pass_label=gtk_label_new(_("password"));
	GtkWidget *user_label=gtk_label_new(_("user name"));
	/* set as default button
	 */
 	GtkWidget *path_set_as_default=gtk_button_new_with_label(_("Default"));
	gtk_signal_connect(GTK_OBJECT(path_set_as_default),"clicked",GTK_SIGNAL_FUNC(edit_browser_path_set_as_default),this);
	/* initing boxes
	 */
	GtkWidget *url_box=gtk_hbox_new(FALSE,0);
	GtkWidget *path_vbox=gtk_vbox_new(FALSE,0);
	GtkWidget *file_vbox=gtk_vbox_new(FALSE,0);
	GtkWidget *desc_vbox=gtk_vbox_new(FALSE,0);
	GtkWidget *pass_box=gtk_hbox_new(FALSE,0);
	GtkWidget *user_box=gtk_hbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(url_box),5);
	gtk_box_set_spacing(GTK_BOX(path_vbox),2);
	gtk_box_set_spacing(GTK_BOX(file_vbox),2);
	gtk_box_set_spacing(GTK_BOX(desc_vbox),2);
	gtk_box_set_spacing(GTK_BOX(user_box),5);
	gtk_box_set_spacing(GTK_BOX(pass_box),5);
	gtk_box_pack_start(GTK_BOX(url_box),url_label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(url_box),url_entry,TRUE,TRUE,0);
	gtk_box_pack_start(GTK_BOX(desc_vbox),desc_label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(desc_vbox),desc_entry,TRUE,TRUE,0);
	gtk_box_pack_start(GTK_BOX(path_entry),path_set_as_default,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(path_vbox),path_label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(path_vbox),path_entry,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(file_vbox),file_label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(file_vbox),file_entry,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(pass_box),pass_entry,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(pass_box),pass_label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(user_box),user_entry,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(user_box),user_label,FALSE,FALSE,0);

	use_pass_check=gtk_check_button_new_with_label(_("Use password for this site"));
	gtk_signal_connect(GTK_OBJECT(use_pass_check),"clicked",GTK_SIGNAL_FUNC(edit_window_password),this);
	if (who->info->username.get())
		GTK_TOGGLE_BUTTON(use_pass_check)->active=TRUE;
	else
		GTK_TOGGLE_BUTTON(use_pass_check)->active=FALSE;

	GtkWidget *vbox=gtk_vbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(vbox),5);
	gtk_box_pack_start(GTK_BOX(vbox),url_box,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),path_vbox,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),file_vbox,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),desc_vbox,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),use_pass_check,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),user_box,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),pass_box,FALSE,FALSE,0);	
	pause_check=gtk_check_button_new_with_label(_("Pause this just after adding"));
	restart_from_begin_check=gtk_check_button_new_with_label(_("Restart this download from begining"));
	GtkWidget *tmp_hbox=gtk_hbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(tmp_hbox),restart_from_begin_check,
			   FALSE,FALSE,0);
	if (add_or_edit){
		to_top_check=gtk_check_button_new_with_label(_("Add to top of queue"));
		gtk_box_pack_end(GTK_BOX(tmp_hbox),to_top_check,
				 FALSE,FALSE,0);
	}else
		to_top_check=NULL;

	if (who->owner()==DL_PAUSE)
		GTK_TOGGLE_BUTTON(pause_check)->active=1;
	else
		GTK_TOGGLE_BUTTON(pause_check)->active=CFG.PAUSE_AFTER_ADDING;
	GTK_TOGGLE_BUTTON(restart_from_begin_check)->active=who->config->restart_from_begin;
	gtk_box_pack_start(GTK_BOX(vbox),pause_check,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),tmp_hbox,FALSE,FALSE,0);
	GtkWidget *frame=gtk_frame_new(_("Download"));
	gtk_container_border_width(GTK_CONTAINER(frame),5);
	gtk_container_add(GTK_CONTAINER(frame),vbox);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook),frame,gtk_label_new(_("Main")));
};

void tDEdit::init_other(tDownload *who) {
	/* initing other
	 */
	GtkWidget *other_vbox=gtk_vbox_new(FALSE,0);
	timeout_entry=my_gtk_entry_new_with_max_length(3,who->config->timeout);
	sleep_entry=my_gtk_entry_new_with_max_length(3,who->config->time_for_sleep);
	attempts_entry=my_gtk_entry_new_with_max_length(3,who->config->number_of_attempts);
	rollback_entry=my_gtk_entry_new_with_max_length(5,who->config->rollback);
	speed_entry=my_gtk_entry_new_with_max_length(5,who->config->speed);
	split_entry=my_gtk_entry_new_with_max_length(2,who->split==NULL?0:who->split->NumOfParts);

	GtkWidget *other_hbox=gtk_hbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(other_hbox),5);
	GtkWidget *other_label=gtk_label_new(_("Timeout for reading from socket (in seconds)"));
	gtk_box_pack_start(GTK_BOX(other_hbox),timeout_entry,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(other_hbox),other_label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(other_vbox),other_hbox,FALSE,FALSE,0);

	other_hbox=gtk_hbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(other_hbox),5);
	other_label=gtk_label_new(_("Timeout before reconnection (in seconds)"));
	gtk_box_pack_start(GTK_BOX(other_hbox),sleep_entry,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(other_hbox),other_label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(other_vbox),other_hbox,FALSE,FALSE,0);

	other_hbox=gtk_hbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(other_hbox),5);
	other_label=gtk_label_new(_("Maximum attempts (0 for unlimited)"));
	gtk_box_pack_start(GTK_BOX(other_hbox),attempts_entry,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(other_hbox),other_label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(other_vbox),other_hbox,FALSE,FALSE,0);

	other_hbox=gtk_hbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(other_hbox),5);
	other_label=gtk_label_new(_("Rollback after reconnecting (in bytes)"));
	gtk_box_pack_start(GTK_BOX(other_hbox),rollback_entry,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(other_hbox),other_label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(other_vbox),other_hbox,FALSE,FALSE,0);

	other_hbox=gtk_hbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(other_hbox),5);
	other_label=gtk_label_new(_("Speed limitation in Bytes/sec (0 for unlimited)"));
	gtk_box_pack_start(GTK_BOX(other_hbox),speed_entry,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(other_hbox),other_label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(other_vbox),other_hbox,FALSE,FALSE,0);

	other_hbox=gtk_hbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(other_hbox),5);
	other_label=gtk_label_new(_("Number of parts for spliting this download"));
	gtk_box_pack_start(GTK_BOX(other_hbox),split_entry,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(other_hbox),other_label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(other_vbox),other_hbox,FALSE,FALSE,0);

	get_date_check=gtk_check_button_new_with_label(_("Get date from the server"));
	GTK_TOGGLE_BUTTON(get_date_check)->active=who->config->get_date;
	gtk_box_pack_start(GTK_BOX(other_vbox),get_date_check,FALSE,FALSE,0);

	retry_check=gtk_check_button_new_with_label(_("Retry if resuming is not supported"));
	GTK_TOGGLE_BUTTON(retry_check)->active=who->config->retry;
	gtk_box_pack_start(GTK_BOX(other_vbox),retry_check,FALSE,FALSE,0);

	sleep_check=gtk_check_button_new_with_label(_("Sleep before completing"));
	GTK_TOGGLE_BUTTON(sleep_check)->active=who->config->sleep_before_complete;
	gtk_box_pack_start(GTK_BOX(other_vbox),sleep_check,FALSE,FALSE,0);

	check_time_check=gtk_check_button_new_with_label(_("Compare date/time of remote file with local one"));
	GTK_TOGGLE_BUTTON(check_time_check)->active=who->config->check_time;
	gtk_box_pack_start(GTK_BOX(other_vbox),check_time_check,FALSE,FALSE,0);

	other_label=gtk_label_new(_("Save log to file"));
	GtkWidget *other_box=gtk_vbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(other_box),5);
	log_save_entry=my_gtk_filesel_new(ALL_HISTORIES[LOG_SAVE_HISTORY]);
	MY_GTK_FILESEL(log_save_entry)->modal=GTK_WINDOW(window);
	if (who->config->log_save_path.get())
		text_to_combo(MY_GTK_FILESEL(log_save_entry)->combo,
			      who->config->log_save_path.get());
	else
		text_to_combo(MY_GTK_FILESEL(log_save_entry)->combo,"");
	GtkWidget *auto_button=gtk_button_new_with_label(_("Auto"));
	gtk_signal_connect(GTK_OBJECT(auto_button),"clicked",GTK_SIGNAL_FUNC(edit_auto_log_clicked),this);
	gtk_box_pack_start(GTK_BOX(log_save_entry),auto_button,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(other_box),other_label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(other_box),log_save_entry,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(other_vbox),other_box,FALSE,FALSE,0);
	
	GtkWidget *other_frame=common_frame=gtk_frame_new(_("Common"));
	gtk_container_border_width(GTK_CONTAINER(other_frame),5);
	gtk_container_add(GTK_CONTAINER(other_frame),other_vbox);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook),other_frame,gtk_label_new(_("Common")));
};

void tDEdit::init_ftp(tDownload *who){
	GtkWidget *ftp_vbox=gtk_vbox_new(FALSE,0);

	ftp_passive_check=gtk_check_button_new_with_label(_("Use passive mode for FTP"));
	GTK_TOGGLE_BUTTON(ftp_passive_check)->active=who->config->passive;
	gtk_box_pack_start(GTK_BOX(ftp_vbox),ftp_passive_check,FALSE,FALSE,0);
	dont_send_quit_check=gtk_check_button_new_with_label(_("Don't send QUIT command (FTP)"));
	GTK_TOGGLE_BUTTON(dont_send_quit_check)->active=who->config->dont_send_quit;
	gtk_box_pack_start(GTK_BOX(ftp_vbox),dont_send_quit_check,FALSE,FALSE,0);	
	permisions_check=gtk_check_button_new_with_label(_("Get permissions of the file from server (FTP only)"));
	GTK_TOGGLE_BUTTON(permisions_check)->active=who->config->permisions;
	gtk_box_pack_start(GTK_BOX(ftp_vbox),permisions_check,FALSE,FALSE,0);
	follow_link_check=gtk_check_button_new_with_label(_("Follow symbolic links"));
	GTK_TOGGLE_BUTTON(follow_link_check)->active=who->config->follow_link;
	gtk_box_pack_start(GTK_BOX(ftp_vbox),follow_link_check,FALSE,FALSE,0);
	ftp_dirontop_check=gtk_check_button_new_with_label(_("Put directories on the top of queue during recursion"));
	GTK_TOGGLE_BUTTON(ftp_dirontop_check)->active=who->config->ftp_dirontop;
	gtk_box_pack_start(GTK_BOX(ftp_vbox),ftp_dirontop_check,FALSE,FALSE,0);
	
	ftp_recurse_depth_entry=my_gtk_entry_new_with_max_length(3,who->config->ftp_recurse_depth);
	GtkWidget *ftp_hbox=gtk_hbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(ftp_hbox),2);
	GtkWidget *other_label=gtk_label_new(_("Depth of recursing (0 unlimited,1 no recurse)"));
	gtk_box_pack_start(GTK_BOX(ftp_hbox),ftp_recurse_depth_entry,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(ftp_hbox),other_label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(ftp_vbox),ftp_hbox,FALSE,FALSE,0);
	
	ftp_frame=gtk_frame_new("FTP");
	gtk_container_border_width(GTK_CONTAINER(ftp_frame),5);
	gtk_container_add(GTK_CONTAINER(ftp_frame),ftp_vbox);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook),ftp_frame,gtk_label_new("FTP"));
};

void tDEdit::init_http(tDownload *who){
	GtkWidget *http_vbox=gtk_vbox_new(FALSE,0);
	
	http_recurse_depth_entry=my_gtk_entry_new_with_max_length(3,who->config->http_recurse_depth);
	GtkWidget *http_hbox=gtk_hbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(http_hbox),5);
	GtkWidget *other_label=gtk_label_new(_("Depth of recursing (0 unlimited,1 no recurse)"));
	gtk_box_pack_start(GTK_BOX(http_hbox),http_recurse_depth_entry,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(http_hbox),other_label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(http_vbox),http_hbox,FALSE,FALSE,0);

	leave_dir_check=gtk_check_button_new_with_label(_("Only subdirs"));
	leave_server_check=gtk_check_button_new_with_label(_("Allow leave this server while recursing via HTTP"));
	change_links_check=gtk_check_button_new_with_label(_("Change links in HTML file to local"));
	ihate_etag_check=gtk_check_button_new_with_label(_("Ignore ETag field in reply"));
	GTK_TOGGLE_BUTTON(leave_server_check)->active=who->config->leave_server;
	GTK_TOGGLE_BUTTON(leave_dir_check)->active=who->config->dont_leave_dir;
	GTK_TOGGLE_BUTTON(change_links_check)->active=who->config->change_links;
	GTK_TOGGLE_BUTTON(ihate_etag_check)->active=who->config->ihate_etag;
	gtk_box_pack_start(GTK_BOX(http_vbox),leave_server_check,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(http_vbox),leave_dir_check,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(http_vbox),change_links_check,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(http_vbox),ihate_etag_check,FALSE,FALSE,0);

	filter=gtk_entry_new();
	gtk_entry_set_editable(GTK_ENTRY(filter),FALSE);
	if (who->config->Filter.get())
		text_to_combo(filter,who->config->Filter.get());
	http_hbox=gtk_hbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(http_hbox),5);
	other_label=gtk_label_new(_("Filter"));
	GtkWidget *button=gtk_button_new_with_label(_("Select"));
 	gtk_signal_connect(GTK_OBJECT(button),"clicked",
			   GTK_SIGNAL_FUNC(edit_filter_sel_clicked),this);
	gtk_box_pack_start(GTK_BOX(http_hbox),other_label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(http_hbox),filter,TRUE,TRUE,0);
	gtk_box_pack_start(GTK_BOX(http_hbox),button,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(http_vbox),http_hbox,FALSE,FALSE,0);	
	
	GtkWidget *user_agent_label=gtk_label_new(_("User-Agent"));
	GtkWidget *user_agent_box=gtk_vbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(user_agent_box),5);
	user_agent_entry=my_gtk_combo_new(ALL_HISTORIES[USER_AGENT_HISTORY]);
	if (who->config->user_agent.get())
		text_to_combo(user_agent_entry,who->config->user_agent.get());
	gtk_box_pack_start(GTK_BOX(user_agent_box),user_agent_label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(user_agent_box),user_agent_entry,TRUE,TRUE,0);
	gtk_box_pack_start(GTK_BOX(http_vbox),user_agent_box,FALSE,FALSE,0);

	GtkWidget *label=gtk_label_new(_("Referer"));
	GtkWidget *vbox=gtk_vbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(user_agent_box),5);
	referer_entry=my_gtk_combo_new(ALL_HISTORIES[REFERER_HISTORY]);
	if (who->config->referer.get())
		text_to_combo(referer_entry,who->config->referer.get());
	else
		text_to_combo(referer_entry,"");
	gtk_box_pack_start(GTK_BOX(vbox),label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),referer_entry,TRUE,TRUE,0);
	gtk_box_pack_start(GTK_BOX(http_vbox),vbox,FALSE,FALSE,0);

	label=gtk_label_new(_("Cookie"));
	vbox=gtk_vbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(user_agent_box),5);
	cookie_entry=my_gtk_combo_new(ALL_HISTORIES[COOKIE_HISTORY]);
	if (who->config->cookie.get())
		text_to_combo(cookie_entry,who->config->cookie.get());
	else
		text_to_combo(cookie_entry,"");
	gtk_box_pack_start(GTK_BOX(vbox),label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),cookie_entry,TRUE,TRUE,0);
	gtk_box_pack_start(GTK_BOX(http_vbox),vbox,FALSE,FALSE,0);

	
	http_frame=gtk_frame_new("HTTP");
	gtk_container_border_width(GTK_CONTAINER(http_frame),5);
	gtk_container_add(GTK_CONTAINER(http_frame),http_vbox);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook),http_frame,gtk_label_new("HTTP"));
	
};


void tDEdit::init_time(tDownload *who){
	/* Init time
	 */
	GtkWidget *time_frame=gtk_frame_new(_("Time"));
	GtkWidget *time_hbox=gtk_hbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(time_hbox),5);
	GtkWidget *time_label,*time_vbox;
	gtk_container_border_width(GTK_CONTAINER(time_frame),5);
	calendar=gtk_calendar_new();
	gtk_calendar_display_options(GTK_CALENDAR(calendar),
				     GtkCalendarDisplayOptions(
				     GTK_CALENDAR_WEEK_START_MONDAY |
				     GTK_CALENDAR_SHOW_DAY_NAMES|
				     GTK_CALENDAR_SHOW_HEADING));
	gtk_box_pack_start(GTK_BOX(time_hbox),calendar,FALSE,FALSE,0);


	hour_entry=my_gtk_combo_new(0,23);
	time_label=gtk_label_new(_("Hours"));
	time_vbox=gtk_vbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(time_vbox),time_label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(time_vbox),hour_entry,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(time_vbox),gtk_vbox_new(FALSE,0),TRUE,TRUE,0);
	gtk_box_pack_start(GTK_BOX(time_hbox),time_vbox,FALSE,FALSE,0);

	minute_entry=my_gtk_combo_new(0,59);
	time_label=gtk_label_new(_("Minutes"));
	time_vbox=gtk_vbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(time_vbox),time_label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(time_vbox),minute_entry,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(time_vbox),gtk_vbox_new(FALSE,0),TRUE,TRUE,0);
	gtk_box_pack_start(GTK_BOX(time_hbox),time_vbox,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(time_hbox),gtk_hbox_new(FALSE,0),TRUE,TRUE,0);

	gtk_widget_set_usize(hour_entry,60,-1);
	gtk_widget_set_usize(minute_entry,60,-1);

	time_vbox=gtk_vbox_new(FALSE,0);
	time_check=gtk_check_button_new_with_label(_("Start this downloading at:"));
 	gtk_signal_connect(GTK_OBJECT(time_check),"clicked",GTK_SIGNAL_FUNC(edit_time_check_clicked),this);
	gtk_box_pack_start(GTK_BOX(time_vbox),time_check,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(time_vbox),time_hbox,FALSE,FALSE,0);
	time_hbox=gtk_hbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(time_vbox),time_hbox,TRUE,TRUE,0);
	gtk_container_add(GTK_CONTAINER(time_frame),time_vbox);
	setup_time(who->ScheduleTime);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook),time_frame,gtk_label_new(_("Time")));
};

void tDEdit::init(tDownload *who) {
	if (!who) return;
	parent=who;
	window=gtk_window_new(GTK_WINDOW_DIALOG);
	gtk_window_set_wmclass(GTK_WINDOW(window),
			       "D4X_Download","D4X");
	gtk_container_border_width(GTK_CONTAINER(window),5);
	gtk_window_set_position(GTK_WINDOW(window),GTK_WIN_POS_CENTER);
	gtk_window_set_policy (GTK_WINDOW(window), FALSE,FALSE,FALSE);
	//    gtk_widget_set_usize(window,470,255);
	gtk_object_set_user_data(GTK_OBJECT(window),this);
	notebook=gtk_notebook_new();

	init_main(who);
	init_other(who);
	init_ftp(who);
	init_http(who);

	/* init proxies
	 */
	proxy=new tProxyWidget;
	proxy->init();
	proxy->init_state();
	switch(who->info->proto){
	case D_PROTO_FTP:{
		proxy->init_state(who->config,1);
		break;
	};
	case D_PROTO_HTTP:{
		proxy->init_state(who->config,0);
		break;
	};
	};
	/* initing window
	 */
	GtkWidget *vbox2=gtk_vbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(vbox2),5);
	GtkWidget *proxy_frame=gtk_frame_new(_("Proxy"));
	gtk_container_border_width(GTK_CONTAINER(proxy_frame),5);
	gtk_container_add(GTK_CONTAINER(proxy_frame),proxy->frame);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook),proxy_frame,gtk_label_new(_("Proxy")));
	init_time(who);

	gtk_box_pack_start(GTK_BOX(vbox2),notebook,FALSE,FALSE,0);

	/* initing buttons
	 */
	GtkWidget *buttons_hbox=gtk_hbutton_box_new();
	gtk_button_box_set_layout(GTK_BUTTON_BOX(buttons_hbox),GTK_BUTTONBOX_END);
	gtk_button_box_set_spacing(GTK_BUTTON_BOX(buttons_hbox),5);
	isdefault_check=gtk_check_button_new_with_label(_("Use default settings"));
 	gtk_signal_connect(GTK_OBJECT(isdefault_check),"clicked",
			   GTK_SIGNAL_FUNC(edit_isdefault_check_clicked),this);
	GTK_TOGGLE_BUTTON(isdefault_check)->active=who->config->isdefault;
	toggle_isdefault();
	ok_button=gtk_button_new_with_label(_("Ok"));
	cancel_button=gtk_button_new_with_label(_("Cancel"));
	GTK_WIDGET_SET_FLAGS(ok_button,GTK_CAN_DEFAULT);
	GTK_WIDGET_SET_FLAGS(cancel_button,GTK_CAN_DEFAULT);
	GtkWidget *hbox_temp=gtk_hbox_new(FALSE,0);
 	gtk_box_pack_start(GTK_BOX(hbox_temp),isdefault_check,TRUE,TRUE,0);
	gtk_box_pack_start(GTK_BOX(buttons_hbox),ok_button,TRUE,TRUE,0);
	gtk_box_pack_start(GTK_BOX(buttons_hbox),cancel_button,TRUE,TRUE,0);
 	gtk_box_pack_start(GTK_BOX(hbox_temp),buttons_hbox,FALSE,FALSE,0);
	gtk_box_pack_end(GTK_BOX(vbox2),hbox_temp,FALSE,FALSE,0);
	gtk_container_add(GTK_CONTAINER(window),vbox2);
	gtk_window_set_default(GTK_WINDOW(window),ok_button);
	gtk_widget_show_all(window);
	gtk_widget_grab_focus(GTK_COMBO(url_entry)->entry);
	setup_entries();
};

void tDEdit::init_filter_sel(){
	if (filter_sel){
		gdk_window_show(GTK_WIDGET(filter_sel)->window);
		return;
	};
	filter_sel=(d4xFilterSel*)d4x_filter_sel_new();
	gtk_signal_connect(GTK_OBJECT(filter_sel->clist),
			   "select_row",
			   GTK_SIGNAL_FUNC(edit_filter_sel_select),
			   this);
	gtk_signal_connect(GTK_OBJECT(filter_sel->ok),
			   "clicked",
			   GTK_SIGNAL_FUNC(edit_filter_sel_ok),
			   this);
	gtk_signal_connect(GTK_OBJECT(filter_sel->cancel),
			   "clicked",
			   GTK_SIGNAL_FUNC(edit_filter_sel_cancel),
			   this);
	gtk_signal_connect(GTK_OBJECT(filter_sel),
			   "delete_event",
			   GTK_SIGNAL_FUNC(edit_filter_sel_delete),
			   this);
	d4x_eschandler_init(GTK_WIDGET(filter_sel),this);
};

void tDEdit::filter_ok(){
	GList *select=(GTK_CLIST(filter_sel->clist))->selection_end;
	if (select) {
		char *name;
		gint row=GPOINTER_TO_INT(select->data);
		gtk_clist_get_text(GTK_CLIST(filter_sel->clist),
				   row,0,&name);
		text_to_combo(filter,name);
	}else{
		text_to_combo(filter,"");
	};
	filter_cancel();
};

void tDEdit::filter_cancel(){
	gtk_widget_destroy(GTK_WIDGET(filter_sel));
	filter_sel=NULL;
};

void tDEdit::auto_fill_log(){
	tAddr *tmp=new tAddr(text_from_combo(url_entry));
	char *filename=tmp->file.get();
	char *savepath=text_from_combo(MY_GTK_FILESEL(path_entry)->combo);
	if (filename && *filename && savepath && *savepath){
		filename=sum_strings(savepath,"/",filename,"_log",NULL);
		normalize_path(filename);
		text_to_combo(MY_GTK_FILESEL(log_save_entry)->combo,filename);
		delete[] filename;
	};
	delete(tmp);
};

int tDEdit::get_pause_check(){
	return(GTK_TOGGLE_BUTTON(pause_check)->active);
};

int tDEdit::get_to_top_check(){
	if (to_top_check)
		return(GTK_TOGGLE_BUTTON(to_top_check)->active);
	return(0);
};

void tDEdit::disable_ok_button() {
	if (window) gtk_widget_set_sensitive(ok_button,FALSE);
};

void tDEdit::enable_ok_button() {
	if (window) gtk_widget_set_sensitive(ok_button,TRUE);
};

int tDEdit::apply_changes() {
	parent->config->isdefault=GTK_TOGGLE_BUTTON(isdefault_check)->active;
	char *temp=copy_string(text_from_combo(url_entry));
	del_crlf(temp);
	tAddr *addr=new tAddr(temp);
	delete[] temp;
	if (!addr) return 1;
	if (parent_in_db)
		ALL_DOWNLOADS->del(parent);
	delete (parent->info);
	parent->info=addr;
	if (parent_in_db)
		ALL_DOWNLOADS->insert(parent);
	if (parent->ALTS)
		parent->ALTS->check(addr->file.get());
	switch(parent->info->proto){
	case D_PROTO_FTP:{
		proxy->apply_changes(parent->config,1);
		break;
	};
	case D_PROTO_HTTP:{
		proxy->apply_changes(parent->config,0);
		break;
	};
	};
	char *pass_string=text_from_combo(pass_entry);
	if (GTK_TOGGLE_BUTTON(use_pass_check)->active) {
		if (strlen(text_from_combo(user_entry))) {
			parent->info->username.set(text_from_combo(user_entry));
			ALL_HISTORIES[USER_HISTORY]->add(parent->info->username.get());
			parent->info->pass.set(pass_string);
			if (CFG.REMEMBER_PASS)
				ALL_HISTORIES[PASS_HISTORY]->add(pass_string);
		};
	};
	char *sp=normalize_path_full(text_from_combo(MY_GTK_FILESEL(path_entry)->combo));
	parent->config->save_path.set(sp);
	parent->Name2Save.set(NULL);
	if (strlen(text_from_combo(MY_GTK_FILESEL(file_entry)->combo)))
		parent->Name2Save.set(text_from_combo(MY_GTK_FILESEL(file_entry)->combo));
	if (parent->Name2Save.get())
		ALL_HISTORIES[FILE_HISTORY]->add(parent->Name2Save.get());
	normalize_path(parent->config->save_path.get());
	ALL_HISTORIES[PATH_HISTORY]->add(sp);
	delete[] sp;
	parent->status=0;
	if (strlen(parent->info->file.get())==0) {
		parent->finfo.type=T_DIR;
		parent->finfo.size=0;
	} else {
		parent->finfo.type=0;
		parent->finfo.size=-1;
	};
	/* change histories
	 */
	char *URL=parent->info->url();
	parent->config->user_agent.set(text_from_combo(user_agent_entry));
	ALL_HISTORIES[URL_HISTORY]->add(URL);
	ALL_HISTORIES[USER_AGENT_HISTORY]->add(parent->config->user_agent.get());
	char *referer=text_from_combo(referer_entry);
	if (referer && *referer){
		parent->config->referer.set(referer);
		ALL_HISTORIES[REFERER_HISTORY]->add(referer);
	}else
		parent->config->referer.set(NULL);
	referer=text_from_combo(cookie_entry);
	if (referer && *referer){
		parent->config->cookie.set(referer);
		ALL_HISTORIES[COOKIE_HISTORY]->add(referer);
	}else
		parent->config->cookie.set(NULL);
	char *save_log=text_from_combo(MY_GTK_FILESEL(log_save_entry)->combo);
	if (save_log && *save_log){
		char *t=normalize_path_full(save_log);
		ALL_HISTORIES[LOG_SAVE_HISTORY]->add(t);
		parent->config->log_save_path.set(t);
		delete[] t;
	}else
		parent->config->log_save_path.set(NULL);
	char *desc=text_from_combo(desc_entry);
	if (desc && *desc){
		ALL_HISTORIES[DESC_HISTORY]->add(desc);
		parent->Description.set(desc);
	}else
		parent->Description.set(NULL);
	desc=text_from_combo(filter);
	if (desc && *desc){
		parent->config->Filter.set(desc);
	}else
		parent->config->Filter.set(NULL);
	
	/*change data in list if available
	 */
	gint row=D4X_QUEUE->qv.get_row(parent);
	if (row > 0) {
		D4X_QUEUE->qv.change_data(row,URL_COL,URL);
		D4X_QUEUE->qv.change_data(row,FILE_COL,parent->info->file.get());
		for (int i=FILE_TYPE_COL;i<URL_COL;i++)
			D4X_QUEUE->qv.change_data(row,i,"");
	};
	delete[] URL;
	int  temp1=0;
	sscanf(gtk_entry_get_text(GTK_ENTRY(timeout_entry)),"%u",&temp1);
	if (temp1>0 && temp1<1000) parent->config->timeout=temp1;
	sscanf(gtk_entry_get_text(GTK_ENTRY(sleep_entry)),"%u",&temp1);
	if (temp1>=0 && temp1<1000) parent->config->time_for_sleep=temp1;
	sscanf(gtk_entry_get_text(GTK_ENTRY(attempts_entry)),"%u",&temp1);
	if (temp1>=0) parent->config->number_of_attempts=temp1;
	temp1=1;
	sscanf(gtk_entry_get_text(GTK_ENTRY(ftp_recurse_depth_entry)),"%u",&temp1);
	if (temp1>=0) parent->config->ftp_recurse_depth=temp1;
	temp1=1;
	sscanf(gtk_entry_get_text(GTK_ENTRY(http_recurse_depth_entry)),"%u",&temp1);
	if (temp1>=0) parent->config->http_recurse_depth=temp1;
	sscanf(gtk_entry_get_text(GTK_ENTRY(rollback_entry)),"%u",&temp1);
	if (temp1>=0) parent->config->rollback=temp1;
	sscanf(gtk_entry_get_text(GTK_ENTRY(speed_entry)),"%u",&temp1);
	if (temp1>=0) parent->config->speed=temp1;
	parent->config->passive=GTK_TOGGLE_BUTTON(ftp_passive_check)->active;
	parent->config->dont_send_quit=GTK_TOGGLE_BUTTON(dont_send_quit_check)->active;
	parent->config->permisions=GTK_TOGGLE_BUTTON(permisions_check)->active;
	parent->config->get_date=GTK_TOGGLE_BUTTON(get_date_check)->active;
	parent->config->retry=GTK_TOGGLE_BUTTON(retry_check)->active;
	parent->config->follow_link=GTK_TOGGLE_BUTTON(follow_link_check)->active;
	parent->config->leave_server=GTK_TOGGLE_BUTTON(leave_server_check)->active;
	parent->config->ihate_etag=GTK_TOGGLE_BUTTON(ihate_etag_check)->active;
	parent->config->dont_leave_dir=GTK_TOGGLE_BUTTON(leave_dir_check)->active;
	parent->config->restart_from_begin=GTK_TOGGLE_BUTTON(restart_from_begin_check)->active;
	parent->config->sleep_before_complete=GTK_TOGGLE_BUTTON(sleep_check)->active;
	parent->config->change_links=GTK_TOGGLE_BUTTON(change_links_check)->active;
	parent->config->http_recursing=parent->config->http_recurse_depth==1?0:1;
	parent->config->ftp_dirontop=GTK_TOGGLE_BUTTON(ftp_dirontop_check)->active;
	parent->config->check_time=GTK_TOGGLE_BUTTON(check_time_check)->active;

	temp1=0;
	sscanf(gtk_entry_get_text(GTK_ENTRY(split_entry)),"%u",&temp1);
	if (temp1>1){
		if (temp1>10) temp1=10;
		if (parent->split==NULL)
			parent->split=new tSplitInfo;
		parent->split->NumOfParts=temp1;
	}else{
		if (parent->split)
			delete(parent->split);
		parent->split=NULL;
	};

	if (GTK_TOGGLE_BUTTON(time_check)->active) {
		time_t NOW=time(NULL);
		struct tm date;
		date.tm_isdst=-1;
		localtime_r(&NOW,&date);
		sscanf(text_from_combo(hour_entry),"%i",&date.tm_hour);
		sscanf(text_from_combo(minute_entry),"%i",&date.tm_min);
		gtk_calendar_get_date(GTK_CALENDAR(calendar),
				      (guint *)&date.tm_year,
				      (guint *)&date.tm_mon,
				      (guint *)&date.tm_mday);
		date.tm_year-=1900;
		date.tm_sec=0;
		parent->ScheduleTime=mktime(&date);
		if (parent_in_db && time(NULL)<parent->ScheduleTime){
			aa.schedule_download(parent);
			parent_in_db=0;
		};
	} else {
		parent->ScheduleTime=0;
	};
	return 0;
};

void tDEdit::toggle_isdefault() {
	int a=!GTK_TOGGLE_BUTTON(isdefault_check)->active;
	gtk_widget_set_sensitive(restart_from_begin_check,a);
	gtk_widget_set_sensitive(proxy->frame,a);
	gtk_widget_set_sensitive(path_entry,a);
	gtk_widget_set_sensitive(timeout_entry,a);
	gtk_widget_set_sensitive(sleep_entry,a);
	gtk_widget_set_sensitive(attempts_entry,a);
	gtk_widget_set_sensitive(rollback_entry,a);
	gtk_widget_set_sensitive(speed_entry,a);
	gtk_widget_set_sensitive(get_date_check,a);
	gtk_widget_set_sensitive(retry_check,a);
	gtk_widget_set_sensitive(sleep_check,a);
	gtk_widget_set_sensitive(check_time_check,a);
	gtk_widget_set_sensitive(log_save_entry,a);

	gtk_widget_set_sensitive(ftp_frame,a);
	gtk_widget_set_sensitive(http_frame,a);
};

void tDEdit::toggle_time() {
	gtk_widget_set_sensitive(calendar,GTK_TOGGLE_BUTTON(time_check)->active);
	gtk_widget_set_sensitive(hour_entry,GTK_TOGGLE_BUTTON(time_check)->active);
	gtk_widget_set_sensitive(minute_entry,GTK_TOGGLE_BUTTON(time_check)->active);
};


void tDEdit::set_description(char *desc){
	if (desc)
		text_to_combo(desc_entry,desc);
	else
		text_to_combo(desc_entry,"");		
};

void tDEdit::setup_entries() {
	set_editable_for_combo(pass_entry,GTK_TOGGLE_BUTTON(use_pass_check)->active);
	gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(user_entry)->entry),GTK_TOGGLE_BUTTON(use_pass_check)->active);
	gtk_widget_set_sensitive(user_entry,GTK_TOGGLE_BUTTON(use_pass_check)->active);
	gtk_widget_set_sensitive(pass_entry,GTK_TOGGLE_BUTTON(use_pass_check)->active);
};

void tDEdit::setup_time(time_t when) {
	char data[MAX_LEN];
	struct tm date;
	if (when){
		localtime_r(&when,&date);
	}else{
		time_t NOW=time(NULL);
		localtime_r(&NOW,&date);
		date.tm_hour=1;
		date.tm_min=0;
		time_t tonight=mktime(&date);
		if (tonight<NOW)
			tonight+=24*60*60;
		localtime_r(&tonight,&date);
	};
	sprintf(data,"%i",date.tm_hour);
	text_to_combo(hour_entry,data);
	sprintf(data,"%i",date.tm_min);
	text_to_combo(minute_entry,data);
	gtk_calendar_select_month(GTK_CALENDAR(calendar),date.tm_mon,date.tm_year+1900);
	gtk_calendar_select_day(GTK_CALENDAR(calendar),date.tm_mday);
	GTK_TOGGLE_BUTTON(time_check)->active=when?TRUE:FALSE;
	toggle_time();
};


void  tDEdit::paste_url() {
	char *a=d4x_mw_clipboard_get();
	if (a && global_url(a)){
		set_url(a);
		return;
	};
	if (old_clipboard_content()!=NULL && global_url(old_clipboard_content())){
		set_url(old_clipboard_content());
		return;
	};
	gtk_editable_paste_clipboard(GTK_EDITABLE(GTK_COMBO(url_entry)->entry));
//	printf("%s\n",text_from_combo(url_entry));
};

void tDEdit::select_url() {
	gtk_entry_select_region(GTK_ENTRY(GTK_COMBO(url_entry)->entry),0,strlen(text_from_combo(url_entry)));
};

void tDEdit::clear_url() {
	text_to_combo(url_entry,"");
};

void tDEdit::clear_save_name() {
	text_to_combo(MY_GTK_FILESEL(file_entry)->combo,"");
};

char *tDEdit::get_url(){
	return(text_from_combo(url_entry));
};

void tDEdit::set_url(char *a) {
	text_to_combo(url_entry,a);
};

void tDEdit::disable_time(){
	gtk_widget_set_sensitive(time_check,FALSE);
};

void tDEdit::disable_save_name(){
	gtk_widget_set_sensitive(file_entry,FALSE);
};

void tDEdit::disable_items(int *array){
	gtk_widget_set_sensitive(url_entry,FALSE);
	gtk_widget_set_sensitive(file_entry,FALSE);
	gtk_widget_set_sensitive(pause_check,FALSE);
	gtk_widget_set_sensitive(log_save_entry,FALSE);
	if (array[EDIT_OPT_USERPASS]==0){
		gtk_widget_set_sensitive(pass_entry,FALSE);
		gtk_widget_set_sensitive(user_entry,FALSE);
		gtk_widget_set_sensitive(use_pass_check,FALSE);
	};
	if (array[EDIT_OPT_SAVEPATH]==0)
		gtk_widget_set_sensitive(path_entry,FALSE);
	if (array[EDIT_OPT_USERAGENT]==0)
		gtk_widget_set_sensitive(user_agent_entry,FALSE);
	if (array[EDIT_OPT_REFERER]==0)
		gtk_widget_set_sensitive(referer_entry,FALSE);
	if (array[EDIT_OPT_COOKIE]==0)
		gtk_widget_set_sensitive(cookie_entry,FALSE);
	if (array[EDIT_OPT_TIMEOUT]==0)
		gtk_widget_set_sensitive(timeout_entry,FALSE);
	if (array[EDIT_OPT_ATTEMPTS]==0)
		gtk_widget_set_sensitive(attempts_entry,FALSE);
	if (array[EDIT_OPT_SLEEPTIME]==0)
		gtk_widget_set_sensitive(sleep_entry,FALSE);
	if (array[EDIT_OPT_ROLLBACK]==0)
		gtk_widget_set_sensitive(rollback_entry,FALSE);
	if (array[EDIT_OPT_PASSIVEFTP]==0)
		gtk_widget_set_sensitive(ftp_passive_check,FALSE);
	if (array[EDIT_OPT_DIRONTOP]==0)
		gtk_widget_set_sensitive(ftp_dirontop_check,FALSE);
	if (array[EDIT_OPT_PERMISSIONS]==0)
		gtk_widget_set_sensitive(permisions_check,FALSE);
	if (array[EDIT_OPT_DATE]==0)
		gtk_widget_set_sensitive(get_date_check,FALSE);
	if (array[EDIT_OPT_IFNOREGET]==0)
		gtk_widget_set_sensitive(retry_check,FALSE);
	if (array[EDIT_OPT_FOLLOWLINK]==0)
		gtk_widget_set_sensitive(follow_link_check,FALSE);
	if (array[EDIT_OPT_LEAVESERVER]==0)
		gtk_widget_set_sensitive(leave_server_check,FALSE);
	if (array[EDIT_OPT_LEAVEDIR]==0)
		gtk_widget_set_sensitive(leave_dir_check,FALSE);
	if (array[EDIT_OPT_CHANGE_LINKS]==0)
		gtk_widget_set_sensitive(change_links_check,FALSE);
	if (array[EDIT_OPT_IHATEETAG]==0)
		gtk_widget_set_sensitive(ihate_etag_check,FALSE);
	if (array[EDIT_OPT_RECURSEDEPTHFTP]==0)
		gtk_widget_set_sensitive(ftp_recurse_depth_entry,FALSE);
	if (array[EDIT_OPT_RECURSEDEPTHHTTP]==0)
		gtk_widget_set_sensitive(http_recurse_depth_entry,FALSE);
	if (array[EDIT_OPT_FROMBEGIN]==0)
		gtk_widget_set_sensitive(restart_from_begin_check,FALSE);
	if (array[EDIT_OPT_SPEED]==0)
		gtk_widget_set_sensitive(speed_entry,FALSE);
	if (array[EDIT_OPT_CHECK_TIME]==0)
		gtk_widget_set_sensitive(check_time_check,FALSE);
	if (array[EDIT_OPT_SLEEP_BEFORE_COMPLETE]==0)
		gtk_widget_set_sensitive(sleep_check,FALSE);
	if (array[EDIT_OPT_TIME]==0){
		gtk_widget_set_sensitive(time_check,FALSE);
		gtk_widget_set_sensitive(calendar,FALSE);
		gtk_widget_set_sensitive(hour_entry,FALSE);
		gtk_widget_set_sensitive(minute_entry,FALSE);
	};
	if (array[EDIT_OPT_SPLIT]==0)
		gtk_widget_set_sensitive(split_entry,FALSE);
	if (array[EDIT_OPT_PROXY]==0)
		gtk_widget_set_sensitive(proxy->frame,FALSE);
	if (array[EDIT_OPT_DONT_SEND_QUIT]==0)
		gtk_widget_set_sensitive(dont_send_quit_check,FALSE);
};

void tDEdit::apply_enabled_changes(){
	if (GTK_WIDGET_SENSITIVE(proxy->frame)){
		switch(parent->info->proto){
		case D_PROTO_FTP:{
			proxy->apply_changes(parent->config,1);
			break;
		};
		case D_PROTO_HTTP:{
			proxy->apply_changes(parent->config,0);
			break;
		};
		};
	};
	if (GTK_WIDGET_SENSITIVE(use_pass_check)){
		char *pass_string=text_from_combo(pass_entry);
		if (GTK_TOGGLE_BUTTON(use_pass_check)->active) {
			if (strlen(text_from_combo(user_entry)) && strlen(pass_string)) {
				parent->info->username.set(text_from_combo(user_entry));
				ALL_HISTORIES[USER_HISTORY]->add(parent->info->username.get());
				parent->info->pass.set(pass_string);
				if (CFG.REMEMBER_PASS)
					ALL_HISTORIES[PASS_HISTORY]->add(pass_string);
			};
		};
	};
	if (GTK_WIDGET_SENSITIVE(path_entry)){
		parent->config->save_path.set(text_from_combo(MY_GTK_FILESEL(path_entry)->combo));
		normalize_path(parent->config->save_path.get());
		ALL_HISTORIES[PATH_HISTORY]->add(text_from_combo(MY_GTK_FILESEL(path_entry)->combo));
	};
	if (GTK_WIDGET_SENSITIVE(user_agent_entry)){
		parent->config->user_agent.set(text_from_combo(user_agent_entry));
		ALL_HISTORIES[USER_AGENT_HISTORY]->add(text_from_combo(user_agent_entry));
	};
	if (GTK_WIDGET_SENSITIVE(referer_entry)){
		char *referer=text_from_combo(referer_entry);
		if (referer && *referer){
			parent->config->referer.set(referer);
			ALL_HISTORIES[REFERER_HISTORY]->add(referer);
		}else
			parent->config->referer.set(NULL);
	};
	if (GTK_WIDGET_SENSITIVE(cookie_entry)){
		char *referer=text_from_combo(cookie_entry);
		if (referer && *referer){
			parent->config->cookie.set(referer);
			ALL_HISTORIES[COOKIE_HISTORY]->add(referer);
		}else
			parent->config->cookie.set(NULL);
	};
	/*change data in list if available
	 */
	int  temp1=0;
	if (GTK_WIDGET_SENSITIVE(timeout_entry)){
		sscanf(gtk_entry_get_text(GTK_ENTRY(timeout_entry)),"%u",&temp1);
		if (temp1>0 && temp1<1000) parent->config->timeout=temp1;
	};
	if (GTK_WIDGET_SENSITIVE(sleep_entry)){
		sscanf(gtk_entry_get_text(GTK_ENTRY(sleep_entry)),"%u",&temp1);
		if (temp1>=0 && temp1<1000) parent->config->time_for_sleep=temp1;
	};
	if (GTK_WIDGET_SENSITIVE(attempts_entry)){
		sscanf(gtk_entry_get_text(GTK_ENTRY(attempts_entry)),"%u",&temp1);
		if (temp1>=0) parent->config->number_of_attempts=temp1;
	};
	if (GTK_WIDGET_SENSITIVE(ftp_recurse_depth_entry)){
		temp1=1;
		sscanf(gtk_entry_get_text(GTK_ENTRY(ftp_recurse_depth_entry)),"%u",&temp1);
		if (temp1>=0) parent->config->ftp_recurse_depth=temp1;
	};
	if (GTK_WIDGET_SENSITIVE(http_recurse_depth_entry)){
		temp1=1;
		sscanf(gtk_entry_get_text(GTK_ENTRY(http_recurse_depth_entry)),"%u",&temp1);
		if (temp1>=0) parent->config->http_recurse_depth=temp1;
	};
	if (GTK_WIDGET_SENSITIVE(rollback_entry)){
		sscanf(gtk_entry_get_text(GTK_ENTRY(rollback_entry)),"%u",&temp1);
		if (temp1>=0) parent->config->rollback=temp1;
	};
	if (GTK_WIDGET_SENSITIVE(speed_entry)){
		sscanf(gtk_entry_get_text(GTK_ENTRY(speed_entry)),"%u",&temp1);
		if (temp1>=0) parent->config->speed=temp1;
	};
	if (GTK_WIDGET_SENSITIVE(ftp_passive_check))
		parent->config->passive=GTK_TOGGLE_BUTTON(ftp_passive_check)->active;
	if (GTK_WIDGET_SENSITIVE(dont_send_quit_check))
		parent->config->dont_send_quit=GTK_TOGGLE_BUTTON(dont_send_quit_check)->active;
	if (GTK_WIDGET_SENSITIVE(permisions_check))
		parent->config->permisions=GTK_TOGGLE_BUTTON(permisions_check)->active;
	if (GTK_WIDGET_SENSITIVE(get_date_check))
		parent->config->get_date=GTK_TOGGLE_BUTTON(get_date_check)->active;
	if (GTK_WIDGET_SENSITIVE(retry_check))
		parent->config->retry=GTK_TOGGLE_BUTTON(retry_check)->active;
	if (GTK_WIDGET_SENSITIVE(follow_link_check))
		parent->config->follow_link=GTK_TOGGLE_BUTTON(follow_link_check)->active;
	if (GTK_WIDGET_SENSITIVE(leave_server_check))
		parent->config->leave_server=GTK_TOGGLE_BUTTON(leave_server_check)->active;
	if (GTK_WIDGET_SENSITIVE(leave_dir_check))
		parent->config->dont_leave_dir=GTK_TOGGLE_BUTTON(leave_dir_check)->active;
	if (GTK_WIDGET_SENSITIVE(restart_from_begin_check))
		parent->config->restart_from_begin=GTK_TOGGLE_BUTTON(restart_from_begin_check)->active;
	if (GTK_WIDGET_SENSITIVE(sleep_check))
		parent->config->sleep_before_complete=GTK_TOGGLE_BUTTON(sleep_check)->active;
	if (GTK_WIDGET_SENSITIVE(check_time_check))
		parent->config->check_time=GTK_TOGGLE_BUTTON(check_time_check)->active;
	if (GTK_WIDGET_SENSITIVE(change_links_check))
		parent->config->change_links=GTK_TOGGLE_BUTTON(change_links_check)->active;
	if (GTK_WIDGET_SENSITIVE(ihate_etag_check))
		parent->config->ihate_etag=GTK_TOGGLE_BUTTON(ihate_etag_check)->active;
	if (GTK_WIDGET_SENSITIVE(ftp_dirontop_check))
		parent->config->ftp_dirontop=GTK_TOGGLE_BUTTON(ftp_dirontop_check)->active;
	parent->config->http_recursing=parent->config->http_recurse_depth==1?0:1;

	if (GTK_WIDGET_SENSITIVE(split_entry)){
		temp1=0;
		sscanf(gtk_entry_get_text(GTK_ENTRY(split_entry)),"%u",&temp1);
		if (temp1>1){
			if (temp1>10) temp1=10;
			if (parent->split==NULL)
				parent->split=new tSplitInfo;
			parent->split->NumOfParts=temp1;
		}else{
			if (parent->split)
				delete(parent->split);
			parent->split=NULL;
		};
	};

	if (GTK_WIDGET_SENSITIVE(time_check)){
		if (GTK_TOGGLE_BUTTON(time_check)->active) {
			time_t NOW=time(NULL);
			struct tm date;
			date.tm_isdst=-1;
			localtime_r(&NOW,&date);
			sscanf(text_from_combo(hour_entry),"%i",&date.tm_hour);
			sscanf(text_from_combo(minute_entry),"%i",&date.tm_min);
			gtk_calendar_get_date(GTK_CALENDAR(calendar),
					      (guint *)&date.tm_year,
					      (guint *)&date.tm_mon,
					      (guint *)&date.tm_mday);
			date.tm_year-=1900;
			date.tm_sec=0;
			parent->ScheduleTime=mktime(&date);
			if (time(NULL)<parent->ScheduleTime){
				aa.schedule_download(parent);
				parent_in_db=0;
			};
		} else {
			parent->ScheduleTime=0;
		};
	};
};

void tDEdit::done() {
	if (parent) parent->editor=NULL;
	gtk_widget_destroy(window);
	if (filter_sel)
		gtk_widget_destroy(GTK_WIDGET(filter_sel));
	filter_sel=NULL;
	delete proxy;
};

tDEdit::~tDEdit() {
	done();
};

/*******************************************************/

static void proxy_toggle_pass_ftp(GtkWidget *parent,tProxyWidget *where) {
	set_editable_for_combo(where->ftp_proxy_pass,GTK_TOGGLE_BUTTON(parent)->active);
	gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(where->ftp_proxy_user)->entry),GTK_TOGGLE_BUTTON(parent)->active);
	gtk_widget_set_sensitive(where->ftp_proxy_user,GTK_TOGGLE_BUTTON(parent)->active);
	gtk_widget_set_sensitive(where->ftp_proxy_pass,GTK_TOGGLE_BUTTON(parent)->active);
};

static void proxy_toggle_pass_http(GtkWidget *parent,tProxyWidget *where) {
	set_editable_for_combo(where->http_proxy_pass,GTK_TOGGLE_BUTTON(parent)->active);
	gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(where->http_proxy_user)->entry),GTK_TOGGLE_BUTTON(parent)->active);
	gtk_widget_set_sensitive(where->http_proxy_user,GTK_TOGGLE_BUTTON(parent)->active);
	gtk_widget_set_sensitive(where->http_proxy_pass,GTK_TOGGLE_BUTTON(parent)->active);
};

static void proxy_toggle_socks(GtkWidget *parent,tProxyWidget *where) {
	set_editable_for_combo(where->socks_port,GTK_TOGGLE_BUTTON(parent)->active);
	set_editable_for_combo(where->socks_user,GTK_TOGGLE_BUTTON(parent)->active);
	set_editable_for_combo(where->socks_pass,GTK_TOGGLE_BUTTON(parent)->active);
	gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(where->socks_host)->entry),GTK_TOGGLE_BUTTON(parent)->active);
	gtk_widget_set_sensitive(where->socks_user,GTK_TOGGLE_BUTTON(parent)->active);
	gtk_widget_set_sensitive(where->socks_pass,GTK_TOGGLE_BUTTON(parent)->active);
	gtk_widget_set_sensitive(where->socks_port,GTK_TOGGLE_BUTTON(parent)->active);
	gtk_widget_set_sensitive(where->socks_host,GTK_TOGGLE_BUTTON(parent)->active);
};

static void _proxy_port_changed_(GtkEntry *entry,GtkEntry *entryh){
	char *tmp=gtk_entry_get_text(entryh);
	char *tmp1=index(tmp,':');
	if (tmp1){
		*tmp1=0;
		char *ns=sum_strings(tmp,":",gtk_entry_get_text(entry),NULL);
		*tmp1=':';
		if (!equal(ns,tmp))
			gtk_entry_set_text(entryh,ns);
		delete[] ns;
	};
};

static void _ftp_host_changed_(GtkEntry *entry,tProxyWidget *parent){
	char *tmp=gtk_entry_get_text(entry);
	tmp=index(tmp,':');
	if (tmp){
		int a=0;
		if (sscanf(tmp+1,"%i",&a)==1){
			char str[100];
			sprintf(str,"%i",a);
			gtk_entry_set_text(GTK_ENTRY(parent->ftp_proxy_port),str);
		};
	};
};

static void _http_host_changed_(GtkEntry *entry,tProxyWidget *parent){
	char *tmp=gtk_entry_get_text(entry);
	tmp=index(tmp,':');
	if (tmp){
		int a=0;
		if (sscanf(tmp+1,"%i",&a)==1){
			char str[100];
			sprintf(str,"%i",a);
			gtk_entry_set_text(GTK_ENTRY(parent->http_proxy_port),str);
		};
	};
};

static void _socks_host_changed_(GtkEntry *entry,tProxyWidget *parent){
	char *tmp=gtk_entry_get_text(entry);
	tmp=index(tmp,':');
	if (tmp){
		int a=0;
		if (sscanf(tmp+1,"%i",&a)==1){
			char str[100];
			sprintf(str,"%i",a);
			gtk_entry_set_text(GTK_ENTRY(parent->socks_port),str);
		};
	};
};

void tProxyWidget::init() {
//	frame=gtk_frame_new(_("Proxy"));
	GtkWidget *proxy_frame1=gtk_frame_new("FTP");
	GtkWidget *proxy_frame2=gtk_frame_new("HTTP");
	GtkWidget *proxy_frame3=gtk_frame_new(_("FTP proxy type"));
//	gtk_container_border_width(GTK_CONTAINER(frame),5);
	gtk_container_border_width(GTK_CONTAINER(proxy_frame1),5);
	gtk_container_border_width(GTK_CONTAINER(proxy_frame2),5);

	GtkWidget *vbox=gtk_vbox_new(FALSE,0);
	GtkWidget *hbox=gtk_hbox_new(FALSE,0);

	gtk_box_pack_start(GTK_BOX(hbox),vbox,FALSE,0,0);
	GtkWidget *table=gtk_table_new(2,1,FALSE);
	gtk_container_add(GTK_CONTAINER(proxy_frame3),table);
	ftp_proxy_type_ftp=gtk_radio_button_new_with_label(NULL,_("FTP (wingate)"));
	GSList *proxy_group1=gtk_radio_button_group(GTK_RADIO_BUTTON(ftp_proxy_type_ftp));
	gtk_table_attach_defaults(GTK_TABLE(table),ftp_proxy_type_ftp,0,1,0,1);
	ftp_proxy_type_http=gtk_radio_button_new_with_label(proxy_group1,"HTTP");
	gtk_table_attach_defaults(GTK_TABLE(table),ftp_proxy_type_http,0,1,1,2);
	GtkWidget *vbox1=gtk_vbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox1),proxy_frame3,FALSE,0,0);
	gtk_box_pack_start(GTK_BOX(hbox),vbox1,FALSE,0,0);
	GtkWidget *box1=gtk_vbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox1),box1,FALSE,0,0);

	gtk_box_set_spacing(GTK_BOX(vbox),2);
	gtk_container_add(GTK_CONTAINER(proxy_frame1),hbox);

	ftp_proxy_check=gtk_check_button_new_with_label(_("Use this proxy for FTP"));

	gtk_box_pack_start(GTK_BOX(vbox),ftp_proxy_check,FALSE,0,0);
	ftp_proxy_host=my_gtk_combo_new(ALL_HISTORIES[PROXY_HISTORY]);
	gtk_signal_connect (GTK_OBJECT (GTK_COMBO(ftp_proxy_host)->entry),
			    "changed",
			    (GtkSignalFunc) _ftp_host_changed_, this);
	gtk_widget_set_usize(ftp_proxy_host,120,-1);

	gtk_box_pack_start(GTK_BOX(vbox),ftp_proxy_host,FALSE,0,0);
	ftp_proxy_port=my_gtk_entry_new_with_max_length(5,0);
	gtk_signal_connect (GTK_OBJECT (ftp_proxy_port),
			    "changed",
			    (GtkSignalFunc) _proxy_port_changed_,
			    GTK_COMBO(ftp_proxy_host)->entry);
	GtkWidget *label=gtk_label_new(_("port"));
	hbox=gtk_hbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(hbox),3);
	gtk_box_pack_start(GTK_BOX(hbox),ftp_proxy_port,FALSE,0,0);
	gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,0,0);

	gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,0,0);
	ftp_proxy_user_check=gtk_check_button_new_with_label(_("need password"));

	gtk_signal_connect(GTK_OBJECT(ftp_proxy_user_check),"clicked",GTK_SIGNAL_FUNC(proxy_toggle_pass_ftp),this);
	gtk_box_pack_start(GTK_BOX(vbox),ftp_proxy_user_check,FALSE,0,0);
	//    ftp_proxy_user=gtk_entry_new();
	ftp_proxy_user=my_gtk_combo_new(ALL_HISTORIES[USER_HISTORY]);
	gtk_widget_set_usize(ftp_proxy_user,100,-1);

	label=gtk_label_new(_("username"));
	hbox=gtk_hbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(hbox),3);
	gtk_box_set_spacing(GTK_BOX(vbox),2);
	gtk_box_pack_start(GTK_BOX(hbox),ftp_proxy_user,FALSE,0,0);
	gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,0,0);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,0,0);
	if (CFG.REMEMBER_PASS)
		ftp_proxy_pass=my_gtk_combo_new(ALL_HISTORIES[PASS_HISTORY]);
	else{
		ftp_proxy_pass=gtk_entry_new();
		gtk_entry_set_visibility(GTK_ENTRY(ftp_proxy_pass),FALSE);
	};
	gtk_widget_set_usize(ftp_proxy_pass,100,-1);

	label=gtk_label_new(_("password"));
	hbox=gtk_hbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(hbox),3);
	gtk_box_set_spacing(GTK_BOX(vbox),2);
	gtk_box_pack_start(GTK_BOX(hbox),ftp_proxy_pass,FALSE,0,0);
	gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,0,0);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,0,0);

	hbox=gtk_hbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(hbox),3);
	gtk_box_pack_start(GTK_BOX(hbox),proxy_frame1,FALSE,0,0);
	gtk_box_pack_end(GTK_BOX(hbox),proxy_frame2,FALSE,0,0);
	GtkWidget *vbox_temp=gtk_vbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox_temp),hbox,FALSE,0,0);
//	gtk_container_add(GTK_CONTAINER(frame),vbox_temp);
	frame=vbox_temp;

	no_cache=gtk_check_button_new_with_label(_("Don't get from cache"));
	gtk_box_pack_start(GTK_BOX(vbox_temp),no_cache,FALSE,0,0);

	vbox=gtk_vbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(vbox),2);
	gtk_container_add(GTK_CONTAINER(proxy_frame2),vbox);

	http_proxy_check=gtk_check_button_new_with_label(_("Use this proxy for HTTP"));

	gtk_box_pack_start(GTK_BOX(vbox),http_proxy_check,FALSE,0,0);
	http_proxy_host=my_gtk_combo_new(ALL_HISTORIES[PROXY_HISTORY]);
	gtk_signal_connect (GTK_OBJECT (GTK_COMBO(http_proxy_host)->entry),
			    "changed",
			    (GtkSignalFunc) _http_host_changed_, this);
	gtk_widget_set_usize(http_proxy_host,120,-1);

	gtk_box_pack_start(GTK_BOX(vbox),http_proxy_host,FALSE,0,0);
	http_proxy_port=my_gtk_entry_new_with_max_length(5,0);//gtk_entry_new();
	gtk_signal_connect (GTK_OBJECT (http_proxy_port),
			    "changed",
			    (GtkSignalFunc) _proxy_port_changed_,
			    GTK_COMBO(http_proxy_host)->entry);
	label=gtk_label_new(_("port"));
	hbox=gtk_hbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(hbox),3);
	gtk_box_pack_start(GTK_BOX(hbox),http_proxy_port,FALSE,0,0);
	gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,0,0);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,0,0);
	http_proxy_user_check=gtk_check_button_new_with_label(_("need password"));

	gtk_signal_connect(GTK_OBJECT(http_proxy_user_check),"clicked",GTK_SIGNAL_FUNC(proxy_toggle_pass_http),this);
	gtk_box_pack_start(GTK_BOX(vbox),http_proxy_user_check,FALSE,0,0);

	http_proxy_user=my_gtk_combo_new(ALL_HISTORIES[USER_HISTORY]);
	gtk_widget_set_usize(http_proxy_user,100,-1);

	label=gtk_label_new(_("username"));
	hbox=gtk_hbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(hbox),3);
	gtk_box_pack_start(GTK_BOX(hbox),http_proxy_user,FALSE,0,0);
	gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,0,0);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,0,0);
	if (CFG.REMEMBER_PASS)
		http_proxy_pass=my_gtk_combo_new(ALL_HISTORIES[PASS_HISTORY]);
	else{
		http_proxy_pass=gtk_entry_new();
		gtk_entry_set_visibility(GTK_ENTRY(http_proxy_pass),FALSE);
	};
	gtk_widget_set_usize(http_proxy_pass,100,-1);

	label=gtk_label_new(_("password"));
	hbox=gtk_hbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(hbox),3);
	gtk_box_pack_start(GTK_BOX(hbox),http_proxy_pass,FALSE,0,0);
	gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,0,0);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,0,0);

/* SOCKS settings */
	use_socks=gtk_check_button_new_with_label(_("Use SOCKS5 proxy"));
	gtk_signal_connect(GTK_OBJECT(use_socks),"clicked",
			   GTK_SIGNAL_FUNC(proxy_toggle_socks),this);
	gtk_box_pack_start(GTK_BOX(vbox_temp),use_socks,FALSE,0,0);

	hbox=gtk_hbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(hbox),5);
	label=gtk_label_new(_("host"));
	socks_host=my_gtk_combo_new(ALL_HISTORIES[PROXY_HISTORY]);
	gtk_signal_connect (GTK_OBJECT (GTK_COMBO(socks_host)->entry),
			    "changed",
			    (GtkSignalFunc) _socks_host_changed_, this);
	gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,0,0);
	gtk_box_pack_start(GTK_BOX(hbox),socks_host,FALSE,0,0);
	label=gtk_label_new(_("port"));
	socks_port=my_gtk_entry_new_with_max_length(5,0);
	gtk_signal_connect (GTK_OBJECT (socks_port),
			    "changed",
			    (GtkSignalFunc) _proxy_port_changed_,
			    GTK_COMBO(socks_host)->entry);
	gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,0,0);
	gtk_box_pack_start(GTK_BOX(hbox),socks_port,FALSE,0,0);
	gtk_box_pack_start(GTK_BOX(vbox_temp),hbox,FALSE,0,0);

	hbox=gtk_hbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(hbox),5);
	label=gtk_label_new(_("username"));
	socks_user=gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox),socks_user,FALSE,0,0);
	gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,0,0);
	gtk_box_pack_start(GTK_BOX(vbox_temp),hbox,FALSE,0,0);
	hbox=gtk_hbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(hbox),5);
	label=gtk_label_new(_("password"));
	socks_pass=gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox),socks_pass,FALSE,0,0);
	gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,0,0);
	gtk_box_pack_start(GTK_BOX(vbox_temp),hbox,FALSE,0,0);
	gtk_entry_set_visibility(GTK_ENTRY(socks_pass),FALSE);

};

void tProxyWidget::init_state(tMainCfg *cfg){
	GTK_TOGGLE_BUTTON(ftp_proxy_check)->active=cfg->USE_PROXY_FOR_FTP;
	GTK_TOGGLE_BUTTON(ftp_proxy_user_check)->active=cfg->NEED_PASS_FTP_PROXY;
	if (cfg->FTP_PROXY_USER)
		text_to_combo(ftp_proxy_user,cfg->FTP_PROXY_USER);
	if (cfg->FTP_PROXY_PORT) {
		char data[MAX_LEN];
		sprintf(data,"%i",cfg->FTP_PROXY_PORT);
		gtk_entry_set_text(GTK_ENTRY(ftp_proxy_port),data);
	};
	if (cfg->FTP_PROXY_HOST)
		text_to_combo(ftp_proxy_host,cfg->FTP_PROXY_HOST);
	if (cfg->FTP_PROXY_PASS)
		text_to_combo(ftp_proxy_pass,cfg->FTP_PROXY_PASS);
	else
		text_to_combo(ftp_proxy_pass,"");
	GTK_TOGGLE_BUTTON(http_proxy_user_check)->active=cfg->NEED_PASS_HTTP_PROXY;
	GTK_TOGGLE_BUTTON(http_proxy_check)->active=cfg->USE_PROXY_FOR_HTTP;
	if (cfg->HTTP_PROXY_HOST)
		text_to_combo(http_proxy_host,cfg->HTTP_PROXY_HOST);
	if (cfg->HTTP_PROXY_PORT) {
		char data[MAX_LEN];
		sprintf(data,"%i",cfg->HTTP_PROXY_PORT);
		gtk_entry_set_text(GTK_ENTRY(http_proxy_port),data);
	};
	if (cfg->HTTP_PROXY_USER)
		text_to_combo(http_proxy_user,cfg->HTTP_PROXY_USER);
	if (cfg->HTTP_PROXY_PASS)
		text_to_combo(http_proxy_pass,cfg->HTTP_PROXY_PASS);
	proxy_toggle_pass_ftp(ftp_proxy_user_check,this);
	proxy_toggle_pass_http(http_proxy_user_check,this);
	if (cfg->FTP_PROXY_TYPE) {
		GTK_TOGGLE_BUTTON(ftp_proxy_type_ftp)->active=FALSE;
		GTK_TOGGLE_BUTTON(ftp_proxy_type_http)->active=TRUE;
	} else {
		GTK_TOGGLE_BUTTON(ftp_proxy_type_ftp)->active=TRUE;
		GTK_TOGGLE_BUTTON(ftp_proxy_type_http)->active=FALSE;
	};
	GTK_TOGGLE_BUTTON(no_cache)->active=cfg->PROXY_NO_CACHE;

	if (cfg->SOCKS_HOST){
		GTK_TOGGLE_BUTTON(use_socks)->active=TRUE;
		text_to_combo(socks_host,cfg->SOCKS_HOST);
	}else{
		text_to_combo(socks_host,"");
		GTK_TOGGLE_BUTTON(use_socks)->active=FALSE;
	};
	proxy_toggle_socks(use_socks,this);
	if  (cfg->SOCKS_USER)
		text_to_combo(socks_user,cfg->SOCKS_USER);
	else
		text_to_combo(socks_user,"");
	if  (cfg->SOCKS_PASS)
		text_to_combo(socks_pass,cfg->SOCKS_PASS);
	else
		text_to_combo(socks_pass,"");
	if (cfg->SOCKS_PORT){
		char data[MAX_LEN];
		sprintf(data,"%i",cfg->SOCKS_PORT);
		text_to_combo(socks_port,data);
	};
};

void tProxyWidget::init_state() {
	init_state(&CFG);
};

void tProxyWidget::init_state(tCfg *cfg,int proto) {
	if (cfg->fproxy_host.get()) {
		text_to_combo(ftp_proxy_host,cfg->fproxy_host.get());
		GTK_TOGGLE_BUTTON(ftp_proxy_check)->active=TRUE;
		if (cfg->fproxy_port) {
			char data[MAX_LEN];
			sprintf(data,"%i",cfg->fproxy_port);
			gtk_entry_set_text(GTK_ENTRY(ftp_proxy_port),data);
		};
		if (cfg->fproxy_user.get() && cfg->fproxy_pass.get()) {
			GTK_TOGGLE_BUTTON(ftp_proxy_user_check)->active=TRUE;
			text_to_combo(ftp_proxy_user,cfg->fproxy_user.get());
			text_to_combo(ftp_proxy_pass,cfg->fproxy_pass.get());
		}else
			GTK_TOGGLE_BUTTON(ftp_proxy_user_check)->active=FALSE;
	} else
		GTK_TOGGLE_BUTTON(ftp_proxy_check)->active=FALSE;
	proxy_toggle_pass_ftp(ftp_proxy_user_check,this);
	if (cfg->hproxy_host.get()) {
		text_to_combo(http_proxy_host,cfg->hproxy_host.get());
		GTK_TOGGLE_BUTTON(http_proxy_check)->active=TRUE;
		if (cfg->hproxy_port) {
			char data[MAX_LEN];
			sprintf(data,"%i",cfg->hproxy_port);
			gtk_entry_set_text(GTK_ENTRY(http_proxy_port),data);
		};
		if (cfg->hproxy_user.get() && cfg->hproxy_pass.get()) {
			GTK_TOGGLE_BUTTON(http_proxy_user_check)->active=TRUE;
			text_to_combo(http_proxy_user,cfg->hproxy_user.get());
			text_to_combo(http_proxy_pass,cfg->hproxy_pass.get());
		}else
			GTK_TOGGLE_BUTTON(http_proxy_user_check)->active=FALSE;
	} else{
		GTK_TOGGLE_BUTTON(http_proxy_check)->active=FALSE;
	};
	proxy_toggle_pass_http(http_proxy_user_check,this);
	if (cfg->proxy_type) {
		GTK_TOGGLE_BUTTON(ftp_proxy_type_ftp)->active=FALSE;
		GTK_TOGGLE_BUTTON(ftp_proxy_type_http)->active=TRUE;
	} else {
		GTK_TOGGLE_BUTTON(ftp_proxy_type_ftp)->active=TRUE;
		GTK_TOGGLE_BUTTON(ftp_proxy_type_http)->active=FALSE;
	};
	GTK_TOGGLE_BUTTON(no_cache)->active=cfg->proxy_no_cache;

	if (cfg->socks_host.get()){
		GTK_TOGGLE_BUTTON(use_socks)->active=TRUE;
		text_to_combo(socks_host,cfg->socks_host.get());
	}else{
		text_to_combo(socks_host,"");
		GTK_TOGGLE_BUTTON(use_socks)->active=FALSE;
	};
	proxy_toggle_socks(use_socks,this);
	if  (cfg->socks_user.get())
		text_to_combo(socks_user,cfg->socks_user.get());
	else
		text_to_combo(socks_user,"");
	if  (cfg->socks_pass.get())
		text_to_combo(socks_pass,cfg->socks_pass.get());
	else
		text_to_combo(socks_pass,"");
	if (cfg->socks_port){
		char data[MAX_LEN];
		sprintf(data,"%i",cfg->socks_port);
		text_to_combo(socks_port,data);
	};
};


static void make_proxy_host(const char *host,int port){
	char *tmp=new char[strlen(host)+10];
	char *b=tmp;
	while(*host!=':' && *host){
		*b=*host;
		host++;
		b++;
	};
	*b=':';
	b++;
	sprintf(b,"%i",port);
	if (*tmp!=':' && *tmp)
		ALL_HISTORIES[PROXY_HISTORY]->add(tmp);
	delete[] tmp;
};

#define REMOVE_SC_FROM_HOST(host) if (host){ \
   char *sc=index(host,':');\
   if (sc) *sc=0; \
  };

void tProxyWidget::apply_changes(tMainCfg *cfg) {
	cfg->NEED_PASS_FTP_PROXY=GTK_TOGGLE_BUTTON(ftp_proxy_user_check)->active;
	cfg->NEED_PASS_HTTP_PROXY=GTK_TOGGLE_BUTTON(http_proxy_user_check)->active;
	cfg->USE_PROXY_FOR_FTP=GTK_TOGGLE_BUTTON(ftp_proxy_check)->active;
	cfg->USE_PROXY_FOR_HTTP=GTK_TOGGLE_BUTTON(http_proxy_check)->active;

	if (cfg->FTP_PROXY_HOST) delete[] cfg->FTP_PROXY_HOST;
	cfg->FTP_PROXY_HOST=copy_string(text_from_combo(ftp_proxy_host));
	sscanf(gtk_entry_get_text(GTK_ENTRY(ftp_proxy_port)),"%i",&cfg->FTP_PROXY_PORT);
	if (cfg->FTP_PROXY_USER) delete[] cfg->FTP_PROXY_USER;
	cfg->FTP_PROXY_USER=copy_string(text_from_combo(ftp_proxy_user));
	if (cfg->FTP_PROXY_PASS) delete[] cfg->FTP_PROXY_PASS;
	cfg->FTP_PROXY_PASS=copy_string(text_from_combo(ftp_proxy_pass));

	if (cfg->HTTP_PROXY_HOST) delete[] cfg->HTTP_PROXY_HOST;
	cfg->HTTP_PROXY_HOST=copy_string(text_from_combo(http_proxy_host));
	sscanf(gtk_entry_get_text(GTK_ENTRY(http_proxy_port)),"%i",&cfg->HTTP_PROXY_PORT);
	if (cfg->HTTP_PROXY_USER) delete[] cfg->HTTP_PROXY_USER;
	cfg->HTTP_PROXY_USER=copy_string(text_from_combo(http_proxy_user));
	if (cfg->HTTP_PROXY_PASS) delete[] cfg->HTTP_PROXY_PASS;
	cfg->HTTP_PROXY_PASS=copy_string(text_from_combo(http_proxy_pass));
	if (strlen(cfg->HTTP_PROXY_USER)) ALL_HISTORIES[USER_HISTORY]->add(cfg->HTTP_PROXY_USER);
	if (strlen(cfg->FTP_PROXY_USER)) ALL_HISTORIES[USER_HISTORY]->add(cfg->FTP_PROXY_USER);
	if (GTK_TOGGLE_BUTTON(ftp_proxy_type_ftp)->active) {
		cfg->FTP_PROXY_TYPE=0;
	} else
		cfg->FTP_PROXY_TYPE=1;
	if (strlen(text_from_combo(ftp_proxy_host))){
		make_proxy_host(text_from_combo(ftp_proxy_host),
				cfg->FTP_PROXY_PORT);
	};
	if (strlen(text_from_combo(http_proxy_host))){
		make_proxy_host(text_from_combo(http_proxy_host),
				cfg->HTTP_PROXY_PORT);
	};
	if (cfg->REMEMBER_PASS){
		if (cfg->HTTP_PROXY_PASS && strlen(cfg->HTTP_PROXY_PASS))
			ALL_HISTORIES[PASS_HISTORY]->add(cfg->HTTP_PROXY_PASS);
		if (cfg->FTP_PROXY_PASS  && strlen(cfg->FTP_PROXY_PASS))
			ALL_HISTORIES[PASS_HISTORY]->add(cfg->FTP_PROXY_PASS);
	};
	cfg->PROXY_NO_CACHE=GTK_TOGGLE_BUTTON(no_cache)->active;
	if (GTK_TOGGLE_BUTTON(use_socks)->active){		
		if (cfg->SOCKS_HOST) delete[] cfg->SOCKS_HOST;
		if (cfg->SOCKS_PASS) delete[] cfg->SOCKS_PASS;
		if (cfg->SOCKS_USER) delete[] cfg->SOCKS_USER;
		sscanf(text_from_combo(socks_port),"%i",&(cfg->SOCKS_PORT));
		char *tmp=text_from_combo(socks_host);
		if (tmp && *tmp){
			make_proxy_host(tmp,
					cfg->SOCKS_PORT);
			cfg->SOCKS_HOST=copy_string(tmp);
		}else{
			cfg->SOCKS_HOST=NULL;
		};
		tmp=text_from_combo(socks_pass);
		if (tmp && *tmp)
			cfg->SOCKS_PASS=copy_string(tmp);
		else
			cfg->SOCKS_PASS=NULL;
		tmp=text_from_combo(socks_user);
		if (tmp && *tmp)
			cfg->SOCKS_USER=copy_string(tmp);
		else
			cfg->SOCKS_USER=NULL;
	}else{
		if (cfg->SOCKS_HOST) delete[] cfg->SOCKS_HOST;
		if (cfg->SOCKS_PASS) delete[] cfg->SOCKS_PASS;
		if (cfg->SOCKS_USER) delete[] cfg->SOCKS_USER;
		cfg->SOCKS_HOST=NULL;
		cfg->SOCKS_USER=NULL;
		cfg->SOCKS_PASS=NULL;
	};
	/* remove ':' from proxies hosts */
	REMOVE_SC_FROM_HOST(cfg->SOCKS_HOST);
	REMOVE_SC_FROM_HOST(cfg->HTTP_PROXY_HOST);
	REMOVE_SC_FROM_HOST(cfg->FTP_PROXY_HOST);
};

void tProxyWidget::apply_changes() {
	apply_changes(&CFG);
};

void tProxyWidget::apply_changes(tCfg *cfg,int proto) {
	cfg->reset_proxy();
	if (GTK_TOGGLE_BUTTON(ftp_proxy_check)->active) {
		cfg->fproxy_host.set(text_from_combo(ftp_proxy_host));
		sscanf(gtk_entry_get_text(GTK_ENTRY(ftp_proxy_port)),"%i",&(cfg->fproxy_port));
		make_proxy_host(text_from_combo(ftp_proxy_host),
				cfg->fproxy_port);
		if (GTK_TOGGLE_BUTTON(ftp_proxy_user_check)->active) {
			cfg->fproxy_user.set(text_from_combo(ftp_proxy_user));
			cfg->fproxy_pass.set(text_from_combo(ftp_proxy_pass));
		};
	};
	if (GTK_TOGGLE_BUTTON(http_proxy_check)->active) {
		cfg->hproxy_host.set(text_from_combo(http_proxy_host));
		sscanf(gtk_entry_get_text(GTK_ENTRY(http_proxy_port)),"%i",&(cfg->hproxy_port));
		make_proxy_host(text_from_combo(http_proxy_host),
				cfg->hproxy_port);
		if (GTK_TOGGLE_BUTTON(http_proxy_user_check)->active) {
			cfg->hproxy_user.set(text_from_combo(http_proxy_user));
			cfg->hproxy_pass.set(text_from_combo(http_proxy_pass));
		};
	};
	if (strlen(text_from_combo(ftp_proxy_user)))
		ALL_HISTORIES[USER_HISTORY]->add(text_from_combo(ftp_proxy_user));
	if (strlen(text_from_combo(http_proxy_user)))
		ALL_HISTORIES[USER_HISTORY]->add(text_from_combo(http_proxy_user));
	if (GTK_TOGGLE_BUTTON(ftp_proxy_type_ftp)->active)
		cfg->proxy_type=0;
	else
		cfg->proxy_type=1;
	cfg->proxy_no_cache=GTK_TOGGLE_BUTTON(no_cache)->active;

	if (GTK_TOGGLE_BUTTON(use_socks)->active){		
		char *tmp=text_from_combo(socks_host);
		if (tmp && *tmp){
			ALL_HISTORIES[PROXY_HISTORY]->add(tmp);
			cfg->socks_host.set(tmp);
		}else{
			cfg->socks_host.set(NULL);
		};
		tmp=text_from_combo(socks_pass);
		if (tmp && *tmp)
			cfg->socks_pass.set(tmp);
		else
			cfg->socks_pass.set(NULL);
		tmp=text_from_combo(socks_user);
		if (tmp && *tmp)
			cfg->socks_user.set(tmp);
		else
			cfg->socks_user.set(NULL);
		sscanf(text_from_combo(socks_port),"%i",&(cfg->socks_port));
	}else{
		cfg->socks_user.set(NULL);
		cfg->socks_pass.set(NULL);
		cfg->socks_host.set(NULL);
	};
	REMOVE_SC_FROM_HOST(cfg->socks_host.get());
	REMOVE_SC_FROM_HOST(cfg->fproxy_host.get());
	REMOVE_SC_FROM_HOST(cfg->hproxy_host.get());
};

/*****************************************************************/

GtkWidget *select_options_window = (GtkWidget *)NULL;

gint select_options_window_hide(GtkWidget *window,GdkEvent *event, gpointer data){
	if (select_options_window){
		gtk_window_set_modal (GTK_WINDOW(select_options_window),FALSE);
//		gtk_window_set_transient_for (GTK_WINDOW (select_options_window), GTK_WINDOW (NULL));
		gtk_widget_hide(select_options_window);
	};
	return TRUE;
};

void select_options_window_ok(GtkWidget *button,GtkWidget *window){
	if (window){
		GtkWidget *list=GTK_WIDGET(gtk_object_get_user_data(GTK_OBJECT(window)));
		if (list && GTK_CLIST(list)->selection){
			gint table[EDIT_OPT_LASTOPTION];
			for (int i=0;i<EDIT_OPT_LASTOPTION;i++)
				table[i]=0;
			GList *selection = GTK_CLIST(list)->selection;
			while (selection){
				gint row=GPOINTER_TO_INT(selection->data);
				int opt=GPOINTER_TO_INT(gtk_clist_get_row_data(GTK_CLIST(list),row));
				if (opt<EDIT_OPT_LASTOPTION){
					table[opt]=1;
//					printf("Selected %s\n",edit_fields_labels[opt]);
				};
				selection=selection->next;
			};
			select_options_window_hide(window,NULL,NULL);
			init_edit_common_properties_window(table);
			return;
		};
	};
	select_options_window_hide(window,NULL,NULL);
};

void select_options_window_unselect_all(GtkWidget *button,GtkWidget *list){
	if (GTK_CLIST(list)->selection)
		gtk_clist_unselect_all(GTK_CLIST(list));
};

void select_options_window_select_all(GtkWidget *button,GtkWidget *list){
	gtk_clist_select_all(GTK_CLIST(list));
};

void select_options_window_init(){
	if (select_options_window){
		select_options_window_unselect_all(NULL,
						   GTK_WIDGET(gtk_object_get_user_data(GTK_OBJECT(select_options_window))));
		gtk_widget_show(select_options_window);
	}else{
		select_options_window=gtk_window_new(GTK_WINDOW_DIALOG);
		gtk_window_set_wmclass(GTK_WINDOW(select_options_window),
				       "D4X_CommonProp","D4X");
		gtk_window_set_title(GTK_WINDOW (select_options_window),_("Select properties"));
		gtk_container_border_width(GTK_CONTAINER(select_options_window),5);
		gtk_window_set_position(GTK_WINDOW(select_options_window),GTK_WIN_POS_CENTER);
		gtk_window_set_policy (GTK_WINDOW(select_options_window), FALSE,FALSE,FALSE);
		gtk_signal_connect(GTK_OBJECT(select_options_window),
				   "delete_event",
				   GTK_SIGNAL_FUNC(select_options_window_hide), NULL);
		d4x_eschandler_init(select_options_window,NULL);

		GtkWidget *list=gtk_clist_new(1);
		gtk_clist_set_selection_mode(GTK_CLIST(list),GTK_SELECTION_EXTENDED);
		gtk_object_set_user_data(GTK_OBJECT(select_options_window),list);
		for (int i=EDIT_OPT_USERPASS;i<EDIT_OPT_LASTOPTION;i++){
			char *tmp[1];
			tmp[0]=_(edit_fields_labels[i]);
			gint row=gtk_clist_append(GTK_CLIST(list),tmp);
			gtk_clist_set_row_data(GTK_CLIST(list),row,GINT_TO_POINTER(i));
		};
		
		GtkWidget *button_ok=gtk_button_new_with_label(_("Ok"));
		GTK_WIDGET_SET_FLAGS(button_ok,GTK_CAN_DEFAULT);
		gtk_signal_connect(GTK_OBJECT(button_ok),
				   "clicked",
				   GTK_SIGNAL_FUNC(select_options_window_ok),
				   select_options_window);
		GtkWidget *button_clear=gtk_button_new_with_label(_("Unselect all"));
		GTK_WIDGET_SET_FLAGS(button_clear,GTK_CAN_DEFAULT);
		gtk_signal_connect(GTK_OBJECT(button_clear),
				   "clicked",
				   GTK_SIGNAL_FUNC(select_options_window_unselect_all),
				   list);
		GtkWidget *button_all=gtk_button_new_with_label(_("Select all"));
		GTK_WIDGET_SET_FLAGS(button_all,GTK_CAN_DEFAULT);
		gtk_signal_connect(GTK_OBJECT(button_all),
				   "clicked",
				   GTK_SIGNAL_FUNC(select_options_window_select_all),
				   list);
		GtkWidget *hbox=gtk_hbutton_box_new();
		gtk_box_set_spacing(GTK_BOX(hbox),5);
		gtk_box_pack_start(GTK_BOX(hbox),button_all,FALSE,FALSE,0);
		gtk_box_pack_start(GTK_BOX(hbox),button_clear,FALSE,FALSE,0);
		gtk_box_pack_start(GTK_BOX(hbox),button_ok,FALSE,FALSE,0);
		GtkWidget *vbox=gtk_vbox_new(FALSE,0);
		gtk_box_set_spacing(GTK_BOX(vbox),5);
		gtk_box_pack_start(GTK_BOX(vbox),list,TRUE,TRUE,0);
		gtk_box_pack_start(GTK_BOX(vbox),hbox,TRUE,TRUE,0);
		gtk_container_add(GTK_CONTAINER(select_options_window),vbox);
		gtk_window_set_default(GTK_WINDOW(select_options_window),button_ok);
		gtk_widget_show_all(select_options_window);
		gtk_window_set_transient_for (GTK_WINDOW (select_options_window), GTK_WINDOW (MainWindow));
	};
	gtk_window_set_modal (GTK_WINDOW(select_options_window),TRUE);
};
