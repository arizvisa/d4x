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
#ifndef MY_FACE_FOR_PASS
#define MY_FACE_FOR_PASS

#include <gtk/gtk.h>
#include "../dlist.h"

class tFacePass{
protected:
	GtkWidget *window;
	GtkWidget *clist;
	GtkWidget *button;
	GtkWidget *add_button;
	GtkWidget *del_button;
	tDList dlist,addlist;
	void del_row(int row);
	void show_url(tDownload *dwn);
public:
	tFacePass();
	void init();
	void edit_row(int row);
	void apply_dialog();
	void addlist_del(tDownload *dwn);
	void addlist_add(tDownload *dwn);
	void redraw_url(tDownload *dwn);
	void delete_rows();
	void open_dialog();
	void save();
	void load();
	void close();
	void set_cfg(tDownload *dwn);
	~tFacePass();
};

extern tFacePass *FaceForPasswords;

#endif
