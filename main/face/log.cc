/*	WebDownloader for X-Window
 *	Copyright (C) 1999-2001 Koshelev Maxim
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
	GtkAdjustment *adj;
	GtkWidget *button;
	GtkWidget *toolbar;
	tDownload *papa; // :))
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

gint log_window_button(GtkWidget *button,int a);

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

void log_window_remember_geometry(GtkWidget *window, tLogWindow *temp){
	if (window->window) {
		int a[4];
		gdk_window_get_root_origin(window->window,&a[0],&a[1]);
		gdk_window_get_size(window->window,&a[2],&a[3]);
		if (temp->papa && temp->papa->LOG)
			temp->papa->LOG->store_geometry(a);
	};
};

void log_window_destroy_by_log(void *a) {
	tLog *log=(tLog *) a;
	if (log==NULL) return;
	tLogWindow *temp=(tLogWindow *)log->Window;
	if (temp) {
		log->Window=NULL;
		log_window_remember_geometry(temp->window,temp);
		temp->papa->CurrentLog=temp->papa->LOG;
		gtk_widget_destroy(GTK_WIDGET(temp->window));
		delete (temp);
	};
};

int log_window_destroy(GtkWidget *window,GdkEvent *event, tLog *log) {
	if (log) {
		tLogWindow *temp=(tLogWindow *)log->Window;
		if (temp) {
			temp->papa->CurrentLog=NULL;
			log->Window=NULL;
			log_window_remember_geometry(window,temp);
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
	delete[] str_temp;
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


static gint log_list_event_handler(	GtkWidget *clist, gint row, gint column,
                                      GdkEventButton *event,tLogWindow *temp) {
	if (temp && event && event->type==GDK_2BUTTON_PRESS &&
	    event->button==1 && temp->papa) {
		if (temp->string==NULL) temp->string=new tStringDialog;
		char data[MAX_LEN];
		sprintf(data,_("Row number %i [log of %s]"),row+1,temp->papa->info->file.get());
		char *text;
		gtk_clist_get_text(GTK_CLIST(clist),row,L_COL_STRING,&text);
		temp->string->init(text,data);
		return TRUE;
	};
	return FALSE;
};

static void my_gtk_auto_scroll( GtkAdjustment *get,tLogWindow *temp){
	if (get==NULL || temp==NULL) return;
	if (temp->value==get->value && get->value<get->upper-get->page_size) {
		get->value=get->upper-get->page_size;
		temp->value=get->value;
		gtk_signal_emit_by_name (GTK_OBJECT (get), "value_changed");
	} else
		temp->value=get->value;
}

gint log_window_button(GtkWidget *button,int a);

static gint log_window_event_handler(GtkWidget *window,GdkEvent *event,tLog *log){
	if (event && event->type == GDK_KEY_PRESS) {
		GdkEventKey *kevent=(GdkEventKey *)event;
		tLogWindow *wnd=(tLogWindow *)log->Window;
		int num=0;
		if (kevent->state & GDK_CONTROL_MASK){
			switch(kevent->keyval) {
			case GDK_1:
				num=1;
				break;
			case GDK_2:
				num=2;
				break;
			case GDK_3:
				num=3;
				break;
			case GDK_4:
				num=4;
				break;
			case GDK_5:
				num=5;
				break;
			case GDK_6:
				num=6;
				break;
			case GDK_7:
				num=7;
				break;
			case GDK_8:
				num=8;
				break;
			case GDK_9:
				num=9;
				break;
			case GDK_0:
				num=10;
				break;
			};
		};
		if (num){
			GList *list=GTK_TOOLBAR(wnd->toolbar)->children;
			int a=1;
			while (list && num>a){
				list=list->next;
				a++;
			};
			if (list){
				GtkToolbarChild *chld=(GtkToolbarChild*)list->data;
				gtk_signal_emit_by_name(GTK_OBJECT(chld->widget),"clicked",num);
			};
		};
		if (kevent->keyval==GDK_Escape){
//			gtk_signal_emit_by_name(GTK_OBJECT(window),"delete_event");
			log_window_destroy_by_log(log);
			return TRUE;
		};
	};
	return FALSE;
};

gint log_window_button(GtkWidget *button,int a){
	tDownload *what=(tDownload *)gtk_object_get_user_data(GTK_OBJECT(button));
	tDownload *withlog=what;
	while (withlog){
		if (withlog->LOG->Window) break;
		withlog=withlog->split->next_part;
	};
	if (what->split==NULL){
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(((tLogWindow *)(withlog->LOG->Window))->button),TRUE);
		return FALSE;
	};
	if (withlog==NULL || withlog->LOG->Window==NULL)
		return FALSE;
	tDownload *forlog=what;
	int b=a;
	while (forlog){
		a-=1;
		if (a==0) break;
		forlog=forlog->split->next_part;
	};
	if (forlog && forlog!=withlog && forlog->LOG!=NULL){
		withlog->LOG->lock();
		forlog->LOG->lock();
		forlog->LOG->Window=withlog->LOG->Window;
		withlog->LOG->Window=NULL;
		withlog->LOG->unlock();
		tLogWindow *temp=(tLogWindow *)(forlog->LOG->Window);
		gtk_object_set_user_data(GTK_OBJECT(temp->window),forlog->LOG);
		gtk_clist_freeze(GTK_CLIST(temp->clist));
		gtk_clist_clear(GTK_CLIST(temp->clist));
		forlog->LOG->print();
		forlog->LOG->unlock();
		what->LOG->last_log=b;
		what->CurrentLog=forlog->LOG;
		/* FIXME: signal_connect again???? */
		gtk_signal_disconnect_by_data(GTK_OBJECT(temp->window),
					      withlog->LOG);
		gtk_signal_connect(GTK_OBJECT(temp->window),
				   "delete_event",
		                   (GtkSignalFunc)log_window_destroy,
				   forlog->LOG);
		gtk_signal_connect(GTK_OBJECT(temp->window), "key_press_event",
		                   (GtkSignalFunc)log_window_event_handler, forlog->LOG);
		/* GTK is buggy if we 'thaw' list after sending signal */
		gtk_signal_emit_by_name (GTK_OBJECT (temp->adj), "changed");
		gtk_clist_thaw(GTK_CLIST(temp->clist));
	};
	if (forlog==NULL || forlog->LOG==NULL){
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(((tLogWindow *)(withlog->LOG->Window))->button),TRUE);
		what->LOG->last_log=1;
	};
	return TRUE;
};


