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
#include <string.h>
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
	EDIT_OPT_LINKASFILE,
	EDIT_OPT_RECURSEDEPTHFTP,
	EDIT_OPT_RECURSEDEPTHHTTP,
	EDIT_OPT_LEAVEDIR,
	EDIT_OPT_LEAVESERVER,
	EDIT_OPT_USERAGENT,
	EDIT_OPT_PROXY,
	EDIT_OPT_TIME,
	EDIT_OPT_LASTOPTION
};

char *edit_fields_labels[]={
	"Use password for this site",
	"Save download to folder",
	"Restart this download from begining",
	"Timeout for reading from socket",
	"Speed limitation",
	"Maximum attempts",
	"Timeout before reconnection",
	"Rollback after reconnecting",
	"Get date from the server",
	"Retry if resuming is not supported",
	"Number of parts for spliting this download",
	"Use passive mode for FTP",
	"Get permissions of the file from server (FTP only)",
	"Don't send QUIT command (FTP)",
	"Try to load symbolic link as file via FTP",
	"Depth of recursing for FTP",
	"Depth of recursing for HTTP",
	"Only subdirs",
	"Allow leave this server while recursing via HTTP",
	"User-Agent",
	"Proxy",
	"Time"
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
	if (list)
		gtk_combo_set_popdown_strings (GTK_COMBO (combo), list);
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
	if (rvalue)
		gtk_combo_set_popdown_strings (GTK_COMBO (combo), rvalue);
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
static gint _edit_window_event_handler(GtkWidget *window,GdkEvent *event,tDEdit *where){
	if (event && event->type == GDK_KEY_PRESS) {
		GdkEventKey *kevent=(GdkEventKey *)event;
		switch(kevent->keyval) {
		case GDK_Escape:{
			if (where) delete(where);
			return TRUE;
			break;
		};
		};
	};
	return FALSE;
};

void init_edit_window(tDownload *what) {
	if (!what) return;
	if (what->editor) {
		what->editor->popup();
		return;
	};
	what->editor=new tDEdit;
	what->editor->init(what);
	if (what->owner==DL_RUN || what->owner==DL_STOPWAIT) what->editor->disable_ok_button();
	gtk_window_set_title(GTK_WINDOW(what->editor->window),_("Edit download"));
	gtk_signal_connect(GTK_OBJECT(what->editor->cancel_button),"clicked",GTK_SIGNAL_FUNC(edit_window_cancel),what->editor);
	gtk_signal_connect(GTK_OBJECT(what->editor->ok_button),"clicked",GTK_SIGNAL_FUNC(edit_window_ok),what->editor);
	gtk_signal_connect(GTK_OBJECT(what->editor->window),"delete_event",GTK_SIGNAL_FUNC(edit_window_delete), what->editor);
	gtk_signal_connect(GTK_OBJECT(what->editor->window), "key_press_event",
			   (GtkSignalFunc)_edit_window_event_handler, what->editor);
};

void edit_window_cancel(GtkWidget *parent,tDEdit *where) {
	delete where;
};

gint edit_window_delete(GtkObject *parent) {
	tDEdit *tmp=(tDEdit *)gtk_object_get_user_data(parent);
	delete tmp;
	return TRUE;
};

void edit_window_ok(GtkWidget *which,tDEdit *where) {
	if (where->apply_changes())
		return;
	list_of_downloads_update(where->get_parent());
	if (!where->get_pause_check())
		aa.continue_download(where->get_parent());
	delete where;
};

static void edit_browser_path_set_as_default(GtkWidget *parent,tDEdit *where){
	where->set_path_as_default();
};

static void edit_window_password(GtkWidget *parent,tDEdit *where) {
	where->setup_entries();
};

static void edit_time_check_clicked(GtkWidget *parent,tDEdit *where) {
	where->toggle_time();
};

/******************************************************/

tDEdit::tDEdit() {
	parent=NULL;
	window=NULL;
	proxy=NULL;
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
		delete(CFG.GLOBAL_SAVE_PATH);
	CFG.GLOBAL_SAVE_PATH=copy_string(text_from_combo(MY_GTK_FILESEL(path_entry)->combo));
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
	MY_GTK_FILESEL(path_entry)->modal=GTK_WINDOW(window);
	MY_GTK_FILESEL(file_entry)->modal=GTK_WINDOW(window);
	url_entry=my_gtk_combo_new(ALL_HISTORIES[URL_HISTORY]);
	MY_GTK_FILESEL(path_entry)->only_dirs=TRUE;
	desc_entry=my_gtk_combo_new(ALL_HISTORIES[DESC_HISTORY]);
	set_description("");
	
//	char temp[MAX_LEN];
//	make_url_from_download(who,temp);
//	text_to_combo(url_entry,temp);
	char *URL=who->info->url();
	text_to_combo(url_entry,URL);
	delete URL;

	text_to_combo(MY_GTK_FILESEL(path_entry)->combo,who->config.save_path.get());
	if (who->config.save_name.get())
		text_to_combo(MY_GTK_FILESEL(file_entry)->combo,who->config.save_name.get());
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
	GTK_TOGGLE_BUTTON(pause_check)->active=FALSE;
	GTK_TOGGLE_BUTTON(restart_from_begin_check)->active=who->config.restart_from_begin;
	gtk_box_pack_start(GTK_BOX(vbox),pause_check,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),restart_from_begin_check,FALSE,FALSE,0);
	GtkWidget *frame=gtk_frame_new(_("Download"));
	gtk_container_border_width(GTK_CONTAINER(frame),5);
	gtk_container_add(GTK_CONTAINER(frame),vbox);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook),frame,gtk_label_new(_("Main")));
};

