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
#include <stdio.h>
#include <pthread.h>
#include <string.h>


#include "../dlist.h"
#include "../var.h"
#include "../ntlocale.h"
#include "list.h"
#include "colors.h"
#include "about.h"
#include "misc.h"

GdkBitmap *log_ok_mask,*log_warning_mask,*log_error_mask,*log_from_mask,*log_to_mask;
GdkPixmap *log_ok_pixmap,*log_warning_pixmap,*log_error_pixmap,*log_from_pixmap,*log_to_pixmap;

struct tLogWindow {
	GtkWidget *window;
	GtkWidget *clist;
	GtkWidget *swindow;
	float value;
	tStringDialog *string;
	pthread_mutex_t mutex;
	tLogWindow();
	~tLogWindow();
};

tLogWindow::tLogWindow() {
	string=NULL;
};

tLogWindow::~tLogWindow() {
	if (string) delete string;
};

void init_pixmaps_for_log() {
#include "pixmaps2/ok.xpm"
#include "pixmaps2/from_server.xpm"
#include "pixmaps2/to_server.xpm"
#include "pixmaps2/error.xpm"
#include "pixmaps2/warning.xpm"
	log_ok_pixmap=make_pixmap_from_xpm(&log_ok_mask,ok_xpm);
	log_to_pixmap=make_pixmap_from_xpm(&log_to_mask,to_server_xpm);
	log_from_pixmap=make_pixmap_from_xpm(&log_from_mask,from_server_xpm);
	log_error_pixmap=make_pixmap_from_xpm(&log_error_mask,error_xpm);
	log_warning_pixmap=make_pixmap_from_xpm(&log_warning_mask,warning_xpm);
};

void log_window_destroy_by_log(void *a) {
	tLog *log=(tLog *) a;
	if (log) {
		tLogWindow *temp=(tLogWindow *)log->Window;
		if (temp) {
			pthread_mutex_lock(&temp->mutex);
			log->Window=NULL;
			gtk_widget_destroy(GTK_WIDGET(temp->window));
			pthread_mutex_unlock(&temp->mutex);
			delete (temp);
		};
	};

};

int log_window_destroy(GtkWidget *window,GdkEvent *event, tLog *log) {
	if (log) {
		tLogWindow *temp=(tLogWindow *)log->Window;
		if (temp) {
			pthread_mutex_lock(&temp->mutex);
			log->Window=NULL;
			if (window->window) {
				int a[4];
				gdk_window_get_position(window->window,&a[0],&a[1]);
				gdk_window_get_size(window->window,&a[2],&a[3]);
				log->store_geometry(a);
			};
			gtk_widget_destroy(GTK_WIDGET(window));
			pthread_mutex_unlock(&temp->mutex);
			delete (temp);
		};
	};
	return TRUE;
};

void log_window_add_string(tLog *log,tLogString *str) {
	tLogWindow *temp=(tLogWindow *)log->Window;
	if (!temp) return;
	pthread_mutex_lock(&temp->mutex);
	char a[MAX_LEN],useless[MAX_LEN],useful[MAX_LEN];
	sprintf(a,"%s ",ctime(&str->time));
	sscanf(a,"%s %s %s %s",useless,useless,useless,useful);
	char *data[]={NULL,useful,str->body};
	int row=gtk_clist_append(GTK_CLIST(temp->clist),data);
	GdkColor color,back_color;
	switch (str->type) {
		case LOG_OK:
			{
				gtk_clist_set_pixmap (GTK_CLIST (temp->clist), row,
				                      0, log_ok_pixmap, log_ok_mask);
				color=BLACK;
				back_color=WHITE;
				break;
			};
		case LOG_TO_SERVER:
			{
				gtk_clist_set_pixmap (GTK_CLIST (temp->clist), row,
				                      0, log_to_pixmap, log_to_mask);
				color=CYAN;
				back_color=LCYAN;
				break;
			};
		case LOG_FROM_SERVER:
			{
				gtk_clist_set_pixmap (GTK_CLIST (temp->clist), row,
				                      0, log_from_pixmap, log_from_mask);
				color=BLUE;
				back_color=LBLUE;
				break;
			};
		case LOG_WARNING:
			{
				gtk_clist_set_pixmap (GTK_CLIST (temp->clist), row,
				                      0, log_warning_pixmap, log_warning_mask);
				color=GREEN;
				back_color=LGREEN;
				break;
			};
		case LOG_ERROR:
			{
				gtk_clist_set_pixmap (GTK_CLIST (temp->clist), row,
				                      0, log_error_pixmap, log_error_mask);
				color=RED;
				back_color=LRED;
				break;
			};
		default:
			color=BLACK;
	};
	GdkColormap *colormap = gtk_widget_get_colormap (temp->window);
	gdk_color_alloc (colormap, &color);
	gdk_color_alloc (colormap, &back_color);
	gtk_clist_set_foreground(GTK_CLIST(temp->clist),row,&color);
	gtk_clist_set_background(GTK_CLIST(temp->clist),row,&back_color);
	pthread_mutex_unlock(&temp->mutex);
};

