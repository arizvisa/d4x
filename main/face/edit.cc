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
#include "../history.h"
#include "../var.h"
#include "../locstr.h"
#include "../main.h"
#include "../ntlocale.h"

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

static void edit_browser_open(GtkWidget *parent,tDEdit *where) {
	where->init_browser();
};

static void edit_browser_ok(GtkWidget *parent,tDEdit *where) {
	where->browser_ok();
};

static void edit_browser_cancel(GtkWidget *parent,tDEdit *where) {
	where->done_browser();
};

static int edit_browser_delete(GtkObject *parent,GdkEvent *event,tDEdit *where) {
	where->done_browser();
	return TRUE;
};

static void edit_browser_open2(GtkWidget *parent,tDEdit *where) {
	where->init_browser2();
};

static void edit_browser_ok2(GtkWidget *parent,tDEdit *where) {
	where->browser_ok2();
};

static void edit_browser_cancel2(GtkWidget *parent,tDEdit *where) {
	where->done_browser2();
};

static int edit_browser_delete2(GtkObject *parent,GdkEvent *event,tDEdit *where) {
	where->done_browser2();
	return TRUE;
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
	dir_browser=dir_browser2=NULL;
	proxy=NULL;
};

void tDEdit::popup() {
	if (window)
		gdk_window_show(window->window);
};

tDownload *tDEdit::get_parent(){
	return parent;
};

void tDEdit::set_path_as_default(){
	if (CFG.GLOBAL_SAVE_PATH)
		delete(CFG.GLOBAL_SAVE_PATH);
	CFG.GLOBAL_SAVE_PATH=copy_string(text_from_combo(path_entry));
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
	path_entry=my_gtk_combo_new(ALL_HISTORIES[PATH_HISTORY]);
	file_entry=my_gtk_combo_new(ALL_HISTORIES[FILE_HISTORY]);
	url_entry=my_gtk_combo_new(ALL_HISTORIES[URL_HISTORY]);
	user_agent_entry=my_gtk_combo_new(ALL_HISTORIES[USER_AGENT_HISTORY]);

//	char temp[MAX_LEN];
//	make_url_from_download(who,temp);
//	text_to_combo(url_entry,temp);
	char *URL=make_simply_url(who);
	text_to_combo(url_entry,URL);
	delete URL;

	text_to_combo(path_entry,who->get_SavePath());
	if (who->get_SaveName()) text_to_combo(file_entry,who->get_SaveName());
	else text_to_combo(file_entry,"");
	if (who->info->pass)
		text_to_combo(pass_entry,who->info->pass);
	else
		text_to_combo(pass_entry,"");
	if (who->info->username)
		text_to_combo(user_entry,who->info->username);
	if (who->config.get_user_agent())
		text_to_combo(user_agent_entry,who->config.get_user_agent());
	/* initing labels
	 */
	GtkWidget *url_label=gtk_label_new("URL:");
	GtkWidget *path_label=gtk_label_new(_("Save download to folder"));
	GtkWidget *file_label=gtk_label_new(_("Save download to file"));
	GtkWidget *pass_label=gtk_label_new(_("password"));
	GtkWidget *user_label=gtk_label_new(_("user name"));
	GtkWidget *user_agent_label=gtk_label_new(_("User-Agent"));
	/* set as default button
	 */
	GtkWidget *path_set_as_default=gtk_button_new_with_label(_("Default"));
	gtk_signal_connect(GTK_OBJECT(path_set_as_default),"clicked",GTK_SIGNAL_FUNC(edit_browser_path_set_as_default),this);
	/* initing boxes
	 */
	GtkWidget *url_box=gtk_hbox_new(FALSE,0);
	GtkWidget *path_box=gtk_hbox_new(FALSE,0);
	GtkWidget *path_vbox=gtk_vbox_new(FALSE,0);
	GtkWidget *file_box=gtk_hbox_new(FALSE,0);
	GtkWidget *file_vbox=gtk_vbox_new(FALSE,0);
	GtkWidget *user_agent_box=gtk_vbox_new(FALSE,0);
	GtkWidget *pass_box=gtk_hbox_new(FALSE,0);
	GtkWidget *user_box=gtk_hbox_new(FALSE,0);
	GtkWidget *dir_browser_button=gtk_button_new_with_label(_("Browse"));
	GtkWidget *dir_browser_button2=gtk_button_new_with_label(_("Browse"));
	gtk_signal_connect(GTK_OBJECT(dir_browser_button),"clicked",GTK_SIGNAL_FUNC(edit_browser_open),this);
	gtk_signal_connect(GTK_OBJECT(dir_browser_button2),"clicked",GTK_SIGNAL_FUNC(edit_browser_open2),this);
	gtk_box_set_spacing(GTK_BOX(url_box),5);
	gtk_box_set_spacing(GTK_BOX(path_box),5);
	gtk_box_set_spacing(GTK_BOX(path_vbox),2);
	gtk_box_set_spacing(GTK_BOX(file_box),5);
	gtk_box_set_spacing(GTK_BOX(file_vbox),2);
	gtk_box_set_spacing(GTK_BOX(user_box),5);
	gtk_box_set_spacing(GTK_BOX(pass_box),5);
	gtk_box_set_spacing(GTK_BOX(user_agent_box),5);
	gtk_box_pack_start(GTK_BOX(url_box),url_label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(url_box),url_entry,TRUE,TRUE,0);
	gtk_box_pack_start(GTK_BOX(path_vbox),path_label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(path_box),path_entry,TRUE,TRUE,0);
	gtk_box_pack_start(GTK_BOX(path_box),dir_browser_button,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(path_box),path_set_as_default,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(path_vbox),path_box,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(file_box),file_entry,TRUE,TRUE,0);
	gtk_box_pack_start(GTK_BOX(file_box),dir_browser_button2,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(file_vbox),file_label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(file_vbox),file_box,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(pass_box),pass_entry,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(pass_box),pass_label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(user_box),user_entry,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(user_box),user_label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(user_agent_box),user_agent_label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(user_agent_box),user_agent_entry,TRUE,TRUE,0);

	button=gtk_check_button_new_with_label(_("Use password for this site"));
	gtk_signal_connect(GTK_OBJECT(button),"clicked",GTK_SIGNAL_FUNC(edit_window_password),this);
	if (who->info->username)
		GTK_TOGGLE_BUTTON(button)->active=TRUE;
	else
		GTK_TOGGLE_BUTTON(button)->active=FALSE;

	GtkWidget *vbox=gtk_vbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(vbox),5);
	gtk_box_pack_start(GTK_BOX(vbox),url_box,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),path_vbox,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),file_vbox,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),user_agent_box,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),button,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),user_box,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),pass_box,FALSE,FALSE,0);	
	pause_check=gtk_check_button_new_with_label(_("Pause this just after adding"));
	GTK_TOGGLE_BUTTON(pause_check)->active=FALSE;
	gtk_box_pack_start(GTK_BOX(vbox),pause_check,FALSE,FALSE,0);
	GtkWidget *frame=gtk_frame_new(_("Download"));
	gtk_container_border_width(GTK_CONTAINER(frame),5);
	gtk_container_add(GTK_CONTAINER(frame),vbox);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook),frame,gtk_label_new(_("Main")));
};