void tDEdit::init_other(tDownload *who) {
	/* initing other
	 */
	GtkWidget *other_vbox=gtk_vbox_new(FALSE,0);
	timeout_entry=my_gtk_entry_new_with_max_length(3,who->config.timeout);
	sleep_entry=my_gtk_entry_new_with_max_length(3,who->config.time_for_sleep);
	attempts_entry=my_gtk_entry_new_with_max_length(3,who->config.number_of_attempts);
	rollback_entry=my_gtk_entry_new_with_max_length(5,who->config.rollback);
	speed_entry=my_gtk_entry_new_with_max_length(5,who->config.speed);
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
	GTK_TOGGLE_BUTTON(get_date_check)->active=who->config.get_date;
	gtk_box_pack_start(GTK_BOX(other_vbox),get_date_check,FALSE,FALSE,0);

	retry_check=gtk_check_button_new_with_label(_("Retry if resuming is not supported"));
	GTK_TOGGLE_BUTTON(retry_check)->active=who->config.retry;
	gtk_box_pack_start(GTK_BOX(other_vbox),retry_check,FALSE,FALSE,0);

	sleep_check=gtk_check_button_new_with_label(_("Sleep before completing"));
	GTK_TOGGLE_BUTTON(sleep_check)->active=who->config.sleep_before_complete;
	gtk_box_pack_start(GTK_BOX(other_vbox),sleep_check,FALSE,FALSE,0);

	other_label=gtk_label_new(_("Save log to file"));
	GtkWidget *other_box=gtk_vbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(other_box),5);
	log_save_entry=my_gtk_filesel_new(ALL_HISTORIES[LOG_SAVE_HISTORY]);
	MY_GTK_FILESEL(log_save_entry)->modal=GTK_WINDOW(window);
	if (who->config.log_save_path.get())
		text_to_combo(MY_GTK_FILESEL(log_save_entry)->combo,
			      who->config.log_save_path.get());
	else
		text_to_combo(MY_GTK_FILESEL(log_save_entry)->combo,"");
	gtk_box_pack_start(GTK_BOX(other_box),other_label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(other_box),log_save_entry,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(other_vbox),other_box,FALSE,FALSE,0);
	
	GtkWidget *other_frame=gtk_frame_new(_("Common"));
	gtk_container_border_width(GTK_CONTAINER(other_frame),5);
	gtk_container_add(GTK_CONTAINER(other_frame),other_vbox);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook),other_frame,gtk_label_new(_("Common")));
};

void tDEdit::init_ftp(tDownload *who){
	GtkWidget *ftp_vbox=gtk_vbox_new(FALSE,0);

	ftp_passive_check=gtk_check_button_new_with_label(_("Use passive mode for FTP"));
	GTK_TOGGLE_BUTTON(ftp_passive_check)->active=who->config.passive;
	gtk_box_pack_start(GTK_BOX(ftp_vbox),ftp_passive_check,FALSE,FALSE,0);
	dont_send_quit_check=gtk_check_button_new_with_label(_("Don't send QUIT command (FTP)"));
	GTK_TOGGLE_BUTTON(dont_send_quit_check)->active=who->config.dont_send_quit;
	gtk_box_pack_start(GTK_BOX(ftp_vbox),dont_send_quit_check,FALSE,FALSE,0);	
	permisions_check=gtk_check_button_new_with_label(_("Get permissions of the file from server (FTP only)"));
	GTK_TOGGLE_BUTTON(permisions_check)->active=who->config.permisions;
	gtk_box_pack_start(GTK_BOX(ftp_vbox),permisions_check,FALSE,FALSE,0);
	link_as_file_check=gtk_check_button_new_with_label(_("Try to load simbolyc link as file via FTP"));
	GTK_TOGGLE_BUTTON(link_as_file_check)->active=who->config.link_as_file;
	gtk_box_pack_start(GTK_BOX(ftp_vbox),link_as_file_check,FALSE,FALSE,0);

	
	ftp_recurse_depth_entry=my_gtk_entry_new_with_max_length(3,who->config.ftp_recurse_depth);
	GtkWidget *ftp_hbox=gtk_hbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(ftp_hbox),2);
	GtkWidget *other_label=gtk_label_new(_("Depth of recursing (0 unlimited,1 no recurse)"));
	gtk_box_pack_start(GTK_BOX(ftp_hbox),ftp_recurse_depth_entry,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(ftp_hbox),other_label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(ftp_vbox),ftp_hbox,FALSE,FALSE,0);
	
	GtkWidget *ftp_frame=gtk_frame_new("FTP");
	gtk_container_border_width(GTK_CONTAINER(ftp_frame),5);
	gtk_container_add(GTK_CONTAINER(ftp_frame),ftp_vbox);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook),ftp_frame,gtk_label_new("FTP"));
};

