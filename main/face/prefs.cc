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
#include "dndtrash.h"
#include "../config.h"
#include "../sndserv.h"
#include <dirent.h>
#include <sys/stat.h>
#include "../xml.h"

const int SEARCH_NUMBER_OF_ENGINES=2;

extern tMain aa;
GtkWidget *d4x_prefs_window=(GtkWidget *)NULL;
GtkWidget *d4x_prefs_frame=(GtkWidget *)NULL;
/* initialisation only for NULL in 'char*' */
tMainCfg TMPCFG={
	{300,5,0,100,0,1,0,0,
	 0,0,0,0,0,1,1,1,0,0,0,0,1,0,0,
	 0,0},
	100,NULL,NULL,NULL,NULL,NULL,NULL,0,
	100,0,0,0,NULL,0,0, //Log
	5,0, //List
	1,600,0,0, //flags
	1,0,0,40,40,500,400,300,300,1,0,1,0,20,30,0,5,1,1,0,0,100,0,//interface
	0,1,NULL,NULL, //clipboard
	0xFFFFFF,0x555555,0xAAAAAA,0,0,
	/* Proxy */
	NULL,0,NULL,NULL,1,NULL,0,NULL,NULL,0,0,0,0,0,
	/* SOCKS */
	NULL,0,NULL,NULL,
	1,1,1,1,1,1,
	3,1024,10*1024,
	NULL,0,
	0x0FFFFFFF,
	0,0,1,
	1,0,15,
	1,0,(char*)NULL,(char*)NULL,(char*)NULL,(char*)NULL,(char*)NULL,(char*)NULL
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
	GtkWidget *change_links_check;
	GtkWidget *permissions;
	/* FTP */
	GtkWidget *ftp_passive_check;
	GtkWidget *dont_send_quit_check;
	GtkWidget *permisions_check;
	GtkWidget *follow_link_check;
	GtkWidget *ftp_dir_in_log;
	GtkWidget *ftp_dirontop;
	GtkWidget *ftp_recurse_depth_entry;
	GtkWidget *ftp_anonymous_pass;
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
	GtkWidget *ihate_etag_check;
	GtkWidget *http_recurse_depth_entry;
	GtkWidget *user_agent_entry;
	GtkWidget *unknown_filename;
	GtkWidget *default_filter;
	d4xFilterSel *filter_sel;
	/* PROXY */
	tProxyWidget proxy;
	/* Main window */
	GtkWidget *mw_use_title;
	GtkWidget *mw_use_title2;
	GtkWidget *mw_scroll_title;
	GtkWidget *window_lower;
	GtkWidget *winpos;
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
	/* MAIN */
	GtkWidget *allow_force_run;
	GtkWidget *remember_pass;
	GtkWidget *description;
	GtkWidget *save_list_entry;
	GtkWidget *save_list_check;
	GtkWidget *speed_limit_1;
	GtkWidget *speed_limit_2;
	/* FTP SEARCH */
	GtkWidget *search_ping_times;
	GtkWidget *search_host;
	GtkWidget *search_entries;
	/* INTERFACE */
	GtkWidget *dnd_trash;
	GtkWidget *fixed_font_log;
	/* GRAPH */
	GtkWidget *graph_order;
	GtkWidget *graph_mode;
	GtkWidget *speed_color_pick;
	GtkWidget *speed_color_fore1;
	GtkWidget *speed_color_fore2;
	GtkWidget *speed_color_back;
	/* SOUNDS */
	GtkWidget *snd_enable;
	GtkWidget *esd_sound;
	GtkWidget *snd_dnd_drop;
	GtkWidget *snd_startup;
	GtkWidget *snd_add;
	GtkWidget *snd_complete;
	GtkWidget *snd_fail;
	GtkWidget *snd_queue_finish;
	/* THEMES */
	GtkWidget *themes_list;
	GtkWidget *theme_info;
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
static void prefs_filter_sel_delete();

gint d4x_prefs_cancel() {
	if (d4x_prefs_window){
		gtk_widget_destroy(d4x_prefs_window);
		d4x_prefs_window=(GtkWidget *)NULL;
		if (D4XPWS.filter_sel)
			prefs_filter_sel_delete();
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

	GtkWidget *tbox=gtk_hbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(tbox),5);
	D4XPWS.permissions=my_gtk_entry_new_with_max_length(3,TMPCFG.DEFAULT_PERMISIONS);
	gtk_box_pack_start(GTK_BOX(tbox),D4XPWS.permissions,FALSE,FALSE,0);
	GtkWidget *tlabel=gtk_label_new(_("Default permissions of local file"));
	gtk_box_pack_start(GTK_BOX(tbox),tlabel,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(tmpbox),tbox,FALSE,FALSE,0);


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

	D4XPWS.follow_link_check=gtk_check_button_new_with_label(_("Follow symbolic links"));
	GTK_TOGGLE_BUTTON(D4XPWS.follow_link_check)->active=TMPCFG.DEFAULT_CFG.follow_link;
	gtk_box_pack_start(GTK_BOX(tmpbox),D4XPWS.follow_link_check,FALSE,FALSE,0);

	D4XPWS.ftp_dirontop=gtk_check_button_new_with_label(_("Put directories on the top of queue during recursion"));
	GTK_TOGGLE_BUTTON(D4XPWS.ftp_dirontop)->active=TMPCFG.DEFAULT_CFG.ftp_dirontop;
	gtk_box_pack_start(GTK_BOX(tmpbox),D4XPWS.ftp_dirontop,FALSE,FALSE,0);

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

	GtkWidget *other_box=gtk_hbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(other_box),5);
	D4XPWS.ftp_anonymous_pass=gtk_entry_new_with_max_length(256);
	if (TMPCFG.ANONYMOUS_PASS)
		text_to_combo(D4XPWS.ftp_anonymous_pass,TMPCFG.ANONYMOUS_PASS);
	else
		text_to_combo(D4XPWS.ftp_anonymous_pass,"-mdem@chat.ru");
	other_label=gtk_label_new(_("default anonymous password"));
	gtk_box_pack_start(GTK_BOX(other_box),D4XPWS.ftp_anonymous_pass,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(other_box),other_label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(tmpbox),other_box,FALSE,FALSE,0);


	gtk_widget_show_all(tmpbox);
};

