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
#ifndef MY_FACE_FOR_LIMITS
#define MY_FACE_FOR_LIMITS

#include <gtk/gtk.h>
#include "about.h"

struct tLimitDialog:public tDialog{
	GtkWidget *host_entry,*port_entry,*limit_entry;
	GtkWidget *frame;
	GtkWidget *cancel_button;
	char *oldhost;
	int oldport;
	tLimitDialog();
	int init();
	void set_old(char *host,int port);
	void done();
	void reset_old();
	~tLimitDialog();
};

class tFaceLimits{
	protected:
	GtkWidget *window;
	GtkWidget *clist;
	GtkWidget *button;
	GtkWidget *add_button;
	GtkWidget *del_button;
	GtkWidget *default_entry;
	tLimitDialog *dialog;
	int size1,size2;
	public:
		tFaceLimits();
		void init();
		void add(char *host,int port);
		void open_dialog();
		void redraw();
		void apply_dialog();
		void update_row(int row);
		void open_row(int row);
		void delete_rows();
		void set_default();
		void get_sizes();
		void close();
		~tFaceLimits();
};


#endif