void tDEdit::init_http(tDownload *who){
	GtkWidget *http_vbox=gtk_vbox_new(FALSE,0);
	
	http_recurse_depth_entry=my_gtk_entry_new_with_max_length(3,who->config.http_recurse_depth);
	GtkWidget *http_hbox=gtk_hbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(http_hbox),5);
	GtkWidget *other_label=gtk_label_new(_("Depth of recursing (0 unlimited,1 no recurse)"));
	gtk_box_pack_start(GTK_BOX(http_hbox),http_recurse_depth_entry,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(http_hbox),other_label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(http_vbox),http_hbox,FALSE,FALSE,0);

	leave_dir_check=gtk_check_button_new_with_label(_("Only subdirs"));
	leave_server_check=gtk_check_button_new_with_label(_("Allow leave this server while recursing via HTTP"));
	GTK_TOGGLE_BUTTON(leave_server_check)->active=who->config.leave_server;
	GTK_TOGGLE_BUTTON(leave_dir_check)->active=who->config.dont_leave_dir;
	gtk_box_pack_start(GTK_BOX(http_vbox),leave_server_check,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(http_vbox),leave_dir_check,FALSE,FALSE,0);

	GtkWidget *user_agent_label=gtk_label_new(_("User-Agent"));
	GtkWidget *user_agent_box=gtk_vbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(user_agent_box),5);
	user_agent_entry=my_gtk_combo_new(ALL_HISTORIES[USER_AGENT_HISTORY]);
	if (who->config.user_agent.get())
		text_to_combo(user_agent_entry,who->config.user_agent.get());
	gtk_box_pack_start(GTK_BOX(user_agent_box),user_agent_label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(user_agent_box),user_agent_entry,TRUE,TRUE,0);
	gtk_box_pack_start(GTK_BOX(http_vbox),user_agent_box,FALSE,FALSE,0);

	GtkWidget *http_frame=gtk_frame_new("HTTP");
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
		proxy->init_state(&(who->config),1);
		break;
	};
	case D_PROTO_HTTP:{
		proxy->init_state(&(who->config),0);
		break;
	};
	};
	/* initing window
	 */
	GtkWidget *vbox2=gtk_vbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(vbox2),5);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook),proxy->frame,gtk_label_new(_("Proxy")));
	init_time(who);

	gtk_box_pack_start(GTK_BOX(vbox2),notebook,FALSE,FALSE,0);

	/* initing buttons
	 */
	GtkWidget *buttons_hbox=gtk_hbutton_box_new();
	gtk_button_box_set_layout(GTK_BUTTON_BOX(buttons_hbox),GTK_BUTTONBOX_END);
	gtk_button_box_set_spacing(GTK_BUTTON_BOX(buttons_hbox),5);
	ok_button=gtk_button_new_with_label(_("Ok"));
	cancel_button=gtk_button_new_with_label(_("Cancel"));
	GTK_WIDGET_SET_FLAGS(ok_button,GTK_CAN_DEFAULT);
	GTK_WIDGET_SET_FLAGS(cancel_button,GTK_CAN_DEFAULT);
	gtk_box_pack_start(GTK_BOX(buttons_hbox),ok_button,TRUE,TRUE,0);
	gtk_box_pack_start(GTK_BOX(buttons_hbox),cancel_button,TRUE,TRUE,0);
	gtk_box_pack_start(GTK_BOX(vbox2),buttons_hbox,FALSE,FALSE,0);
	gtk_container_add(GTK_CONTAINER(window),vbox2);
	gtk_window_set_default(GTK_WINDOW(window),ok_button);
	gtk_widget_show_all(window);
	gtk_widget_grab_focus(GTK_COMBO(url_entry)->entry);
	setup_entries();
};

int tDEdit::get_pause_check(){
	return(GTK_TOGGLE_BUTTON(pause_check)->active);
};

void tDEdit::disable_ok_button() {
	if (window) gtk_widget_set_sensitive(ok_button,FALSE);
};

void tDEdit::enable_ok_button() {
	if (window) gtk_widget_set_sensitive(ok_button,TRUE);
};

