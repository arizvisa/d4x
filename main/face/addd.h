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
#ifndef T_GTK_ADD_DOWNLOAD_DIALOG
#define T_GTK_ADD_DOWNLOAD_DIALOG

extern GtkWidget *AddWindow; 
extern GtkWidget *UrlEntry,*SavePathEntry;
void init_add_window(...);
void init_add_clipboard_window(...);
void init_add_dnd_window(char *url);
void add_window_cancel();

#endif