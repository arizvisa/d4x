/*	WebDownloader for X-Window
 *	Copyright (C) 1999 Koshelev Maxim
 *	This Program is free but not GPL!!! You can't modify it
 *	without agreement with author. You can't distribute modified
 *	program but you can distribute unmodified program.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */
#ifndef MY_MISC_FACE
#define MY_MISC_FACE
#include <gdk/gdk.h>
#include <gtk/gtk.h>
void my_gdk_window_iconify(GdkWindow *window);
GdkPixmap *make_pixmap_from_xpm(GdkBitmap **mask,char **xpm);
GdkPixmap *make_pixmap_from_xpm(GdkBitmap **mask,char **xpm,GtkWidget *parent);
gchar *text_from_combo(GtkWidget *combo);
void text_to_combo(GtkWidget *combo,gchar *text);
void set_editable_for_combo(GtkWidget *widget,gboolean flag);
void motion_notify_get_coords(GdkEventMotion * event);
#endif