int tDEdit::apply_changes() {
	char *temp=copy_string(text_from_combo(url_entry));
	del_crlf(temp);
	tAddr *addr=new tAddr(temp);
	delete(temp);
	if (!addr) return 1;
	delete (parent->info);
	parent->info=addr;
	switch(parent->info->proto){
	case D_PROTO_FTP:{
		proxy->apply_changes(&(parent->config),1);
		break;
	};
	case D_PROTO_HTTP:{
		proxy->apply_changes(&(parent->config),0);
		break;
	};
	};
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
	parent->config.save_path.set(text_from_combo(MY_GTK_FILESEL(path_entry)->combo));
	parent->config.save_name.set(NULL);
	if (strlen(text_from_combo(MY_GTK_FILESEL(file_entry)->combo)))
		parent->config.save_name.set(text_from_combo(MY_GTK_FILESEL(file_entry)->combo));
	else
		parent->config.save_name.set(NULL);
	if (parent->config.save_name.get())
		ALL_HISTORIES[FILE_HISTORY]->add(parent->config.save_name.get());
	normalize_path(parent->config.save_path.get());
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
	parent->config.user_agent.set(text_from_combo(user_agent_entry));
	ALL_HISTORIES[PATH_HISTORY]->add(text_from_combo(MY_GTK_FILESEL(path_entry)->combo));
	ALL_HISTORIES[URL_HISTORY]->add(URL);
	ALL_HISTORIES[USER_AGENT_HISTORY]->add(text_from_combo(user_agent_entry));
	char *save_log=text_from_combo(MY_GTK_FILESEL(log_save_entry)->combo);
	if (save_log && *save_log){
		ALL_HISTORIES[LOG_SAVE_HISTORY]->add(save_log);
		parent->config.log_save_path.set(save_log);
	}else
		parent->config.log_save_path.set(NULL);
	char *desc=text_from_combo(desc_entry);
	if (desc && *desc){
		ALL_HISTORIES[DESC_HISTORY]->add(desc);
		parent->Description.set(desc);
	}else
		parent->Description.set(NULL);

	/*change data in list if available
	 */
	if (parent->GTKCListRow > 0) {
		list_of_downloads_change_data(parent->GTKCListRow,URL_COL,URL);
		list_of_downloads_change_data(parent->GTKCListRow,FILE_COL,parent->info->file.get());
		for (int i=FILE_TYPE_COL;i<URL_COL;i++)
			list_of_downloads_change_data(parent->GTKCListRow,i,"");
	};
	delete URL;
	int  temp1=0;
	sscanf(gtk_entry_get_text(GTK_ENTRY(timeout_entry)),"%u",&temp1);
	if (temp1>0 && temp1<1000) parent->config.timeout=temp1;
	sscanf(gtk_entry_get_text(GTK_ENTRY(sleep_entry)),"%u",&temp1);
	if (temp1>=0 && temp1<1000) parent->config.time_for_sleep=temp1;
	sscanf(gtk_entry_get_text(GTK_ENTRY(attempts_entry)),"%u",&temp1);
	if (temp1>=0) parent->config.number_of_attempts=temp1;
	temp1=1;
	sscanf(gtk_entry_get_text(GTK_ENTRY(ftp_recurse_depth_entry)),"%u",&temp1);
	if (temp1>=0) parent->config.ftp_recurse_depth=temp1;
	temp1=1;
	sscanf(gtk_entry_get_text(GTK_ENTRY(http_recurse_depth_entry)),"%u",&temp1);
	if (temp1>=0) parent->config.http_recurse_depth=temp1;
	sscanf(gtk_entry_get_text(GTK_ENTRY(rollback_entry)),"%u",&temp1);
	if (temp1>=0) parent->config.rollback=temp1;
	sscanf(gtk_entry_get_text(GTK_ENTRY(speed_entry)),"%u",&temp1);
	if (temp1>=0) parent->config.speed=temp1;
	parent->config.passive=GTK_TOGGLE_BUTTON(ftp_passive_check)->active;
	parent->config.dont_send_quit=GTK_TOGGLE_BUTTON(dont_send_quit_check)->active;
	parent->config.permisions=GTK_TOGGLE_BUTTON(permisions_check)->active;
	parent->config.get_date=GTK_TOGGLE_BUTTON(get_date_check)->active;
	parent->config.retry=GTK_TOGGLE_BUTTON(retry_check)->active;
	parent->config.link_as_file=GTK_TOGGLE_BUTTON(link_as_file_check)->active;
	parent->config.leave_server=GTK_TOGGLE_BUTTON(leave_server_check)->active;
	parent->config.dont_leave_dir=GTK_TOGGLE_BUTTON(leave_dir_check)->active;
	parent->config.restart_from_begin=GTK_TOGGLE_BUTTON(restart_from_begin_check)->active;
	parent->config.sleep_before_complete=GTK_TOGGLE_BUTTON(sleep_check)->active;
	parent->config.http_recursing=parent->config.http_recurse_depth==1?0:1;

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
	} else {
		parent->ScheduleTime=0;
	};
	return 0;
};

void tDEdit::toggle_time() {
	gtk_widget_set_sensitive(calendar,GTK_TOGGLE_BUTTON(time_check)->active);
	gtk_widget_set_sensitive(hour_entry,GTK_TOGGLE_BUTTON(time_check)->active);
	gtk_widget_set_sensitive(minute_entry,GTK_TOGGLE_BUTTON(time_check)->active);
};


void tDEdit::set_description(char *desc){
	text_to_combo(desc_entry,desc);
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
/*
	char *clipboard=my_xclipboard_get();
	if (clipboard){
		if (*clipboard){
			set_url(clipboard);
		}else{
			if (old_clipboard_content()!=NULL)
				set_url(old_clipboard_content());
		};
		my_xclipboard_free(clipboard);
	};
*/
	if (old_clipboard_content()!=NULL)
		set_url(old_clipboard_content());
	else
		gtk_editable_paste_clipboard(GTK_EDITABLE(GTK_COMBO(url_entry)->entry));
//	printf("%s\n",text_from_combo(url_entry));
};

void tDEdit::select_url() {
	gtk_entry_select_region(GTK_ENTRY(GTK_COMBO(url_entry)->entry),0,strlen(text_from_combo(url_entry)));
};

void tDEdit::clear_url() {
	text_to_combo(url_entry,"");
};

