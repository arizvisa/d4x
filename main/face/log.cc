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


GdkPixbuf *log_pixbufs[5];

struct tLogWindow {
	GtkWidget *window;
	GtkTreeView *view;
	GtkAdjustment *adj;
	GtkWidget *button;
	GtkWidget *toolbar;
	GtkWidget *label;
	tDownload *papa; // :))
	tDownload *current;
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
	log_pixbufs[0]=gdk_pixbuf_new_from_xpm_data((const char **)ok_xpm);
	log_pixbufs[1]=gdk_pixbuf_new_from_xpm_data((const char **)to_server_xpm);
	log_pixbufs[2]=gdk_pixbuf_new_from_xpm_data((const char **)from_server_xpm);
	log_pixbufs[3]=gdk_pixbuf_new_from_xpm_data((const char **)error_xpm);
	log_pixbufs[4]=gdk_pixbuf_new_from_xpm_data((const char **)warning_xpm);
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

void log_model_view_add_string(GtkTreeView *view,tLogString *str){
	char useful[MAX_LEN+1];
	struct tm msgtime;
	localtime_r(&(str->time),&msgtime);
	strftime(useful,MAX_LEN,"%T",&msgtime);
	/* replace all nonprint symbols by space */
	char *str_temp=copy_string(str->body);
	str_non_print_replace(str_temp,' ');
	GtkTreeIter iter;
	GtkListStore *list_store=GTK_LIST_STORE(gtk_tree_view_get_model(view));
	gtk_list_store_append(list_store, &iter);
	gtk_list_store_set(list_store, &iter,
			   L_COL_NUM,str->temp,
			   L_COL_TIME,useful,
			   L_COL_STRING,str_temp,
			   -1);
	delete[] str_temp;
	const GdkColor *color,*back_color;
	switch (str->type) {
	case LOG_OK:{
		gtk_list_store_set(list_store,&iter,L_COL_TYPE,log_pixbufs[0],-1);
		color=&BLACK;
		back_color=&WHITE;
		break;
	};
	case LOG_TO_SERVER: {
		gtk_list_store_set(list_store,&iter,L_COL_TYPE,log_pixbufs[1],-1);
		color=&CYAN;
		back_color=&LCYAN;
		break;
	};
	case LOG_FROM_SERVER: {
		gtk_list_store_set(list_store,&iter,L_COL_TYPE,log_pixbufs[2],-1);
		color=&BLUE;
		back_color=&LBLUE;
		break;
	};
	case LOG_WARNING:{
		gtk_list_store_set(list_store,&iter,L_COL_TYPE,log_pixbufs[4],-1);
		color=&GREEN;
		back_color=&LGREEN;
		break;
	};
	case LOG_ERROR: {
		gtk_list_store_set(list_store,&iter,L_COL_TYPE,log_pixbufs[3],-1);
		color=&RED;
		back_color=&LRED;
		break;
	};
	default:
		color=&BLACK;
		back_color=&WHITE;
	};
	gtk_list_store_set(list_store,&iter,
			   L_COL_LAST,color,
			   L_COL_LAST+1,back_color,
			   -1);
};

void log_window_add_string(tLog *log,tLogString *str) {
	tLogWindow *temp=(tLogWindow *)log->Window;
	if (!temp) return;
	log_model_view_add_string(temp->view,str);
};


static gint log_list_event_handler(GtkWidget *widget,GdkEventButton *event,tLogWindow *temp) {
	if (temp && event && event->type==GDK_2BUTTON_PRESS &&
	    event->button==1 && temp->papa) {
		GtkTreeSelection *sel=gtk_tree_view_get_selection(temp->view);
		GtkTreeIter iter;
		if (!gtk_tree_selection_get_selected(sel,NULL,&iter)) return FALSE;
		if (temp->string==NULL) temp->string=new tStringDialog;
		GValue val={0,};
		GtkTreeModel *model=gtk_tree_view_get_model(temp->view);
		gtk_tree_model_get_value(model,&iter,L_COL_NUM,&val);
		int num=g_value_get_int(&val);
		char data[MAX_LEN];
		char *rfile=unparse_percents(temp->papa->info->file.get());
		sprintf(data,_("Row number %i [log of %s]"),num,rfile);
		delete[] rfile;
		g_value_unset(&val);
		gtk_tree_model_get_value(model,&iter,L_COL_STRING,&val);
		char *text=(char*)g_value_get_string(&val);
/*
		int err_code=0;//GPOINTER_TO_INT(gtk_clist_get_row_data(GTK_CLIST(temp->clist),row));
		char *error_name=NULL;
		switch(err_code){
		case LOG_ERROR:
			error_name=_("Erorr!");
			break;
		case LOG_WARNING:
			error_name=_("Warning!");
			break;
		case LOG_TO_SERVER:
			error_name=_("Message to server");
			break;
		case LOG_FROM_SERVER:
			error_name=_("Message from server");
			break;
		default:
		case LOG_OK:
			error_name=_("All ok");
			break;
		};
*/
		temp->string->init(text,data,"");
		g_value_unset(&val);
		return TRUE;
	};
	return FALSE;
};

static void my_gtk_auto_scroll( GtkAdjustment *get,tLogWindow *temp){
	if (get==NULL || temp==NULL) return;
	if (temp->value==get->value && get->value<get->upper-get->page_size) {
		get->value=get->upper-get->page_size;
		temp->value=get->value;
		g_signal_emit_by_name(G_OBJECT (get), "value_changed");
	} else
		temp->value=get->value;
}

gint log_window_button(GtkWidget *button,int a);

static gint log_window_event_handler(GtkWidget *window,GdkEvent *event,tLog *log){
	if (event && event->type == GDK_KEY_PRESS) {
		GdkEventKey *kevent=(GdkEventKey *)event;
		tLogWindow *wnd=(tLogWindow *)log->Window;
		int num=-1;
		if (kevent->state & GDK_CONTROL_MASK){
			switch(kevent->keyval) {
			case GDK_1:
				num=0;
				break;
			case GDK_2:
				num=1;
				break;
			case GDK_3:
				num=2;
				break;
			case GDK_4:
				num=3;
				break;
			case GDK_5:
				num=4;
				break;
			case GDK_6:
				num=5;
				break;
			case GDK_7:
				num=6;
				break;
			case GDK_8:
				num=7;
				break;
			case GDK_9:
				num=8;
				break;
			case GDK_0:
				num=9;
				break;
			};
		};
		gint max=gtk_toolbar_get_n_items(GTK_TOOLBAR(wnd->toolbar));
		if (num>=0 && num<max-1 && wnd->toolbar){
			GtkToolItem *item=gtk_toolbar_get_nth_item(GTK_TOOLBAR(wnd->toolbar),num);
			gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(item),TRUE);
		};
		if (kevent->keyval==GDK_Escape){
//			g_signal_emit_by_name(G_OBJECT(window),"delete_event");
			log_window_destroy_by_log(log);
			return TRUE;
		};
	};
	return FALSE;
};

