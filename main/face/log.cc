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

#include <stdio.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <pthread.h>
#include <string.h>

#include "../dlist.h"
#include "../locstr.h"
#include "../var.h"
#include "../ntlocale.h"
#include "list.h"
#include "colors.h"
#include "about.h"
#include "misc.h"

enum LOG_COLUMNS{
	L_COL_TYPE,
	L_COL_NUM,
	L_COL_TIME,
	L_COL_STRING,
	L_COL_LAST
};


GdkBitmap *log_ok_mask,*log_warning_mask,*log_error_mask,*log_from_mask,*log_to_mask;
GdkPixmap *log_ok_pixmap,*log_warning_pixmap,*log_error_pixmap,*log_from_pixmap,*log_to_pixmap;

struct tLogWindow {
	GtkWidget *window;
	GtkWidget *clist;
	GtkWidget *swindow;
	float value;
	tStringDialog *string;
	tLogWindow();
	~tLogWindow();
};

tLogWindow::tLogWindow() {
	string=(tStringDialog *)NULL;
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
			log->Window=NULL;
			gtk_widget_destroy(GTK_WIDGET(temp->window));
			delete (temp);
		};
	};

};

int log_window_destroy(GtkWidget *window,GdkEvent *event, tLog *log) {
	if (log) {
		tLogWindow *temp=(tLogWindow *)log->Window;
		if (temp) {
			log->Window=NULL;
			if (window->window) {
				int a[4];
				gdk_window_get_position(window->window,&a[0],&a[1]);
				gdk_window_get_size(window->window,&a[2],&a[3]);
				log->store_geometry(a);
			};
			gtk_widget_destroy(GTK_WIDGET(window));
			delete (temp);
		};
	};
	return TRUE;
};

