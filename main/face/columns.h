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
#ifndef MY_GTK_COLUMNS_PREFS
#define MY_GTK_COLUMNS_PREFS

#include "lod.h"
#include "../queue.h"
#include "../dlist.h"

class tColumnsPrefs{
	protected:
	int tmp_apply[NOTHING_COL];
	int tmp_apply_flag;
	GtkWidget *box;
	GtkWidget *frame;
	GtkWidget *columns[NOTHING_COL];
	int first;
	GList *sort_list;
	GList *find_by_data(GList *where, char *what);	
	void add_to_list(tDList *list);
	void add_to_sort(tDownload *what);
	public:
		tColumnsPrefs();
		void init();
		void apply_changes();
		void apply_changes_tmp();
		void reset();
		GtkWidget *body();
		~tColumnsPrefs(){};
};

#endif