void tDEdit::set_url(char *a) {
	text_to_combo(url_entry,a);
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
	if (array[EDIT_OPT_PERMISSIONS]==0)
		gtk_widget_set_sensitive(permisions_check,FALSE);
	if (array[EDIT_OPT_DATE]==0)
		gtk_widget_set_sensitive(get_date_check,FALSE);
	if (array[EDIT_OPT_IFNOREGET]==0)
		gtk_widget_set_sensitive(retry_check,FALSE);
	if (array[EDIT_OPT_LINKASFILE]==0)
		gtk_widget_set_sensitive(link_as_file_check,FALSE);
	if (array[EDIT_OPT_LEAVESERVER]==0)
		gtk_widget_set_sensitive(leave_server_check,FALSE);
	if (array[EDIT_OPT_LEAVEDIR]==0)
		gtk_widget_set_sensitive(leave_dir_check,FALSE);
	if (array[EDIT_OPT_RECURSEDEPTHFTP]==0)
		gtk_widget_set_sensitive(ftp_recurse_depth_entry,FALSE);
	if (array[EDIT_OPT_RECURSEDEPTHHTTP]==0)
		gtk_widget_set_sensitive(http_recurse_depth_entry,FALSE);
	if (array[EDIT_OPT_FROMBEGIN]==0)
		gtk_widget_set_sensitive(restart_from_begin_check,FALSE);
	if (array[EDIT_OPT_SPEED]==0)
		gtk_widget_set_sensitive(speed_entry,FALSE);
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
			proxy->apply_changes(&(parent->config),1);
			break;
		};
		case D_PROTO_HTTP:{
			proxy->apply_changes(&(parent->config),0);
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
		parent->config.save_path.set(text_from_combo(MY_GTK_FILESEL(path_entry)->combo));
		normalize_path(parent->config.save_path.get());
		ALL_HISTORIES[PATH_HISTORY]->add(text_from_combo(MY_GTK_FILESEL(path_entry)->combo));
	};
	if (GTK_WIDGET_SENSITIVE(user_agent_entry)){
		parent->config.user_agent.set(text_from_combo(user_agent_entry));
		ALL_HISTORIES[USER_AGENT_HISTORY]->add(text_from_combo(user_agent_entry));
	};
	/*change data in list if available
	 */
	int  temp1=0;
	if (GTK_WIDGET_SENSITIVE(timeout_entry)){
		sscanf(gtk_entry_get_text(GTK_ENTRY(timeout_entry)),"%u",&temp1);
		if (temp1>0 && temp1<1000) parent->config.timeout=temp1;
	};
	if (GTK_WIDGET_SENSITIVE(sleep_entry)){
		sscanf(gtk_entry_get_text(GTK_ENTRY(sleep_entry)),"%u",&temp1);
		if (temp1>=0 && temp1<1000) parent->config.time_for_sleep=temp1;
	};
	if (GTK_WIDGET_SENSITIVE(attempts_entry)){
		sscanf(gtk_entry_get_text(GTK_ENTRY(attempts_entry)),"%u",&temp1);
		if (temp1>=0) parent->config.number_of_attempts=temp1;
	};
	if (GTK_WIDGET_SENSITIVE(ftp_recurse_depth_entry)){
		temp1=1;
		sscanf(gtk_entry_get_text(GTK_ENTRY(ftp_recurse_depth_entry)),"%u",&temp1);
		if (temp1>=0) parent->config.ftp_recurse_depth=temp1;
	};
	if (GTK_WIDGET_SENSITIVE(http_recurse_depth_entry)){
		temp1=1;
		sscanf(gtk_entry_get_text(GTK_ENTRY(http_recurse_depth_entry)),"%u",&temp1);
		if (temp1>=0) parent->config.http_recurse_depth=temp1;
	};
	if (GTK_WIDGET_SENSITIVE(rollback_entry)){
		sscanf(gtk_entry_get_text(GTK_ENTRY(rollback_entry)),"%u",&temp1);
		if (temp1>=0) parent->config.rollback=temp1;
	};
	if (GTK_WIDGET_SENSITIVE(speed_entry)){
		sscanf(gtk_entry_get_text(GTK_ENTRY(speed_entry)),"%u",&temp1);
		if (temp1>=0) parent->config.speed=temp1;
	};
	if (GTK_WIDGET_SENSITIVE(ftp_passive_check))
		parent->config.passive=GTK_TOGGLE_BUTTON(ftp_passive_check)->active;
	if (GTK_WIDGET_SENSITIVE(dont_send_quit_check))
		parent->config.dont_send_quit=GTK_TOGGLE_BUTTON(dont_send_quit_check)->active;
	if (GTK_WIDGET_SENSITIVE(permisions_check))
		parent->config.permisions=GTK_TOGGLE_BUTTON(permisions_check)->active;
	if (GTK_WIDGET_SENSITIVE(get_date_check))
		parent->config.get_date=GTK_TOGGLE_BUTTON(get_date_check)->active;
	if (GTK_WIDGET_SENSITIVE(retry_check))
		parent->config.retry=GTK_TOGGLE_BUTTON(retry_check)->active;
	if (GTK_WIDGET_SENSITIVE(link_as_file_check))
		parent->config.link_as_file=GTK_TOGGLE_BUTTON(link_as_file_check)->active;
	if (GTK_WIDGET_SENSITIVE(leave_server_check))
		parent->config.leave_server=GTK_TOGGLE_BUTTON(leave_server_check)->active;
	if (GTK_WIDGET_SENSITIVE(leave_dir_check))
		parent->config.dont_leave_dir=GTK_TOGGLE_BUTTON(leave_dir_check)->active;
	if (GTK_WIDGET_SENSITIVE(restart_from_begin_check))
		parent->config.restart_from_begin=GTK_TOGGLE_BUTTON(restart_from_begin_check)->active;
	parent->config.http_recursing=parent->config.http_recurse_depth==1?0:1;

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
		} else {
			parent->ScheduleTime=0;
		};
	};
};

