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
#include "prefs.h"
#include "edit.h"
#include "colors.h"
#include "misc.h"
#include "columns.h"
#include "dndtrash.h"
#include "../var.h"
#include "../main.h"
#include "../config.h"
#include "../locstr.h"
#include "../ntlocale.h"

extern tMain aa;

GtkWidget *options_window=NULL;
GtkWidget *options_window_notebook;
GtkWidget *options_window_vbox;

GtkWidget *dir_browser_button2;

GtkWidget *prefs_common_vbox;
GtkWidget *prefs_common_table;
GtkWidget *prefs_common_frame;
GtkWidget *prefs_common_del_completed;
GtkWidget *prefs_common_del_fataled;
GtkWidget *prefs_common_recursive;
GtkWidget *prefs_common_retry_if_noreget;
GtkWidget *prefs_common_ftp_permisions;
GtkWidget *prefs_common_save_list_check,*prefs_common_save_list_entry;
GtkWidget *prefs_common_use_title,*prefs_common_use_title2,*prefs_common_scroll_title;
GtkWidget *prefs_common_get_date;
GtkWidget *prefs_common_dnd_dialog;
GtkWidget *prefs_common_window_lower;
GtkWidget *prefs_common_graph_order;
GtkWidget *prefs_common_default_permisions;
GtkWidget *prefs_common_dnd_trash;

GtkWidget *prefs_limits_frame;
GtkWidget *prefs_limits_vbox;
GtkWidget *prefs_limits_table;
GtkWidget *prefs_limits_max_threads,*prefs_limits_tbox,*prefs_limits_tlabel;
GtkWidget *prefs_limits_timeout,*prefs_limits_tibox,*prefs_limits_tilabel;
GtkWidget *prefs_limits_log,*prefs_limits_lbox,*prefs_limits_llabel;
GtkWidget *prefs_limits_mlog,*prefs_limits_mlbox,*prefs_limits_mllabel;
GtkWidget *prefs_limits_retry_timeout,*prefs_limits_rtbox,*prefs_limits_rtlabel;
GtkWidget *prefs_limits_max_retries,*prefs_limits_rbox,*prefs_limits_rlabel;
GtkWidget *prefs_limits_ftp_recurse_depth,*prefs_limits_http_recurse_depth,*prefs_limits_rdbox,*prefs_limits_rdlabel;
GtkWidget *prefs_limits_rollback,*prefs_limits_rbbox,*prefs_limits_rblabel;

GtkWidget *prefs_other_frame;
GtkWidget *prefs_other_vbox;
GtkWidget *prefs_other_table;
GtkWidget *prefs_other_savepath,*prefs_other_sbox,*prefs_other_slabel;
GtkWidget *prefs_other_filename,*prefs_other_fbox,*prefs_other_flabel;
GtkWidget *prefs_other_ftp_passive;
GtkWidget *prefs_other_save_log;
GtkWidget *prefs_other_append_log,*prefs_other_rewrite_log;
GtkWidget *prefs_other_save_log_path;

GtkWidget *prefs_columns_nums_button1,*prefs_columns_nums_button2,*prefs_columns_nums_button3;
GtkWidget *prefs_columns_time_button1,*prefs_columns_time_button2;
tColumnsPrefs prefs_columns_order;

GtkWidget *prefs_confirm_delete,*prefs_confirm_delete_all,*prefs_confirm_delete_completed,*prefs_confirm_delete_fataled,*prefs_confirm_exit;

GtkWidget *prefs_speed_limit_1,*prefs_speed_limit_2;

tProxyWidget *prefs_proxy=NULL;

GtkWidget *dir_browser=NULL;
GtkWidget *dir_browser2=NULL;
GtkWidget *ok_button,*cancel_button;
GtkWidget *buttons_hbox;

static void options_browser_ok(...) {
	char *tmp;
	tmp=gtk_file_selection_get_filename(GTK_FILE_SELECTION(dir_browser));
	text_to_combo(prefs_other_savepath,tmp);
	gtk_widget_destroy(dir_browser);
	dir_browser=NULL;
};

static gint options_browser_cancel(...) {
	gtk_widget_destroy(dir_browser);
	dir_browser=NULL;
	return TRUE;
};

static void options_browser_open() {
	if (dir_browser) {
		gdk_window_show(dir_browser->window);
		return;
	};
	dir_browser=gtk_file_selection_new(_("Select directory"));
	gtk_widget_set_sensitive(GTK_FILE_SELECTION(dir_browser)->file_list,FALSE);
	char *tmp=text_from_combo(prefs_other_savepath);
	if (tmp && *tmp)
		gtk_file_selection_set_filename(GTK_FILE_SELECTION(dir_browser),tmp);
	gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(dir_browser)->ok_button),
	                   "clicked",GTK_SIGNAL_FUNC(options_browser_ok),prefs_other_savepath);
	gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(dir_browser)->cancel_button),
	                   "clicked",GTK_SIGNAL_FUNC(options_browser_cancel),dir_browser);
	gtk_signal_connect(GTK_OBJECT(&(GTK_FILE_SELECTION(dir_browser)->window)),
	                   "delete_event",GTK_SIGNAL_FUNC(options_browser_cancel),dir_browser);
	gtk_widget_show(dir_browser);
};

static void options_browser_ok2(...) {
	char *tmp;
	tmp=gtk_file_selection_get_filename(GTK_FILE_SELECTION(dir_browser2));
	text_to_combo(prefs_other_save_log_path,tmp);
	gtk_widget_destroy(dir_browser2);
	dir_browser2=NULL;
};

static gint options_browser_cancel2(...) {
	gtk_widget_destroy(dir_browser2);
	dir_browser2=NULL;
	return TRUE;
};

