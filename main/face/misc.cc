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
/* including stdio.h only for NULL on BSD
 */
#include <package_config.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <gdk/gdkkeysyms.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xmd.h>
#include <string.h>

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
// buf is null		
		return NULL;
	}
	return buf;
}

void my_xclipboard_put(const char *buf) {
	int size;

	if (buf == NULL) buf = "";
	size = strlen(buf);
	XStoreBytes(GDK_DISPLAY(), buf, size);
/*	fprintf(stderr, "my_xplipboard_put(\"%s\");\n", buf);*/
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

static Atom _XA_WIN_HINTS;
static Atom _XA_WIN_LAYER;
static Atom _XA_WIN_STATE;
static int _xa_win_hints_allocated_=0;

enum WIN_HINTS_ENUM{
	WIN_HINTS_SKIP_FOCUS      = 1<<0,
	WIN_HINTS_SKIP_WINLIST    = 1<<1,
	WIN_HINTS_SKIP_TASKBAR    = 1<<2,
	WIN_HINTS_GROUP_TRANSIENT = 1<<3,
	WIN_HINTS_FOCUS_ON_CLICK  = 1<<4,
	WIN_HINTS_DO_NOT_COVER    = 1<<5
};

#define WIN_STATE_STICKY        1
#define WIN_LAYER_ONTOP		6

void wm_skip_window(GtkWidget *widget){
	if (!_xa_win_hints_allocated_){
		_XA_WIN_HINTS = XInternAtom(GDK_DISPLAY(), "_WIN_HINTS", False);
		_XA_WIN_LAYER = XInternAtom(GDK_DISPLAY(), "_WIN_LAYER", False);
		_XA_WIN_STATE = XInternAtom(GDK_DISPLAY(), "_WIN_STATE", False);
		_xa_win_hints_allocated_=1;
	};
	/* set always on top flag (GNOME) */
	XEvent xev;
	gint prev_error;
	prev_error = gdk_error_warnings;
	gdk_error_warnings = 0;
	xev.type = ClientMessage;
	xev.xclient.type = ClientMessage;
	xev.xclient.window = GDK_WINDOW_XWINDOW(widget->window);
	xev.xclient.message_type = _XA_WIN_LAYER;
	xev.xclient.format = 32;
	xev.xclient.data.l[0] = (CARD32) WIN_LAYER_ONTOP;
	XSendEvent(GDK_DISPLAY(), GDK_ROOT_WINDOW(), False,
		   SubstructureNotifyMask, (XEvent *) & xev);
	gdk_error_warnings = prev_error;
	/* set sticky */
	xev.type = ClientMessage;
	xev.xclient.type = ClientMessage;
	xev.xclient.window = GDK_WINDOW_XWINDOW(widget->window);
	xev.xclient.message_type = _XA_WIN_STATE;
	xev.xclient.format = 32;
	xev.xclient.data.l[0] = WIN_STATE_STICKY;
	xev.xclient.data.l[1] = WIN_STATE_STICKY;
	XSendEvent(GDK_DISPLAY(), GDK_ROOT_WINDOW(), False,
	    	   SubstructureNotifyMask, (XEvent *) & xev);
	/* set skip flags (GNOME) */ 
	long data[1];
	data[0] = WIN_HINTS_SKIP_FOCUS |
		WIN_HINTS_SKIP_WINLIST |
		WIN_HINTS_SKIP_TASKBAR;
	XChangeProperty(GDK_DISPLAY(), GDK_WINDOW_XWINDOW(widget->window), _XA_WIN_HINTS,
			XA_CARDINAL, 32, PropModeReplace,
			(unsigned char *) data,
			1);
};

/* write to buffer "nice" string representation of percentage:
 * %2.1f	- if 99 <= percent < 100
 * 100		- if percent >= 100
 * %2.0f		- overwise
 * (plus stuff to avoid unneeded "100.0" because of rounding...
 */
void d4x_percent_str(float percent, char *buf, int bufsize) {
	if (percent >= 99) {
		if (percent < 99.9)
			g_snprintf(buf, bufsize, "%2.1f", percent);
		else if (percent < 100)
			g_snprintf(buf, bufsize, "%2.1f", 99.9);
		else
			g_snprintf(buf, bufsize, "100");
	} else {
		g_snprintf(buf, bufsize, "%2.0f", percent);
	}
}


static gint _esc_handler_(GtkWidget *widget,GdkEvent *event,gpointer pointer){
	if (event && event->type == GDK_KEY_PRESS) {
		GdkEventKey *kevent=(GdkEventKey *)event;
		switch(kevent->keyval) {
		case GDK_Escape:{
			gtk_signal_emit_by_name(GTK_OBJECT(widget),
						"delete_event",
						widget,event,pointer);
			return TRUE;
			break;
		};
		};
	};
	return FALSE;
};

void d4x_eschandler_init(GtkWidget *widget,gpointer data){
	gtk_signal_connect(GTK_OBJECT(widget),
			   "key_press_event",
			   (GtkSignalFunc)_esc_handler_, data);
};
