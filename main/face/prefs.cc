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
#include <gdk/gdkkeysyms.h>
#include "../ntlocale.h"
#include "../var.h"
#include "../locstr.h"
#include "../main.h"
#include "misc.h"
#include "prefs.h"
#include "mywidget.h"
#include "columns.h"
#include "buttons.h"
#include "edit.h"
#include "graph.h"
#include "../config.h"

extern tMain aa;
GtkWidget *d4x_prefs_window=(GtkWidget *)NULL;
GtkWidget *d4x_prefs_frame=(GtkWidget *)NULL;
/* initialisation only for NULL in 'char*' */
tMainCfg TMPCFG={
	{300,5,100,0,1,0,0,0,
	 0,0,0,0,1,1,1,0,0,0,0,0,
	 0},
	100,1,(char*)NULL,(char*)NULL,(char*)NULL,(char*)NULL,0,0,
	100,0,0,0,(char*)NULL,0,0, //Log
	5,0, //List
	1,0,0,600,0,0, //flags
	{0,0},0,1,0,0,40,40,500,400,300,300,1,150,50,0,1,0,20,30,0,5,1,//interface
	0,1,(char*)NULL,(char*)NULL, //clipboard
	0xFFFFFF,0x555555,0xAAAAAA,0,
	/* Proxy */
	(char*)NULL,0,(char*)NULL,(char*)NULL,1,(char*)NULL,0,(char*)NULL,(char*)NULL,0,0,0,0,0,
	1,1,1,1,1,1,
	3,1024,10*1024,
	(char*)NULL,0,
	1,1,1,1,
	0,1
};

struct D4xPrefsWidget{
	/* TREE ITEMS */
	GtkWidget *root_tree;
	GtkWidget *tree_download;
	GtkWidget *tree_interface;
	GtkWidget *tree_main;
	GtkWidget *tree_integration;
	/* DOWNLOAD */
	GtkWidget *savepath;
	GtkWidget *sleep_check;
	GtkWidget *get_date_check;
	GtkWidget *retry_check;
	GtkWidget *recursive;
	GtkWidget *pause_check;
	GtkWidget *check_time_check;
	/* FTP */
	GtkWidget *ftp_passive_check;
	GtkWidget *dont_send_quit_check;
	GtkWidget *permisions_check;
	GtkWidget *link_as_file_check;
	GtkWidget *ftp_dir_in_log;
	GtkWidget *ftp_recurse_depth_entry;
	/* LIMITS */
	GtkWidget *limits_log;
	GtkWidget *timeout_entry;
	GtkWidget *sleep_entry;
	GtkWidget *attempts_entry;
	GtkWidget *rollback_entry;
	GtkWidget *speed_entry;
	GtkWidget *split_entry;
	/* HTTP */
	GtkWidget *leave_dir_check;
	GtkWidget *leave_server_check;
	GtkWidget *http_recurse_depth_entry;
	GtkWidget *user_agent_entry;
	GtkWidget *unknown_filename;
	/* PROXY */
	tProxyWidget proxy;
	/* Main window */
	GtkWidget *mw_use_title;
	GtkWidget *mw_use_title2;
	GtkWidget *mw_scroll_title;
	GtkWidget *window_lower;
	/* CONFIRM */
	GtkWidget *confirm_delete;
	GtkWidget *confirm_delete_all;
	GtkWidget *confirm_delete_fataled;
	GtkWidget *confirm_delete_completed;
	GtkWidget *confirm_exit;
	GtkWidget *confirm_opening_many;
	/* CLIPBOARD */
	GtkWidget *clipboard_monitor;
	GtkWidget *clipboard_skip;
	GtkWidget *clipboard_skip_button;
	GtkWidget *clipboard_catch_button;
	GtkWidget *clipboard_catch;
	/* MAINLOG */
	GtkWidget *log_rewrite;
	GtkWidget *log_append;
	GtkWidget *log_save;
	GtkWidget *log_save_path;
	GtkWidget *log_detailed;
	GtkWidget *log_length;
	GtkWidget *log_fsize;
	GtkWidget *log_fslabel;
	/* INTEGRATION */
	GtkWidget *exit_complete;
	GtkWidget *exit_complete_time;
	GtkWidget *exec_on_exit;
	GtkWidget *dnd_dialog;
	/* COLUMNS */
	GtkWidget *columns_nums1;
	GtkWidget *columns_nums2;
	GtkWidget *columns_nums3;
	GtkWidget *columns_nums4;
	GtkWidget *columns_time1;
	GtkWidget *columns_time2;
	tColumnsPrefs columns_order;
	/* MAIN */
	GtkWidget *del_completed;
	GtkWidget *del_fataled;
	GtkWidget *allow_force_run;
	GtkWidget *remember_pass;
	GtkWidget *description;
	GtkWidget *save_list_entry;
	GtkWidget *save_list_check;
	GtkWidget *max_threads;
	GtkWidget *speed_limit_1;
	GtkWidget *speed_limit_2;
	/* INTERFACE */
	GtkWidget *dnd_trash;
	GtkWidget *fixed_font_log;
	/* GRAPH */
	GtkWidget *graph_order;
	GtkWidget *speed_color_pick;
	GtkWidget *speed_color_fore1;
	GtkWidget *speed_color_fore2;
	GtkWidget *speed_color_back;
//	GtkWidget *;
};

static D4xPrefsWidget D4XPWS;
void d4x_prefs_apply_tmp();
void d4x_prefs_apply();
void d4x_prefs_ok();

void toggle_button_set_state(GtkToggleButton *tb,gboolean state) {
#if (GTK_MAJOR_VERSION==1) && (GTK_MINOR_VERSION==1) && (GTK_MICRO_VERSION<=12)
	gtk_toggle_button_set_state(tb,state);
#else
	gtk_toggle_button_set_active(tb,state);
#endif
}

gint d4x_prefs_cancel() {
	if (d4x_prefs_window){
		gtk_widget_destroy(d4x_prefs_window);
		d4x_prefs_window=(GtkWidget *)NULL;
		D4XPWS.columns_order.reset();
	};
	return TRUE;
};

static gint d4x_prefs_esc_handler(GtkWidget *window,GdkEvent *event){
	if (event && event->type == GDK_KEY_PRESS) {
		GdkEventKey *kevent=(GdkEventKey *)event;
		switch(kevent->keyval) {
		case GDK_Escape:{
			d4x_prefs_cancel();
			return TRUE;
			break;
		};
		};
	};
	return FALSE;
};

GtkWidget *d4x_prefs_child_destroy(char *title){
	GtkWidget *child=GTK_BIN(d4x_prefs_frame)->child;
	if (child){
		d4x_prefs_apply_tmp();
		gtk_widget_destroy(child);
//		gtk_container_remove(GTK_CONTAINER(d4x_prefs_frame),child);
//		gtk_widget_destroy(child);
		
	};
	gtk_frame_set_label(GTK_FRAME(d4x_prefs_frame),title);
	GtkWidget *tmpbox=gtk_vbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(tmpbox),5);
	gtk_container_add(GTK_CONTAINER(d4x_prefs_frame),tmpbox);
	return(tmpbox);
};

void d4x_prefs_download(){
	GtkWidget *tmpbox=d4x_prefs_child_destroy(_("Download"));
	
	D4XPWS.get_date_check=gtk_check_button_new_with_label(_("Get date from the server"));
	GTK_TOGGLE_BUTTON(D4XPWS.get_date_check)->active=TMPCFG.DEFAULT_CFG.get_date;
	gtk_box_pack_start(GTK_BOX(tmpbox),D4XPWS.get_date_check,FALSE,FALSE,0);

	D4XPWS.retry_check=gtk_check_button_new_with_label(_("Retry if resuming is not supported"));
	GTK_TOGGLE_BUTTON(D4XPWS.retry_check)->active=TMPCFG.DEFAULT_CFG.retry;
	gtk_box_pack_start(GTK_BOX(tmpbox),D4XPWS.retry_check,FALSE,FALSE,0);

	D4XPWS.sleep_check=gtk_check_button_new_with_label(_("Sleep before completing"));
	GTK_TOGGLE_BUTTON(D4XPWS.sleep_check)->active=TMPCFG.DEFAULT_CFG.sleep_before_complete;
	gtk_box_pack_start(GTK_BOX(tmpbox),D4XPWS.sleep_check,FALSE,FALSE,0);

	D4XPWS.recursive=gtk_check_button_new_with_label(_("Optimize recursive downloads"));
	gtk_box_pack_start(GTK_BOX(tmpbox),D4XPWS.recursive,FALSE,FALSE,0);
	GTK_TOGGLE_BUTTON(D4XPWS.recursive)->active=TMPCFG.RECURSIVE_OPTIMIZE;

	D4XPWS.pause_check=gtk_check_button_new_with_label(_("Pause this just after adding"));
	GTK_TOGGLE_BUTTON(D4XPWS.pause_check)->active=TMPCFG.PAUSE_AFTER_ADDING;
	gtk_box_pack_start(GTK_BOX(tmpbox),D4XPWS.pause_check,FALSE,FALSE,0);

	D4XPWS.check_time_check=gtk_check_button_new_with_label(_("Compare date/time of remote file with local one"));
	GTK_TOGGLE_BUTTON(D4XPWS.check_time_check)->active=TMPCFG.DEFAULT_CFG.check_time;
	gtk_box_pack_start(GTK_BOX(tmpbox),D4XPWS.check_time_check,FALSE,FALSE,0);

	GtkWidget *prefs_other_sbox=gtk_vbox_new(FALSE,0);
	D4XPWS.savepath=my_gtk_filesel_new(ALL_HISTORIES[PATH_HISTORY]);
	MY_GTK_FILESEL(D4XPWS.savepath)->modal=GTK_WINDOW(d4x_prefs_window);
	MY_GTK_FILESEL(D4XPWS.savepath)->only_dirs=TRUE;
	if (TMPCFG.GLOBAL_SAVE_PATH)
		text_to_combo(MY_GTK_FILESEL(D4XPWS.savepath)->combo,TMPCFG.GLOBAL_SAVE_PATH);
	GtkWidget *prefs_other_slabel=gtk_label_new(_("Save downloads to folder"));
	gtk_box_pack_start(GTK_BOX(prefs_other_sbox),prefs_other_slabel,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(prefs_other_sbox),D4XPWS.savepath,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(tmpbox),prefs_other_sbox,FALSE,FALSE,0);

	gtk_widget_show_all(tmpbox);
};

