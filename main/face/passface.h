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
#ifndef MY_FACE_FOR_PASS
#define MY_FACE_FOR_PASS

#include <gtk/gtk.h>
#include "about.h"
#include "../pass.h"

struct tPassDialog:public tDialog{
	GtkWidget *host_entry,*user_entry,*pass_entry;
	GtkWidget *proto_select;
	GtkWidget *frame;
	GtkWidget *cancel_button;
	tUserPass *data;
	int row;

	tPassDialog();
	int init();
	void done();
	~tPassDialog();
};

class tFacePass{
	protected:
	GtkWidget *window;
	GtkWidget *clist;
	GtkWidget *button;
	GtkWidget *add_button;
	GtkWidget *del_button;
	tPassDialog *dialog;
	void del_row(int row);
	public:
		tFacePass();
		void init();
		void add();
		void add(tUserPass *a);
		void edit_row(int row);
		void apply_dialog();
		void delete_rows();
		void open_dialog();
		void close();
		~tFacePass();
};


#endif
