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

#ifndef T_MAIN_LOG_STRING
#define T_MAIN_LOG_STRING
#include <time.h>
#include <gtk/gtk.h>
#include "log.h"
#include "liststr.h"
#include "face/about.h"

class tMLog:public tStringList{
	protected:
	int current_line;
	time_t start;
	GtkCList *list;
	GtkWidget *open_row_item;
	GtkWidget *clear_item;
	void add_to_list();
	int fd;
	tStringDialog *string;
	public:
		GtkWidget *popup_menu;
		tMLog();
		void open_row(int row);
		void open_selected_row();
		void print();
		void reinit(int a);
		void init_list(GtkCList *clist);
		void add(char *str,int len,int type);
		void add(char *str,int type);
		void add(char *str);
		void dispose();
		void reinit_file();
		void myprintf(int type,char *fmt,...);
		int popup(GdkEventButton *event);
		tLogString *last();
		tLogString *next();
		tLogString *first();
		void done();
		~tMLog();
};

enum MAIN_LOG_COLUMNS{
	ML_COL_NUM,
	ML_COL_TIME,
	ML_COL_DATE,
	ML_COL_STRING,
	ML_COL_LAST
};

#endif