void log_window_set_split_info(tDownload *what){
	if (what && what->split && what->who && what->CurrentLog && what->CurrentLog->Window){
		tLogWindow *temp=(tLogWindow *)what->CurrentLog->Window;
		if (temp && temp->current && temp->label){
			fsize_t loaded=0,begin=0,size=0;
			if (temp->current->who)
				loaded=temp->current->who->get_readed();
			if (temp->current->split){
				size=temp->current->split->LastByte;
				begin=temp->current->split->FirstByte;
			};
			char text[100];
			if (temp->current->thread_id)
				sprintf(text," %lli-%lli (%lli)",begin,size,loaded);
			else
				sprintf(text," %lli-%lli (not active)",begin,size);
			gtk_tool_button_set_label(GTK_TOOL_BUTTON(temp->label),text);
//			gtk_label_set_text(GTK_LABEL(temp->label),text);
		};
	};
};

gint log_window_button(GtkWidget *button,int a){
	tDownload *what=(tDownload *)g_object_get_data(G_OBJECT(button),"d4x_user_data");
	tDownload *withlog=what;
	while (withlog){
		if (withlog->LOG->Window) break;
		withlog=withlog->split->next_part;
	};
	if (what->split==NULL){
		gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(((tLogWindow *)(withlog->LOG->Window))->button),TRUE);
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
		temp->current=forlog;
		g_object_set_data(G_OBJECT(temp->window),"d4x_user_data",forlog->LOG);
		gtk_list_store_clear(GTK_LIST_STORE(gtk_tree_view_get_model(temp->view)));
		forlog->LOG->print();
		forlog->LOG->unlock();
		what->LOG->last_log=b;
		what->CurrentLog=forlog->LOG;
		/* FIXME: signal_connect again???? */
		g_signal_handlers_disconnect_matched(GTK_OBJECT(temp->window),
					    G_SIGNAL_MATCH_DATA,
					    0,0,NULL,NULL,
					    withlog->LOG);
		g_signal_connect(G_OBJECT(temp->window),
				   "delete_event",
		                   (GtkSignalFunc)log_window_destroy,
				   forlog->LOG);
		g_signal_connect(G_OBJECT(temp->window), "key_press_event",
		                   (GtkSignalFunc)log_window_event_handler, forlog->LOG);
		g_signal_emit_by_name(G_OBJECT (temp->adj), "changed");
		log_window_set_split_info(temp->papa);
	};
	if (forlog==NULL || forlog->LOG==NULL){
		gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(((tLogWindow *)(withlog->LOG->Window))->button),TRUE);
		what->LOG->last_log=1;
	};
	return TRUE;
};