void tDEdit::init_other(tDownload *who) {
	/* initing other
	 */
	GtkWidget *other_vbox=gtk_vbox_new(FALSE,0);
	timeout_entry=gtk_entry_new_with_max_length(3);
	sleep_entry=gtk_entry_new_with_max_length(3);
	attempts_entry=gtk_entry_new_with_max_length(3);
	ftp_recurse_depth_entry=gtk_entry_new_with_max_length(3);
	http_recurse_depth_entry=gtk_entry_new_with_max_length(3);
	rollback_entry=gtk_entry_new_with_max_length(5);
	speed_entry=gtk_entry_new_with_max_length(5);

	char temp[MAX_LEN];
	sprintf(temp,"%i",who->config.timeout);
	gtk_entry_set_text(GTK_ENTRY(timeout_entry),temp);
	sprintf(temp,"%i",who->config.time_for_sleep);
	gtk_entry_set_text(GTK_ENTRY(sleep_entry),temp);
	sprintf(temp,"%i",who->config.number_of_attempts);
	gtk_entry_set_text(GTK_ENTRY(attempts_entry),temp);
	sprintf(temp,"%i",who->config.ftp_recurse_depth);
	gtk_entry_set_text(GTK_ENTRY(ftp_recurse_depth_entry),temp);
	sprintf(temp,"%i",who->config.http_recurse_depth);
	gtk_entry_set_text(GTK_ENTRY(http_recurse_depth_entry),temp);
	sprintf(temp,"%i",who->config.rollback);
	gtk_entry_set_text(GTK_ENTRY(rollback_entry),temp);
	sprintf(temp,"%i",who->config.speed);
	gtk_entry_set_text(GTK_ENTRY(speed_entry),temp);

	gtk_widget_set_usize(timeout_entry,30,-1);
	gtk_widget_set_usize(attempts_entry,30,-1);
	gtk_widget_set_usize(sleep_entry,30,-1);
	gtk_widget_set_usize(ftp_recurse_depth_entry,30,-1);
	gtk_widget_set_usize(http_recurse_depth_entry,30,-1);
	gtk_widget_set_usize(rollback_entry,50,-1);
	gtk_widget_set_usize(speed_entry,50,-1);

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
	other_label=gtk_label_new(_("Depth of recursing (0 unlimited,1 no recurse)"));
	GtkWidget *temp_frame=gtk_frame_new("ftp");
	gtk_container_add(GTK_CONTAINER(temp_frame),ftp_recurse_depth_entry);
	gtk_box_pack_start(GTK_BOX(other_hbox),temp_frame,FALSE,FALSE,0);
	temp_frame=gtk_frame_new("http");
	gtk_container_add(GTK_CONTAINER(temp_frame),http_recurse_depth_entry);
	gtk_box_pack_start(GTK_BOX(other_hbox),temp_frame,FALSE,FALSE,0);

	//	gtk_box_pack_start(GTK_BOX(other_hbox),ftp_recurse_depth_entry,FALSE,FALSE,0);
	//	gtk_box_pack_start(GTK_BOX(other_hbox),http_recurse_depth_entry,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(other_hbox),other_label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(other_vbox),other_hbox,FALSE,FALSE,0);

	ftp_passive_check=gtk_check_button_new_with_label(_("Use passive mode for ftp"));
	GTK_TOGGLE_BUTTON(ftp_passive_check)->active=who->config.passive;
	gtk_box_pack_start(GTK_BOX(other_vbox),ftp_passive_check,FALSE,FALSE,0);
	permisions_check=gtk_check_button_new_with_label(_("Get permisions of the file from server (FTP only)"));
	GTK_TOGGLE_BUTTON(permisions_check)->active=who->config.permisions;
	gtk_box_pack_start(GTK_BOX(other_vbox),permisions_check,FALSE,FALSE,0);
	get_date_check=gtk_check_button_new_with_label(_("Get date from the server"));
	GTK_TOGGLE_BUTTON(get_date_check)->active=who->config.get_date;
	gtk_box_pack_start(GTK_BOX(other_vbox),get_date_check,FALSE,FALSE,0);
	retry_check=gtk_check_button_new_with_label(_("Retry if resuming is not supported"));
	GTK_TOGGLE_BUTTON(retry_check)->active=who->config.retry;
	gtk_box_pack_start(GTK_BOX(other_vbox),retry_check,FALSE,FALSE,0);
	GtkWidget *other_frame=gtk_frame_new(_("Other"));
	gtk_container_border_width(GTK_CONTAINER(other_frame),5);
	gtk_container_add(GTK_CONTAINER(other_frame),other_vbox);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook),other_frame,gtk_label_new(_("Other")));
};