void d4x_prefs_download_limits(){
	GtkWidget *tmpbox=d4x_prefs_child_destroy(_("Limits"));

	D4XPWS.timeout_entry=my_gtk_entry_new_with_max_length(3,TMPCFG.DEFAULT_CFG.timeout);
	D4XPWS.sleep_entry=my_gtk_entry_new_with_max_length(3,TMPCFG.DEFAULT_CFG.time_for_sleep);
	D4XPWS.attempts_entry=my_gtk_entry_new_with_max_length(3,TMPCFG.DEFAULT_CFG.number_of_attempts);
	D4XPWS.rollback_entry=my_gtk_entry_new_with_max_length(5,TMPCFG.DEFAULT_CFG.rollback);
	D4XPWS.speed_entry=my_gtk_entry_new_with_max_length(5,TMPCFG.DEFAULT_CFG.speed);
//	D4XPWS.split_entry=my_gtk_entry_new_with_max_length(2,who->split==NULL?0:who->split->NumOfParts);

	GtkWidget *other_hbox=gtk_hbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(other_hbox),5);
	GtkWidget *other_label=gtk_label_new(_("Timeout for reading from socket (in seconds)"));
	gtk_box_pack_start(GTK_BOX(other_hbox),D4XPWS.timeout_entry,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(other_hbox),other_label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(tmpbox),other_hbox,FALSE,FALSE,0);

	other_hbox=gtk_hbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(other_hbox),5);
	other_label=gtk_label_new(_("Timeout before reconnection (in seconds)"));
	gtk_box_pack_start(GTK_BOX(other_hbox),D4XPWS.sleep_entry,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(other_hbox),other_label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(tmpbox),other_hbox,FALSE,FALSE,0);

	other_hbox=gtk_hbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(other_hbox),5);
	other_label=gtk_label_new(_("Maximum attempts (0 for unlimited)"));
	gtk_box_pack_start(GTK_BOX(other_hbox),D4XPWS.attempts_entry,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(other_hbox),other_label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(tmpbox),other_hbox,FALSE,FALSE,0);

	other_hbox=gtk_hbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(other_hbox),5);
	other_label=gtk_label_new(_("Rollback after reconnecting (in bytes)"));
	gtk_box_pack_start(GTK_BOX(other_hbox),D4XPWS.rollback_entry,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(other_hbox),other_label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(tmpbox),other_hbox,FALSE,FALSE,0);

	other_hbox=gtk_hbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(other_hbox),5);
	other_label=gtk_label_new(_("Speed limitation in Bytes/sec (0 for unlimited)"));
	gtk_box_pack_start(GTK_BOX(other_hbox),D4XPWS.speed_entry,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(other_hbox),other_label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(tmpbox),other_hbox,FALSE,FALSE,0);
/*
	other_hbox=gtk_hbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(other_hbox),5);
	other_label=gtk_label_new(_("Number of parts for spliting this download"));
	gtk_box_pack_start(GTK_BOX(other_hbox),D4XPWS.split_entry,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(other_hbox),other_label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(tmpbox),other_hbox,FALSE,FALSE,0);
*/

	GtkWidget *prefs_limits_lbox=gtk_hbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(prefs_limits_lbox),5);
	D4XPWS.limits_log=my_gtk_entry_new_with_max_length(3,TMPCFG.MAX_LOG_LENGTH);
	gtk_box_pack_start(GTK_BOX(prefs_limits_lbox),D4XPWS.limits_log,FALSE,FALSE,0);
	GtkWidget *prefs_limits_llabel=gtk_label_new(_("Maximum lines in log"));
	gtk_box_pack_start(GTK_BOX(prefs_limits_lbox),prefs_limits_llabel,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(tmpbox),prefs_limits_lbox,FALSE,FALSE,0);
	
	gtk_widget_show_all(tmpbox);
};

void d4x_prefs_download_ftp(){
	GtkWidget *tmpbox=d4x_prefs_child_destroy(_("FTP"));

	D4XPWS.ftp_passive_check=gtk_check_button_new_with_label(_("Use passive mode for FTP"));
	GTK_TOGGLE_BUTTON(D4XPWS.ftp_passive_check)->active=TMPCFG.DEFAULT_CFG.passive;
	gtk_box_pack_start(GTK_BOX(tmpbox),D4XPWS.ftp_passive_check,FALSE,FALSE,0);

	D4XPWS.dont_send_quit_check=gtk_check_button_new_with_label(_("Don't send QUIT command (FTP)"));
	GTK_TOGGLE_BUTTON(D4XPWS.dont_send_quit_check)->active=TMPCFG.DEFAULT_CFG.dont_send_quit;
	gtk_box_pack_start(GTK_BOX(tmpbox),D4XPWS.dont_send_quit_check,FALSE,FALSE,0);	

	D4XPWS.permisions_check=gtk_check_button_new_with_label(_("Get permissions of the file from server (FTP only)"));
	GTK_TOGGLE_BUTTON(D4XPWS.permisions_check)->active=TMPCFG.DEFAULT_CFG.permisions;
	gtk_box_pack_start(GTK_BOX(tmpbox),D4XPWS.permisions_check,FALSE,FALSE,0);

	D4XPWS.link_as_file_check=gtk_check_button_new_with_label(_("Try to load symbolic link as file via FTP"));
	GTK_TOGGLE_BUTTON(D4XPWS.link_as_file_check)->active=TMPCFG.DEFAULT_CFG.link_as_file;
	gtk_box_pack_start(GTK_BOX(tmpbox),D4XPWS.link_as_file_check,FALSE,FALSE,0);
	
	D4XPWS.ftp_recurse_depth_entry=my_gtk_entry_new_with_max_length(3,TMPCFG.DEFAULT_CFG.ftp_recurse_depth);
	GtkWidget *ftp_hbox=gtk_hbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(ftp_hbox),2);
	GtkWidget *other_label=gtk_label_new(_("Depth of recursing (0 unlimited,1 no recurse)"));
	gtk_box_pack_start(GTK_BOX(ftp_hbox),D4XPWS.ftp_recurse_depth_entry,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(ftp_hbox),other_label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(tmpbox),ftp_hbox,FALSE,FALSE,0);

	D4XPWS.ftp_dir_in_log=gtk_check_button_new_with_label(_("Output FTP dirs in logs"));
	GTK_TOGGLE_BUTTON(D4XPWS.ftp_dir_in_log)->active=TMPCFG.FTP_DIR_IN_LOG;
	gtk_box_pack_start(GTK_BOX(tmpbox),D4XPWS.ftp_dir_in_log,FALSE,FALSE,0);

	gtk_widget_show_all(tmpbox);
};