static void prefs_filter_sel_delete(){
	gtk_widget_destroy(GTK_WIDGET(D4XPWS.filter_sel));
	D4XPWS.filter_sel=(d4xFilterSel *)NULL;
};

static void prefs_filter_sel_ok(){
	GList *select=(GTK_CLIST(D4XPWS.filter_sel->clist))->selection_end;
	if (select) {
		char *name;
		gint row=GPOINTER_TO_INT(select->data);
		gtk_clist_get_text(GTK_CLIST(D4XPWS.filter_sel->clist),
				   row,0,&name);
		text_to_combo(D4XPWS.default_filter,name);
	}else{
		text_to_combo(D4XPWS.default_filter,"");
	};
	prefs_filter_sel_delete();
};

static void prefs_filter_sel_select(GtkWidget *clist, gint row, gint column,
				    GdkEventButton *event,
				    tDEdit *where) {
	if (event && event->type==GDK_2BUTTON_PRESS && event->button==1)
		prefs_filter_sel_ok();
};

static void prefs_filter_sel_clicked(GtkWidget *parent){
	if (D4XPWS.filter_sel){
		gdk_window_show(GTK_WIDGET(D4XPWS.filter_sel)->window);
		return;
	};
	D4XPWS.filter_sel=(d4xFilterSel*)d4x_filter_sel_new();
	gtk_window_set_modal(GTK_WINDOW(D4XPWS.filter_sel),TRUE);
	gtk_window_set_transient_for(GTK_WINDOW(D4XPWS.filter_sel),GTK_WINDOW(d4x_prefs_window));
	gtk_signal_connect(GTK_OBJECT(D4XPWS.filter_sel->clist),
			   "select_row",
			   GTK_SIGNAL_FUNC(prefs_filter_sel_select),
			   NULL);
	gtk_signal_connect(GTK_OBJECT(D4XPWS.filter_sel->ok),
			   "clicked",
			   GTK_SIGNAL_FUNC(prefs_filter_sel_ok),
			   NULL);
	gtk_signal_connect(GTK_OBJECT(D4XPWS.filter_sel->cancel),
			   "clicked",
			   GTK_SIGNAL_FUNC(prefs_filter_sel_delete),
			   NULL);
	gtk_signal_connect(GTK_OBJECT(D4XPWS.filter_sel),
			   "delete_event",
			   GTK_SIGNAL_FUNC(prefs_filter_sel_delete),
			   NULL);
};

