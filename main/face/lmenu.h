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
#ifndef MY_GTK_LIST_MENU
#define MY_GTK_LIST_MENU

extern GtkWidget *ListMenu;
extern GtkWidget *ListMenuArray[];
GtkWidget *make_menu_item(char *name,char *accel,GdkPixmap *pixmap,GdkBitmap *bitmap,int size);
void init_list_menu();
void list_menu_prepare();

enum {
	LM_LOG,
	LM_STOP,
	LM_CONTINUE,
	LM_EDIT,
	LM_EDIT_COMMON,
	LM_DEL,
	LM_MOVEUP,
	LM_MOVEDOWN,
	LM_SET_LIMIT,
	LM_DELC,
	LM_DELF
};

#endif
