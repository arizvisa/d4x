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
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <X11/Xlib.h>

extern GtkWidget *MainWindow;



void my_gdk_window_iconify(GdkWindow *window) {
	XIconifyWindow( GDK_DISPLAY() , GDK_WINDOW_XWINDOW(window),DefaultScreen(GDK_DISPLAY()));
};

GdkPixmap *make_pixmap_from_xpm(GdkBitmap **mask,char **xpm,GtkWidget *parent) {
	GtkStyle *style=gtk_widget_get_style(MainWindow);
	GdkPixmap *pixmap=gdk_pixmap_create_from_xpm_d (parent->window,
	                  mask,&style->bg[GTK_STATE_NORMAL] , xpm);
	GdkColormap *colormap = gtk_widget_get_colormap (MainWindow);
	GdkColor wait_color={0,0,0,0};
	gdk_color_alloc (colormap, &wait_color);
	return pixmap;
};

GdkPixmap *make_pixmap_from_xpm(GdkBitmap **mask,char **xpm) {
	return make_pixmap_from_xpm(mask,xpm,MainWindow);
};

gchar *text_from_combo(GtkWidget *combo) {
	return(gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(combo)->entry)));
};

void text_to_combo(GtkWidget *combo,gchar *text) {
	gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(combo)->entry),text);
};

void motion_notify_get_coords(GdkEventMotion * event){
    XEvent ev;
    gint i = 0;
    XSync(GDK_DISPLAY(), False);
    while (XCheckTypedEvent(GDK_DISPLAY(), MotionNotify, &ev)){
	event->x = ev.xmotion.x;
	event->y = ev.xmotion.y;
	i++;
    }
};