void d4x_prefs_download_http(){
	GtkWidget *tmpbox=d4x_prefs_child_destroy(_("HTTP"));

	D4XPWS.leave_dir_check=gtk_check_button_new_with_label(_("Only subdirs"));
	D4XPWS.leave_server_check=gtk_check_button_new_with_label(_("Allow leave this server while recursing via HTTP"));
	D4XPWS.change_links_check=gtk_check_button_new_with_label(_("Change links in HTML file to local"));
	D4XPWS.ihate_etag_check=gtk_check_button_new_with_label(_("Ignore ETag field in reply"));
	GTK_TOGGLE_BUTTON(D4XPWS.leave_server_check)->active=TMPCFG.DEFAULT_CFG.leave_server;
	GTK_TOGGLE_BUTTON(D4XPWS.leave_dir_check)->active=TMPCFG.DEFAULT_CFG.dont_leave_dir;
	GTK_TOGGLE_BUTTON(D4XPWS.change_links_check)->active=TMPCFG.DEFAULT_CFG.change_links;
	GTK_TOGGLE_BUTTON(D4XPWS.ihate_etag_check)->active=TMPCFG.DEFAULT_CFG.ihate_etag;
	gtk_box_pack_start(GTK_BOX(tmpbox),D4XPWS.leave_server_check,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(tmpbox),D4XPWS.leave_dir_check,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(tmpbox),D4XPWS.change_links_check,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(tmpbox),D4XPWS.ihate_etag_check,FALSE,FALSE,0);

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

	D4XPWS.default_filter=gtk_entry_new();
	gtk_entry_set_editable(GTK_ENTRY(D4XPWS.default_filter),FALSE);
	if (CFG.DEFAULT_FILTER)
		text_to_combo(D4XPWS.default_filter,CFG.DEFAULT_FILTER);
	http_hbox=gtk_hbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(http_hbox),5);
	other_label=gtk_label_new(_("Filter"));
	GtkWidget *button=gtk_button_new_with_label(_("Select"));
 	gtk_signal_connect(GTK_OBJECT(button),"clicked",
			   GTK_SIGNAL_FUNC(prefs_filter_sel_clicked),NULL);
	gtk_box_pack_start(GTK_BOX(http_hbox),other_label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(http_hbox),D4XPWS.default_filter,TRUE,TRUE,0);
	gtk_box_pack_start(GTK_BOX(http_hbox),button,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(tmpbox),http_hbox,FALSE,FALSE,0);
	
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
	D4XPWS.winpos=gtk_check_button_new_with_label(_("Do not remember position of the main window"));
	gtk_box_pack_start(GTK_BOX(tmpbox),D4XPWS.winpos,FALSE,FALSE,0);

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(D4XPWS.mw_use_title),TMPCFG.USE_MAINWIN_TITLE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(D4XPWS.mw_use_title2),TMPCFG.USE_MAINWIN_TITLE2);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(D4XPWS.mw_scroll_title),TMPCFG.SCROLL_MAINWIN_TITLE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(D4XPWS.window_lower),TMPCFG.WINDOW_LOWER);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(D4XPWS.winpos),TMPCFG.DONOTSET_WINPOS);
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
	D4XPWS.log_length=my_gtk_entry_new_with_max_length(4,TMPCFG.MAX_MAIN_LOG_LENGTH);
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

static void d4x_prefs_toggle_save_list(GtkWidget *parent) {
	gtk_widget_set_sensitive(D4XPWS.save_list_entry,GTK_TOGGLE_BUTTON(parent)->active);
};

void d4x_prefs_main(){
	GtkWidget *vbox=d4x_prefs_child_destroy(_("Main"));

	GtkWidget *prefs_limits_tbox=gtk_hbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(prefs_limits_tbox),5);

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
	
	D4XPWS.allow_force_run=gtk_check_button_new_with_label(_("Allow to user force run downloads"));
	D4XPWS.remember_pass=gtk_check_button_new_with_label(_("Remember passwords"));
	D4XPWS.description=gtk_check_button_new_with_label(_("Write Descript.ion file"));
	GTK_TOGGLE_BUTTON(D4XPWS.allow_force_run)->active=TMPCFG.ALLOW_FORCE_RUN;
	GTK_TOGGLE_BUTTON(D4XPWS.remember_pass)->active=TMPCFG.REMEMBER_PASS;
	GTK_TOGGLE_BUTTON(D4XPWS.description)->active=TMPCFG.WRITE_DESCRIPTION;
	gtk_box_pack_start(GTK_BOX(vbox),D4XPWS.allow_force_run,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),D4XPWS.remember_pass,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),D4XPWS.description,FALSE,FALSE,0);

#include "pixmaps/speed1.xpm"
#include "pixmaps/speed2.xpm"
	GtkWidget *label=gtk_label_new(_("bytes/sec speed level one (red button)"));
	GtkWidget *hbox=gtk_hbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(hbox),5);
	D4XPWS.speed_limit_1=my_gtk_entry_new_with_max_length(5,TMPCFG.SPEED_LIMIT_1);
	gtk_box_pack_start(GTK_BOX(hbox),new_pixmap(speed1_xpm,NULL),FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(hbox),D4XPWS.speed_limit_1,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);
	label=gtk_label_new(_("bytes/sec speed level two (yellow button)"));
	hbox=gtk_hbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(hbox),5);
	D4XPWS.speed_limit_2=my_gtk_entry_new_with_max_length(5,TMPCFG.SPEED_LIMIT_2);
	gtk_box_pack_start(GTK_BOX(hbox),new_pixmap(speed2_xpm,NULL),FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(hbox),D4XPWS.speed_limit_2,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);
	
//	GTK_TOGGLE_BUTTON(D4XPWS.)->active=TMPCFG.;
	gtk_widget_show_all(vbox);
};

