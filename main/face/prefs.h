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
#ifndef GTK_PREFS_WINDOW
#define GTK_PREFS_WINDOW
#include "../history.h"


void options_window_ok();
gint options_window_cancel();
void init_options_window();
void init_options_window_page(int page_num);
void toggle_button_set_state(GtkToggleButton *tb,gboolean state);
GtkWidget *my_gtk_combo_new(tHistory *history);

enum PREFS_PAGES_ENUM{
	PREFS_PAGE_COMMON,
	PREFS_PAGE_LIMITS,
	PREFS_PAGE_OTHER,
	PREFS_PAGE_MAINLOG,
	PREFS_PAGE_CULUMNS,
	PREFS_PAGE_PROXY,
	PREFS_PAGE_CONFIRMATIONS,
	PREFS_PAGE_SPEED,
	PREFS_PAGE_CLIPBOARD
};
 
#endif