void tDEdit::done() {
	if (parent) parent->editor=NULL;
	gtk_widget_destroy(window);
	delete proxy;
};

tDEdit::~tDEdit() {
	done();
};

/*******************************************************/

void proxy_toggle_pass_ftp(GtkWidget *parent,tProxyWidget *where) {
	set_editable_for_combo(where->ftp_proxy_pass,GTK_TOGGLE_BUTTON(parent)->active);
	gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(where->ftp_proxy_user)->entry),GTK_TOGGLE_BUTTON(parent)->active);
	gtk_widget_set_sensitive(where->ftp_proxy_user,GTK_TOGGLE_BUTTON(parent)->active);
	gtk_widget_set_sensitive(where->ftp_proxy_pass,GTK_TOGGLE_BUTTON(parent)->active);
};

void proxy_toggle_pass_http(GtkWidget *parent,tProxyWidget *where) {
	set_editable_for_combo(where->http_proxy_pass,GTK_TOGGLE_BUTTON(parent)->active);
	gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(where->http_proxy_user)->entry),GTK_TOGGLE_BUTTON(parent)->active);
	gtk_widget_set_sensitive(where->http_proxy_user,GTK_TOGGLE_BUTTON(parent)->active);
	gtk_widget_set_sensitive(where->http_proxy_pass,GTK_TOGGLE_BUTTON(parent)->active);
};

void tProxyWidget::init() {
	frame=gtk_frame_new(_("Proxy"));
	GtkWidget *proxy_frame1=gtk_frame_new("FTP");
	GtkWidget *proxy_frame2=gtk_frame_new("HTTP");
	GtkWidget *proxy_frame3=gtk_frame_new(_("FTP proxy type"));
	gtk_container_border_width(GTK_CONTAINER(frame),5);
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
	gtk_widget_set_usize(ftp_proxy_host,150,-1);

	gtk_box_pack_start(GTK_BOX(vbox),ftp_proxy_host,FALSE,0,0);
	ftp_proxy_port=gtk_entry_new();
	gtk_widget_set_usize(ftp_proxy_port,50,-1);
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
	gtk_container_add(GTK_CONTAINER(frame),vbox_temp);

	no_cache=gtk_check_button_new_with_label(_("Don't get from cache"));
	gtk_box_pack_start(GTK_BOX(vbox_temp),no_cache,FALSE,0,0);

	vbox=gtk_vbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(vbox),2);
	gtk_container_add(GTK_CONTAINER(proxy_frame2),vbox);

	http_proxy_check=gtk_check_button_new_with_label(_("Use this proxy for HTTP"));

	gtk_box_pack_start(GTK_BOX(vbox),http_proxy_check,FALSE,0,0);
	http_proxy_host=my_gtk_combo_new(ALL_HISTORIES[PROXY_HISTORY]);
	gtk_widget_set_usize(http_proxy_host,150,-1);

	gtk_box_pack_start(GTK_BOX(vbox),http_proxy_host,FALSE,0,0);
	http_proxy_port=gtk_entry_new();
	gtk_widget_set_usize(http_proxy_port,50,-1);
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

};

void tProxyWidget::init_state() {
	GTK_TOGGLE_BUTTON(ftp_proxy_check)->active=CFG.USE_PROXY_FOR_FTP;
	GTK_TOGGLE_BUTTON(ftp_proxy_user_check)->active=CFG.NEED_PASS_FTP_PROXY;
	if (CFG.FTP_PROXY_USER)
		text_to_combo(ftp_proxy_user,CFG.FTP_PROXY_USER);
	if (CFG.FTP_PROXY_PORT) {
		char data[MAX_LEN];
		sprintf(data,"%i",CFG.FTP_PROXY_PORT);
		gtk_entry_set_text(GTK_ENTRY(ftp_proxy_port),data);
	};
	if (CFG.FTP_PROXY_HOST)
		text_to_combo(ftp_proxy_host,CFG.FTP_PROXY_HOST);
	if (CFG.FTP_PROXY_PASS)
		text_to_combo(ftp_proxy_pass,CFG.FTP_PROXY_PASS);
	else
		text_to_combo(ftp_proxy_pass,"");
	GTK_TOGGLE_BUTTON(http_proxy_user_check)->active=CFG.NEED_PASS_HTTP_PROXY;
	GTK_TOGGLE_BUTTON(http_proxy_check)->active=CFG.USE_PROXY_FOR_HTTP;
	if (CFG.HTTP_PROXY_HOST)
		text_to_combo(http_proxy_host,CFG.HTTP_PROXY_HOST);
	if (CFG.HTTP_PROXY_PORT) {
		char data[MAX_LEN];
		sprintf(data,"%i",CFG.HTTP_PROXY_PORT);
		gtk_entry_set_text(GTK_ENTRY(http_proxy_port),data);
	};
	if (CFG.HTTP_PROXY_USER)
		text_to_combo(http_proxy_user,CFG.HTTP_PROXY_USER);
	if (CFG.HTTP_PROXY_PASS)
		text_to_combo(http_proxy_pass,CFG.HTTP_PROXY_PASS);
	proxy_toggle_pass_ftp(ftp_proxy_user_check,this);
	proxy_toggle_pass_http(http_proxy_user_check,this);
	if (CFG.FTP_PROXY_TYPE) {
		GTK_TOGGLE_BUTTON(ftp_proxy_type_ftp)->active=FALSE;
		GTK_TOGGLE_BUTTON(ftp_proxy_type_http)->active=TRUE;
	} else {
		GTK_TOGGLE_BUTTON(ftp_proxy_type_ftp)->active=TRUE;
		GTK_TOGGLE_BUTTON(ftp_proxy_type_http)->active=FALSE;
	};
	GTK_TOGGLE_BUTTON(no_cache)->active=CFG.PROXY_NO_CACHE;
};