void d4x_prefs_search(){
	GtkWidget *vbox=d4x_prefs_child_destroy(_("FTP search"));

	GtkWidget *box=gtk_hbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(box),5);
	D4XPWS.search_ping_times=my_gtk_entry_new_with_max_length(3,TMPCFG.SEARCH_PING_TIMES);
	gtk_box_pack_start(GTK_BOX(box),D4XPWS.search_ping_times,FALSE,FALSE,0);
	GtkWidget *label=gtk_label_new(_("Number of attempts to ping hosts"));
	gtk_box_pack_start(GTK_BOX(box),label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),box,FALSE,FALSE,0);

	box=gtk_hbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(box),5);
	D4XPWS.search_entries=my_gtk_entry_new_with_max_length(3,TMPCFG.SEARCH_ENTRIES);
	gtk_box_pack_start(GTK_BOX(box),D4XPWS.search_entries,FALSE,FALSE,0);
	label=gtk_label_new(_("Number of hosts in list"));
	gtk_box_pack_start(GTK_BOX(box),label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),box,FALSE,FALSE,0);

	char *labels[]={
		"archie.is.co.za",
		"www.filesearch.ru"
	};
	D4XPWS.search_host=gtk_option_menu_new();
	GtkWidget *menu=gtk_menu_new ();
	GtkWidget *menu_item;
	GSList *group=(GSList *)NULL;
	gint i;
	for (i = 0; (unsigned int)i <sizeof(labels)/sizeof(char*); i++){
		menu_item = gtk_radio_menu_item_new_with_label (group, labels[i]);
		group = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (menu_item));
		gtk_menu_append (GTK_MENU (menu), menu_item);
		if (i==TMPCFG.SEARCH_HOST)
			gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (menu_item), TRUE);
		gtk_widget_show (menu_item);
	};
	gtk_option_menu_set_menu (GTK_OPTION_MENU (D4XPWS.search_host), menu);
	gtk_option_menu_set_history (GTK_OPTION_MENU (D4XPWS.search_host),TMPCFG.SEARCH_HOST);

	box=gtk_hbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(box),5);
	label=gtk_label_new(_("search engine to use"));
	gtk_box_pack_start(GTK_BOX(box),D4XPWS.search_host,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(box),label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),box,FALSE,FALSE,0);

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
	D4XPWS.graph_mode=gtk_check_button_new_with_label(_("Compressed mode"));
	GTK_TOGGLE_BUTTON(D4XPWS.graph_mode)->active=TMPCFG.GRAPH_MODE;
	gtk_box_pack_start(GTK_BOX(vbox),D4XPWS.graph_mode,FALSE,FALSE,0);

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

#define SND_ENTRY_INIT(a,b) { 					\
	a=my_gtk_filesel_new(ALL_HISTORIES[SOUNDS_HISTORY]);	\
	gtk_widget_set_usize(a,320,-1);				\
	MY_GTK_FILESEL(a)->modal=GTK_WINDOW(d4x_prefs_window);	\
	if (b)							\
		text_to_combo(MY_GTK_FILESEL(a)->combo,b);	\
	else							\
		text_to_combo(MY_GTK_FILESEL(a)->combo,"");	\
	gtk_box_pack_start(GTK_BOX(vbox),a,TRUE,FALSE,0);	\
}

#define SND_LABEL_INIT(a){					\
	label=gtk_label_new(_(a));				\
	gtk_box_pack_start(GTK_BOX(vbox),label,FALSE,FALSE,0);	\
}

void d4x_prefs_sounds(){
	GtkWidget *vbox=d4x_prefs_child_destroy(_("Sounds"));

	GtkWidget *hbox=gtk_hbox_new(FALSE,0);
	D4XPWS.snd_enable=gtk_check_button_new_with_label(_("enable sounds"));
	GTK_TOGGLE_BUTTON(D4XPWS.snd_enable)->active=TMPCFG.ENABLE_SOUNDS;
	gtk_box_pack_start(GTK_BOX(hbox),D4XPWS.snd_enable,FALSE,FALSE,0);

#ifdef D4X_WITH_ESD
	D4XPWS.esd_sound=gtk_check_button_new_with_label(_("output via ESD"));
	GTK_TOGGLE_BUTTON(D4XPWS.esd_sound)->active=TMPCFG.ESD_SOUND;
	gtk_box_pack_start(GTK_BOX(hbox),D4XPWS.esd_sound,FALSE,FALSE,0);
#endif// D4X_WITH_ESD
	gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);

	GtkWidget *label;
	SND_LABEL_INIT(_("Startup"));
	SND_ENTRY_INIT(D4XPWS.snd_startup,TMPCFG.SOUND_STARTUP);
	SND_LABEL_INIT(_("Adding a download"));
	SND_ENTRY_INIT(D4XPWS.snd_add,TMPCFG.SOUND_ADD);
	SND_LABEL_INIT(_("Downloading completed"));
	SND_ENTRY_INIT(D4XPWS.snd_complete,TMPCFG.SOUND_COMPLETE);
	SND_LABEL_INIT(_("Downloading failed"));
	SND_ENTRY_INIT(D4XPWS.snd_fail,TMPCFG.SOUND_FAIL);
	SND_LABEL_INIT(_("Downloading of queue is completed"));
	SND_ENTRY_INIT(D4XPWS.snd_queue_finish,TMPCFG.SOUND_QUEUE_FINISH);
	SND_LABEL_INIT(_("Drag'n'Drop event"));
	SND_ENTRY_INIT(D4XPWS.snd_dnd_drop,TMPCFG.SOUND_DND_DROP);
	
	gtk_widget_show_all(vbox);

};