static void log_window_event_handler(	GtkWidget *clist, gint row, gint column,
                                      GdkEventButton *event,tDownload *what) {
	if (event && event->type==GDK_2BUTTON_PRESS && event->button==1) {
		tLog *log=what->LOG;
		if (!log || !log->Window) return;
		tLogWindow *temp=(tLogWindow *)(log->Window);
		if (temp->string==NULL) temp->string=new tStringDialog;
		char data[MAX_LEN];
		sprintf(data,_("Row number %i [log of %s]"),row+1,what->info->file);
		char *text;
		gtk_clist_get_text(GTK_CLIST(clist),row,2,&text);
		temp->string->init(text,data);
	};
};

static void my_gtk_auto_scroll( GtkAdjustment *get,tLog *log){
	if (get==NULL || log==NULL) return;
	tLogWindow *temp=(tLogWindow *)log->Window;
	if (temp->value==get->value && get->value<get->upper-get->page_size) {
		get->value=get->upper-get->page_size;
		temp->value=get->value;
		gtk_signal_emit_by_name (GTK_OBJECT (get), "changed");
	} else
		temp->value=get->value;
}

void log_window_init(tDownload *what) {
	gchar *titles[]={"","",""};
	if (what && what->LOG) {
		if (what->LOG->Window) {
			tLogWindow *temp=(tLogWindow *)what->LOG->Window;
			gdk_window_show(temp->window->window);
			return;
		};
		tLogWindow *temp=new tLogWindow;
		pthread_mutex_init(&temp->mutex,NULL);
		temp->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		gtk_widget_set_usize( GTK_WIDGET (temp->window), 400, 150);
		char title[MAX_LEN];
		title[0]=0;
		strcat(title,_("Log: "));
		strcat(title,what->info->file);
		gtk_window_set_title(GTK_WINDOW (temp->window), title);
		gtk_signal_connect(GTK_OBJECT(temp->window), "delete_event",
		                   (GtkSignalFunc)log_window_destroy, what->LOG);

		temp->clist = gtk_clist_new_with_titles( 3, titles);
		gtk_signal_connect(GTK_OBJECT(temp->clist),"select_row",GTK_SIGNAL_FUNC(log_window_event_handler),what);
		gtk_clist_column_titles_hide(GTK_CLIST(temp->clist));
		gtk_clist_set_shadow_type (GTK_CLIST(temp->clist), GTK_SHADOW_IN);
		gtk_clist_set_column_width (GTK_CLIST(temp->clist), 0 , 16);
		gtk_clist_set_column_width (GTK_CLIST(temp->clist), 1 , 50);
		gtk_clist_set_column_auto_resize(GTK_CLIST(temp->clist),2,TRUE);
		gtk_clist_set_column_auto_resize(GTK_CLIST(temp->clist),3,TRUE);

		GtkAdjustment *adj = (GtkAdjustment *)gtk_adjustment_new (0.0, 0.0, 0.0, 0.1, 1.0, 1.0);

		temp->swindow=gtk_scrolled_window_new(NULL,adj);
		gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (temp->swindow),
		                                GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
		gtk_container_add(GTK_CONTAINER(temp->swindow),temp->clist);
		gtk_container_add(GTK_CONTAINER(temp->window),temp->swindow);

		what->LOG->Window=temp;

		gtk_object_set_user_data(GTK_OBJECT(temp->window),what->LOG);
		gtk_widget_show_all(temp->window);
		int a[4];
		what->LOG->get_geometry(a);
		if (a[3]!=0 && a[2]!=0)
			gdk_window_move_resize(temp->window->window,a[0],a[1],a[2],a[3]);

		what->LOG->print();

		gtk_signal_connect (GTK_OBJECT(adj), "changed",GTK_SIGNAL_FUNC(my_gtk_auto_scroll), what->LOG);
		adj->value=adj->upper-adj->page_size;
		temp->value=adj->value;
		gtk_signal_emit_by_name (GTK_OBJECT (adj), "changed");
	};
};

void log_window_set_title(tDownload *what,char *title) {
	if (what && what->LOG && what->LOG->Window) {
		tLogWindow *temp=(tLogWindow *)what->LOG->Window;
		gtk_window_set_title(GTK_WINDOW (temp->window), title);
	};
};

void del_first_from_log(tLog *what) {
	tLogWindow *temp=(tLogWindow *)what->Window;
	if (temp) {
		pthread_mutex_lock(&temp->mutex);
		gtk_clist_remove(GTK_CLIST(temp->clist),0);
		pthread_mutex_unlock(&temp->mutex);
	};

};
