/*	WebDownloader for X-Window
 *	Copyright (C) 1999 Koshelev Maxim
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
struct tProxyWidget{
	GtkWidget *frame;
	GtkWidget *http_proxy_host,*http_proxy_port,*http_proxy_pass,*http_proxy_user,*http_proxy_user_check,*http_proxy_check;
	GtkWidget *ftp_proxy_host,*ftp_proxy_port,*ftp_proxy_pass,*ftp_proxy_user,*ftp_proxy_user_check,*ftp_proxy_check;
	GtkWidget *ftp_proxy_type_ftp,*ftp_proxy_type_http;
	void init();
	void init_state();
	void init_state(tCfg *cfg,int proto);
	void apply_changes();
	void apply_changes(tCfg *cfg,int proto);
};

class tDEdit{
	tDownload *parent;
	GtkWidget *notebook;
	GtkWidget *pass_entry,*user_entry,*path_entry,*url_entry,*file_entry,*user_agent_entry;
	GtkWidget *timeout_entry,*attempts_entry,*sleep_entry,*rollback_entry;
	GtkWidget *use_pass_check,*ftp_passive_check,*permisions_check,*get_date_check,*retry_check;
	GtkWidget *ftp_recurse_depth_entry,*http_recurse_depth_entry;
	GtkWidget *speed_entry;
	GtkWidget *dir_browser,*dir_browser2;
	GtkWidget *button;
	GtkWidget *time_check;
	GtkWidget *year_entry,*month_entry,*day_entry,*hour_entry,*minute_entry;
	tProxyWidget *proxy;
	void setup_time(time_t when);
	void init_main(tDownload *who);
	void init_other(tDownload *who);
	void init_time(tDownload *who);
	public:
		GtkWidget *ok_button,*cancel_button;
		GtkWidget *window;
		tDEdit();
		void init(tDownload *who);
		void init_browser();
		void done_browser();
		void browser_ok();
		void init_browser2();
		void done_browser2();
		void browser_ok2();
		int apply_changes();
		void disable_ok_button();
		void enable_ok_button();
		void setup_entries();
		void toggle_time();
		void paste_url();
		void select_url();
		void clear_url();
		void set_url(char *a);
		void done();
		void popup();
		~tDEdit();
};

void init_edit_window(tDownload *what);
GList *make_glist_from_mylist(tHistory *parent);
GtkWidget *my_gtk_combo_new(tHistory *history);

#endif