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
#ifndef MY_GTK_BUTTONS_BAR
#define MY_GTK_BUTTONS_BAR

extern GtkWidget *ButtonsBar;
extern GtkWidget *HandleBox;
extern GtkWidget *buttons_array[];

GtkWidget *new_pixmap(char **xpm,char *themename);
void init_buttons_bar();
void prepare_buttons();
void set_dndtrash_button();
void set_speed_buttons();
void buttons_speed_set_text();
void buttons_cfg_init();
void buttons_configure();
void buttons_configure_close();
void bb_theme_changed();
void buttons_theme_changed();

enum {
	BUTTON_ADD=0,
	BUTTON_ADD_CLIPBOARD,
	BUTTON_DEL,
	BUTTON_STOP,
	BUTTON_CONTINUE,
	BUTTON_DEL_COMPLETED,
	BUTTON_UP,
	BUTTON_DOWN,
	BUTTON_LOG,
	BUTTON_SPEED1,
	BUTTON_SPEED2,
	BUTTON_SPEED3,
	BUTTON_OPTIONS,
	BUTTON_DEL_ALL,
	BUTTON_SAVE,
	BUTTON_LOAD,
	BUTTON_DND_TRASH,
	BUTTON_PROGRESS,
	BUTTON_CONFIGURE,
	BUTTON_LAST
};
#endif