GtkTreeView *log_model_view_init(){
	GtkListStore *list_store = gtk_list_store_new(L_COL_LAST+2,
						      GDK_TYPE_PIXBUF, //L_COL_TYPE,
						      G_TYPE_INT,      //L_COL_NUM,
						      G_TYPE_STRING,   //L_COL_TIME,
						      G_TYPE_STRING,   //L_COL_STRING,
						      GDK_TYPE_COLOR,  //L_COL_LAST;
						      GDK_TYPE_COLOR);
	GtkTreeView *view = GTK_TREE_VIEW(gtk_tree_view_new_with_model(GTK_TREE_MODEL(list_store)));
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *col;
	renderer = gtk_cell_renderer_pixbuf_new();
	col=gtk_tree_view_column_new_with_attributes ("Tittle",
						      renderer,
						      "pixbuf",0,
						      NULL);
	gtk_tree_view_append_column(view,col);
	for (int i=L_COL_NUM;i<L_COL_LAST;i++){
		renderer = gtk_cell_renderer_text_new();
		col=gtk_tree_view_column_new_with_attributes("Tittle",
							     renderer,
							     "text",i,
							     "foreground-gdk",ML_COL_LAST,
							     "background-gdk",ML_COL_LAST+1,
							     NULL);
		gtk_tree_view_append_column(view,col);
	};
	
	gtk_tree_view_set_headers_visible(view,FALSE);
	gtk_tree_view_set_reorderable(view,FALSE);
	return(view);
};

void log_window_init(tDownload *what) {
	if (what) {
		if (what->LOG==NULL){
			what->LOG=new tLog;
			what->LOG->init(CFG.MAX_LOG_LENGTH);
			what->LOG->ref_inc();
		};
		if (what->LOG->Window) {
			tLogWindow *temp=(tLogWindow *)what->LOG->Window;
			gtk_window_present(GTK_WINDOW(temp->window));
			return;
		}else{
			if (what->split){
				tDownload *next_part=what->split->next_part;
				while (next_part){
					if (next_part->LOG && next_part->LOG->Window){
						tLogWindow *temp=(tLogWindow *)(next_part->LOG->Window);
						gtk_window_present(GTK_WINDOW(temp->window));
						return;
					};
					next_part=next_part->split->next_part;
				};
			};
		};
		what->LOG->lock();
		tLogWindow *temp=new tLogWindow;
		temp->papa=temp->current=what;
		temp->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		gtk_window_set_wmclass(GTK_WINDOW(temp->window),
				       "D4X_Log","D4X");
		int a[4];
		what->LOG->get_geometry(a);
		if (a[3]!=0 && a[2]!=0){
			gtk_window_move(GTK_WINDOW(temp->window), a[0], a[1]);
			gtk_window_set_default_size( GTK_WINDOW (temp->window), a[2], a[3]);
		};
		gtk_widget_set_size_request( GTK_WIDGET (temp->window), 400, 200);
		char title[MAX_LEN];
		title[0]=0;
		strcat(title,_("Log: "));
		char *rfile=unparse_percents(what->info->file.get());
		strcat(title,rfile);
		delete[] rfile;
		gtk_window_set_title(GTK_WINDOW (temp->window), title);
		g_signal_connect(G_OBJECT(temp->window), "key_press_event",
		                   (GtkSignalFunc)log_window_event_handler, what->LOG);
		g_signal_connect(G_OBJECT(temp->window), "delete_event",
				 (GtkSignalFunc)log_window_destroy, what->LOG);
		
		GtkListStore *list_store = gtk_list_store_new(L_COL_LAST+2,
							      GDK_TYPE_PIXBUF, //L_COL_TYPE,
							      G_TYPE_INT,      //L_COL_NUM,
							      G_TYPE_STRING,   //L_COL_TIME,
							      G_TYPE_STRING,   //L_COL_STRING,
							      GDK_TYPE_COLOR,  //L_COL_LAST;
							      GDK_TYPE_COLOR);
		temp->view = log_model_view_init();
		g_signal_connect(G_OBJECT(temp->view),"event",G_CALLBACK(log_list_event_handler),temp);

		temp->adj = (GtkAdjustment *)gtk_adjustment_new (0.0, 0.0, 0.0, 0.1, 1.0, 1.0);

		GtkWidget *swindow=gtk_scrolled_window_new((GtkAdjustment*)NULL,temp->adj);
		gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swindow),
		                                GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
		gtk_container_add(GTK_CONTAINER(swindow),GTK_WIDGET(temp->view));
		if (what->split){
			GtkWidget *buttonsbar=temp->toolbar=gtk_toolbar_new();
			gtk_toolbar_set_orientation(GTK_TOOLBAR(buttonsbar),GTK_ORIENTATION_HORIZONTAL);
			gtk_toolbar_set_style(GTK_TOOLBAR(buttonsbar),GTK_TOOLBAR_TEXT);
			GtkWidget *tmpbutton=NULL;
			GSList *group=NULL;
			for (int i=1;i<=what->split->NumOfParts;i++){
				char data[MAX_LEN];
				char tip[MAX_LEN];
				g_snprintf(data,MAX_LEN," %i ",i);
				g_snprintf(tip,MAX_LEN,"Ctrl+%i",i);
				tmpbutton = GTK_WIDGET(gtk_radio_tool_button_new(group));
				gtk_tool_button_set_label(GTK_TOOL_BUTTON(tmpbutton),data);
				gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(tmpbutton),GTK_TOOLBAR(buttonsbar)->tooltips,tip,NULL);
				gtk_toolbar_insert(GTK_TOOLBAR(buttonsbar),GTK_TOOL_ITEM(tmpbutton),-1);
				g_signal_connect(G_OBJECT(tmpbutton),"clicked",G_CALLBACK(log_window_button),GINT_TO_POINTER(i));
				group=gtk_radio_tool_button_get_group(GTK_RADIO_TOOL_BUTTON(tmpbutton));
				
				if (what->LOG->last_log==i){
					temp->button=tmpbutton;
				};
				
				g_object_set_data(G_OBJECT(tmpbutton),"d4x_user_data",what);
			};
			temp->label=GTK_WIDGET(gtk_tool_button_new(NULL,""));
			gtk_toolbar_insert(GTK_TOOLBAR(buttonsbar),GTK_TOOL_ITEM(temp->label),-1);
//			temp->label=gtk_label_new("");
//			gtk_toolbar_append_widget(GTK_TOOLBAR (buttonsbar),temp->label,NULL,NULL);
			GtkWidget *tmpvbox=gtk_vbox_new(FALSE,0);
			gtk_box_pack_start(GTK_BOX(tmpvbox),buttonsbar,FALSE,FALSE,0);
			gtk_box_pack_end(GTK_BOX(tmpvbox),swindow,TRUE,TRUE,0);
			gtk_container_add(GTK_CONTAINER(temp->window),tmpvbox);
		}else{
			temp->toolbar=NULL;
			temp->label=NULL;
			gtk_container_add(GTK_CONTAINER(temp->window),swindow);
		};

		what->LOG->Window=temp;

		g_object_set_data(G_OBJECT(temp->window),"d4x_user_data",what->LOG);

