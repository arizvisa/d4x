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
#ifndef __DFORX_PREFERENCE_HEADER__
#define __DFORX_PREFERENCE_HEADER__

void d4x_prefs_init();
gint d4x_prefs_cancel();
void d4x_prefs_init_page(int page_num);
void toggle_button_set_state(GtkToggleButton *tb,gboolean state);
void d4x_prefs_init_pre();

enum PREFS_PAGES_ENUM{
	PREFS_PAGE_MAINLOG,
	PREFS_PAGE_MAIN
};


#endif