static void d4x_prefs_themes_select_row(GtkWidget *clist, gint row, gint column,
					GdkEventButton *event, gpointer nothing){
	int textlen=gtk_text_get_length(GTK_TEXT(D4XPWS.theme_info));
	gtk_text_backward_delete(GTK_TEXT(D4XPWS.theme_info),textlen);
	if (row==0){
		gtk_text_insert(GTK_TEXT(D4XPWS.theme_info),
				NULL,NULL,NULL,
				" ",1);
		TMPCFG.USE_THEME=0;
	}else{
		char *text;
		gtk_clist_get_text(GTK_CLIST(D4XPWS.themes_list),row,0,&text);
		char *path=sum_strings(D4X_SHARE_PATH,"/themes/",text,".xml",NULL);
		tQueue *q=d4x_xml_parse_file(path);
		delete[] path;
		d4xXmlObject *info=d4x_xml_find_obj(q,"info");
		gtk_text_backward_delete(GTK_TEXT(D4XPWS.theme_info),textlen);
		if (info && info->value.get()){
			d4xXmlField *fld=info->get_attr("author");
			if (fld && fld->value.get()){
				gtk_text_insert(GTK_TEXT(D4XPWS.theme_info),
						NULL,NULL,NULL,"Author: ",strlen("Author: "));
				gtk_text_insert(GTK_TEXT(D4XPWS.theme_info),
						NULL,NULL,NULL,fld->value.get(),strlen(fld->value.get()));
				fld=info->get_attr("email");
				if (fld && fld->value.get()){
					gtk_text_insert(GTK_TEXT(D4XPWS.theme_info),
							NULL,NULL,NULL," <",strlen(" <"));
					gtk_text_insert(GTK_TEXT(D4XPWS.theme_info),
							NULL,NULL,NULL,fld->value.get(),strlen(fld->value.get()));
					gtk_text_insert(GTK_TEXT(D4XPWS.theme_info),
							NULL,NULL,NULL,">",strlen(">"));
				};
				gtk_text_insert(GTK_TEXT(D4XPWS.theme_info),
						NULL,NULL,NULL,"\n",strlen("\n"));
			};
			gtk_text_insert(GTK_TEXT(D4XPWS.theme_info),
					NULL,NULL,NULL,info->value.get(),strlen(info->value.get()));
		}else
			gtk_text_insert(GTK_TEXT(D4XPWS.theme_info),
					NULL,NULL,NULL,
					_("No info about this theme."),
					strlen(_("No info about this theme.")));
		if (q) delete(q);
		if (TMPCFG.THEME_FILE) delete[] TMPCFG.THEME_FILE;
		TMPCFG.THEME_FILE=copy_string(text);
		TMPCFG.USE_THEME=1;
	};
};

void d4x_prefs_themes(){
	GtkWidget *vbox=d4x_prefs_child_destroy(_("Themes"));
	char *titles[]={"Name"};
	char *data[1];
	data[0]=_("Default theme");
	D4XPWS.themes_list = gtk_clist_new_with_titles(1, titles);
	gtk_box_pack_start(GTK_BOX(vbox),D4XPWS.themes_list,TRUE,TRUE,0);
	gtk_signal_connect(GTK_OBJECT(D4XPWS.themes_list), "select_row",
	                   GTK_SIGNAL_FUNC(d4x_prefs_themes_select_row),NULL);
	gtk_clist_append(GTK_CLIST(D4XPWS.themes_list),data);
	char *path=sum_strings(D4X_SHARE_PATH,"/themes/",NULL);
	D4XPWS.theme_info=gtk_text_new(NULL,NULL);
	gtk_box_pack_start(GTK_BOX(vbox),D4XPWS.theme_info,TRUE,FALSE,0);
	gtk_text_set_editable(GTK_TEXT(D4XPWS.theme_info),FALSE);
	DIR *d=opendir(path);
	if (d){
		struct dirent *de=NULL;
		while((de=readdir(d))){
			if (de->d_name && strlen(de->d_name)>4 &&
			    string_ended(".xml",de->d_name)==0){
				char *tmppath=sum_strings(path,"/",de->d_name,NULL);
				struct stat s;
				stat(tmppath,&s);
				if (S_ISREG(s.st_mode)){
					char *a=rindex(de->d_name,'.');
					if (a)
						*a=0;
					data[0]=de->d_name;
					int row=gtk_clist_append(GTK_CLIST(D4XPWS.themes_list),data);
					if (CFG.THEME_FILE && equal(CFG.THEME_FILE,de->d_name))
						gtk_clist_select_row(GTK_CLIST(D4XPWS.themes_list),row,0);
					if (a)
						*a='.';
				};
				delete[] tmppath;
			};
		};
	};
	delete[] path;
	if (CFG.USE_THEME==0)
		gtk_clist_select_row(GTK_CLIST(D4XPWS.themes_list),0,0);
	gtk_widget_show_all(vbox);
};

static int d4x_prefs_w=0,d4x_prefs_h=0;
static int d4x_prefs_first=1;
static int d4x_prefs_page=0;

