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
#ifndef GTK_PREFS_WINDOW
#define GTK_PREFS_WINDOW
#include "../history.h"

void options_window_ok();
gint options_window_cancel();
void init_options_window(...);
void toggle_button_set_state(GtkToggleButton *tb,gboolean state);
GtkWidget *my_gtk_combo_new(tHistory *history);

#endif