static void options_browser_open2() {
	if (dir_browser2) {
		gdk_window_show(dir_browser2->window);
		return;
	};
	dir_browser2=gtk_file_selection_new(_("Select file"));
	char *tmp=text_from_combo(prefs_other_save_log_path);
	if (tmp && *tmp)
		gtk_file_selection_set_filename(GTK_FILE_SELECTION(dir_browser2),tmp);
	gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(dir_browser2)->ok_button),
	                   "clicked",GTK_SIGNAL_FUNC(options_browser_ok2),prefs_other_savepath);
	gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(dir_browser2)->cancel_button),
	                   "clicked",GTK_SIGNAL_FUNC(options_browser_cancel2),dir_browser2);
	gtk_signal_connect(GTK_OBJECT(&(GTK_FILE_SELECTION(dir_browser2)->window)),
	                   "delete_event",GTK_SIGNAL_FUNC(options_browser_cancel2),dir_browser2);
	gtk_widget_show(dir_browser2);
};

static void options_toggle_save_log(GtkWidget *parent) {
	gtk_widget_set_sensitive(prefs_other_append_log,GTK_TOGGLE_BUTTON(parent)->active);
	gtk_widget_set_sensitive(prefs_other_rewrite_log,GTK_TOGGLE_BUTTON(parent)->active);
	gtk_widget_set_sensitive(prefs_other_save_log_path,GTK_TOGGLE_BUTTON(parent)->active);
	gtk_widget_set_sensitive(dir_browser_button2,GTK_TOGGLE_BUTTON(parent)->active);
};

static void options_toggle_save_list(GtkWidget *parent) {
	gtk_widget_set_sensitive(prefs_common_save_list_entry,GTK_TOGGLE_BUTTON(parent)->active);
};

static void options_toggle_title(GtkWidget *parent) {
	gtk_widget_set_sensitive(prefs_common_use_title2,GTK_TOGGLE_BUTTON(parent)->active);
	gtk_widget_set_sensitive(prefs_common_scroll_title,GTK_TOGGLE_BUTTON(parent)->active);
};

