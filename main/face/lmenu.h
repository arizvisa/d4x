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
#ifndef MY_GTK_LIST_MENU
#define MY_GTK_LIST_MENU

extern GtkWidget *ListMenu;
GtkWidget *make_menu_item(char *name,char *accel,GdkPixmap *pixmap,GdkBitmap *bitmap);
void init_list_menu();
void list_menu_prepare();

enum {
	LM_LOG,
	LM_STOP,
	LM_CONTINUE,
	LM_EDIT,
	LM_DEL,
	LM_DELC,
	LM_DELF,
	LM_MOVEUP,
	LM_MOVEDOWN,
	LM_SET_LIMIT
};

#endif