void d4x_prefs_download_http(){
	GtkWidget *tmpbox=d4x_prefs_child_destroy(_("HTTP"));

	D4XPWS.leave_dir_check=gtk_check_button_new_with_label(_("Only subdirs"));
	D4XPWS.leave_server_check=gtk_check_button_new_with_label(_("Allow leave this server while recursing via HTTP"));
	GTK_TOGGLE_BUTTON(D4XPWS.leave_server_check)->active=TMPCFG.DEFAULT_CFG.leave_server;
	GTK_TOGGLE_BUTTON(D4XPWS.leave_dir_check)->active=TMPCFG.DEFAULT_CFG.dont_leave_dir;
	gtk_box_pack_start(GTK_BOX(tmpbox),D4XPWS.leave_server_check,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(tmpbox),D4XPWS.leave_dir_check,FALSE,FALSE,0);

	D4XPWS.http_recurse_depth_entry=my_gtk_entry_new_with_max_length(3,TMPCFG.DEFAULT_CFG.http_recurse_depth);
	GtkWidget *http_hbox=gtk_hbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(http_hbox),5);
	GtkWidget *other_label=gtk_label_new(_("Depth of recursing (0 unlimited,1 no recurse)"));
	gtk_box_pack_start(GTK_BOX(http_hbox),D4XPWS.http_recurse_depth_entry,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(http_hbox),other_label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(tmpbox),http_hbox,FALSE,FALSE,0);

	GtkWidget *user_agent_label=gtk_label_new(_("User-Agent"));
	GtkWidget *user_agent_box=gtk_vbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(user_agent_box),5);
	D4XPWS.user_agent_entry=my_gtk_combo_new(ALL_HISTORIES[USER_AGENT_HISTORY]);
	if (TMPCFG.USER_AGENT)
		text_to_combo(D4XPWS.user_agent_entry,TMPCFG.USER_AGENT);
	gtk_box_pack_start(GTK_BOX(user_agent_box),user_agent_label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(user_agent_box),D4XPWS.user_agent_entry,TRUE,TRUE,0);
	gtk_box_pack_start(GTK_BOX(tmpbox),user_agent_box,FALSE,FALSE,0);

	GtkWidget *prefs_other_fbox=gtk_vbox_new(FALSE,0);
	D4XPWS.unknown_filename=my_gtk_filesel_new(ALL_HISTORIES[FILE_HISTORY]);
	MY_GTK_FILESEL(D4XPWS.unknown_filename)->modal=GTK_WINDOW(d4x_prefs_window);
	text_to_combo(MY_GTK_FILESEL(D4XPWS.unknown_filename)->combo,CFG.DEFAULT_NAME);
	GtkWidget *prefs_other_flabel=gtk_label_new(_("Filename for saving if it is unknown"));
	gtk_box_pack_start(GTK_BOX(prefs_other_fbox),prefs_other_flabel,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(prefs_other_fbox),D4XPWS.unknown_filename,TRUE,TRUE,0);
	gtk_box_pack_start(GTK_BOX(tmpbox),prefs_other_fbox,FALSE,FALSE,0);
	
	gtk_widget_show_all(tmpbox);
};

void d4x_prefs_proxy(){
	GtkWidget *tmpbox=d4x_prefs_child_destroy(_("Proxy"));

	D4XPWS.proxy.init();
	D4XPWS.proxy.init_state(&TMPCFG);
	gtk_box_pack_start(GTK_BOX(tmpbox),D4XPWS.proxy.frame,FALSE,FALSE,0);

	gtk_widget_show_all(tmpbox);
};

static void d4x_prefs_toggle_title(GtkWidget *parent) {
	gtk_widget_set_sensitive(D4XPWS.mw_use_title2,GTK_TOGGLE_BUTTON(parent)->active);
	gtk_widget_set_sensitive(D4XPWS.mw_scroll_title,GTK_TOGGLE_BUTTON(parent)->active);
};

void d4x_prefs_mwin(){
	GtkWidget *tmpbox=d4x_prefs_child_destroy(_("Main window"));

	GtkWidget *frame=gtk_frame_new(_("Using title"));
	GtkWidget *vbox=gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(frame),vbox);
	gtk_container_border_width(GTK_CONTAINER(frame),5);
	D4XPWS.mw_use_title=gtk_check_button_new_with_label(_("Use title of main window for info"));
	gtk_signal_connect(GTK_OBJECT(D4XPWS.mw_use_title),
			   "clicked",
			   GTK_SIGNAL_FUNC(d4x_prefs_toggle_title),
			   NULL);
	D4XPWS.mw_use_title2=gtk_check_button_new_with_label(_("Display queue statistics too"));
	D4XPWS.mw_scroll_title=gtk_check_button_new_with_label(_("Scroll title"));
	gtk_box_pack_start(GTK_BOX(vbox),D4XPWS.mw_use_title,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),D4XPWS.mw_use_title2,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),D4XPWS.mw_scroll_title,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(tmpbox),frame,FALSE,FALSE,0);
	D4XPWS.window_lower=gtk_check_button_new_with_label(_("Iconfiy main window instead of closing"));
	gtk_box_pack_start(GTK_BOX(tmpbox),D4XPWS.window_lower,FALSE,FALSE,0);

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(D4XPWS.mw_use_title),TMPCFG.USE_MAINWIN_TITLE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(D4XPWS.mw_use_title2),TMPCFG.USE_MAINWIN_TITLE2);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(D4XPWS.mw_scroll_title),TMPCFG.SCROLL_MAINWIN_TITLE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(D4XPWS.window_lower),TMPCFG.WINDOW_LOWER);
	d4x_prefs_toggle_title(D4XPWS.mw_use_title);
	gtk_widget_show_all(tmpbox);
};

void d4x_prefs_confirm(){
	GtkWidget *vbox=d4x_prefs_child_destroy(_("Confirmation"));

	D4XPWS.confirm_delete=gtk_check_button_new_with_label(_("Confirm delete selected downloads"));
	D4XPWS.confirm_delete_all=gtk_check_button_new_with_label(_("Confirm delete all downloads"));
	D4XPWS.confirm_delete_completed=gtk_check_button_new_with_label(_("Confirm delete completed downloads"));
	D4XPWS.confirm_delete_fataled=gtk_check_button_new_with_label(_("Confirm delete failed downloads"));
	D4XPWS.confirm_opening_many=gtk_check_button_new_with_label(_("Confirm opening large amount of windows"));
	D4XPWS.confirm_exit=gtk_check_button_new_with_label(_("Confirm exit from program"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(D4XPWS.confirm_delete),TMPCFG.CONFIRM_DELETE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(D4XPWS.confirm_delete_all),TMPCFG.CONFIRM_DELETE_ALL);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(D4XPWS.confirm_delete_completed),TMPCFG.CONFIRM_DELETE_COMPLETED);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(D4XPWS.confirm_delete_fataled),TMPCFG.CONFIRM_DELETE_FATALED);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(D4XPWS.confirm_opening_many),TMPCFG.CONFIRM_OPENING_MANY);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(D4XPWS.confirm_exit),TMPCFG.CONFIRM_EXIT);
	gtk_box_pack_start(GTK_BOX(vbox),D4XPWS.confirm_delete,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),D4XPWS.confirm_delete_all,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),D4XPWS.confirm_delete_completed,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),D4XPWS.confirm_delete_fataled,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),D4XPWS.confirm_opening_many,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),D4XPWS.confirm_exit,FALSE,FALSE,0);

	gtk_widget_show_all(vbox);
};

static void d4x_prefs_toggle_clipboard_monitor(GtkWidget *parent){
	gtk_widget_set_sensitive(D4XPWS.clipboard_skip_button,GTK_TOGGLE_BUTTON(parent)->active);
	gtk_widget_set_sensitive(D4XPWS.clipboard_skip,GTK_TOGGLE_BUTTON(parent)->active);
	gtk_widget_set_sensitive(D4XPWS.clipboard_catch_button,GTK_TOGGLE_BUTTON(parent)->active);
	gtk_widget_set_sensitive(D4XPWS.clipboard_catch,GTK_TOGGLE_BUTTON(parent)->active);
};

void d4x_prefs_clipboard(){
	GtkWidget *vbox=d4x_prefs_child_destroy(_("Clipboard"));

	D4XPWS.clipboard_monitor=gtk_check_button_new_with_label(_("Monitor clipboard"));
	gtk_box_pack_start(GTK_BOX(vbox),D4XPWS.clipboard_monitor,FALSE,FALSE,0);
	gtk_signal_connect(GTK_OBJECT(D4XPWS.clipboard_monitor),
			   "clicked",
			   GTK_SIGNAL_FUNC(d4x_prefs_toggle_clipboard_monitor),NULL);
	
	GtkWidget *prefs_other_scbox=gtk_vbox_new(FALSE,0);
	D4XPWS.clipboard_skip=my_gtk_combo_new(ALL_HISTORIES[SKIP_HISTORY]);
	text_to_combo(D4XPWS.clipboard_skip,TMPCFG.SKIP_IN_CLIPBOARD);
	D4XPWS.clipboard_skip_button=gtk_radio_button_new_with_label((GSList *)NULL,
								     _("Skip these extensions in clipboard"));
	gtk_box_pack_start(GTK_BOX(prefs_other_scbox),D4XPWS.clipboard_skip_button,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(prefs_other_scbox),D4XPWS.clipboard_skip,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),prefs_other_scbox,FALSE,FALSE,0);

	D4XPWS.clipboard_catch=my_gtk_combo_new(ALL_HISTORIES[SKIP_HISTORY]);
	text_to_combo(D4XPWS.clipboard_catch,TMPCFG.CATCH_IN_CLIPBOARD);
	D4XPWS.clipboard_catch_button=gtk_radio_button_new_with_label(gtk_radio_button_group(GTK_RADIO_BUTTON(D4XPWS.clipboard_skip_button)),
							  _("Catch these extensions in clipboard"));
	prefs_other_scbox=gtk_vbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(prefs_other_scbox),D4XPWS.clipboard_catch_button,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(prefs_other_scbox),D4XPWS.clipboard_catch,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),prefs_other_scbox,FALSE,FALSE,0);

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(D4XPWS.clipboard_monitor),TMPCFG.CLIPBOARD_MONITOR);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(D4XPWS.clipboard_skip_button),!TMPCFG.CLIPBOARD_SKIP_OR_CATCH);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(D4XPWS.clipboard_catch_button),TMPCFG.CLIPBOARD_SKIP_OR_CATCH);
	d4x_prefs_toggle_clipboard_monitor(D4XPWS.clipboard_monitor);
	
	gtk_widget_show_all(vbox);
};