void d4x_prefs_window_configure(GtkWidget *window){
	gint w=0,h=0;
	gdk_window_get_size(window->window,&w,&h);
	int wc=0,hc=0;
	if (w>d4x_prefs_w){
		d4x_prefs_w=w;
		wc=1;
	};
	if (h>d4x_prefs_h){
		d4x_prefs_h=h;
		hc=1;
	};
	switch (d4x_prefs_first){
	case 1:
		gtk_tree_select_item(GTK_TREE(D4XPWS.tree_interface),3);
		d4x_prefs_first=2;
		break;
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:{
		if (!hc){
		    d4x_prefs_first+=1;
		    break;
		};
	};
	case 8:{
		d4x_prefs_first=0;
		gtk_widget_set_usize(GTK_WIDGET (d4x_prefs_window),
				     d4x_prefs_w,d4x_prefs_h);
		switch (d4x_prefs_page){
		case PREFS_PAGE_MAINLOG:
			gtk_tree_select_item(GTK_TREE(D4XPWS.tree_main),0);
			break;
		case PREFS_PAGE_MAIN:
			gtk_tree_select_item(GTK_TREE(D4XPWS.root_tree),3);
			break;
		};
	};
	};
//	printf("Conf:%i %i\n",w,h);
};

void d4x_prefs_init_pre(){
	if (d4x_prefs_window) {
		gdk_window_show(d4x_prefs_window->window);
		return;
	};
	var_copy_cfg(&TMPCFG,&CFG);
	/* create preferences window */
	d4x_prefs_window=gtk_window_new(GTK_WINDOW_DIALOG);
	gtk_window_set_wmclass(GTK_WINDOW(d4x_prefs_window),
			       "D4X_Preferences","D4X");
	gtk_window_set_position(GTK_WINDOW(d4x_prefs_window),GTK_WIN_POS_NONE);
	gtk_window_set_policy (GTK_WINDOW(d4x_prefs_window), FALSE,FALSE,FALSE);
	gtk_window_set_title(GTK_WINDOW(d4x_prefs_window),_("Options"));
	if (d4x_prefs_first==0 && d4x_prefs_w && d4x_prefs_h){
		gtk_widget_set_usize(GTK_WIDGET (d4x_prefs_window),
				     d4x_prefs_w,d4x_prefs_h);
	};

	gtk_signal_connect(GTK_OBJECT(d4x_prefs_window), "key_press_event",
			   (GtkSignalFunc)d4x_prefs_esc_handler, NULL);
	gtk_signal_connect(GTK_OBJECT(d4x_prefs_window),"delete_event",
			   GTK_SIGNAL_FUNC(d4x_prefs_cancel), NULL);
	gtk_signal_connect(GTK_OBJECT(d4x_prefs_window), "configure_event",
	                   GTK_SIGNAL_FUNC(d4x_prefs_window_configure),
	                   d4x_prefs_window);
	
	/* first box inside window */
	GtkWidget *tmphbox=gtk_hbox_new(FALSE,0);
	GtkWidget *tmpvbox=gtk_vbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(tmphbox),5);
	gtk_box_set_spacing(GTK_BOX(tmpvbox),5);
	gtk_container_border_width(GTK_CONTAINER(d4x_prefs_window),5);
	gtk_container_add(GTK_CONTAINER(d4x_prefs_window),tmpvbox);
	/* container for tree */
	GtkWidget *scroll_win=gtk_scrolled_window_new((GtkAdjustment*)NULL,(GtkAdjustment*)NULL);
	gtk_widget_set_usize(scroll_win,150,-1);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll_win),
					GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_box_pack_start (GTK_BOX (tmphbox), scroll_win, FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (tmpvbox), tmphbox, TRUE, TRUE, 0);
	gtk_widget_show (scroll_win);
	/* containder for all other */
	d4x_prefs_frame=gtk_frame_new("test");
//	gtk_widget_set_usize(d4x_prefs_frame,480,-1);
	gtk_box_pack_start (GTK_BOX (tmphbox), d4x_prefs_frame, TRUE, TRUE, 0);
	gtk_container_border_width(GTK_CONTAINER(d4x_prefs_frame),5);
	/* create tree of options */
	GtkWidget *root_tree=D4XPWS.root_tree=gtk_tree_new();
//	gtk_widget_set_usize(root_tree,150,-1);
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
	tmpitem=gtk_tree_item_new_with_label(_("Sounds"));
	gtk_signal_connect(GTK_OBJECT(tmpitem), "select",
			   (GtkSignalFunc)d4x_prefs_sounds, NULL);
	gtk_tree_append(GTK_TREE(sub_tree), tmpitem);
	gtk_widget_show(tmpitem);
	tmpitem=gtk_tree_item_new_with_label(_("Themes"));
	gtk_signal_connect(GTK_OBJECT(tmpitem), "select",
			   (GtkSignalFunc)d4x_prefs_themes, NULL);
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

	tmpitem=gtk_tree_item_new_with_label(_("FTP search"));
	gtk_signal_connect(GTK_OBJECT(tmpitem), "select",
			   (GtkSignalFunc)d4x_prefs_search, NULL);
	gtk_tree_append(GTK_TREE(sub_tree), tmpitem);
	gtk_widget_show(tmpitem);
	/* show window */

	GtkWidget *buttons_hbox=gtk_hbutton_box_new();
	gtk_button_box_set_layout(GTK_BUTTON_BOX(buttons_hbox),GTK_BUTTONBOX_END);
	gtk_button_box_set_spacing(GTK_BUTTON_BOX(buttons_hbox),5);
	gtk_box_pack_start (GTK_BOX (tmpvbox), buttons_hbox, FALSE, FALSE, 0);
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
	
	if (d4x_prefs_first)
		gtk_tree_select_item(GTK_TREE(D4XPWS.tree_download),3);
	

	gtk_widget_show_all(d4x_prefs_window);
};