void log_window_init(tDownload *what) {
	gchar *titles[L_COL_LAST];
	for (int i=0;i<L_COL_LAST;i++)
		titles[i]="";
	if (what) {
		if (what->LOG==NULL){
			what->LOG=new tLog;
			what->LOG->init(CFG.MAX_LOG_LENGTH);
			what->LOG->ref_inc();
		};
		if (what->LOG->Window) {
			tLogWindow *temp=(tLogWindow *)what->LOG->Window;
			gdk_window_show(temp->window->window);
			return;
		}else{
			if (what->split){
				tDownload *next_part=what->split->next_part;
				while (next_part){
					if (next_part->LOG && next_part->LOG->Window){
						tLogWindow *temp=(tLogWindow *)(next_part->LOG->Window);
						gdk_window_show(temp->window->window);
						return;
					};
					next_part=next_part->split->next_part;
				};
			};
		};
		what->LOG->lock();
		tLogWindow *temp=new tLogWindow;
		temp->papa=what;
		temp->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		gtk_window_set_wmclass(GTK_WINDOW(temp->window),
				       "D4X_Log","D4X");
		int a[4];
		what->LOG->get_geometry(a);
		if (a[3]!=0 && a[2]!=0){
			gtk_widget_set_uposition( GTK_WIDGET (temp->window), a[0], a[1]);
			gtk_window_set_default_size( GTK_WINDOW (temp->window), a[2], a[3]);
		}else
			gtk_widget_set_usize( GTK_WIDGET (temp->window), 400, 200);
		char title[MAX_LEN];
		title[0]=0;
		strcat(title,_("Log: "));
		strcat(title,what->info->file.get());
		gtk_window_set_title(GTK_WINDOW (temp->window), title);
		gtk_signal_connect(GTK_OBJECT(temp->window), "key_press_event",
		                   (GtkSignalFunc)log_window_event_handler, what->LOG);
		gtk_signal_connect(GTK_OBJECT(temp->window), "delete_event",
		                   (GtkSignalFunc)log_window_destroy, what->LOG);

		temp->clist = gtk_clist_new_with_titles(L_COL_LAST, titles);
		gtk_signal_connect(GTK_OBJECT(temp->clist),"select_row",GTK_SIGNAL_FUNC(log_list_event_handler),temp);
		gtk_clist_column_titles_hide(GTK_CLIST(temp->clist));
		gtk_clist_set_shadow_type (GTK_CLIST(temp->clist), GTK_SHADOW_IN);
		gtk_clist_set_column_width (GTK_CLIST(temp->clist), L_COL_TYPE , 18);
		gtk_clist_set_column_width (GTK_CLIST(temp->clist), L_COL_NUM , 16);
		gtk_clist_set_column_width (GTK_CLIST(temp->clist), L_COL_TIME , 50);
		gtk_clist_set_column_auto_resize(GTK_CLIST(temp->clist),L_COL_NUM,TRUE);
		gtk_clist_set_column_auto_resize(GTK_CLIST(temp->clist),L_COL_TIME,TRUE);
		gtk_clist_set_column_auto_resize(GTK_CLIST(temp->clist),L_COL_STRING,TRUE);

		temp->adj = (GtkAdjustment *)gtk_adjustment_new (0.0, 0.0, 0.0, 0.1, 1.0, 1.0);

		GtkWidget *swindow=gtk_scrolled_window_new((GtkAdjustment*)NULL,temp->adj);
		gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swindow),
		                                GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
		gtk_container_add(GTK_CONTAINER(swindow),temp->clist);
		if (what->split){
			GtkWidget *buttonsbar=temp->toolbar=gtk_toolbar_new (GTK_ORIENTATION_HORIZONTAL, GTK_TOOLBAR_TEXT);
			GtkWidget *tmpbutton=NULL;
			for (int i=1;i<=what->split->NumOfParts;i++){
				char data[MAX_LEN];
				char tip[MAX_LEN];
				g_snprintf(data,MAX_LEN," %i ",i);
				g_snprintf(tip,MAX_LEN,"Ctrl+%i",i);
				tmpbutton=gtk_toolbar_append_element (GTK_TOOLBAR (buttonsbar),
								      GTK_TOOLBAR_CHILD_RADIOBUTTON,
								      (GtkWidget *)tmpbutton,
								      data,tip,"",NULL,
								      GTK_SIGNAL_FUNC (log_window_button),
								      GINT_TO_POINTER(i));
				if (what->LOG->last_log==i){
					temp->button=tmpbutton;
				};
				gtk_object_set_user_data(GTK_OBJECT(tmpbutton),what);
			};
			GtkWidget *tmpvbox=gtk_vbox_new(FALSE,0);
			gtk_box_pack_start(GTK_BOX(tmpvbox),buttonsbar,FALSE,FALSE,0);
			gtk_box_pack_end(GTK_BOX(tmpvbox),swindow,TRUE,TRUE,0);
			gtk_container_add(GTK_CONTAINER(temp->window),tmpvbox);
		}else{
			gtk_container_add(GTK_CONTAINER(temp->window),swindow);
		};

		what->LOG->Window=temp;

		gtk_object_set_user_data(GTK_OBJECT(temp->window),what->LOG);

		if (CFG.FIXED_LOG_FONT){
			GtkStyle *current_style =gtk_style_copy(gtk_widget_get_style(GTK_WIDGET(temp->clist)));
			gdk_font_unref(current_style->font);
			current_style->font = gdk_fontset_load("-*-fixed-medium-r-*-*-*-120-*-*-*-*-*-*");;
			if (current_style->font==NULL){
				current_style->font = gdk_fontset_load("-*-*-medium-r-*-*-*-120-*-*-m-*-*-*");;
			};
			gtk_widget_set_style(GTK_WIDGET(temp->clist), current_style);
		};

		gtk_widget_show_all(temp->window);
		gtk_clist_freeze(GTK_CLIST(temp->clist));
		what->LOG->print();
		what->LOG->unlock(); // unlock by main thread?
		what->CurrentLog=what->LOG;
		gtk_clist_thaw(GTK_CLIST(temp->clist));

		gtk_signal_connect (GTK_OBJECT(temp->adj), "changed",GTK_SIGNAL_FUNC(my_gtk_auto_scroll), temp);
		temp->adj->value=temp->adj->upper-temp->adj->page_size;
		temp->value=temp->adj->value;
		gtk_signal_emit_by_name (GTK_OBJECT (temp->adj), "changed");
		if (what->LOG->last_log>1 && what->split &&
		    what->LOG->last_log<=what->split->NumOfParts){
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(temp->button),TRUE);
			log_window_button(temp->button,
					  what->LOG->last_log);
	
		}else{
			what->LOG->last_log=1;
		};
	};
};