void tDEdit::init_time(tDownload *who){
	/* Init time
	 */
	GtkWidget *time_frame=gtk_frame_new(_("Time"));
	GtkWidget *time_hbox=gtk_hbox_new(FALSE,0);
	GtkWidget *time_label,*time_vbox;
	gtk_container_border_width(GTK_CONTAINER(time_frame),5);

	year_entry=my_gtk_combo_new(1999,2010);
	time_label=gtk_label_new(_("Year"));
	time_vbox=gtk_vbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(time_vbox),time_label,TRUE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(time_vbox),year_entry,TRUE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(time_hbox),time_vbox,TRUE,FALSE,0);

	month_entry=my_gtk_combo_new_month();
	time_label=gtk_label_new(_("Month"));
	time_vbox=gtk_vbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(time_vbox),time_label,TRUE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(time_vbox),month_entry,TRUE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(time_hbox),time_vbox,TRUE,FALSE,0);

	day_entry=my_gtk_combo_new(1,31);
	time_label=gtk_label_new(_("Day"));
	time_vbox=gtk_vbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(time_vbox),time_label,TRUE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(time_vbox),day_entry,TRUE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(time_hbox),time_vbox,TRUE,FALSE,0);

	hour_entry=my_gtk_combo_new(0,23);
	time_label=gtk_label_new(_("Hours"));
	time_vbox=gtk_vbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(time_vbox),time_label,TRUE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(time_vbox),hour_entry,TRUE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(time_hbox),time_vbox,TRUE,FALSE,0);

	minute_entry=my_gtk_combo_new(0,59);
	time_label=gtk_label_new(_("Minutes"));
	time_vbox=gtk_vbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(time_vbox),time_label,TRUE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(time_vbox),minute_entry,TRUE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(time_hbox),time_vbox,TRUE,FALSE,0);

	gtk_widget_set_usize(year_entry,60,-1);
	gtk_widget_set_usize(month_entry,60,-1);
	gtk_widget_set_usize(day_entry,60,-1);
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

	/* init proxies
	 */
	proxy=new tProxyWidget;
	proxy->init();
	proxy->init_state();
	if (equal_uncase("ftp",who->info->protocol))
		proxy->init_state(&(who->config),1);
	else
		proxy->init_state(&(who->config),0);
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