void tProxyWidget::init_state(tCfg *cfg,int proto) {
	if (proto) {
		if (cfg->proxy_host.get()) {
			text_to_combo(ftp_proxy_host,cfg->proxy_host.get());
			GTK_TOGGLE_BUTTON(ftp_proxy_check)->active=TRUE;
			if (cfg->proxy_port) {
				char data[MAX_LEN];
				sprintf(data,"%i",cfg->proxy_port);
				gtk_entry_set_text(GTK_ENTRY(ftp_proxy_port),data);
			};
			if (cfg->proxy_user.get() && cfg->proxy_pass.get()) {
				GTK_TOGGLE_BUTTON(ftp_proxy_user_check)->active=TRUE;
				text_to_combo(ftp_proxy_user,cfg->proxy_user.get());
				text_to_combo(ftp_proxy_pass,cfg->proxy_pass.get());
			}else
				GTK_TOGGLE_BUTTON(ftp_proxy_user_check)->active=FALSE;
		} else
			GTK_TOGGLE_BUTTON(ftp_proxy_check)->active=FALSE;
		proxy_toggle_pass_ftp(ftp_proxy_user_check,this);
	} else {
		if (cfg->proxy_host.get()) {
			text_to_combo(http_proxy_host,cfg->proxy_host.get());
			GTK_TOGGLE_BUTTON(http_proxy_check)->active=TRUE;
			if (cfg->proxy_port) {
				char data[MAX_LEN];
				sprintf(data,"%i",cfg->proxy_port);
				gtk_entry_set_text(GTK_ENTRY(http_proxy_port),data);
			};
			if (cfg->proxy_user.get() && cfg->proxy_pass.get()) {
				GTK_TOGGLE_BUTTON(http_proxy_user_check)->active=TRUE;
				text_to_combo(http_proxy_user,cfg->proxy_user.get());
				text_to_combo(http_proxy_pass,cfg->proxy_pass.get());
			}else
				GTK_TOGGLE_BUTTON(http_proxy_user_check)->active=FALSE;
		} else{
			GTK_TOGGLE_BUTTON(http_proxy_check)->active=FALSE;
		};
		proxy_toggle_pass_http(http_proxy_user_check,this);
	};
	if (cfg->proxy_type) {
		GTK_TOGGLE_BUTTON(ftp_proxy_type_ftp)->active=FALSE;
		GTK_TOGGLE_BUTTON(ftp_proxy_type_http)->active=TRUE;
	} else {
		GTK_TOGGLE_BUTTON(ftp_proxy_type_ftp)->active=TRUE;
		GTK_TOGGLE_BUTTON(ftp_proxy_type_http)->active=FALSE;
	};
	GTK_TOGGLE_BUTTON(no_cache)->active=cfg->proxy_no_cache;
};