void init_options_window(...) {
	char temp[MAX_LEN];

	if (options_window) {
		gdk_window_show(options_window->window);
		return;
	};
	options_window=gtk_window_new(GTK_WINDOW_DIALOG);
	gtk_window_set_title(GTK_WINDOW(options_window),_("Options"));
	gtk_window_set_position(GTK_WINDOW(options_window),GTK_WIN_POS_CENTER);
	gtk_window_set_policy (GTK_WINDOW(options_window), FALSE,FALSE,FALSE);
	//    gtk_widget_set_usize(options_window,460,273);
	gtk_signal_connect(GTK_OBJECT(options_window),"delete_event",GTK_SIGNAL_FUNC(options_window_cancel), NULL);
	options_window_vbox=gtk_vbox_new(FALSE,10);
	gtk_box_set_spacing(GTK_BOX(options_window_vbox),5);
	gtk_container_border_width(GTK_CONTAINER(options_window),5);
	gtk_container_add(GTK_CONTAINER(options_window),options_window_vbox);
	options_window_notebook=gtk_notebook_new();
	gtk_box_pack_start(GTK_BOX(options_window_vbox),options_window_notebook,TRUE,TRUE,0);

	/*
	    Common options
	 */

	prefs_common_vbox=gtk_vbox_new(FALSE,0);
	prefs_common_frame=gtk_frame_new(_("Options"));
	GtkWidget *prefs_common_frame2=gtk_frame_new(_("Using title"));
	GtkWidget *prefs_common_vbox2=gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(prefs_common_frame2),prefs_common_vbox2);
	gtk_box_pack_start(GTK_BOX(prefs_common_vbox),prefs_common_frame,FALSE,FALSE,0);
	gtk_container_border_width(GTK_CONTAINER(prefs_common_frame),5);
	prefs_common_table=gtk_table_new(6,2,FALSE);
	gtk_container_add(GTK_CONTAINER(prefs_common_frame),prefs_common_table);
	gtk_container_border_width(GTK_CONTAINER(prefs_common_table),5);
	prefs_common_del_completed=gtk_check_button_new_with_label(_("Automatically delete completed downloads"));
	gtk_table_attach_defaults(GTK_TABLE(prefs_common_table),prefs_common_del_completed,0,1,0,1);
	prefs_common_del_fataled=gtk_check_button_new_with_label(_("Automatically delete failed downloads"));
	gtk_table_attach_defaults(GTK_TABLE(prefs_common_table),prefs_common_del_fataled,0,1,1,2);
	prefs_common_recursive=gtk_check_button_new_with_label(_("Optimize recursive downloads"));
	gtk_table_attach_defaults(GTK_TABLE(prefs_common_table),prefs_common_recursive,0,1,2,3);
	prefs_common_retry_if_noreget=gtk_check_button_new_with_label(_("Retry if resuming is not supported"));
	gtk_table_attach_defaults(GTK_TABLE(prefs_common_table),prefs_common_retry_if_noreget,0,1,3,4);
	prefs_common_window_lower=gtk_check_button_new_with_label(_("Iconify main window when closing"));
	gtk_table_attach_defaults(GTK_TABLE(prefs_common_table),prefs_common_window_lower,0,1,5,6);
	prefs_common_ftp_permisions=gtk_check_button_new_with_label(_("Get permisions of the file from server (FTP only)"));
	gtk_table_attach_defaults(GTK_TABLE(prefs_common_table),prefs_common_ftp_permisions,0,1,7,8);
	prefs_common_use_title=gtk_check_button_new_with_label(_("Use title of main window for info"));
	//    gtk_table_attach_defaults(GTK_TABLE(prefs_common_table),prefs_common_use_title,1,2,0,1);
	gtk_signal_connect(GTK_OBJECT(prefs_common_use_title),"clicked",GTK_SIGNAL_FUNC(options_toggle_title),NULL);
	prefs_common_use_title2=gtk_check_button_new_with_label(_("Display queue statistc too"));
	//    gtk_table_attach_defaults(GTK_TABLE(prefs_common_table),prefs_common_use_title2,1,2,1,2);
	prefs_common_scroll_title=gtk_check_button_new_with_label(_("Scroll title"));
	gtk_box_pack_start(GTK_BOX(prefs_common_vbox2),prefs_common_use_title,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(prefs_common_vbox2),prefs_common_use_title2,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(prefs_common_vbox2),prefs_common_scroll_title,FALSE,FALSE,0);
	gtk_table_attach_defaults(GTK_TABLE(prefs_common_table),prefs_common_frame2,1,2,0,4);
	prefs_common_get_date=gtk_check_button_new_with_label(_("Get date from the server"));
	gtk_table_attach_defaults(GTK_TABLE(prefs_common_table),prefs_common_get_date,1,2,4,5);
	prefs_common_dnd_dialog=gtk_check_button_new_with_label(_("Open dialog for Drag-n-Drop"));
	gtk_table_attach_defaults(GTK_TABLE(prefs_common_table),prefs_common_dnd_dialog,1,2,5,6);
	prefs_common_graph_order=gtk_check_button_new_with_label(_("Revert drawing graph of speeds"));
	gtk_table_attach_defaults(GTK_TABLE(prefs_common_table),prefs_common_graph_order,1,2,7,8);
	prefs_common_save_list_check=gtk_check_button_new_with_label(_("Save list of downloads every"));
	gtk_signal_connect(GTK_OBJECT(prefs_common_save_list_check),"clicked",GTK_SIGNAL_FUNC(options_toggle_save_list),NULL);
	prefs_common_save_list_entry=gtk_entry_new_with_max_length(3);
	gtk_widget_set_usize(prefs_common_save_list_entry,30,-1);
	GtkWidget *prefs_common_hbox=gtk_hbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(prefs_common_hbox),5);
	GtkWidget *prefs_common_label=gtk_label_new(_("minutes"));
	sprintf(temp,"%i",CFG.SAVE_LIST_INTERVAL);
	gtk_entry_set_text(GTK_ENTRY(prefs_common_save_list_entry),temp);
	gtk_box_pack_start(GTK_BOX(prefs_common_hbox),prefs_common_save_list_check,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(prefs_common_hbox),prefs_common_save_list_entry,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(prefs_common_hbox),prefs_common_label,FALSE,FALSE,0);
	gtk_table_attach_defaults(GTK_TABLE(prefs_common_table),prefs_common_hbox,0,1,4,5);

	prefs_common_default_permisions=gtk_entry_new();
	gtk_widget_set_usize(prefs_common_default_permisions,30,-1);
	GtkWidget *prefs_common_default_permisions_label=gtk_label_new(_("Default permisions of file"));
	GtkWidget *prefs_common_default_permisions_box=gtk_hbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(prefs_common_default_permisions_box),prefs_common_default_permisions,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(prefs_common_default_permisions_box),prefs_common_default_permisions_label,FALSE,FALSE,0);
	sprintf(temp,"%i",CFG.DEFAULT_PERMISIONS);
	gtk_entry_set_text(GTK_ENTRY(prefs_common_default_permisions),temp);
	gtk_table_attach_defaults(GTK_TABLE(prefs_common_table),prefs_common_default_permisions_box,0,1,9,10);

	prefs_common_dnd_trash=gtk_check_button_new_with_label(_("Show trash for DnD"));
	gtk_table_attach_defaults(GTK_TABLE(prefs_common_table),prefs_common_dnd_trash,1,2,9,10);
	
	gtk_notebook_append_page(GTK_NOTEBOOK(options_window_notebook),prefs_common_vbox,gtk_label_new(_("Common")));

	toggle_button_set_state(GTK_TOGGLE_BUTTON(prefs_common_del_fataled),CFG.DELETE_FATAL);
	toggle_button_set_state(GTK_TOGGLE_BUTTON(prefs_common_del_completed),CFG.DELETE_COMPLETED);
	toggle_button_set_state(GTK_TOGGLE_BUTTON(prefs_common_recursive),CFG.RECURSIVE_OPTIMIZE);
	toggle_button_set_state(GTK_TOGGLE_BUTTON(prefs_common_retry_if_noreget),CFG.RETRY_IF_NOREGET);
	toggle_button_set_state(GTK_TOGGLE_BUTTON(prefs_common_ftp_permisions),CFG.FTP_PERMISIONS);
	toggle_button_set_state(GTK_TOGGLE_BUTTON(prefs_common_save_list_check),CFG.SAVE_LIST);
	toggle_button_set_state(GTK_TOGGLE_BUTTON(prefs_common_use_title),CFG.USE_MAINWIN_TITLE);
	toggle_button_set_state(GTK_TOGGLE_BUTTON(prefs_common_use_title2),CFG.USE_MAINWIN_TITLE2);
	toggle_button_set_state(GTK_TOGGLE_BUTTON(prefs_common_scroll_title),CFG.SCROLL_MAINWIN_TITLE);
	toggle_button_set_state(GTK_TOGGLE_BUTTON(prefs_common_get_date),CFG.GET_DATE);
	toggle_button_set_state(GTK_TOGGLE_BUTTON(prefs_common_dnd_dialog),CFG.NEED_DIALOG_FOR_DND);
	toggle_button_set_state(GTK_TOGGLE_BUTTON(prefs_common_window_lower),CFG.WINDOW_LOWER);
	toggle_button_set_state(GTK_TOGGLE_BUTTON(prefs_common_graph_order),CFG.GRAPH_ORDER);
	toggle_button_set_state(GTK_TOGGLE_BUTTON(prefs_common_dnd_trash),CFG.DND_TRASH);
	options_toggle_save_list(prefs_common_save_list_check);
	options_toggle_title(prefs_common_use_title);
	/*
	    Limits options
	 */
	prefs_limits_vbox=gtk_vbox_new(FALSE,0);
	prefs_limits_frame=gtk_frame_new(_("Limits"));
	gtk_box_pack_start(GTK_BOX(prefs_limits_vbox),prefs_limits_frame,FALSE,FALSE,0);
	gtk_container_border_width(GTK_CONTAINER(prefs_limits_frame),5);
	prefs_limits_table=gtk_table_new(1,5,FALSE);
	gtk_container_add(GTK_CONTAINER(prefs_limits_frame),prefs_limits_table);
	gtk_container_border_width(GTK_CONTAINER(prefs_limits_table),5);

	prefs_limits_tbox=gtk_hbox_new(FALSE,0);
	prefs_limits_max_threads=gtk_entry_new_with_max_length(2);
	gtk_widget_set_usize(prefs_limits_max_threads,30,-1);
	sprintf(temp,"%i",CFG.MAX_THREADS);
	gtk_entry_set_text(GTK_ENTRY(prefs_limits_max_threads),temp);
	gtk_box_pack_start(GTK_BOX(prefs_limits_tbox),prefs_limits_max_threads,FALSE,FALSE,0);
	prefs_limits_tlabel=gtk_label_new(_("Maximum active downloads"));
	gtk_box_pack_start(GTK_BOX(prefs_limits_tbox),prefs_limits_tlabel,FALSE,FALSE,0);
	gtk_table_attach_defaults(GTK_TABLE(prefs_limits_table),prefs_limits_tbox,1,2,0,1);

	prefs_limits_rtbox=gtk_hbox_new(FALSE,0);
	prefs_limits_retry_timeout=gtk_entry_new_with_max_length(2);
	gtk_widget_set_usize(prefs_limits_retry_timeout,30,-1);
	sprintf(temp,"%i",CFG.RETRY_TIME_OUT);
	gtk_entry_set_text(GTK_ENTRY(prefs_limits_retry_timeout),temp);
	gtk_box_pack_start(GTK_BOX(prefs_limits_rtbox),prefs_limits_retry_timeout,FALSE,FALSE,0);
	prefs_limits_rtlabel=gtk_label_new(_("Timeout before reconnection (in seconds)"));
	gtk_box_pack_start(GTK_BOX(prefs_limits_rtbox),prefs_limits_rtlabel,FALSE,FALSE,0);
	gtk_table_attach_defaults(GTK_TABLE(prefs_limits_table),prefs_limits_rtbox,1,2,1,2);

	prefs_limits_tibox=gtk_hbox_new(FALSE,0);
	prefs_limits_timeout=gtk_entry_new_with_max_length(3);
	gtk_widget_set_usize(prefs_limits_timeout,30,-1);
	sprintf(temp,"%i",CFG.TIME_OUT);
	gtk_entry_set_text(GTK_ENTRY(prefs_limits_timeout),temp);
	gtk_box_pack_start(GTK_BOX(prefs_limits_tibox),prefs_limits_timeout,FALSE,FALSE,0);
	prefs_limits_tilabel=gtk_label_new(_("Timeout for reading from socket (in seconds)"));
	gtk_box_pack_start(GTK_BOX(prefs_limits_tibox),prefs_limits_tilabel,FALSE,FALSE,0);
	gtk_table_attach_defaults(GTK_TABLE(prefs_limits_table),prefs_limits_tibox,1,2,2,3);

	prefs_limits_lbox=gtk_hbox_new(FALSE,0);
	prefs_limits_log=gtk_entry_new_with_max_length(3);
	gtk_widget_set_usize(prefs_limits_log,30,-1);
	sprintf(temp,"%i",CFG.MAX_LOG_LENGTH);
	gtk_entry_set_text(GTK_ENTRY(prefs_limits_log),temp);
	gtk_box_pack_start(GTK_BOX(prefs_limits_lbox),prefs_limits_log,FALSE,FALSE,0);
	prefs_limits_llabel=gtk_label_new(_("Maximum lines in log"));
	gtk_box_pack_start(GTK_BOX(prefs_limits_lbox),prefs_limits_llabel,FALSE,FALSE,0);
	gtk_table_attach_defaults(GTK_TABLE(prefs_limits_table),prefs_limits_lbox,1,2,3,4);

	prefs_limits_mlbox=gtk_hbox_new(FALSE,0);
	prefs_limits_mlog=gtk_entry_new_with_max_length(3);
	gtk_widget_set_usize(prefs_limits_mlog,30,-1);
	sprintf(temp,"%i",CFG.MAX_MAIN_LOG_LENGTH);
	gtk_entry_set_text(GTK_ENTRY(prefs_limits_mlog),temp);
	gtk_box_pack_start(GTK_BOX(prefs_limits_mlbox),prefs_limits_mlog,FALSE,FALSE,0);
	prefs_limits_mllabel=gtk_label_new(_("Maximum lines in MAIN log"));
	gtk_box_pack_start(GTK_BOX(prefs_limits_mlbox),prefs_limits_mllabel,FALSE,FALSE,0);
	gtk_table_attach_defaults(GTK_TABLE(prefs_limits_table),prefs_limits_mlbox,1,2,4,5);

	prefs_limits_rbox=gtk_hbox_new(FALSE,0);
	prefs_limits_max_retries=gtk_entry_new_with_max_length(3);
	gtk_widget_set_usize(prefs_limits_max_retries,30,-1);
	sprintf(temp,"%i",CFG.MAX_RETRIES);
	gtk_entry_set_text(GTK_ENTRY(prefs_limits_max_retries),temp);
	gtk_box_pack_start(GTK_BOX(prefs_limits_rbox),prefs_limits_max_retries,FALSE,FALSE,0);
	prefs_limits_rlabel=gtk_label_new(_("Maximum attempts (0 for unlimited)"));
	gtk_box_pack_start(GTK_BOX(prefs_limits_rbox),prefs_limits_rlabel,FALSE,FALSE,0);
	gtk_table_attach_defaults(GTK_TABLE(prefs_limits_table),prefs_limits_rbox,1,2,5,6);
	prefs_limits_rbox=gtk_hbox_new(FALSE,0);

	prefs_limits_rbbox=gtk_hbox_new(FALSE,0);
	prefs_limits_rollback=gtk_entry_new_with_max_length(5);
	gtk_widget_set_usize(prefs_limits_rollback,50,-1);
	sprintf(temp,"%i",CFG.ROLLBACK);
	gtk_entry_set_text(GTK_ENTRY(prefs_limits_rollback),temp);
	gtk_box_pack_start(GTK_BOX(prefs_limits_rbbox),prefs_limits_rollback,FALSE,FALSE,0);
	prefs_limits_rblabel=gtk_label_new(_("Rollback after reconnecting (in bytes)"));
	gtk_box_pack_start(GTK_BOX(prefs_limits_rbbox),prefs_limits_rblabel,FALSE,FALSE,0);
	gtk_table_attach_defaults(GTK_TABLE(prefs_limits_table),prefs_limits_rbbox,1,2,6,7);

	prefs_limits_rdbox=gtk_hbox_new(FALSE,0);
	prefs_limits_ftp_recurse_depth=gtk_entry_new_with_max_length(3);
	gtk_widget_set_usize(prefs_limits_ftp_recurse_depth,30,-1);
	sprintf(temp,"%i",CFG.FTP_RECURSE_DEPTH);
	gtk_entry_set_text(GTK_ENTRY(prefs_limits_ftp_recurse_depth),temp);
	prefs_limits_http_recurse_depth=gtk_entry_new_with_max_length(3);
	gtk_widget_set_usize(prefs_limits_http_recurse_depth,30,-1);
	sprintf(temp,"%i",CFG.HTTP_RECURSE_DEPTH);
	gtk_entry_set_text(GTK_ENTRY(prefs_limits_http_recurse_depth),temp);
	GtkWidget *temp_frame=gtk_frame_new("ftp");
	gtk_container_add(GTK_CONTAINER(temp_frame),prefs_limits_ftp_recurse_depth);
	gtk_box_pack_start(GTK_BOX(prefs_limits_rdbox),temp_frame,FALSE,FALSE,0);

	temp_frame=gtk_frame_new("http");
	gtk_container_add(GTK_CONTAINER(temp_frame),prefs_limits_http_recurse_depth);
	gtk_box_pack_start(GTK_BOX(prefs_limits_rdbox),temp_frame,FALSE,FALSE,0);
	//    gtk_box_pack_start(GTK_BOX(prefs_limits_rdbox),prefs_limits_http_recurse_depth,FALSE,FALSE,0);
	prefs_limits_rdlabel=gtk_label_new(_("Depth of recursing (0 unlimited,1 no recurse)"));
	gtk_box_pack_start(GTK_BOX(prefs_limits_rdbox),prefs_limits_rdlabel,FALSE,FALSE,0);
	gtk_table_attach_defaults(GTK_TABLE(prefs_limits_table),prefs_limits_rdbox,1,2,7,8);


	gtk_notebook_append_page(GTK_NOTEBOOK(options_window_notebook),prefs_limits_vbox,gtk_label_new(_("Limits")));
	/*
	 *	Other options
	 */
	prefs_other_vbox=gtk_vbox_new(FALSE,0);
	prefs_other_frame=gtk_frame_new(_("Other"));
	gtk_box_pack_start(GTK_BOX(prefs_other_vbox),prefs_other_frame,FALSE,FALSE,0);
	gtk_container_border_width(GTK_CONTAINER(prefs_other_frame),5);
	prefs_other_table=gtk_table_new(2,5,FALSE);
	gtk_container_add(GTK_CONTAINER(prefs_other_frame),prefs_other_table);
	gtk_container_border_width(GTK_CONTAINER(prefs_other_table),5);

	prefs_other_sbox=gtk_vbox_new(FALSE,0);
	prefs_other_savepath=my_gtk_combo_new(PathHistory);

	GtkWidget *dir_browser_button=gtk_button_new_with_label(_("Browse"));
	GtkWidget *hboxtemp=gtk_hbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(hboxtemp),5);
	gtk_box_pack_start(GTK_BOX(hboxtemp),prefs_other_savepath,TRUE,TRUE,0);
	gtk_box_pack_start(GTK_BOX(hboxtemp),dir_browser_button,FALSE,FALSE,0);
	if (CFG.GLOBAL_SAVE_PATH)
		text_to_combo(prefs_other_savepath,CFG.GLOBAL_SAVE_PATH);
	gtk_signal_connect(GTK_OBJECT(dir_browser_button),"clicked",GTK_SIGNAL_FUNC(options_browser_open),NULL);
	prefs_other_slabel=gtk_label_new(_("Save downloads to folder"));
	gtk_box_pack_start(GTK_BOX(prefs_other_sbox),prefs_other_slabel,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(prefs_other_sbox),hboxtemp,FALSE,FALSE,0);
	gtk_table_attach_defaults(GTK_TABLE(prefs_other_table),prefs_other_sbox,0,2,0,1);

	prefs_other_fbox=gtk_vbox_new(FALSE,0);
	prefs_other_filename=my_gtk_combo_new(FileHistory);
	text_to_combo(prefs_other_filename,CFG.DEFAULT_NAME);
	prefs_other_flabel=gtk_label_new(_("Filename for saving if it is unknown"));
	gtk_box_pack_start(GTK_BOX(prefs_other_fbox),prefs_other_flabel,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(prefs_other_fbox),prefs_other_filename,FALSE,FALSE,0);
	gtk_table_attach_defaults(GTK_TABLE(prefs_other_table),prefs_other_fbox,0,2,1,2);

	prefs_other_ftp_passive=gtk_check_button_new_with_label(_("Use passive mode for ftp"));
	GTK_TOGGLE_BUTTON(prefs_other_ftp_passive)->active=CFG.FTP_PASSIVE_MODE;
	gtk_table_attach_defaults(GTK_TABLE(prefs_other_table),prefs_other_ftp_passive,0,1,2,3);


	prefs_other_save_log=gtk_check_button_new_with_label(_("Save main log into file"));
	GTK_TOGGLE_BUTTON(prefs_other_save_log)->active=CFG.SAVE_MAIN_LOG;
	gtk_table_attach_defaults(GTK_TABLE(prefs_other_table),prefs_other_save_log,0,1,3,4);
	gtk_signal_connect(GTK_OBJECT(prefs_other_save_log),"clicked",GTK_SIGNAL_FUNC(options_toggle_save_log),NULL);
	prefs_other_save_log_path=my_gtk_combo_new(LogHistory);
	//    gtk_widget_set_usize(GTK_COMBO(prefs_other_save_log_path)->entry,350,-1);

	dir_browser_button2=gtk_button_new_with_label(_("Browse"));
	gtk_signal_connect(GTK_OBJECT(dir_browser_button2),"clicked",GTK_SIGNAL_FUNC(options_browser_open2),NULL);
	hboxtemp=gtk_hbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(hboxtemp),5);
	gtk_box_pack_start(GTK_BOX(hboxtemp),prefs_other_save_log_path,TRUE,TRUE,0);
	gtk_box_pack_start(GTK_BOX(hboxtemp),dir_browser_button2,FALSE,FALSE,0);
	if (CFG.SAVE_LOG_PATH)
		text_to_combo(prefs_other_save_log_path,CFG.SAVE_LOG_PATH);
	gtk_table_attach_defaults(GTK_TABLE(prefs_other_table),hboxtemp,0,2,5,6);

	hboxtemp=gtk_hbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(hboxtemp),5);
	prefs_other_append_log=gtk_radio_button_new_with_label(NULL,_("Append to file"));
	gtk_box_pack_start(GTK_BOX(hboxtemp),prefs_other_append_log,FALSE,FALSE,0);
	GSList *other_group=gtk_radio_button_group(GTK_RADIO_BUTTON(prefs_other_append_log));
	prefs_other_rewrite_log=gtk_radio_button_new_with_label(other_group,_("Rewrite file"));
	gtk_box_pack_start(GTK_BOX(hboxtemp),prefs_other_rewrite_log,FALSE,FALSE,0);
	GTK_TOGGLE_BUTTON(prefs_other_append_log)->active=CFG.APPEND_REWRITE_LOG;
	GTK_TOGGLE_BUTTON(prefs_other_rewrite_log)->active= ! CFG.APPEND_REWRITE_LOG;
	gtk_table_attach_defaults(GTK_TABLE(prefs_other_table),hboxtemp,0,1,6,7);
	options_toggle_save_log(prefs_other_save_log);

	gtk_notebook_append_page(GTK_NOTEBOOK(options_window_notebook),prefs_other_vbox,gtk_label_new(_("Other")));
	/*
	 * Columns options
	 */
	GtkWidget *columns_hbox=gtk_hbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(columns_hbox),5);
	GtkWidget *columns_frame1=gtk_frame_new(_("Size format"));
	GtkWidget *columns_frame2=gtk_frame_new(_("Time format"));
	gtk_container_border_width(GTK_CONTAINER(columns_frame1),5);
	gtk_container_border_width(GTK_CONTAINER(columns_frame2),5);
	GtkWidget *columns_vbox1=gtk_vbox_new(FALSE,0);
	GtkWidget *columns_vbox2=gtk_vbox_new(FALSE,0);

	prefs_columns_nums_button1=gtk_radio_button_new_with_label(NULL,"123456");
	gtk_box_pack_start(GTK_BOX(columns_vbox1),prefs_columns_nums_button1,FALSE,FALSE,0);
	GSList *columns_group1=gtk_radio_button_group(GTK_RADIO_BUTTON(prefs_columns_nums_button1));
	prefs_columns_nums_button2=gtk_radio_button_new_with_label(columns_group1,"123 456");
	gtk_box_pack_start(GTK_BOX(columns_vbox1),prefs_columns_nums_button2,FALSE,FALSE,0);
	prefs_columns_nums_button3=gtk_radio_button_new_with_label(
	                               gtk_radio_button_group(GTK_RADIO_BUTTON(prefs_columns_nums_button2)),"123K");
	gtk_box_pack_start(GTK_BOX(columns_vbox1),prefs_columns_nums_button3,FALSE,FALSE,0);

	switch(CFG.NICE_DEC_DIGITALS.curent) {
		case 1:
			{
				gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(prefs_columns_nums_button2),TRUE);
				break;
			};
		case 2:
			{
				gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(prefs_columns_nums_button3),TRUE);
				break;
			};
		default:
			gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(prefs_columns_nums_button1),TRUE);
	};
	prefs_columns_time_button1=gtk_radio_button_new_with_label(NULL,"12:34:56");
	gtk_box_pack_start(GTK_BOX(columns_vbox2),prefs_columns_time_button1,FALSE,FALSE,0);
	GSList *columns_group2=gtk_radio_button_group(GTK_RADIO_BUTTON(prefs_columns_time_button1));
	prefs_columns_time_button2=gtk_radio_button_new_with_label(columns_group2,"12:34");
	gtk_box_pack_start(GTK_BOX(columns_vbox2),prefs_columns_time_button2,FALSE,FALSE,0);
	if (CFG.TIME_FORMAT)
		gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(prefs_columns_time_button2),TRUE);
	else
		gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(prefs_columns_time_button1),TRUE);
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

	gtk_box_pack_start(GTK_BOX(columns_hbox),columns_vbox11,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(columns_hbox),columns_vbox21,FALSE,FALSE,0);
	prefs_columns_order.init();
	gtk_box_pack_start(GTK_BOX(columns_hbox),prefs_columns_order.body(),FALSE,FALSE,0);

	GtkWidget *columns_frame=gtk_frame_new(_("Columns"));
	gtk_container_border_width(GTK_CONTAINER(columns_frame),5);
	gtk_container_add(GTK_CONTAINER(columns_frame),columns_hbox);
	gtk_notebook_append_page(GTK_NOTEBOOK(options_window_notebook),columns_frame,gtk_label_new(_("Columns")));

	/* PROXY
	 */
	if (!prefs_proxy) prefs_proxy=new tProxyWidget;
	prefs_proxy->init();
	prefs_proxy->init_state();
	gtk_notebook_append_page(GTK_NOTEBOOK(options_window_notebook),prefs_proxy->frame,gtk_label_new(_("Proxy")));
	/* confirm
	 */
	GtkWidget *vbox=gtk_vbox_new(FALSE,0);
	GtkWidget *frame=gtk_frame_new(_("Confirmation"));
	gtk_container_border_width(GTK_CONTAINER(frame),5);
	gtk_container_add(GTK_CONTAINER(frame),vbox);
	prefs_confirm_delete=gtk_check_button_new_with_label(_("Confirm delete selected downloads"));
	prefs_confirm_delete_all=gtk_check_button_new_with_label(_("Confirm delete all downloads"));
	prefs_confirm_delete_completed=gtk_check_button_new_with_label(_("Confirm delete completed downloads"));
	prefs_confirm_delete_fataled=gtk_check_button_new_with_label(_("Confirm delete fataled downloads"));
	prefs_confirm_exit=gtk_check_button_new_with_label(_("Confirm exit from program"));
	toggle_button_set_state(GTK_TOGGLE_BUTTON(prefs_confirm_delete),CFG.CONFIRM_DELETE);
	toggle_button_set_state(GTK_TOGGLE_BUTTON(prefs_confirm_delete_completed),CFG.CONFIRM_DELETE_COMPLETED);
	toggle_button_set_state(GTK_TOGGLE_BUTTON(prefs_confirm_delete_fataled),CFG.CONFIRM_DELETE_FATALED);
	toggle_button_set_state(GTK_TOGGLE_BUTTON(prefs_confirm_delete_all),CFG.CONFIRM_DELETE_ALL);
	toggle_button_set_state(GTK_TOGGLE_BUTTON(prefs_confirm_exit),CFG.CONFIRM_EXIT);
	gtk_box_pack_start(GTK_BOX(vbox),prefs_confirm_delete,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),prefs_confirm_delete_all,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),prefs_confirm_delete_completed,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),prefs_confirm_delete_fataled,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),prefs_confirm_exit,FALSE,FALSE,0);
	gtk_notebook_append_page(GTK_NOTEBOOK(options_window_notebook),frame,gtk_label_new(_("Confirmation")));
	/* Speed
	 */
	vbox=gtk_vbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(vbox),5);
	frame=gtk_frame_new(_("Speed"));
	gtk_container_border_width(GTK_CONTAINER(frame),5);
	gtk_container_add(GTK_CONTAINER(frame),vbox);
	GtkWidget *label=gtk_label_new(_("bytes/sec speed level one (red button)"));
	GtkWidget *hbox=gtk_hbox_new(FALSE,0);
	prefs_speed_limit_1=gtk_entry_new();
	gtk_widget_set_usize(prefs_speed_limit_1,100,-1);
	gtk_box_pack_start(GTK_BOX(hbox),prefs_speed_limit_1,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);
	label=gtk_label_new(_("bytes/sec speed level two (yellow button)"));
	hbox=gtk_hbox_new(FALSE,0);
	prefs_speed_limit_2=gtk_entry_new();
	gtk_widget_set_usize(prefs_speed_limit_2,100,-1);
	gtk_box_pack_start(GTK_BOX(hbox),prefs_speed_limit_2,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);
	sprintf(temp,"%i",CFG.SPEED_LIMIT_1 / 2);
	gtk_entry_set_text(GTK_ENTRY(prefs_speed_limit_1),temp);
	sprintf(temp,"%i",CFG.SPEED_LIMIT_2 / 2);
	gtk_entry_set_text(GTK_ENTRY(prefs_speed_limit_2),temp);
	gtk_notebook_append_page(GTK_NOTEBOOK(options_window_notebook),frame,gtk_label_new(_("Speed")));
	/*
	    Buttons Ok and Cancel
	 */
	buttons_hbox=gtk_hbutton_box_new();
	gtk_button_box_set_layout(GTK_BUTTON_BOX(buttons_hbox),GTK_BUTTONBOX_END);
	gtk_button_box_set_spacing(GTK_BUTTON_BOX(buttons_hbox),5);
	gtk_box_pack_start(GTK_BOX(options_window_vbox),buttons_hbox,FALSE,FALSE,0);
	ok_button=gtk_button_new_with_label(_("Ok"));
	cancel_button=gtk_button_new_with_label(_("Cancel"));
	gtk_signal_connect(GTK_OBJECT(cancel_button),"clicked",GTK_SIGNAL_FUNC(options_window_cancel),NULL);
	gtk_signal_connect(GTK_OBJECT(ok_button),"clicked",GTK_SIGNAL_FUNC(options_window_ok),NULL);
	GTK_WIDGET_SET_FLAGS(ok_button,GTK_CAN_DEFAULT);
	GTK_WIDGET_SET_FLAGS(cancel_button,GTK_CAN_DEFAULT);
	gtk_box_pack_start(GTK_BOX(buttons_hbox),ok_button,TRUE,TRUE,0);
	gtk_box_pack_start(GTK_BOX(buttons_hbox),cancel_button,TRUE,TRUE,0);

	gtk_window_set_default(GTK_WINDOW(options_window),ok_button);
	gtk_widget_show_all(options_window);
};