void tDEdit::init_browser() {
	if (dir_browser) return;
	dir_browser=gtk_file_selection_new(_("Select directory"));
	gtk_widget_set_sensitive(GTK_FILE_SELECTION(dir_browser)->file_list,FALSE);
	char *tmp=text_from_combo(path_entry);
	if (tmp && *tmp)
		gtk_file_selection_set_filename(GTK_FILE_SELECTION(dir_browser),tmp);
	gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(dir_browser)->ok_button),
	                   "clicked",GTK_SIGNAL_FUNC(edit_browser_ok),this);
	gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(dir_browser)->cancel_button),
	                   "clicked",GTK_SIGNAL_FUNC(edit_browser_cancel),this);
	gtk_signal_connect(GTK_OBJECT(&(GTK_FILE_SELECTION(dir_browser)->window)),
	                   "delete_event",GTK_SIGNAL_FUNC(edit_browser_delete),this);
	gtk_widget_show(dir_browser);
};

void tDEdit::init_browser2() {
	if (dir_browser2) return;
	dir_browser2=gtk_file_selection_new(_("Select file"));
	char *tmp=sum_strings(text_from_combo(path_entry),"/",NULL);
	if (tmp && *tmp)
		gtk_file_selection_set_filename(GTK_FILE_SELECTION(dir_browser2),tmp);
	delete tmp;
	gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(dir_browser2)->ok_button),
	                   "clicked",GTK_SIGNAL_FUNC(edit_browser_ok2),this);
	gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(dir_browser2)->cancel_button),
	                   "clicked",GTK_SIGNAL_FUNC(edit_browser_cancel2),this);
	gtk_signal_connect(GTK_OBJECT(&(GTK_FILE_SELECTION(dir_browser2)->window)),
	                   "delete_event",GTK_SIGNAL_FUNC(edit_browser_delete2),this);
	gtk_widget_show(dir_browser2);
};


void tDEdit::browser_ok() {
	if (!dir_browser) return;
	char *tmp;
	tmp=gtk_file_selection_get_filename(GTK_FILE_SELECTION(dir_browser));
	gtk_entry_set_text(GTK_ENTRY(GTK_ENTRY(GTK_COMBO(path_entry)->entry)),tmp);
	gtk_widget_destroy(dir_browser);
	dir_browser=NULL;
};

void tDEdit::browser_ok2() {
	if (!dir_browser2) return;
	char *tmp=gtk_entry_get_text(GTK_ENTRY(((GtkFileSelection *)dir_browser2)->selection_entry));
	gtk_entry_set_text(GTK_ENTRY(GTK_ENTRY(GTK_COMBO(file_entry)->entry)),tmp);
	gtk_widget_destroy(dir_browser2);
	dir_browser2=NULL;
};

void tDEdit::done_browser() {
	if (!dir_browser) return;
	gtk_widget_destroy(dir_browser);
	dir_browser=NULL;
};

void tDEdit::done_browser2() {
	if (!dir_browser2) return;
	gtk_widget_destroy(dir_browser2);
	dir_browser2=NULL;
};

