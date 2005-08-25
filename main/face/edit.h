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

#ifndef MY_DOWNLOAD_EDITOR
#define MY_DOWNLOAD_EDITOR
#include <gtk/gtk.h>
#include <time.h>
#include "../dlist.h"
#include "../history.h"

struct tMainCfg;

struct tProxyWidget{
	GtkWidget *frame;
	GtkWidget *http_proxy_host,*http_proxy_port,*http_proxy_pass,*http_proxy_user,*http_proxy_user_check,*http_proxy_check;
	GtkWidget *ftp_proxy_host,*ftp_proxy_port,*ftp_proxy_pass,*ftp_proxy_user,*ftp_proxy_user_check,*ftp_proxy_check;
	GtkWidget *ftp_proxy_type_ftp,*ftp_proxy_type_http;
	GtkWidget *no_cache;	
	GtkWidget *use_socks;
	GtkWidget *socks_host,*socks_port,*socks_user,*socks_pass;
	void init();
	void init_state();
	void init_state(tMainCfg *cfg);
	void init_state(tCfg *cfg,int proto);
	void apply_changes();
	void apply_changes(tMainCfg *cfg);
	void apply_changes(tCfg *cfg,int proto);
};

struct d4xFNode;
struct d4xFilterSel;

class tDEdit{
	tDownload *parent;
	GtkWidget *notebook;
	GtkWidget *common_frame,*ftp_frame,*http_frame;
	GtkWidget *pass_entry,*user_entry,*path_entry,*url_entry,*file_entry,*user_agent_entry;
	GtkWidget *timeout_entry,*attempts_entry,*sleep_entry,*rollback_entry;
	GtkWidget *use_pass_check,*ftp_passive_check,*permisions_check,*get_date_check,*retry_check;
	GtkWidget *follow_link_check,*leave_server_check,*leave_dir_check,*ihate_etag_check;
	GtkWidget *load_link_check,*link_as_file_check;
	GtkWidget *ftp_recurse_depth_entry,*http_recurse_depth_entry;
	GtkWidget *ftp_dirontop_check;
	GtkWidget *quest_sign_check;
	GtkWidget *dont_send_quit_check;
	GtkWidget *restart_from_begin_check,*to_top_check;
	GtkWidget *speed_entry;
	GtkWidget *time_check;
	GtkWidget *pause_check;
	GtkWidget *sleep_check;
	GtkWidget *check_time_check;
	GtkWidget *change_links_check;
	GtkWidget *split_entry;
	GtkWidget *calendar,*hour_entry,*minute_entry;
	GtkWidget *log_save_entry;
	GtkWidget *desc_entry;
	GtkWidget *referer_entry,*cookie_entry;
	GtkWidget *filter;
	GtkWidget *con_limit_entry;
	d4xFilterSel *filter_sel;
	tProxyWidget *proxy;
	void setup_time(time_t when);
	void init_main(tDownload *who);
	void init_other(tDownload *who);
	void init_ftp(tDownload *who);
	void init_http(tDownload *who);
	void init_time(tDownload *who);
public:
	GtkWidget *ok_button,*cancel_button,*isdefault_check;
	GtkWidget *window;
	int parent_in_db;
	int add_or_edit,not_url_history;
	int dnd;
	int limit;
	tDEdit();
	void init(tDownload *who);
	int apply_changes();
	void apply_enabled_changes();
	int get_pause_check();
	int get_to_top_check();
	void disable_ok_button();
	void enable_ok_button();
	void setup_entries();
	void toggle_time();
	void toggle_isdefault();
	void set_path_as_default();
	void paste_url();
	void set_description(const char *desc);
	tDownload *get_parent();
	void set_parent(tDownload *);
	void file_from_url();
	void file_check();
	void select_url();
	void clear_url();
	void clear_save_name();
	void set_url(const char *a);
	char *get_url();
	void done();
	void popup();
	void init_filter_sel();
	void filter_cancel();
	void filter_ok();
	void disable_items(int *array);
	void disable_time();
	void auto_fill_log();
	void disable_save_name();
	void file_recode_from_url();
	~tDEdit();
};

void init_edit_window(tDownload *what);
void init_edit_window_without_ok(tDownload *what,int flag=0);
GList *make_glist_from_mylist(tHistory *parent);
void history_to_combo_box_entry(tHistory *history,GtkWidget *combo);
GtkWidget *my_gtk_combo_new(tHistory *history);
void select_options_window_init();

#endif