void d4x_prefs_init_page(int page){
	d4x_prefs_page=page;
	d4x_prefs_init_pre();
	if (d4x_prefs_first) return;
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
	if (*where) delete[] (*where);
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
		sscanf(gtk_entry_get_text(GTK_ENTRY(D4XPWS.permissions)),"%u",&(TMPCFG.DEFAULT_PERMISIONS));
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
		TMPCFG.DEFAULT_CFG.follow_link=GTK_TOGGLE_BUTTON(D4XPWS.follow_link_check)->active;
		TMPCFG.FTP_DIR_IN_LOG=GTK_TOGGLE_BUTTON(D4XPWS.ftp_dir_in_log)->active;
		TMPCFG.DEFAULT_CFG.ftp_dirontop=GTK_TOGGLE_BUTTON(D4XPWS.ftp_dirontop)->active;
		sscanf(gtk_entry_get_text(GTK_ENTRY(D4XPWS.ftp_recurse_depth_entry)),"%u",&TMPCFG.DEFAULT_CFG.ftp_recurse_depth);
		if (TMPCFG.ANONYMOUS_PASS)
			delete[] TMPCFG.ANONYMOUS_PASS;
		TMPCFG.ANONYMOUS_PASS=copy_string(text_from_combo(D4XPWS.ftp_anonymous_pass));
		return;
	};
	if (equal(label,_("HTTP"))){
		TMPCFG.DEFAULT_CFG.leave_server=GTK_TOGGLE_BUTTON(D4XPWS.leave_server_check)->active;
		TMPCFG.DEFAULT_CFG.dont_leave_dir=GTK_TOGGLE_BUTTON(D4XPWS.leave_dir_check)->active;
		TMPCFG.DEFAULT_CFG.change_links=GTK_TOGGLE_BUTTON(D4XPWS.change_links_check)->active;
		TMPCFG.DEFAULT_CFG.ihate_etag=GTK_TOGGLE_BUTTON(D4XPWS.ihate_etag_check)->active;
//		TMPCFG.DEFAULT_CFG.=GTK_TOGGLE_BUTTON(D4XPWS.)->active;
		sscanf(gtk_entry_get_text(GTK_ENTRY(D4XPWS.http_recurse_depth_entry)),"%u",&TMPCFG.DEFAULT_CFG.http_recurse_depth);
		d4x_prefs_get_field(D4XPWS.user_agent_entry,
				    &TMPCFG.USER_AGENT,
				    ALL_HISTORIES[USER_AGENT_HISTORY]);
		d4x_prefs_get_field(MY_GTK_FILESEL(D4XPWS.unknown_filename)->combo,
				    &TMPCFG.DEFAULT_NAME,
				    ALL_HISTORIES[FILE_HISTORY]);
		if (TMPCFG.DEFAULT_FILTER)
			delete[] TMPCFG.DEFAULT_FILTER;
		TMPCFG.DEFAULT_FILTER=copy_string(text_from_combo(D4XPWS.default_filter));
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
	if (equal(label,_("Main window"))){
		TMPCFG.USE_MAINWIN_TITLE=GTK_TOGGLE_BUTTON(D4XPWS.mw_use_title)->active;
		TMPCFG.USE_MAINWIN_TITLE2=GTK_TOGGLE_BUTTON(D4XPWS.mw_use_title2)->active;
		TMPCFG.SCROLL_MAINWIN_TITLE=GTK_TOGGLE_BUTTON(D4XPWS.mw_scroll_title)->active;
		TMPCFG.WINDOW_LOWER=GTK_TOGGLE_BUTTON(D4XPWS.window_lower)->active;
		TMPCFG.DONOTSET_WINPOS=GTK_TOGGLE_BUTTON(D4XPWS.winpos)->active;
		return;
	};
	if (equal(label,_("Confirmation"))){
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
		TMPCFG.GRAPH_MODE=GTK_TOGGLE_BUTTON(D4XPWS.graph_mode)->active;
		return;
	};
	if (equal(label,_("Sounds"))){
		TMPCFG.ENABLE_SOUNDS=GTK_TOGGLE_BUTTON(D4XPWS.snd_enable)->active;
#ifdef D4X_WITH_ESD
		TMPCFG.ESD_SOUND=GTK_TOGGLE_BUTTON(D4XPWS.esd_sound)->active;
#endif //D4X_WITH_ESD
		d4x_prefs_get_field(MY_GTK_FILESEL(D4XPWS.snd_startup)->combo,
				    &TMPCFG.SOUND_STARTUP,
				    ALL_HISTORIES[SOUNDS_HISTORY]);
		d4x_prefs_get_field(MY_GTK_FILESEL(D4XPWS.snd_fail)->combo,
				    &TMPCFG.SOUND_FAIL,
				    ALL_HISTORIES[SOUNDS_HISTORY]);
		d4x_prefs_get_field(MY_GTK_FILESEL(D4XPWS.snd_complete)->combo,
				    &TMPCFG.SOUND_COMPLETE,
				    ALL_HISTORIES[SOUNDS_HISTORY]);
		d4x_prefs_get_field(MY_GTK_FILESEL(D4XPWS.snd_add)->combo,
				    &TMPCFG.SOUND_ADD,
				    ALL_HISTORIES[SOUNDS_HISTORY]);
		d4x_prefs_get_field(MY_GTK_FILESEL(D4XPWS.snd_dnd_drop)->combo,
				    &TMPCFG.SOUND_DND_DROP,
				    ALL_HISTORIES[SOUNDS_HISTORY]);
		d4x_prefs_get_field(MY_GTK_FILESEL(D4XPWS.snd_queue_finish)->combo,
				    &TMPCFG.SOUND_QUEUE_FINISH,
				    ALL_HISTORIES[SOUNDS_HISTORY]);
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
	if (equal(label,_("FTP search"))){
		sscanf(gtk_entry_get_text(GTK_ENTRY(D4XPWS.search_ping_times)),"%u",&TMPCFG.SEARCH_PING_TIMES);
		sscanf(gtk_entry_get_text(GTK_ENTRY(D4XPWS.search_entries)),"%u",&TMPCFG.SEARCH_ENTRIES);
		GSList *group=gtk_radio_menu_item_group((GtkRadioMenuItem *)((GtkOptionMenu *)D4XPWS.search_host)->menu_item);
		int i=SEARCH_NUMBER_OF_ENGINES-1;
		while(group && !((GtkCheckMenuItem *)(group->data))->active){
			group = group->next;
			i--;
		};
		TMPCFG.SEARCH_HOST=i;
		return;
	};
	if (equal(label,_("Main"))){
		sscanf(gtk_entry_get_text(GTK_ENTRY(D4XPWS.speed_limit_1)),"%u",&TMPCFG.SPEED_LIMIT_1);
		sscanf(gtk_entry_get_text(GTK_ENTRY(D4XPWS.speed_limit_2)),"%u",&TMPCFG.SPEED_LIMIT_2);
		sscanf(gtk_entry_get_text(GTK_ENTRY(D4XPWS.save_list_entry)),"%u",&TMPCFG.SAVE_LIST_INTERVAL);
		TMPCFG.SAVE_LIST=GTK_TOGGLE_BUTTON(D4XPWS.save_list_check)->active;
		TMPCFG.WRITE_DESCRIPTION=GTK_TOGGLE_BUTTON(D4XPWS.description)->active;
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
	int need_reinit_dnd=0;
 	if (TMPCFG.SAVE_LOG_PATH==NULL || CFG.SAVE_LOG_PATH==NULL ||
	    strcmp(TMPCFG.SAVE_LOG_PATH,CFG.SAVE_LOG_PATH) ||
	    CFG.SAVE_MAIN_LOG!=TMPCFG.SAVE_MAIN_LOG)
		need_reinit_log=1;
	if (TMPCFG.GRAPH_BACK!=CFG.GRAPH_BACK ||
	    TMPCFG.GRAPH_FORE1!=CFG.GRAPH_FORE1 ||
	    TMPCFG.GRAPH_FORE2!=CFG.GRAPH_FORE2 ||
	    TMPCFG.GRAPH_PICK!=CFG.GRAPH_PICK)
		need_reinit_graph=1;
	if (TMPCFG.USE_THEME!=CFG.USE_THEME ||
	    !equal(TMPCFG.THEME_FILE,CFG.THEME_FILE))
		need_reinit_dnd=1;
	var_copy_cfg(&CFG,&TMPCFG);
	var_check_all_limits();
	if (need_reinit_graph)
		my_gtk_graph_cmap_reinit(GLOBAL_GRAPH);
	if (need_reinit_log)
 		aa.reinit_main_log();
	buttons_speed_set_text();
	dnd_trash_set_speed_text();
	save_config();
	SOUND_SERVER->reinit_sounds();
	if (D4X_THEME_DATA) delete(D4X_THEME_DATA);
	if (CFG.USE_THEME){
		char *path=sum_strings(D4X_SHARE_PATH,"/themes/",CFG.THEME_FILE,".xml",NULL);
		D4X_THEME_DATA=d4x_xml_parse_file(path);
		delete[] path;
	}else
		D4X_THEME_DATA=NULL;
	if (need_reinit_dnd){
		bb_theme_changed();
		dnd_trash_real_destroy();
		dnd_trash_destroy_theme();
		CFG.DND_TRASH=TMPCFG.DND_TRASH;
		gdk_window_show(d4x_prefs_window->window);
		buttons_theme_changed();
	};
	if (CFG.DND_TRASH){
		dnd_trash_init();
	}else
		dnd_trash_destroy();
	GlobalMeter->set_mode(CFG.GRAPH_MODE);
};

void d4x_prefs_ok(){
	d4x_prefs_apply();
	d4x_prefs_cancel();
};