int tDEdit::apply_changes() {
	char *temp=copy_string(text_from_combo(url_entry));
	del_crlf(temp);
	tAddr *addr=make_addr_from_url(temp);
	if (!addr) return 1;
	delete (parent->info);
	parent->info=addr;
	if (equal(parent->info->protocol,"ftp"))
		proxy->apply_changes(&(parent->config),1);
	else
		proxy->apply_changes(&(parent->config),0);
	char *pass_string=text_from_combo(pass_entry);
	if (GTK_TOGGLE_BUTTON(button)->active) {
		if (strlen(text_from_combo(user_entry)) && strlen(pass_string)) {
			if (parent->info->pass) delete(parent->info->pass);
			if (parent->info->username) delete(parent->info->username);
			parent->info->username=copy_string(text_from_combo(user_entry));
			ALL_HISTORIES[USER_HISTORY]->add(parent->info->username);
			parent->info->pass=copy_string(pass_string);
			if (CFG.REMEMBER_PASS)
				ALL_HISTORIES[PASS_HISTORY]->add(pass_string);
		};
	};
	parent->set_SavePath(text_from_combo(path_entry));
	parent->set_SaveName(NULL);
	if (strlen(text_from_combo(file_entry)))
		parent->set_SaveName(text_from_combo(file_entry));
	if (parent->get_SaveName())
		ALL_HISTORIES[FILE_HISTORY]->add(parent->get_SaveName());
	normalize_path(parent->get_SavePath());
	parent->status=0;
	if (strlen(parent->info->file)==0) {
		parent->finfo.type=T_DIR;
		parent->finfo.size=0;
	} else {
		parent->finfo.type=0;
		parent->finfo.size=-1;
	};
	/* change histories
	 */
	char *URL=make_simply_url(parent);
	parent->config.set_user_agent(text_from_combo(user_agent_entry));
	ALL_HISTORIES[PATH_HISTORY]->add(text_from_combo(path_entry));
	ALL_HISTORIES[URL_HISTORY]->add(URL);
	ALL_HISTORIES[USER_AGENT_HISTORY]->add(text_from_combo(user_agent_entry));
	/*change data in list if available
	 */
	if (parent->GTKCListRow > 0) {
		list_of_downloads_change_data(parent->GTKCListRow,URL_COL,URL);
		list_of_downloads_change_data(parent->GTKCListRow,FILE_COL,parent->info->file);
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
	parent->config.permisions=GTK_TOGGLE_BUTTON(permisions_check)->active;
	parent->config.get_date=GTK_TOGGLE_BUTTON(get_date_check)->active;
	parent->config.retry=GTK_TOGGLE_BUTTON(retry_check)->active;
	parent->config.http_recursing=parent->config.http_recurse_depth==1?0:1;

	if (GTK_TOGGLE_BUTTON(time_check)->active) {
		time_t NOW=time(NULL);
		struct tm date;
		date.tm_isdst=-1;
		localtime_r(&NOW,&date);
		sscanf(text_from_combo(year_entry),"%i",&date.tm_year);
		sscanf(text_from_combo(day_entry),"%i",&date.tm_mday);
		sscanf(text_from_combo(hour_entry),"%i",&date.tm_hour);
		sscanf(text_from_combo(minute_entry),"%i",&date.tm_min);
		date.tm_mon=convert_month(text_from_combo(month_entry));
		date.tm_year-=1900;
		date.tm_sec=0;
		parent->ScheduleTime=mktime(&date);
	} else {
		parent->ScheduleTime=0;
	};
	return 0;
};

void tDEdit::toggle_time() {
	gtk_widget_set_sensitive(year_entry,GTK_TOGGLE_BUTTON(time_check)->active);
	gtk_widget_set_sensitive(hour_entry,GTK_TOGGLE_BUTTON(time_check)->active);
	gtk_widget_set_sensitive(minute_entry,GTK_TOGGLE_BUTTON(time_check)->active);
	gtk_widget_set_sensitive(day_entry,GTK_TOGGLE_BUTTON(time_check)->active);
	gtk_widget_set_sensitive(month_entry,GTK_TOGGLE_BUTTON(time_check)->active);
};


void tDEdit::setup_entries() {
	set_editable_for_combo(pass_entry,GTK_TOGGLE_BUTTON(button)->active);
	gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(user_entry)->entry),GTK_TOGGLE_BUTTON(button)->active);
	gtk_widget_set_sensitive(user_entry,GTK_TOGGLE_BUTTON(button)->active);
	gtk_widget_set_sensitive(pass_entry,GTK_TOGGLE_BUTTON(button)->active);
};