gint options_window_cancel() {
	gtk_widget_destroy(options_window);
	options_window=NULL;
	if (dir_browser) options_browser_cancel();
	if (dir_browser2) options_browser_cancel2();
	return TRUE;
};

void options_window_ok() {
	CFG.DELETE_FATAL=GTK_TOGGLE_BUTTON(prefs_common_del_fataled)->active;
	CFG.DELETE_COMPLETED=GTK_TOGGLE_BUTTON(prefs_common_del_completed)->active;
	CFG.RECURSIVE_OPTIMIZE=GTK_TOGGLE_BUTTON(prefs_common_recursive)->active;
	CFG.RETRY_IF_NOREGET=GTK_TOGGLE_BUTTON(prefs_common_retry_if_noreget)->active;
	CFG.TIME_FORMAT=GTK_TOGGLE_BUTTON(prefs_columns_time_button2)->active;
	CFG.FTP_PASSIVE_MODE=GTK_TOGGLE_BUTTON(prefs_other_ftp_passive)->active;
	CFG.FTP_PERMISIONS=GTK_TOGGLE_BUTTON(prefs_common_ftp_permisions)->active;
	CFG.SAVE_LIST=GTK_TOGGLE_BUTTON(prefs_common_save_list_check)->active;
	CFG.USE_MAINWIN_TITLE=GTK_TOGGLE_BUTTON(prefs_common_use_title)->active;
	CFG.USE_MAINWIN_TITLE2=GTK_TOGGLE_BUTTON(prefs_common_use_title2)->active;
	CFG.SCROLL_MAINWIN_TITLE=GTK_TOGGLE_BUTTON(prefs_common_scroll_title)->active;
	CFG.GET_DATE=GTK_TOGGLE_BUTTON(prefs_common_get_date)->active;
	CFG.NEED_DIALOG_FOR_DND=GTK_TOGGLE_BUTTON(prefs_common_dnd_dialog)->active;
	CFG.WINDOW_LOWER=GTK_TOGGLE_BUTTON(prefs_common_window_lower)->active;
	CFG.CONFIRM_DELETE=GTK_TOGGLE_BUTTON(prefs_confirm_delete)->active;
	CFG.CONFIRM_DELETE_ALL=GTK_TOGGLE_BUTTON(prefs_confirm_delete_all)->active;
	CFG.CONFIRM_DELETE_COMPLETED=GTK_TOGGLE_BUTTON(prefs_confirm_delete_completed)->active;
	CFG.CONFIRM_DELETE_FATALED=GTK_TOGGLE_BUTTON(prefs_confirm_delete_fataled)->active;
	CFG.CONFIRM_EXIT=GTK_TOGGLE_BUTTON(prefs_confirm_exit)->active;
	CFG.GRAPH_ORDER=GTK_TOGGLE_BUTTON(prefs_common_graph_order)->active;
	CFG.DND_TRASH=GTK_TOGGLE_BUTTON(prefs_common_dnd_trash)->active;
	if (CFG.DND_TRASH) dnd_trash_init();
	else dnd_trash_destroy();

	prefs_proxy->apply_changes();

	if (GTK_TOGGLE_BUTTON(prefs_columns_nums_button2)->active)
		CFG.NICE_DEC_DIGITALS.curent=1;
	else {
		if (GTK_TOGGLE_BUTTON(prefs_columns_nums_button3)->active)
			CFG.NICE_DEC_DIGITALS.curent=2;
		else
			CFG.NICE_DEC_DIGITALS.curent=0;
	};
	if (CFG.DELETE_COMPLETED) aa.del_completed();
	if (CFG.DELETE_FATAL) aa.del_fataled();
	int  temp=0;
	sscanf(gtk_entry_get_text(GTK_ENTRY(prefs_limits_timeout)),"%u",&temp);
	if (temp>30) CFG.TIME_OUT=temp;
	sscanf(gtk_entry_get_text(GTK_ENTRY(prefs_limits_max_threads)),"%u",&temp);
	if (temp>0 && temp<51) CFG.MAX_THREADS=temp;
	sscanf(gtk_entry_get_text(GTK_ENTRY(prefs_limits_retry_timeout)),"%u",&temp);
	if (temp>0) CFG.RETRY_TIME_OUT=temp;
	sscanf(gtk_entry_get_text(GTK_ENTRY(prefs_limits_max_retries)),"%u",&temp);
	CFG.MAX_RETRIES=temp;
	temp=1;
	sscanf(gtk_entry_get_text(GTK_ENTRY(prefs_limits_ftp_recurse_depth)),"%u",&temp);
	CFG.FTP_RECURSE_DEPTH=temp;
	sscanf(gtk_entry_get_text(GTK_ENTRY(prefs_limits_http_recurse_depth)),"%u",&temp);
	CFG.HTTP_RECURSE_DEPTH=temp;
	temp=0; sscanf(gtk_entry_get_text(GTK_ENTRY(prefs_limits_rollback)),"%u",&temp);
	CFG.ROLLBACK=temp;
	sscanf(gtk_entry_get_text(GTK_ENTRY(prefs_limits_log)),"%u",&temp);
	if (temp>=100) CFG.MAX_LOG_LENGTH=temp;
	sscanf(gtk_entry_get_text(GTK_ENTRY(prefs_limits_mlog)),"%u",&temp);
	if (temp>=100) CFG.MAX_MAIN_LOG_LENGTH=temp;
	sscanf(gtk_entry_get_text(GTK_ENTRY(prefs_speed_limit_1)),"%u",&temp);
	if (temp>=1024) CFG.SPEED_LIMIT_1=2*temp;
	sscanf(gtk_entry_get_text(GTK_ENTRY(prefs_speed_limit_2)),"%u",&temp);
	if (temp>=1024) CFG.SPEED_LIMIT_2=2*temp;
	if (CFG.GLOBAL_SAVE_PATH) delete (CFG.GLOBAL_SAVE_PATH);
	CFG.GLOBAL_SAVE_PATH=copy_string(text_from_combo(prefs_other_savepath));
	normalize_path(CFG.GLOBAL_SAVE_PATH);
	PathHistory->add(CFG.GLOBAL_SAVE_PATH);
/* Main log settings */
	CFG.APPEND_REWRITE_LOG=GTK_TOGGLE_BUTTON(prefs_other_append_log)->active;
 	if (CFG.SAVE_LOG_PATH==NULL || strcmp(CFG.SAVE_LOG_PATH,text_from_combo(prefs_other_save_log_path)) || 
 				CFG.SAVE_MAIN_LOG!=(int)GTK_TOGGLE_BUTTON(prefs_other_save_log)->active){
		if (CFG.SAVE_LOG_PATH) delete (CFG.SAVE_LOG_PATH);
		LogHistory->add(text_from_combo(prefs_other_save_log_path));
		CFG.SAVE_LOG_PATH=copy_string(text_from_combo(prefs_other_save_log_path));
		CFG.SAVE_MAIN_LOG=GTK_TOGGLE_BUTTON(prefs_other_save_log)->active;
 		aa.reinit_main_log();
	};
/* default name */
	delete(CFG.DEFAULT_NAME);
	CFG.DEFAULT_NAME=copy_string(text_from_combo(prefs_other_filename));
	if (strlen(CFG.DEFAULT_NAME))
		FileHistory->add(CFG.DEFAULT_NAME);
	sscanf(gtk_entry_get_text(GTK_ENTRY(prefs_common_save_list_entry)),"%u",&temp);
	if (temp>0 && temp<1000) CFG.SAVE_LIST_INTERVAL=temp;
	temp=0;
	sscanf(gtk_entry_get_text(GTK_ENTRY(prefs_common_default_permisions)),"%u",&temp);
	CFG.DEFAULT_PERMISIONS=temp;
	/*  There we need to make new loglimit for all logs
	 */
	prefs_columns_order.apply_changes();
	options_window_cancel();
	save_config();
};

void toggle_button_set_state(GtkToggleButton *tb,gboolean state) {
#if (GTK_MAJOR_VERSION==1) && (GTK_MINOR_VERSION==1) && (GTK_MICRO_VERSION<=12)
	gtk_toggle_button_set_state(tb,state);
#else
	gtk_toggle_button_set_active(tb,state);
#endif
}