void log_window_add_string(tLog *log,tLogString *str) {
	tLogWindow *temp=(tLogWindow *)log->Window;
	if (!temp) return;
	char useful[MAX_LEN+1];
	struct tm msgtime;
	localtime_r(&(str->time),&msgtime);
	strftime(useful,MAX_LEN,"%T",&msgtime);
	/* replace all nonprint symbols by space */
	char *str_temp=copy_string(str->body);
	str_non_print_replace(str_temp,' ');
	char *data[L_COL_LAST];
	data[L_COL_TYPE]=NULL;
	data[L_COL_TIME]=useful;
	data[L_COL_STRING]=str_temp;

	char row_num[MAX_LEN];
	sprintf(row_num,"[%i]",str->temp);
	data[L_COL_NUM]=row_num;

	int row=gtk_clist_append(GTK_CLIST(temp->clist),data);
	delete (str_temp);
	GdkColor color,back_color;
	switch (str->type) {
		case LOG_OK:
			{
				gtk_clist_set_pixmap (GTK_CLIST (temp->clist), row,
				                      L_COL_TYPE, log_ok_pixmap, log_ok_mask);
				color=BLACK;
				back_color=WHITE;
				break;
			};
		case LOG_TO_SERVER:
			{
				gtk_clist_set_pixmap (GTK_CLIST (temp->clist), row,
				                      L_COL_TYPE, log_to_pixmap, log_to_mask);
				color=CYAN;
				back_color=LCYAN;
				break;
			};
		case LOG_FROM_SERVER:
			{
				gtk_clist_set_pixmap (GTK_CLIST (temp->clist), row,
				                      L_COL_TYPE, log_from_pixmap, log_from_mask);
				color=BLUE;
				back_color=LBLUE;
				break;
			};
		case LOG_WARNING:
			{
				gtk_clist_set_pixmap (GTK_CLIST (temp->clist), row,
				                      L_COL_TYPE, log_warning_pixmap, log_warning_mask);
				color=GREEN;
				back_color=LGREEN;
				break;
			};
		case LOG_ERROR:
			{
				gtk_clist_set_pixmap (GTK_CLIST (temp->clist), row,
				                      L_COL_TYPE, log_error_pixmap, log_error_mask);
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
};

static gint log_window_event_handler(GtkWidget *window,GdkEvent *event,tLog *log){
	if (event && event->type == GDK_KEY_PRESS) {
		GdkEventKey *kevent=(GdkEventKey *)event;
		switch(kevent->keyval) {
		case GDK_Escape:{
//			gtk_signal_emit_by_name(GTK_OBJECT(window),"delete_event");
			log_window_destroy_by_log(log);
			return TRUE;
			break;
		};
		};
	};
	return FALSE;
};

static gint log_list_event_handler(	GtkWidget *clist, gint row, gint column,
                                      GdkEventButton *event,tDownload *what) {
	if (event && event->type==GDK_2BUTTON_PRESS && event->button==1) {
		tLog *log=what->LOG;
		if (!log || !log->Window) return FALSE;
		tLogWindow *temp=(tLogWindow *)(log->Window);
		if (temp->string==NULL) temp->string=new tStringDialog;
		char data[MAX_LEN];
		sprintf(data,_("Row number %i [log of %s]"),row+1,what->info->get_file());
		char *text;
		gtk_clist_get_text(GTK_CLIST(clist),row,L_COL_STRING,&text);
		temp->string->init(text,data);
		return TRUE;
	};
	return FALSE;
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
	gchar *titles[L_COL_LAST];
	for (int i=0;i<L_COL_LAST;i++)
		titles[i]="";
	if (what) {
		if (what->LOG==NULL){
			what->LOG=new tLog;
			what->LOG->init(CFG.MAX_LOG_LENGTH);
		};
		if (what->LOG->Window) {
			tLogWindow *temp=(tLogWindow *)what->LOG->Window;
			gdk_window_show(temp->window->window);
			return;
		};
		what->LOG->lock();
		tLogWindow *temp=new tLogWindow;
		temp->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		int a[4];
		what->LOG->get_geometry(a);
		gtk_widget_set_usize( GTK_WIDGET (temp->window), 400, 200);
		if (a[3]!=0 && a[2]!=0){
			gtk_window_set_default_size( GTK_WINDOW (temp->window), a[2], a[3]);
			gtk_widget_set_uposition( GTK_WIDGET (temp->window), a[0], a[1]);
		};
		char title[MAX_LEN];
		title[0]=0;
		strcat(title,_("Log: "));
		strcat(title,what->info->get_file());
		gtk_window_set_title(GTK_WINDOW (temp->window), title);
		gtk_signal_connect(GTK_OBJECT(temp->window), "key_press_event",
		                   (GtkSignalFunc)log_window_event_handler, what->LOG);
		gtk_signal_connect(GTK_OBJECT(temp->window), "delete_event",
		                   (GtkSignalFunc)log_window_destroy, what->LOG);

		temp->clist = gtk_clist_new_with_titles(L_COL_LAST, titles);
		gtk_signal_connect(GTK_OBJECT(temp->clist),"select_row",GTK_SIGNAL_FUNC(log_list_event_handler),what);
		gtk_clist_column_titles_hide(GTK_CLIST(temp->clist));
		gtk_clist_set_shadow_type (GTK_CLIST(temp->clist), GTK_SHADOW_IN);
		gtk_clist_set_column_width (GTK_CLIST(temp->clist), L_COL_TYPE , 16);
		gtk_clist_set_column_width (GTK_CLIST(temp->clist), L_COL_NUM , 16);
		gtk_clist_set_column_width (GTK_CLIST(temp->clist), L_COL_TIME , 50);
		gtk_clist_set_column_auto_resize(GTK_CLIST(temp->clist),L_COL_NUM,TRUE);
		gtk_clist_set_column_auto_resize(GTK_CLIST(temp->clist),L_COL_TIME,TRUE);
		gtk_clist_set_column_auto_resize(GTK_CLIST(temp->clist),L_COL_STRING,TRUE);

		GtkAdjustment *adj = (GtkAdjustment *)gtk_adjustment_new (0.0, 0.0, 0.0, 0.1, 1.0, 1.0);

		temp->swindow=gtk_scrolled_window_new((GtkAdjustment*)NULL,adj);
		gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (temp->swindow),
		                                GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
		gtk_container_add(GTK_CONTAINER(temp->swindow),temp->clist);
		gtk_container_add(GTK_CONTAINER(temp->window),temp->swindow);

		what->LOG->Window=temp;

		gtk_object_set_user_data(GTK_OBJECT(temp->window),what->LOG);
//		if (a[3]!=0 && a[2]!=0)
//			gdk_window_move_resize(temp->window->window,a[0],a[1],a[2],a[3]);

		GtkStyle *current_style =gtk_style_copy(gtk_widget_get_style(GTK_WIDGET(temp->clist)));
		gdk_font_unref(current_style->font);
		current_style->font = gdk_font_load("-*-fixed-medium-*-*-*-*-120-*-*-*-*-*-*");;
		gtk_widget_set_style(GTK_WIDGET(temp->clist), current_style);

		gtk_widget_show_all(temp->window);
		gtk_clist_freeze(GTK_CLIST(temp->clist));
		what->LOG->print();
		what->LOG->unlock();
		gtk_clist_thaw(GTK_CLIST(temp->clist));

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
		gtk_clist_remove(GTK_CLIST(temp->clist),0);
	};

};