void tDEdit::setup_time(time_t when) {
	char data[MAX_LEN];
	if (when) {
		ctime_r(&when,data);
	} else {
		time_t NOW=time(NULL);
		ctime_r(&NOW,data);
	};
	int year=1999,day=1,hour=0,min=0,temp;
	char mon[MAX_LEN];
	sscanf(index(data,' ')+1,"%s %i %i:%i:%i %i",mon,&day,&hour,&min,&temp,&year);
	sprintf(data,"%i",year);
	text_to_combo(year_entry,data);
	text_to_combo(month_entry,mon);
	sprintf(data,"%i",day);
	text_to_combo(day_entry,data);
	sprintf(data,"%i",hour);
	text_to_combo(hour_entry,data);
	sprintf(data,"%i",min);
	text_to_combo(minute_entry,data);
	GTK_TOGGLE_BUTTON(time_check)->active=when?TRUE:FALSE;
	toggle_time();
};


void tDEdit::paste_url() {
	gtk_editable_paste_clipboard(GTK_EDITABLE(GTK_COMBO(url_entry)->entry));
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


void tDEdit::done() {
	if (parent) parent->editor=NULL;
	gtk_widget_destroy(window);
	done_browser();
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
	GtkWidget *proxy_frame3=gtk_frame_new(_("ftp proxy type"));
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

	ftp_proxy_check=gtk_check_button_new_with_label(_("Use this proxy for ftp"));

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


	vbox=gtk_vbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(vbox),2);
	gtk_container_add(GTK_CONTAINER(proxy_frame2),vbox);

	http_proxy_check=gtk_check_button_new_with_label(_("Use this proxy for http"));

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
};

void tProxyWidget::init_state(tCfg *cfg,int proto) {
	if (proto) {
		if (cfg->get_proxy_host()) {
			text_to_combo(ftp_proxy_host,cfg->get_proxy_host());
			GTK_TOGGLE_BUTTON(ftp_proxy_check)->active=TRUE;
			if (cfg->proxy_port) {
				char data[MAX_LEN];
				sprintf(data,"%i",cfg->proxy_port);
				gtk_entry_set_text(GTK_ENTRY(ftp_proxy_port),data);
			};
			if (cfg->get_proxy_user() && cfg->get_proxy_pass()) {
				GTK_TOGGLE_BUTTON(ftp_proxy_user_check)->active=TRUE;
				text_to_combo(ftp_proxy_user,cfg->get_proxy_user());
				text_to_combo(ftp_proxy_pass,cfg->get_proxy_pass());
			}else
				GTK_TOGGLE_BUTTON(ftp_proxy_user_check)->active=FALSE;
		} else
			GTK_TOGGLE_BUTTON(ftp_proxy_check)->active=FALSE;
		proxy_toggle_pass_ftp(ftp_proxy_user_check,this);
	} else {
		if (cfg->get_proxy_host()) {
			text_to_combo(http_proxy_host,cfg->get_proxy_host());
			GTK_TOGGLE_BUTTON(http_proxy_check)->active=TRUE;
			if (cfg->proxy_port) {
				char data[MAX_LEN];
				sprintf(data,"%i",cfg->proxy_port);
				gtk_entry_set_text(GTK_ENTRY(http_proxy_port),data);
			};
			if (cfg->get_proxy_user() && cfg->get_proxy_pass()) {
				GTK_TOGGLE_BUTTON(http_proxy_user_check)->active=TRUE;
				text_to_combo(http_proxy_user,cfg->get_proxy_user());
				text_to_combo(http_proxy_pass,cfg->get_proxy_pass());
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
};

void tProxyWidget::apply_changes(tCfg *cfg,int proto) {
	cfg->reset_proxy();
	if (proto) {
		if (GTK_TOGGLE_BUTTON(ftp_proxy_check)->active) {
			cfg->set_proxy_host(text_from_combo(ftp_proxy_host));
			sscanf(gtk_entry_get_text(GTK_ENTRY(ftp_proxy_port)),"%i",&(cfg->proxy_port));
			if (GTK_TOGGLE_BUTTON(ftp_proxy_user_check)->active) {
				cfg->set_proxy_user(text_from_combo(ftp_proxy_user));
				cfg->set_proxy_pass(text_from_combo(ftp_proxy_pass));
			};
		};
	} else {
		if (GTK_TOGGLE_BUTTON(http_proxy_check)->active) {
			cfg->set_proxy_host(text_from_combo(http_proxy_host));
			sscanf(gtk_entry_get_text(GTK_ENTRY(http_proxy_port)),"%i",&(cfg->proxy_port));
			if (GTK_TOGGLE_BUTTON(http_proxy_user_check)->active) {
				cfg->set_proxy_user(text_from_combo(http_proxy_user));
				cfg->set_proxy_pass(text_from_combo(http_proxy_pass));
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
};