/*
		if (CFG.FIXED_LOG_FONT){
			GtkStyle *current_style =gtk_style_copy(gtk_widget_get_style(GTK_WIDGET(temp->view)));
			GdkFont *font=gdk_fontset_load("-*-fixed-medium-r-*-*-*-120-*-*-*-*-*-*");
			if (font==NULL){
				font = gdk_fontset_load("-*-*-medium-r-*-*-*-120-*-*-m-*-*-*");;
			};
			if (font)
				gtk_style_set_font(current_style,font);
			gtk_widget_set_style(GTK_WIDGET(temp->view), current_style);
		};
*/
		gtk_widget_show_all(temp->window);
		what->LOG->print();
		what->LOG->unlock(); // unlock by main thread?
		what->CurrentLog=what->LOG;

		g_signal_connect(G_OBJECT(temp->adj), "changed",G_CALLBACK(my_gtk_auto_scroll), temp);
		temp->adj->value=temp->adj->upper-temp->adj->page_size;
		temp->value=temp->adj->value;
		g_signal_emit_by_name(G_OBJECT (temp->adj), "changed");
		if (what->LOG->last_log>1 && what->split &&
		    what->LOG->last_log<=what->split->NumOfParts){
			gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(temp->button),TRUE);
			log_window_button(temp->button,
					  what->LOG->last_log);
	
		}else{
			what->LOG->last_log=1;
		};
		log_window_set_split_info(what);
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
		GtkTreeIter iter;
		GtkListStore *store=(GtkListStore *)gtk_tree_view_get_model(temp->view);
		gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store),&iter);
		gtk_list_store_remove(store,&iter);
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
		gtk_widget_queue_draw(GTK_WIDGET(temp->view));
	};
	return(next);
};

void log_print_to_view(tLog *log,GtkTreeView *view){
	log->lock();
	tLogString *prom=(tLogString *)(log->first());
	while (prom) {
		log_model_view_add_string(view,prom);
		prom=(tLogString *)prom->prev;
	};
	log->unlock();
};