static void d4x_prefs_toggle_save_log(GtkWidget *parent) {
	gtk_widget_set_sensitive(D4XPWS.log_append,GTK_TOGGLE_BUTTON(parent)->active);
	gtk_widget_set_sensitive(D4XPWS.log_fsize,GTK_TOGGLE_BUTTON(parent)->active);
	gtk_widget_set_sensitive(D4XPWS.log_fslabel,GTK_TOGGLE_BUTTON(parent)->active);
	gtk_widget_set_sensitive(D4XPWS.log_rewrite,GTK_TOGGLE_BUTTON(parent)->active);
	gtk_widget_set_sensitive(D4XPWS.log_save_path,GTK_TOGGLE_BUTTON(parent)->active);
};

void d4x_prefs_main_log(){
	GtkWidget *vbox=d4x_prefs_child_destroy(_("Main log"));

	GtkWidget *prefs_limits_mlbox=gtk_hbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(prefs_limits_mlbox),5);
	D4XPWS.log_length=my_gtk_entry_new_with_max_length(3,TMPCFG.MAX_MAIN_LOG_LENGTH);
	gtk_box_pack_start(GTK_BOX(prefs_limits_mlbox),D4XPWS.log_length,FALSE,FALSE,0);
	GtkWidget *prefs_limits_mllabel=gtk_label_new(_("Maximum lines in MAIN log"));
	gtk_box_pack_start(GTK_BOX(prefs_limits_mlbox),prefs_limits_mllabel,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),prefs_limits_mlbox,FALSE,FALSE,0);
	
	D4XPWS.log_detailed=gtk_check_button_new_with_label(_("Output detailed information"));
	GTK_TOGGLE_BUTTON(D4XPWS.log_detailed)->active=TMPCFG.MAIN_LOG_DETAILED;
	gtk_box_pack_start(GTK_BOX(vbox),D4XPWS.log_detailed,FALSE,FALSE,0);

	D4XPWS.log_save=gtk_check_button_new_with_label(_("Save main log into file"));
	GTK_TOGGLE_BUTTON(D4XPWS.log_save)->active=TMPCFG.SAVE_MAIN_LOG;
	gtk_box_pack_start(GTK_BOX(vbox),D4XPWS.log_save,FALSE,FALSE,0);
	gtk_signal_connect(GTK_OBJECT(D4XPWS.log_save),"clicked",GTK_SIGNAL_FUNC(d4x_prefs_toggle_save_log),NULL);

	D4XPWS.log_save_path=my_gtk_filesel_new(ALL_HISTORIES[LOG_HISTORY]);
	MY_GTK_FILESEL(D4XPWS.log_save_path)->modal=GTK_WINDOW(d4x_prefs_window);
	if (TMPCFG.SAVE_LOG_PATH)
		text_to_combo(MY_GTK_FILESEL(D4XPWS.log_save_path)->combo,TMPCFG.SAVE_LOG_PATH);
	gtk_box_pack_start(GTK_BOX(vbox),D4XPWS.log_save_path,FALSE,FALSE,0);

	char temp[MAX_LEN];
	GtkWidget *prefs_log_mlfbox=gtk_hbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(prefs_log_mlfbox),5);
	D4XPWS.log_fsize=gtk_entry_new_with_max_length(9);
	gtk_widget_set_usize(D4XPWS.log_fsize,80,-1);
	sprintf(temp,"%li",TMPCFG.MAIN_LOG_FILE_LIMIT);
	gtk_entry_set_text(GTK_ENTRY(D4XPWS.log_fsize),temp);
	gtk_box_pack_start(GTK_BOX(prefs_log_mlfbox),D4XPWS.log_fsize,FALSE,FALSE,0);
	D4XPWS.log_fslabel=gtk_label_new(_("Maximum size for file of main log (in KBytes)"));
	gtk_box_pack_start(GTK_BOX(prefs_log_mlfbox),D4XPWS.log_fslabel,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),prefs_log_mlfbox,FALSE,FALSE,0);

	GtkWidget *hboxtemp=gtk_hbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(hboxtemp),5);
	D4XPWS.log_append=gtk_radio_button_new_with_label((GSList *)NULL,_("Append to file"));
	gtk_box_pack_start(GTK_BOX(hboxtemp),D4XPWS.log_append,FALSE,FALSE,0);
	GSList *other_group=gtk_radio_button_group(GTK_RADIO_BUTTON(D4XPWS.log_append));
	D4XPWS.log_rewrite=gtk_radio_button_new_with_label(other_group,_("Overwrite file"));
	gtk_box_pack_start(GTK_BOX(hboxtemp),D4XPWS.log_rewrite,FALSE,FALSE,0);
	GTK_TOGGLE_BUTTON(D4XPWS.log_append)->active=TMPCFG.APPEND_REWRITE_LOG;
	GTK_TOGGLE_BUTTON(D4XPWS.log_rewrite)->active=!TMPCFG.APPEND_REWRITE_LOG;
	gtk_box_pack_start(GTK_BOX(vbox),hboxtemp,FALSE,FALSE,0);
	d4x_prefs_toggle_save_log(D4XPWS.log_save);

	gtk_widget_show_all(vbox);
};

static void d4x_prefs_toggle_exit_complete(GtkWidget *parent) {
	gtk_widget_set_sensitive(D4XPWS.exit_complete_time,GTK_TOGGLE_BUTTON(parent)->active);
};

void d4x_prefs_integration(){
	GtkWidget *vbox=d4x_prefs_child_destroy(_("Integration"));
	
	D4XPWS.exit_complete=gtk_check_button_new_with_label(_("Exit if nothing to do after"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(D4XPWS.exit_complete),TMPCFG.EXIT_COMPLETE);
	gtk_signal_connect(GTK_OBJECT(D4XPWS.exit_complete),"clicked",GTK_SIGNAL_FUNC(d4x_prefs_toggle_exit_complete),NULL);
	D4XPWS.exit_complete_time=my_gtk_entry_new_with_max_length(3,TMPCFG.EXIT_COMPLETE_TIME);
	GtkWidget *prefs_common_hbox=gtk_hbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(prefs_common_hbox),5);
	GtkWidget *prefs_common_label=gtk_label_new(_("minutes"));
	gtk_box_pack_start(GTK_BOX(prefs_common_hbox),D4XPWS.exit_complete,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(prefs_common_hbox),D4XPWS.exit_complete_time,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(prefs_common_hbox),prefs_common_label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),prefs_common_hbox,FALSE,FALSE,0);
	d4x_prefs_toggle_exit_complete(D4XPWS.exit_complete);

	GtkWidget *prefs_other_ebox=gtk_vbox_new(FALSE,0);
	D4XPWS.exec_on_exit=my_gtk_filesel_new(ALL_HISTORIES[EXEC_HISTORY]);
	MY_GTK_FILESEL(D4XPWS.exec_on_exit)->modal=GTK_WINDOW(d4x_prefs_window);
	text_to_combo(MY_GTK_FILESEL(D4XPWS.exec_on_exit)->combo,TMPCFG.EXEC_WHEN_QUIT);
	GtkWidget *prefs_other_elabel=gtk_label_new(_("Run this on exit"));
	gtk_box_pack_start(GTK_BOX(prefs_other_ebox),prefs_other_elabel,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(prefs_other_ebox),D4XPWS.exec_on_exit,TRUE,TRUE,0);
	gtk_box_pack_start(GTK_BOX(vbox),prefs_other_ebox,FALSE,FALSE,0);

	D4XPWS.dnd_dialog=gtk_check_button_new_with_label(_("Open dialog for Drag-n-Drop"));
	gtk_box_pack_start(GTK_BOX(vbox),D4XPWS.dnd_dialog,FALSE,FALSE,0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(D4XPWS.dnd_dialog),TMPCFG.NEED_DIALOG_FOR_DND);

	gtk_widget_show_all(vbox);
};

