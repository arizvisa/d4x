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
#ifndef MY_GTK_ABOUT
#define MY_GTK_ABOUT

void init_about_window(...);

struct tDialog{
	GtkWidget *window;
	GtkWidget *ok_button;
	tDialog();
	virtual void done();
	virtual ~tDialog();
};

struct tDialogWidget:public tDialog{
	GtkWidget *label;
	GtkWidget *cancel_button;
	tDialogWidget();
	int init(char *ask,char *title);
	~tDialogWidget();
};

struct tStringDialog:public tDialog{
	GtkWidget *entry;
	GtkWidget *frame;
	tStringDialog();
	int init(char *str,char *title);
	~tStringDialog();
};

#endif