/*	WebDownloader for X-Window
 *	Copyright (C) 1999-2001 Koshelev Maxim
 *	This Program is free but not GPL!!! You can't modify it
 *	without agreement with author. You can't distribute modified
 *	program but you can distribute unmodified program.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */
#ifndef T_GTK_ADD_DOWNLOAD_DIALOG
#define T_GTK_ADD_DOWNLOAD_DIALOG

extern GtkWidget *AddWindow; 
extern tDList *list_for_adding;
void init_add_window(...);
void init_add_clipboard_window(...);
void init_add_dnd_window(char *url,char *desc);
void init_edit_common_properties_window(int *array);
void d4x_automated_add();

#endif