void tProxyWidget::apply_changes() {
	CFG.NEED_PASS_FTP_PROXY=GTK_TOGGLE_BUTTON(ftp_proxy_user_check)->active;
	CFG.NEED_PASS_HTTP_PROXY=GTK_TOGGLE_BUTTON(http_proxy_user_check)->active;
	CFG.USE_PROXY_FOR_FTP=GTK_TOGGLE_BUTTON(ftp_proxy_check)->active;
	CFG.USE_PROXY_FOR_HTTP=GTK_TOGGLE_BUTTON(http_proxy_check)->active;

	if (CFG.FTP_PROXY_HOST) delete CFG.FTP_PROXY_HOST;
	CFG.FTP_PROXY_HOST=copy_string(text_from_combo(ftp_proxy_host));
	sscanf(gtk_entry_get_text(GTK_ENTRY(ftp_proxy_port)),"%i",&CFG.FTP_PROXY_PORT);
	if (CFG.FTP_PROXY_USER) delete CFG.FTP_PROXY_USER;
	CFG.FTP_PROXY_USER=copy_string(text_from_combo(ftp_proxy_user));
	if (CFG.FTP_PROXY_PASS) delete CFG.FTP_PROXY_PASS;
	CFG.FTP_PROXY_PASS=copy_string(text_from_combo(ftp_proxy_pass));

	if (CFG.HTTP_PROXY_HOST) delete CFG.HTTP_PROXY_HOST;
	CFG.HTTP_PROXY_HOST=copy_string(text_from_combo(http_proxy_host));
	sscanf(gtk_entry_get_text(GTK_ENTRY(http_proxy_port)),"%i",&CFG.HTTP_PROXY_PORT);
	if (CFG.HTTP_PROXY_USER) delete CFG.HTTP_PROXY_USER;
	CFG.HTTP_PROXY_USER=copy_string(text_from_combo(http_proxy_user));
	if (CFG.HTTP_PROXY_PASS) delete CFG.HTTP_PROXY_PASS;
	CFG.HTTP_PROXY_PASS=copy_string(text_from_combo(http_proxy_pass));
	if (strlen(CFG.HTTP_PROXY_USER)) ALL_HISTORIES[USER_HISTORY]->add(CFG.HTTP_PROXY_USER);
	if (strlen(CFG.FTP_PROXY_USER)) ALL_HISTORIES[USER_HISTORY]->add(CFG.FTP_PROXY_USER);
	if (GTK_TOGGLE_BUTTON(ftp_proxy_type_ftp)->active) {
		CFG.FTP_PROXY_TYPE=0;
	} else
		CFG.FTP_PROXY_TYPE=1;
	if (strlen(CFG.FTP_PROXY_HOST)) ALL_HISTORIES[PROXY_HISTORY]->add(CFG.FTP_PROXY_HOST);
	if (strlen(CFG.HTTP_PROXY_HOST)) ALL_HISTORIES[PROXY_HISTORY]->add(CFG.HTTP_PROXY_HOST);
	if (strlen(text_from_combo(ftp_proxy_host)))
		ALL_HISTORIES[PROXY_HISTORY]->add(text_from_combo(ftp_proxy_host));
	if (strlen(text_from_combo(http_proxy_host)))
		ALL_HISTORIES[PROXY_HISTORY]->add(text_from_combo(http_proxy_host));
	if (CFG.REMEMBER_PASS){
		if (CFG.HTTP_PROXY_PASS && strlen(CFG.HTTP_PROXY_PASS))
			ALL_HISTORIES[PASS_HISTORY]->add(CFG.HTTP_PROXY_PASS);
		if (CFG.FTP_PROXY_PASS  && strlen(CFG.FTP_PROXY_PASS))
			ALL_HISTORIES[PASS_HISTORY]->add(CFG.FTP_PROXY_PASS);
	};
	CFG.PROXY_NO_CACHE=GTK_TOGGLE_BUTTON(no_cache)->active;
};

void tProxyWidget::apply_changes(tCfg *cfg,int proto) {
	cfg->reset_proxy();
	if (proto) {
		if (GTK_TOGGLE_BUTTON(ftp_proxy_check)->active) {
			cfg->proxy_host.set(text_from_combo(ftp_proxy_host));
			sscanf(gtk_entry_get_text(GTK_ENTRY(ftp_proxy_port)),"%i",&(cfg->proxy_port));
			if (GTK_TOGGLE_BUTTON(ftp_proxy_user_check)->active) {
				cfg->proxy_user.set(text_from_combo(ftp_proxy_user));
				cfg->proxy_pass.set(text_from_combo(ftp_proxy_pass));
			};
		};
	} else {
		if (GTK_TOGGLE_BUTTON(http_proxy_check)->active) {
			cfg->proxy_host.set(text_from_combo(http_proxy_host));
			sscanf(gtk_entry_get_text(GTK_ENTRY(http_proxy_port)),"%i",&(cfg->proxy_port));
			if (GTK_TOGGLE_BUTTON(http_proxy_user_check)->active) {
				cfg->proxy_user.set(text_from_combo(http_proxy_user));
				cfg->proxy_pass.set(text_from_combo(http_proxy_pass));
			};
		};
	};
	if (strlen(text_from_combo(ftp_proxy_user)))
		ALL_HISTORIES[USER_HISTORY]->add(text_from_combo(ftp_proxy_user));
	if (strlen(text_from_combo(http_proxy_user)))
		ALL_HISTORIES[USER_HISTORY]->add(text_from_combo(http_proxy_user));
	if (strlen(text_from_combo(ftp_proxy_host)))
		ALL_HISTORIES[PROXY_HISTORY]->add(text_from_combo(ftp_proxy_host));
	if (strlen(text_from_combo(http_proxy_host)))
		ALL_HISTORIES[PROXY_HISTORY]->add(text_from_combo(http_proxy_host));
	if (GTK_TOGGLE_BUTTON(ftp_proxy_type_ftp)->active)
		cfg->proxy_type=0;
	else
		cfg->proxy_type=1;
	cfg->proxy_no_cache=GTK_TOGGLE_BUTTON(no_cache)->active;
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
		gtk_window_set_title(GTK_WINDOW (select_options_window),_("Select properties"));
		gtk_container_border_width(GTK_CONTAINER(select_options_window),5);
		gtk_window_set_position(GTK_WINDOW(select_options_window),GTK_WIN_POS_CENTER);
		gtk_window_set_policy (GTK_WINDOW(select_options_window), FALSE,FALSE,FALSE);
		gtk_signal_connect(GTK_OBJECT(select_options_window),
				   "delete_event",
				   GTK_SIGNAL_FUNC(select_options_window_hide), NULL);

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
