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
#ifndef MY_GTK_BUTTONS_BAR
#define MY_GTK_BUTTONS_BAR

extern GtkWidget *ButtonsBar;
extern GtkWidget *HandleBox;

GtkWidget *new_pixmap(char **xpm);
void init_buttons_bar();
void prepare_buttons();
void set_dndtrash_button();

enum {
	BUTTON_ADD=0,
	BUTTON_ADD_CLIPBOARD,
	BUTTON_STOP,
	BUTTON_CONTINUE,
	BUTTON_DEL_COMPLETED,
	BUTTON_DEL,
	BUTTON_DEL_ALL,
	BUTTON_OPTIONS,
	BUTTON_UP,
	BUTTON_DOWN,
	BUTTON_LOG,
	BUTTON_SPEED1,
	BUTTON_SPEED2,
	BUTTON_SPEED3,
	BUTTON_SAVE,
	BUTTON_DND_TRASH
};
#endif