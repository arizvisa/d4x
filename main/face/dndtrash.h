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
#ifndef MY_DNDTRASH_HEADER
#define MY_DNDTRASH_HEADER

#include <gtk/gtk.h>

void dnd_trash_init();
void dnd_trash_destroy();
void dnd_trash_real_destroy();
void dnd_trash_init_menu();
void dnd_trash_menu_prepare();

#endif