void d4x_prefs_columns(){
	GtkWidget *vbox=d4x_prefs_child_destroy(_("Columns"));

	GtkWidget *columns_hbox=gtk_hbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(columns_hbox),5);
	GtkWidget *columns_frame1=gtk_frame_new(_("Size format"));
	GtkWidget *columns_frame2=gtk_frame_new(_("Time format"));
	gtk_container_border_width(GTK_CONTAINER(columns_frame1),5);
	gtk_container_border_width(GTK_CONTAINER(columns_frame2),5);
	GtkWidget *columns_vbox1=gtk_vbox_new(FALSE,0);
	GtkWidget *columns_vbox2=gtk_vbox_new(FALSE,0);

	D4XPWS.columns_nums1=gtk_radio_button_new_with_label((GSList *)NULL,"123456");
	gtk_box_pack_start(GTK_BOX(columns_vbox1),D4XPWS.columns_nums1,FALSE,FALSE,0);
	GSList *columns_group1=gtk_radio_button_group(GTK_RADIO_BUTTON(D4XPWS.columns_nums1));
	D4XPWS.columns_nums2=gtk_radio_button_new_with_label(columns_group1,"123 456");
	gtk_box_pack_start(GTK_BOX(columns_vbox1),D4XPWS.columns_nums2,FALSE,FALSE,0);
	D4XPWS.columns_nums3=gtk_radio_button_new_with_label(
		gtk_radio_button_group(GTK_RADIO_BUTTON(D4XPWS.columns_nums2)),"123K");
	gtk_box_pack_start(GTK_BOX(columns_vbox1),D4XPWS.columns_nums3,FALSE,FALSE,0);
	D4XPWS.columns_nums4=gtk_radio_button_new_with_label(
		gtk_radio_button_group(GTK_RADIO_BUTTON(D4XPWS.columns_nums3)),"123'456");
	gtk_box_pack_start(GTK_BOX(columns_vbox1),D4XPWS.columns_nums4,FALSE,FALSE,0);

	switch(TMPCFG.NICE_DEC_DIGITALS.curent) {
		case 1:
			{
				gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(D4XPWS.columns_nums2),TRUE);
				break;
			};
		case 2:
			{
				gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(D4XPWS.columns_nums3),TRUE);
				break;
			};
		case 3:
			{
				gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(D4XPWS.columns_nums4),TRUE);
				break;
			};
		default:
			gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(D4XPWS.columns_nums1),TRUE);
	};
	D4XPWS.columns_time1=gtk_radio_button_new_with_label((GSList *)NULL,"12:34:56");
	gtk_box_pack_start(GTK_BOX(columns_vbox2),D4XPWS.columns_time1,FALSE,FALSE,0);
	GSList *columns_group2=gtk_radio_button_group(GTK_RADIO_BUTTON(D4XPWS.columns_time1));
	D4XPWS.columns_time2=gtk_radio_button_new_with_label(columns_group2,"12:34");
	gtk_box_pack_start(GTK_BOX(columns_vbox2),D4XPWS.columns_time2,FALSE,FALSE,0);
	if (TMPCFG.TIME_FORMAT)
		gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(D4XPWS.columns_time2),TRUE);
	else
		gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(D4XPWS.columns_time1),TRUE);
	gtk_container_add(GTK_CONTAINER(columns_frame1),columns_vbox1);
	GtkWidget *columns_vbox11=gtk_vbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(columns_vbox11),columns_frame1,FALSE,FALSE,0);
	GtkWidget *columns_vbox12=gtk_vbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(columns_vbox11),columns_vbox12,FALSE,FALSE,0);

	gtk_container_add(GTK_CONTAINER(columns_frame2),columns_vbox2);
	GtkWidget *columns_vbox21=gtk_vbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(columns_vbox21),columns_frame2,FALSE,FALSE,0);
	GtkWidget *columns_vbox22=gtk_vbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(columns_vbox21),columns_vbox22,FALSE,FALSE,0);

	GtkWidget *columns_vbox=gtk_vbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(columns_hbox),5);
	gtk_box_pack_start(GTK_BOX(columns_vbox),columns_vbox11,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(columns_vbox),columns_vbox21,FALSE,FALSE,0);
	D4XPWS.columns_order.init();
	gtk_box_pack_start(GTK_BOX(columns_hbox),columns_vbox,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(columns_hbox),D4XPWS.columns_order.body(),FALSE,FALSE,0);

	gtk_box_pack_start(GTK_BOX(vbox),columns_hbox,FALSE,FALSE,0);
	gtk_widget_show_all(vbox);
};

static void d4x_prefs_toggle_save_list(GtkWidget *parent) {
	gtk_widget_set_sensitive(D4XPWS.save_list_entry,GTK_TOGGLE_BUTTON(parent)->active);
};

void d4x_prefs_main(){
	GtkWidget *vbox=d4x_prefs_child_destroy(_("Main"));

	GtkWidget *prefs_limits_tbox=gtk_hbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(prefs_limits_tbox),5);
	D4XPWS.max_threads=my_gtk_entry_new_with_max_length(3,TMPCFG.MAX_THREADS);
	gtk_box_pack_start(GTK_BOX(prefs_limits_tbox),D4XPWS.max_threads,FALSE,FALSE,0);
	GtkWidget *prefs_limits_tlabel=gtk_label_new(_("Maximum active downloads"));
	gtk_box_pack_start(GTK_BOX(prefs_limits_tbox),prefs_limits_tlabel,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),prefs_limits_tbox,FALSE,FALSE,0);

	D4XPWS.save_list_check=gtk_check_button_new_with_label(_("Save list of downloads every"));
	gtk_signal_connect(GTK_OBJECT(D4XPWS.save_list_check),"clicked",GTK_SIGNAL_FUNC(d4x_prefs_toggle_save_list),NULL);
	GTK_TOGGLE_BUTTON(D4XPWS.save_list_check)->active=TMPCFG.SAVE_LIST;
	D4XPWS.save_list_entry=my_gtk_entry_new_with_max_length(3,TMPCFG.SAVE_LIST_INTERVAL);
	GtkWidget *prefs_common_hbox=gtk_hbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(prefs_common_hbox),5);
	GtkWidget *prefs_common_label=gtk_label_new(_("minutes"));
	gtk_box_pack_start(GTK_BOX(prefs_common_hbox),D4XPWS.save_list_check,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(prefs_common_hbox),D4XPWS.save_list_entry,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(prefs_common_hbox),prefs_common_label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),prefs_common_hbox,FALSE,FALSE,0);
	d4x_prefs_toggle_save_list(D4XPWS.save_list_check);
	
	D4XPWS.del_completed=gtk_check_button_new_with_label(_("Automatically delete completed downloads"));
	D4XPWS.del_fataled=gtk_check_button_new_with_label(_("Automatically delete failed downloads"));
	D4XPWS.allow_force_run=gtk_check_button_new_with_label(_("Allow to user force run downloads"));
	D4XPWS.remember_pass=gtk_check_button_new_with_label(_("Remember passwords"));
	D4XPWS.description=gtk_check_button_new_with_label(_("Write Descript.ion file"));
	GTK_TOGGLE_BUTTON(D4XPWS.del_completed)->active=TMPCFG.DELETE_COMPLETED;
	GTK_TOGGLE_BUTTON(D4XPWS.del_fataled)->active=TMPCFG.DELETE_FATAL;
	GTK_TOGGLE_BUTTON(D4XPWS.allow_force_run)->active=TMPCFG.ALLOW_FORCE_RUN;
	GTK_TOGGLE_BUTTON(D4XPWS.remember_pass)->active=TMPCFG.REMEMBER_PASS;
	GTK_TOGGLE_BUTTON(D4XPWS.description)->active=TMPCFG.WRITE_DESCRIPTION;
	gtk_box_pack_start(GTK_BOX(vbox),D4XPWS.del_completed,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),D4XPWS.del_fataled,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),D4XPWS.allow_force_run,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),D4XPWS.remember_pass,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),D4XPWS.description,FALSE,FALSE,0);

#include "pixmaps/speed1.xpm"
#include "pixmaps/speed2.xpm"
	GtkWidget *label=gtk_label_new(_("bytes/sec speed level one (red button)"));
	GtkWidget *hbox=gtk_hbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(hbox),5);
	D4XPWS.speed_limit_1=my_gtk_entry_new_with_max_length(5,TMPCFG.SPEED_LIMIT_1);
	gtk_box_pack_start(GTK_BOX(hbox),new_pixmap(speed1_xpm),FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(hbox),D4XPWS.speed_limit_1,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);
	label=gtk_label_new(_("bytes/sec speed level two (yellow button)"));
	hbox=gtk_hbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(hbox),5);
	D4XPWS.speed_limit_2=my_gtk_entry_new_with_max_length(5,TMPCFG.SPEED_LIMIT_2);
	gtk_box_pack_start(GTK_BOX(hbox),new_pixmap(speed2_xpm),FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(hbox),D4XPWS.speed_limit_2,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);
	
//	GTK_TOGGLE_BUTTON(D4XPWS.)->active=TMPCFG.;
	gtk_widget_show_all(vbox);
};

void d4x_prefs_interface(){
	GtkWidget *vbox=d4x_prefs_child_destroy(_("Interface"));

	D4XPWS.dnd_trash=gtk_check_button_new_with_label(_("Show DnD basket"));
	D4XPWS.fixed_font_log=gtk_check_button_new_with_label(_("Use fixed font in logs"));
	GTK_TOGGLE_BUTTON(D4XPWS.dnd_trash)->active=TMPCFG.DND_TRASH;
	GTK_TOGGLE_BUTTON(D4XPWS.fixed_font_log)->active=TMPCFG.FIXED_LOG_FONT;
	gtk_box_pack_start(GTK_BOX(vbox),D4XPWS.dnd_trash,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),D4XPWS.fixed_font_log,FALSE,FALSE,0);
	gtk_widget_show_all(vbox);
};

static gint d4x_prefs_reset_colors(){
	my_gtk_colorsel_set_color(MY_GTK_COLORSEL(D4XPWS.speed_color_back),0xFFFFFF);
	my_gtk_colorsel_set_color(MY_GTK_COLORSEL(D4XPWS.speed_color_fore1),0x555555);
	my_gtk_colorsel_set_color(MY_GTK_COLORSEL(D4XPWS.speed_color_fore2),0xAAAAAA);
	my_gtk_colorsel_set_color(MY_GTK_COLORSEL(D4XPWS.speed_color_pick),0);
	return TRUE;
};

