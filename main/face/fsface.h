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
#ifndef __MY_FTP_SEARCH_FACE_HEADER__
#define __MY_FTP_SEARCH_FACE_HEADER__

#include <gtk/gtk.h>
#include "../dlist.h"

enum FS_FACE_COLUMNS{
	FS_COL_ICON,
	FS_COL_NAME,
	FS_COL_SIZE,
	FS_COL_COUNT,
	FS_COL_LAST
};

//extern tDownload *FS_CUR_SELECTED;

void fs_list_remove(GtkTreeView *view,tDownload *what);
void fs_list_add(GtkTreeView *view,tDownload *what);
void fs_list_set_icon(GtkTreeView *view,tDownload *what,int icon);
void fs_list_set_count(GtkTreeView *view,tDownload *what);
GtkTreeView *fs_list_init();
GtkTreeView *fs_list_init_sublist();
void fs_list_set_size();
void fs_list_get_size();
void fs_list_hide();
void fs_list_show();
void fs_list_allocation(GtkWidget *paned,GtkAllocation *allocation);

#endif