void log_window_set_title(tDownload *what,char *title) {
	if (what && what->CurrentLog && what->CurrentLog->Window) {
		tLogWindow *temp=(tLogWindow *)what->CurrentLog->Window;
		gtk_window_set_title(GTK_WINDOW (temp->window), title);
	};
};

void del_first_from_log(tLog *what) {
	tLogWindow *temp=(tLogWindow *)what->Window;
	if (temp) {
		gtk_clist_remove(GTK_CLIST(temp->clist),0);
	};

};

GList *log_window_freeze(GList *list,tLog *what){
	tLogWindow *temp=(tLogWindow *)what->Window;
	if (temp){
		what->freezed_flag=1;
		GList *tlist=(GList *)g_malloc(sizeof(GList));
		tlist->next=list;
		tlist->data=what;
		tlist->prev=NULL;
		gtk_clist_freeze(GTK_CLIST(temp->clist));
		return(tlist);
	};
	return(list);
};

GList *log_window_unfreeze(GList *list){
	tLog *what=(tLog *)list->data;
	tLogWindow *temp=(tLogWindow *)what->Window;
	GList *next=list->next;
	g_free(list);
	what->freezed_flag=0;
	if (temp){
		gtk_clist_thaw(GTK_CLIST(temp->clist));
		gtk_widget_queue_draw(temp->clist);
	};
	return(next);
};