void d4x_prefs_graph(){
	GtkWidget *vbox=d4x_prefs_child_destroy(_("Graph"));

	D4XPWS.graph_order=gtk_check_button_new_with_label(_("Revert drawing graph of speeds"));
	GTK_TOGGLE_BUTTON(D4XPWS.graph_order)->active=TMPCFG.GRAPH_ORDER;
	gtk_box_pack_start(GTK_BOX(vbox),D4XPWS.graph_order,FALSE,FALSE,0);

	GtkWidget *vbox_colors=gtk_vbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(vbox_colors),5);
	D4XPWS.speed_color_pick=my_gtk_colorsel_new(TMPCFG.GRAPH_PICK,_("Color for picks"));
	D4XPWS.speed_color_fore1=my_gtk_colorsel_new(TMPCFG.GRAPH_FORE1,_("Color for total speed"));
	D4XPWS.speed_color_fore2=my_gtk_colorsel_new(TMPCFG.GRAPH_FORE2,_("Color for speed of selected"));
	D4XPWS.speed_color_back=my_gtk_colorsel_new(TMPCFG.GRAPH_BACK,_("Background color"));
	MY_GTK_COLORSEL(D4XPWS.speed_color_pick)->modal=GTK_WINDOW(d4x_prefs_window);
	MY_GTK_COLORSEL(D4XPWS.speed_color_fore1)->modal=GTK_WINDOW(d4x_prefs_window);
	MY_GTK_COLORSEL(D4XPWS.speed_color_fore2)->modal=GTK_WINDOW(d4x_prefs_window);
	MY_GTK_COLORSEL(D4XPWS.speed_color_back)->modal=GTK_WINDOW(d4x_prefs_window);
	gtk_box_pack_start(GTK_BOX(vbox_colors),D4XPWS.speed_color_back,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox_colors),D4XPWS.speed_color_fore1,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox_colors),D4XPWS.speed_color_fore2,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox_colors),D4XPWS.speed_color_pick,FALSE,FALSE,0);
	GtkWidget *button_reset=gtk_button_new_with_label(_("Reset to default"));
	gtk_signal_connect(GTK_OBJECT(button_reset),"clicked",GTK_SIGNAL_FUNC(d4x_prefs_reset_colors),NULL);
	gtk_box_pack_start(GTK_BOX(vbox_colors),button_reset,FALSE,FALSE,0);
	GtkWidget *frame_colors=gtk_frame_new(_("Colors for graph"));
	gtk_container_border_width(GTK_CONTAINER(frame_colors),5);
	gtk_container_add(GTK_CONTAINER(frame_colors),vbox_colors);
	gtk_container_border_width(GTK_CONTAINER(vbox_colors),5);
	gtk_box_pack_start(GTK_BOX(vbox),frame_colors,FALSE,FALSE,0);
	
	gtk_widget_show_all(vbox);
};

void d4x_prefs_init_pre(){
	if (d4x_prefs_window) {
		gdk_window_show(d4x_prefs_window->window);
		return;
	};
	var_copy_cfg(&TMPCFG,&CFG);
	/* create preferences window */
	d4x_prefs_window=gtk_window_new(GTK_WINDOW_DIALOG);
	gtk_window_set_title(GTK_WINDOW(d4x_prefs_window),_("Options"));
	gtk_window_set_position(GTK_WINDOW(d4x_prefs_window),GTK_WIN_POS_NONE);
	gtk_window_set_policy (GTK_WINDOW(d4x_prefs_window), FALSE,FALSE,FALSE);
	gtk_signal_connect(GTK_OBJECT(d4x_prefs_window), "key_press_event",
			   (GtkSignalFunc)d4x_prefs_esc_handler, NULL);
	gtk_signal_connect(GTK_OBJECT(d4x_prefs_window),"delete_event",
			   GTK_SIGNAL_FUNC(d4x_prefs_cancel), NULL);
	
	/* first box inside window */
	GtkWidget *tmphbox=gtk_hbox_new(FALSE,0);
	GtkWidget *tmpvbox=gtk_vbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(tmphbox),5);
	gtk_box_set_spacing(GTK_BOX(tmpvbox),5);
	gtk_container_border_width(GTK_CONTAINER(d4x_prefs_window),5);
	gtk_container_add(GTK_CONTAINER(d4x_prefs_window),tmpvbox);
	/* container for tree */
	GtkWidget *scroll_win=gtk_scrolled_window_new((GtkAdjustment*)NULL,(GtkAdjustment*)NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll_win),
					GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_box_pack_start (GTK_BOX (tmphbox), scroll_win, TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (tmpvbox), tmphbox, TRUE, TRUE, 0);
	gtk_widget_set_usize(scroll_win,150,300);
	gtk_widget_show (scroll_win);
	/* containder for all other */
	d4x_prefs_frame=gtk_frame_new("test");
//	gtk_widget_set_usize(d4x_prefs_frame,480,-1);
	gtk_box_pack_start (GTK_BOX (tmphbox), d4x_prefs_frame, TRUE, TRUE, 0);
	gtk_container_border_width(GTK_CONTAINER(d4x_prefs_frame),5);
	/* create tree of options */
	GtkWidget *root_tree=D4XPWS.root_tree=gtk_tree_new();
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW (scroll_win),
					      root_tree);
	gtk_widget_show(root_tree);
	
	GtkWidget *tmpitem=gtk_tree_item_new_with_label(_("Download"));
	gtk_tree_append(GTK_TREE(root_tree), tmpitem);
	gtk_signal_connect(GTK_OBJECT(tmpitem), "select",
			   (GtkSignalFunc)d4x_prefs_download, NULL);
	gtk_widget_show(tmpitem);
	GtkWidget *sub_tree=D4XPWS.tree_download=gtk_tree_new();
	gtk_tree_item_set_subtree(GTK_TREE_ITEM(tmpitem), sub_tree);
	gtk_signal_emit_by_name(GTK_OBJECT(tmpitem),"expand",NULL);
	
	tmpitem=gtk_tree_item_new_with_label(_("Limits"));
	gtk_signal_connect(GTK_OBJECT(tmpitem), "select",
			   (GtkSignalFunc)d4x_prefs_download_limits, NULL);
	gtk_tree_append(GTK_TREE(sub_tree), tmpitem);
	gtk_widget_show(tmpitem);
	tmpitem=gtk_tree_item_new_with_label(_("FTP"));
	gtk_tree_append(GTK_TREE(sub_tree), tmpitem);
	gtk_widget_show(tmpitem);
	gtk_signal_connect(GTK_OBJECT(tmpitem), "select",
			   (GtkSignalFunc)d4x_prefs_download_ftp, NULL);
	tmpitem=gtk_tree_item_new_with_label(_("HTTP"));
	gtk_tree_append(GTK_TREE(sub_tree), tmpitem);
	gtk_widget_show(tmpitem);
	gtk_signal_connect(GTK_OBJECT(tmpitem), "select",
			   (GtkSignalFunc)d4x_prefs_download_http, NULL);
	tmpitem=gtk_tree_item_new_with_label(_("Proxy"));
	gtk_signal_connect(GTK_OBJECT(tmpitem), "select",
			   (GtkSignalFunc)d4x_prefs_proxy, NULL);
	gtk_tree_append(GTK_TREE(sub_tree), tmpitem);
	gtk_widget_show(tmpitem);
	
	tmpitem=gtk_tree_item_new_with_label(_("Interface"));
	gtk_signal_connect(GTK_OBJECT(tmpitem), "select",
			   (GtkSignalFunc)d4x_prefs_interface, NULL);
	gtk_tree_append(GTK_TREE(root_tree), tmpitem);
	gtk_widget_show(tmpitem);
	sub_tree=D4XPWS.tree_interface=gtk_tree_new();
	gtk_tree_item_set_subtree(GTK_TREE_ITEM(tmpitem), sub_tree);
	gtk_signal_emit_by_name(GTK_OBJECT(tmpitem),"expand",NULL);

	tmpitem=gtk_tree_item_new_with_label(_("Columns"));
	gtk_signal_connect(GTK_OBJECT(tmpitem), "select",
			   (GtkSignalFunc)d4x_prefs_columns, NULL);
	gtk_tree_append(GTK_TREE(sub_tree), tmpitem);
	gtk_widget_show(tmpitem);
	tmpitem=gtk_tree_item_new_with_label(_("Main window"));
	gtk_signal_connect(GTK_OBJECT(tmpitem), "select",
			   (GtkSignalFunc)d4x_prefs_mwin, NULL);
	gtk_tree_append(GTK_TREE(sub_tree), tmpitem);
	gtk_widget_show(tmpitem);
	tmpitem=gtk_tree_item_new_with_label(_("Confirmation"));
	gtk_signal_connect(GTK_OBJECT(tmpitem), "select",
			   (GtkSignalFunc)d4x_prefs_confirm, NULL);
	gtk_tree_append(GTK_TREE(sub_tree), tmpitem);
	gtk_widget_show(tmpitem);
	tmpitem=gtk_tree_item_new_with_label(_("Graph"));
	gtk_signal_connect(GTK_OBJECT(tmpitem), "select",
			   (GtkSignalFunc)d4x_prefs_graph, NULL);
	gtk_tree_append(GTK_TREE(sub_tree), tmpitem);
	gtk_widget_show(tmpitem);

	tmpitem=gtk_tree_item_new_with_label(_("Integration"));
	gtk_signal_connect(GTK_OBJECT(tmpitem), "select",
			   (GtkSignalFunc)d4x_prefs_integration, NULL);
	gtk_tree_append(GTK_TREE(root_tree), tmpitem);
	gtk_widget_show(tmpitem);
	sub_tree=D4XPWS.tree_integration=gtk_tree_new();
	gtk_tree_item_set_subtree(GTK_TREE_ITEM(tmpitem), sub_tree);
	gtk_signal_emit_by_name(GTK_OBJECT(tmpitem),"expand",NULL);
