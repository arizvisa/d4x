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
/* including stdio.h only for NULL on BSD
 */
#include <stdio.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <X11/Xlib.h>

extern GtkWidget *MainWindow;



void my_gdk_window_iconify(GdkWindow *window) {
	XIconifyWindow( GDK_DISPLAY() , GDK_WINDOW_XWINDOW(window),DefaultScreen(GDK_DISPLAY()));
//	XUnmapWindow( GDK_DISPLAY(), GDK_WINDOW_XWINDOW(window));
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
	if (GTK_IS_COMBO(combo))
		return(gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(combo)->entry)));
	else
		return(gtk_entry_get_text(GTK_ENTRY(combo)));
};

void text_to_combo(GtkWidget *widget,gchar *text) {
	if (GTK_IS_COMBO(widget))
		gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(widget)->entry),text);
	else
		if (GTK_IS_ENTRY(widget))
			gtk_entry_set_text(GTK_ENTRY(widget),text);
};

void set_editable_for_combo(GtkWidget *widget,gboolean flag){
	if (GTK_IS_COMBO(widget))
		gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(widget)->entry),flag);
	else
		if (GTK_IS_ENTRY(widget))
			gtk_entry_set_editable(GTK_ENTRY(widget),flag);
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

char *my_xclipboard_get() {
	char *buf;
	int buf_size;

	buf = XFetchBytes(GDK_DISPLAY(), &buf_size);
	if (buf_size == 0) {
		XFree(buf);
		return NULL;
	}
	return buf;
}

void my_xclipboard_free(char *buf) {
	if (buf) XFree(buf);
}

GtkWidget *my_gtk_entry_new_with_max_length(gint length, int val){
	GtkWidget *entry=gtk_entry_new_with_max_length(length);
	char tmp[length+2];
	g_snprintf(tmp,length+1,"%i",val);
	gtk_entry_set_text(GTK_ENTRY(entry),tmp);
	GtkStyle *style = gtk_widget_get_style(entry);
	gint real_size=gdk_string_width(style->font,"00");
	real_size=(real_size*(length+1))/2+3;
	gtk_widget_set_usize(entry,real_size,-1);
	return(entry);
};