/*	
	tmpitem=gtk_tree_item_new_with_label(_("DnD"));
	gtk_tree_append(GTK_TREE(sub_tree), tmpitem);
	gtk_widget_show(tmpitem);
*/
	tmpitem=gtk_tree_item_new_with_label(_("Clipboard"));
	gtk_signal_connect(GTK_OBJECT(tmpitem), "select",
			   (GtkSignalFunc)d4x_prefs_clipboard, NULL);
	gtk_tree_append(GTK_TREE(sub_tree), tmpitem);
	gtk_widget_show(tmpitem);

	tmpitem=gtk_tree_item_new_with_label(_("Main"));
	gtk_signal_connect(GTK_OBJECT(tmpitem), "select",
			   (GtkSignalFunc)d4x_prefs_main, NULL);
	gtk_tree_append(GTK_TREE(root_tree), tmpitem);
	gtk_widget_show(tmpitem);
	sub_tree=D4XPWS.tree_main=gtk_tree_new();
	gtk_tree_item_set_subtree(GTK_TREE_ITEM(tmpitem), sub_tree);
	gtk_signal_emit_by_name(GTK_OBJECT(tmpitem),"expand",NULL);

	tmpitem=gtk_tree_item_new_with_label(_("Main log"));
	gtk_signal_connect(GTK_OBJECT(tmpitem), "select",
			   (GtkSignalFunc)d4x_prefs_main_log, NULL);
	gtk_tree_append(GTK_TREE(sub_tree), tmpitem);
	gtk_widget_show(tmpitem);
        /* show window */

	GtkWidget *buttons_hbox=gtk_hbutton_box_new();
	gtk_button_box_set_layout(GTK_BUTTON_BOX(buttons_hbox),GTK_BUTTONBOX_END);
	gtk_button_box_set_spacing(GTK_BUTTON_BOX(buttons_hbox),5);
	gtk_box_pack_start (GTK_BOX (tmpvbox), buttons_hbox, TRUE, TRUE, 0);
	GtkWidget *ok_button=gtk_button_new_with_label(_("Ok"));
	GtkWidget *cancel_button=gtk_button_new_with_label(_("Cancel"));
	GtkWidget *apply_button=gtk_button_new_with_label(_("Apply"));
	gtk_signal_connect(GTK_OBJECT(cancel_button),"clicked",GTK_SIGNAL_FUNC(d4x_prefs_cancel),NULL);
	gtk_signal_connect(GTK_OBJECT(apply_button),"clicked",GTK_SIGNAL_FUNC(d4x_prefs_apply),NULL);
	gtk_signal_connect(GTK_OBJECT(ok_button),"clicked",GTK_SIGNAL_FUNC(d4x_prefs_ok),NULL);
	GTK_WIDGET_SET_FLAGS(ok_button,GTK_CAN_DEFAULT);
	GTK_WIDGET_SET_FLAGS(cancel_button,GTK_CAN_DEFAULT);
	GTK_WIDGET_SET_FLAGS(apply_button,GTK_CAN_DEFAULT);
	gtk_box_pack_start(GTK_BOX(buttons_hbox),ok_button,TRUE,TRUE,0);
	gtk_box_pack_start(GTK_BOX(buttons_hbox),apply_button,TRUE,TRUE,0);
	gtk_box_pack_start(GTK_BOX(buttons_hbox),cancel_button,TRUE,TRUE,0);
	gtk_window_set_default(GTK_WINDOW(d4x_prefs_window),ok_button);

	gtk_widget_show_all(d4x_prefs_window);
};

void d4x_prefs_init_page(int page){
	d4x_prefs_init_pre();
	switch (page){
	case PREFS_PAGE_MAINLOG:
		gtk_tree_select_item(GTK_TREE(D4XPWS.tree_main),0);
		break;
	case PREFS_PAGE_MAIN:
		gtk_tree_select_item(GTK_TREE(D4XPWS.root_tree),3);
		break;
	};
};

void d4x_prefs_init(){
	d4x_prefs_init_page(PREFS_PAGE_MAIN);
};

static void d4x_prefs_get_field(GtkWidget *widget,char **where,tHistory *history){
	if (where==NULL) return;
	if (*where) delete(*where);
	*where=copy_string(text_from_combo(widget));
	if (history!=NULL) history->add(*where);
};

void d4x_prefs_apply_tmp(){
	char *label=GTK_FRAME(d4x_prefs_frame)->label;
	if (equal(label,_("Download"))){
		TMPCFG.DEFAULT_CFG.get_date=GTK_TOGGLE_BUTTON(D4XPWS.get_date_check)->active;
		TMPCFG.DEFAULT_CFG.retry=GTK_TOGGLE_BUTTON(D4XPWS.retry_check)->active;
		TMPCFG.DEFAULT_CFG.sleep_before_complete=GTK_TOGGLE_BUTTON(D4XPWS.sleep_check)->active;
		TMPCFG.RECURSIVE_OPTIMIZE=GTK_TOGGLE_BUTTON(D4XPWS.recursive)->active;
		TMPCFG.PAUSE_AFTER_ADDING=GTK_TOGGLE_BUTTON(D4XPWS.pause_check)->active;
		TMPCFG.DEFAULT_CFG.check_time=GTK_TOGGLE_BUTTON(D4XPWS.check_time_check)->active;
		d4x_prefs_get_field(MY_GTK_FILESEL(D4XPWS.savepath)->combo,
					 &TMPCFG.GLOBAL_SAVE_PATH,
					 ALL_HISTORIES[PATH_HISTORY]);
		return;
	};
	if (equal(label,_("Limits"))){
		sscanf(gtk_entry_get_text(GTK_ENTRY(D4XPWS.timeout_entry)),"%u",&TMPCFG.DEFAULT_CFG.timeout);
		sscanf(gtk_entry_get_text(GTK_ENTRY(D4XPWS.sleep_entry)),"%u",&TMPCFG.DEFAULT_CFG.time_for_sleep);
		sscanf(gtk_entry_get_text(GTK_ENTRY(D4XPWS.attempts_entry)),"%u",&TMPCFG.DEFAULT_CFG.number_of_attempts);
		sscanf(gtk_entry_get_text(GTK_ENTRY(D4XPWS.limits_log)),"%u",&TMPCFG.MAX_LOG_LENGTH);
		sscanf(gtk_entry_get_text(GTK_ENTRY(D4XPWS.rollback_entry)),"%u",&TMPCFG.DEFAULT_CFG.rollback);
		sscanf(gtk_entry_get_text(GTK_ENTRY(D4XPWS.speed_entry)),"%u",&TMPCFG.DEFAULT_CFG.speed);
		return;
	};
	if (equal(label,_("FTP"))){
		TMPCFG.DEFAULT_CFG.passive=GTK_TOGGLE_BUTTON(D4XPWS.ftp_passive_check)->active;
		TMPCFG.DEFAULT_CFG.permisions=GTK_TOGGLE_BUTTON(D4XPWS.permisions_check)->active;
		TMPCFG.DEFAULT_CFG.dont_send_quit=GTK_TOGGLE_BUTTON(D4XPWS.dont_send_quit_check)->active;
		TMPCFG.DEFAULT_CFG.link_as_file=GTK_TOGGLE_BUTTON(D4XPWS.link_as_file_check)->active;
		TMPCFG.FTP_DIR_IN_LOG=GTK_TOGGLE_BUTTON(D4XPWS.ftp_dir_in_log)->active;
		sscanf(gtk_entry_get_text(GTK_ENTRY(D4XPWS.ftp_recurse_depth_entry)),"%u",&TMPCFG.DEFAULT_CFG.ftp_recurse_depth);
		return;
	};
	if (equal(label,_("HTTP"))){
		TMPCFG.DEFAULT_CFG.leave_server=GTK_TOGGLE_BUTTON(D4XPWS.leave_server_check)->active;
		TMPCFG.DEFAULT_CFG.dont_leave_dir=GTK_TOGGLE_BUTTON(D4XPWS.leave_dir_check)->active;
//		TMPCFG.DEFAULT_CFG.=GTK_TOGGLE_BUTTON(D4XPWS.)->active;
		sscanf(gtk_entry_get_text(GTK_ENTRY(D4XPWS.http_recurse_depth_entry)),"%u",&TMPCFG.DEFAULT_CFG.http_recurse_depth);
		d4x_prefs_get_field(D4XPWS.user_agent_entry,
				    &TMPCFG.USER_AGENT,
				    ALL_HISTORIES[USER_AGENT_HISTORY]);
		d4x_prefs_get_field(MY_GTK_FILESEL(D4XPWS.unknown_filename)->combo,
				    &TMPCFG.DEFAULT_NAME,
				    ALL_HISTORIES[FILE_HISTORY]);
		return;
	};
	if (equal(label,_("Proxy"))){
		D4XPWS.proxy.apply_changes(&TMPCFG);
		return;
	};
	if (equal(label,_("Interface"))){
		TMPCFG.DND_TRASH=GTK_TOGGLE_BUTTON(D4XPWS.dnd_trash)->active;
		TMPCFG.FIXED_LOG_FONT=GTK_TOGGLE_BUTTON(D4XPWS.fixed_font_log)->active;
		return;
	};
	if (equal(label,_("Columns"))){
		D4XPWS.columns_order.apply_changes_tmp();
		TMPCFG.NICE_DEC_DIGITALS.curent=(GTK_TOGGLE_BUTTON(D4XPWS.columns_nums2)->active?1:0)+
			(GTK_TOGGLE_BUTTON(D4XPWS.columns_nums3)->active?2:0)+
			(GTK_TOGGLE_BUTTON(D4XPWS.columns_nums4)->active?3:0);
		TMPCFG.TIME_FORMAT=GTK_TOGGLE_BUTTON(D4XPWS.columns_time2)->active;
		return;
	};
	if (equal(label,_("Main window"))){
		TMPCFG.USE_MAINWIN_TITLE=GTK_TOGGLE_BUTTON(D4XPWS.mw_use_title)->active;
		TMPCFG.USE_MAINWIN_TITLE2=GTK_TOGGLE_BUTTON(D4XPWS.mw_use_title2)->active;
		TMPCFG.SCROLL_MAINWIN_TITLE=GTK_TOGGLE_BUTTON(D4XPWS.mw_scroll_title)->active;
		TMPCFG.WINDOW_LOWER=GTK_TOGGLE_BUTTON(D4XPWS.window_lower)->active;
		return;
	};
	if (equal(label,_("Main window"))){
		TMPCFG.CONFIRM_DELETE=GTK_TOGGLE_BUTTON(D4XPWS.confirm_delete)->active;
		TMPCFG.CONFIRM_DELETE_ALL=GTK_TOGGLE_BUTTON(D4XPWS.confirm_delete_all)->active;
		TMPCFG.CONFIRM_DELETE_COMPLETED=GTK_TOGGLE_BUTTON(D4XPWS.confirm_delete_completed)->active;
		TMPCFG.CONFIRM_DELETE_FATALED=GTK_TOGGLE_BUTTON(D4XPWS.confirm_delete_fataled)->active;
		TMPCFG.CONFIRM_EXIT=GTK_TOGGLE_BUTTON(D4XPWS.confirm_exit)->active;
		TMPCFG.CONFIRM_OPENING_MANY=GTK_TOGGLE_BUTTON(D4XPWS.confirm_opening_many)->active;
		return;
	};
	if (equal(label,_("Graph"))){
		TMPCFG.GRAPH_BACK=my_gtk_colorsel_get_color(MY_GTK_COLORSEL(D4XPWS.speed_color_back));
		TMPCFG.GRAPH_FORE1=my_gtk_colorsel_get_color(MY_GTK_COLORSEL(D4XPWS.speed_color_fore1));
		TMPCFG.GRAPH_FORE2=my_gtk_colorsel_get_color(MY_GTK_COLORSEL(D4XPWS.speed_color_fore2));
		TMPCFG.GRAPH_PICK=my_gtk_colorsel_get_color(MY_GTK_COLORSEL(D4XPWS.speed_color_pick));
		TMPCFG.GRAPH_ORDER=GTK_TOGGLE_BUTTON(D4XPWS.graph_order)->active;
		return;
	};
	if (equal(label,_("Integration"))){
		TMPCFG.EXIT_COMPLETE=GTK_TOGGLE_BUTTON(D4XPWS.exit_complete)->active;
		TMPCFG.NEED_DIALOG_FOR_DND=GTK_TOGGLE_BUTTON(D4XPWS.dnd_dialog)->active;
		sscanf(gtk_entry_get_text(GTK_ENTRY(D4XPWS.exit_complete_time)),"%u",&TMPCFG.EXIT_COMPLETE_TIME);
		d4x_prefs_get_field(MY_GTK_FILESEL(D4XPWS.exec_on_exit)->combo,
				    &TMPCFG.EXEC_WHEN_QUIT,
				    ALL_HISTORIES[EXEC_HISTORY]);
		return;
	};
	if (equal(label,_("Clipboard"))){
		TMPCFG.CLIPBOARD_SKIP_OR_CATCH=GTK_TOGGLE_BUTTON(D4XPWS.clipboard_catch_button)->active;
		TMPCFG.CLIPBOARD_MONITOR=GTK_TOGGLE_BUTTON(D4XPWS.clipboard_monitor)->active;
		d4x_prefs_get_field(D4XPWS.clipboard_skip,
				    &TMPCFG.SKIP_IN_CLIPBOARD,
				    ALL_HISTORIES[SKIP_HISTORY]);
		d4x_prefs_get_field(D4XPWS.clipboard_catch,
				    &TMPCFG.CATCH_IN_CLIPBOARD,
				    ALL_HISTORIES[SKIP_HISTORY]);
		return;
	};
	if (equal(label,_("Main"))){
		sscanf(gtk_entry_get_text(GTK_ENTRY(D4XPWS.speed_limit_1)),"%u",&TMPCFG.SPEED_LIMIT_1);
		sscanf(gtk_entry_get_text(GTK_ENTRY(D4XPWS.speed_limit_2)),"%u",&TMPCFG.SPEED_LIMIT_2);
		sscanf(gtk_entry_get_text(GTK_ENTRY(D4XPWS.max_threads)),"%u",&TMPCFG.MAX_THREADS);
		sscanf(gtk_entry_get_text(GTK_ENTRY(D4XPWS.save_list_entry)),"%u",&TMPCFG.SAVE_LIST_INTERVAL);
		TMPCFG.SAVE_LIST=GTK_TOGGLE_BUTTON(D4XPWS.save_list_check)->active;
		TMPCFG.WRITE_DESCRIPTION=GTK_TOGGLE_BUTTON(D4XPWS.description)->active;
		TMPCFG.DELETE_FATAL=GTK_TOGGLE_BUTTON(D4XPWS.del_fataled)->active;
		TMPCFG.DELETE_COMPLETED=GTK_TOGGLE_BUTTON(D4XPWS.del_completed)->active;
		TMPCFG.ALLOW_FORCE_RUN=GTK_TOGGLE_BUTTON(D4XPWS.allow_force_run)->active;
		TMPCFG.REMEMBER_PASS=GTK_TOGGLE_BUTTON(D4XPWS.remember_pass)->active;
		return;
	};
	if (equal(label,_("Main log"))){
		sscanf(gtk_entry_get_text(GTK_ENTRY(D4XPWS.log_length)),"%u",&TMPCFG.MAX_MAIN_LOG_LENGTH);
		sscanf(gtk_entry_get_text(GTK_ENTRY(D4XPWS.log_fsize)),"%li",&TMPCFG.MAIN_LOG_FILE_LIMIT);
		TMPCFG.MAIN_LOG_DETAILED=GTK_TOGGLE_BUTTON(D4XPWS.log_detailed)->active;
		TMPCFG.APPEND_REWRITE_LOG=GTK_TOGGLE_BUTTON(D4XPWS.log_append)->active;
		TMPCFG.SAVE_MAIN_LOG=GTK_TOGGLE_BUTTON(D4XPWS.log_save)->active;
		d4x_prefs_get_field(MY_GTK_FILESEL(D4XPWS.log_save_path)->combo,
				    &TMPCFG.SAVE_LOG_PATH,
				    ALL_HISTORIES[LOG_HISTORY]);
		return;
	};
};

void d4x_prefs_apply(){
	d4x_prefs_apply_tmp();
	int need_reinit_log=0;
	int need_reinit_graph=0;
 	if (TMPCFG.SAVE_LOG_PATH==NULL || CFG.SAVE_LOG_PATH==NULL ||
	    strcmp(TMPCFG.SAVE_LOG_PATH,CFG.SAVE_LOG_PATH) ||
	    CFG.SAVE_MAIN_LOG!=TMPCFG.SAVE_MAIN_LOG)
		need_reinit_log=1;
	if (TMPCFG.GRAPH_BACK!=CFG.GRAPH_BACK ||
	    TMPCFG.GRAPH_FORE1!=CFG.GRAPH_FORE1 ||
	    TMPCFG.GRAPH_FORE2!=CFG.GRAPH_FORE2 ||
	    TMPCFG.GRAPH_PICK!=CFG.GRAPH_PICK)
		need_reinit_graph=1;

	var_copy_cfg(&CFG,&TMPCFG);
	var_check_all_limits();
	if (need_reinit_graph)
		my_gtk_graph_cmap_reinit(GLOBAL_GRAPH);
	if (need_reinit_log)
 		aa.reinit_main_log();
	D4XPWS.columns_order.apply_changes();
	buttons_speed_set_text();
	save_config();
};

void d4x_prefs_ok(){
	d4x_prefs_apply();
	d4x_prefs_cancel();
};
