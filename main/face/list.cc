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
#include <gdk/gdkkeysyms.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include "../dlist.h"
#include "../main.h"
#include "../var.h"
#include "../savelog.h"
#include "../config.h"
#include "../locstr.h"
#include "../ntlocale.h"
#include "log.h"
#include "addd.h"
#include "list.h"
#include "prefs.h"
#include "buttons.h"
#include "about.h"
#include "graph.h"
#include "edit.h"
#include "lmenu.h"
#include "saveload.h"
#include "limface.h"
#include "misc.h"
#include "dndtrash.h"


GtkWidget *MainMenu;
GtkAdjustment *ProgressBarValues;
GtkWidget *ProgressOfDownload;
GtkWidget *MainStatusBar,*ReadedBytesStatusBar;
GtkWidget *MainWindow=NULL;
GtkWidget *MainHBox;
GtkWidget *ContainerForCList=NULL;
GdkGC *MainWindowGC;
GtkWidget *BoxForGraph;
GtkWidget *MainLogList;
int main_log_mask;
unsigned int ScrollShift[2];
gfloat main_log_value;
GtkAdjustment *main_log_adj;

GtkItemFactory *list_menu_itemfact;

tDialogWidget *AskDelete=NULL;
tDialogWidget *AskDeleteCompleted=NULL;
tDialogWidget *AskDeleteFataled=NULL;
tDialogWidget *AskExit=NULL;

tFaceLimits *FaceForLimits=NULL;

gint StatusBarContext,RBStatusBarContext;
int MainTimer,LogsTimer,GraphTimer,ListTimer;
int SAVE_LIST_INTERVAL;
pthread_mutex_t MAIN_GTK_MUTEX=PTHREAD_MUTEX_INITIALIZER;

int FirstConfigureEvent;
int UpdateTitleCycle=0;

enum{
	ROLL_STAT=0,
	ROLL_INFO,
	ROLL_LAST
};


static void open_limits_window(...) {
	FaceForLimits->init();
};

/*********************************************************************
    End of first part of DnD's code
 *********************************************************************/

void util_item_factory_popup(GtkItemFactory *ifactory,guint x, guint y,guint mouse_button,guint32 time) {
	static GQuark quark_user_menu_pos=0;
	struct Pos {
		gint x;
		gint y;
	}
	*pos;

	if(!quark_user_menu_pos)
		quark_user_menu_pos=g_quark_from_static_string("user_menu_pos");
	pos=(Pos *)gtk_object_get_data_by_id(GTK_OBJECT(ifactory),quark_user_menu_pos);
	if(!pos) {
		pos=(Pos *)g_malloc0(sizeof(struct Pos));
		gtk_object_set_data_by_id_full(GTK_OBJECT(ifactory->widget),quark_user_menu_pos,pos,g_free);
	}
	pos->x=x;
	pos->y=y;
	gtk_menu_popup(GTK_MENU(ifactory->widget),NULL,NULL,NULL,pos,mouse_button,time);
};

void init_main_menu() {
	GtkItemFactoryEntry menu_items[] = {
	{_("/_File"),         NULL,         NULL, 0, "<Branch>"},
	{_("/File/_Save List"),     "<control>S", init_save_list, 0, NULL},
	{_("/File/_Load List"),     "<control>L", init_load_list, 0, NULL},
	{_("/File/sep1"),     NULL,         NULL, 0, "<Separator>"},
	{_("/File/_New Download"),     "<control>N", init_add_window, 0, NULL},
	{_("/File/_Paste Download"), "<control>P", init_add_clipboard_window, 0, NULL},
	{_("/File/sep1"),     NULL,         NULL, 0, "<Separator>"},
	{_("/File/Exit"),     "<alt>X", ask_exit, 0, NULL},
	{_("/_Download"),      NULL,         NULL, 0, "<Branch>"},
	{_("/Download/View _Log"),  NULL,  open_log_for_selected, 0, NULL},
	{_("/Download/_Stop downloads"),  "<alt>S",  stop_downloads, 0, NULL},
	{_("/Download/Edit download"), "<alt>E",  open_edit_for_selected,	0, NULL},
	{_("/Download/_Delete downloads"),"<alt>C",  ask_delete_download, 0, NULL},
	{_("/Download/Continue downloads"),"<alt>A",  continue_downloads, 0, NULL},
	{_("/Download/Delete completed"),  NULL,  ask_delete_completed_downloads, 0, NULL},
	{_("/Download/Delete failed"),  NULL,  ask_delete_fataled_downloads, 0, NULL},
	{_("/_Options"),      NULL,         NULL, 0, "<Branch>"},
	{_("/Options/Limitations"),NULL,       open_limits_window, 0, NULL},
	{_("/Options/Common"),  "<control>C",       init_options_window, 0, NULL},
	{_("/_Help"),         NULL,         NULL, 0, "<LastBranch>"},
	{_("/_Help/About"),   NULL,         init_about_window, 0, NULL},
	};
	int nmenu_items = sizeof(menu_items) / sizeof(menu_items[0]);
	GtkItemFactory *item_factory;
	GtkAccelGroup *accel_group;

	accel_group = gtk_accel_group_new();
	item_factory = gtk_item_factory_new(GTK_TYPE_MENU_BAR, "<main>",accel_group);
	gtk_item_factory_create_items(item_factory, nmenu_items, menu_items, NULL);
	MainMenu= gtk_item_factory_get_widget(item_factory, "<main>");
	gtk_accel_group_attach(accel_group,GTK_OBJECT(MainWindow));
};

void my_main_quit(...) {
	gtk_timeout_remove(MainTimer);
	gtk_timeout_remove(LogsTimer);
	gtk_timeout_remove(GraphTimer);
	save_list();
	save_config();
	save_limits();
	aa.done();
	gtk_main_quit();
};

void open_log_for_selected(...) {
	GList *select=((GtkCList *)ListOfDownloads)->selection;
	while (select) {
		tDownload *temp=(tDownload *)gtk_clist_get_row_data(
		                    GTK_CLIST(ListOfDownloads),GPOINTER_TO_INT(select->data));
		log_window_init(temp);
		select=select->next;
	};
};

void set_limit_to_download() {
	tDownload *temp=list_of_downloads_last_selected();
	if (temp)
		FaceForLimits->add(temp->info->host,temp->info->port);
};

void open_edit_for_selected(...) {
	tDownload *temp=list_of_downloads_last_selected();
	init_edit_window(temp);
};

void del_completed_downloads(...) {
	aa.del_completed();
	if (AskDeleteCompleted) AskDeleteCompleted->done();
};

void del_fataled_downloads(...) {
	aa.del_fataled();
	if (AskDeleteFataled) AskDeleteFataled->done();
};

void stop_downloads(...) {
	GList *select=((GtkCList *)ListOfDownloads)->selection;
	while (select) {
		tDownload *temp=(tDownload *)gtk_clist_get_row_data(
		                    GTK_CLIST(ListOfDownloads),GPOINTER_TO_INT(select->data));
		aa.stop_download(temp);
		select=select->next;
	};
	prepare_buttons();
};

void ask_exit(...) {
	if (CFG.CONFIRM_EXIT) {
		if (!AskExit) AskExit=new tDialogWidget;
		if (AskExit->init(_("Do you realy want to quit?"),_("Quit?")))
			gtk_signal_connect(GTK_OBJECT(AskExit->ok_button),"clicked",GTK_SIGNAL_FUNC(my_main_quit),NULL);
	} else
		my_main_quit();
};

gint ask_exit2() {
	if (CFG.WINDOW_LOWER)// gdk_window_lower(MainWindow->window);
		my_gdk_window_iconify(MainWindow->window);
	else ask_exit();
	return TRUE;
};


void ask_delete_download(...) {
	if (CFG.CONFIRM_DELETE) {
		if (!AskDelete) AskDelete=new tDialogWidget;
		if (AskDelete->init(_("Delete selected downloads?"),_("Delete?")))
			gtk_signal_connect(GTK_OBJECT(AskDelete->ok_button),"clicked",GTK_SIGNAL_FUNC(delete_downloads),NULL);
	} else
		delete_downloads();
};

void ask_delete_completed_downloads(...) {
	if (CFG.CONFIRM_DELETE_COMPLETED) {
		if (!AskDeleteCompleted) AskDeleteCompleted=new tDialogWidget;
		if (AskDeleteCompleted->init(_("Do you wish delete completed downloads?"),_("Delete?")))
			gtk_signal_connect(GTK_OBJECT(AskDeleteCompleted->ok_button),"clicked",GTK_SIGNAL_FUNC(del_completed_downloads),NULL);
	} else
		del_completed_downloads();
};

void ask_delete_fataled_downloads(...) {
	if (CFG.CONFIRM_DELETE_FATALED) {
		if (!AskDeleteFataled) AskDeleteFataled=new tDialogWidget;
		if (AskDeleteFataled->init(_("Do you wish delete failed downloads?"),_("Delete?")))
			gtk_signal_connect(GTK_OBJECT(AskDeleteFataled->ok_button),"clicked",GTK_SIGNAL_FUNC(del_fataled_downloads),NULL);
	} else
		del_completed_downloads();
};

void delete_downloads(...) {
	GList *select=((GtkCList *)ListOfDownloads)->selection;
	while (select) {
		tDownload *temp=(tDownload *)gtk_clist_get_row_data(
		                    GTK_CLIST(ListOfDownloads),GPOINTER_TO_INT(select->data));
		if (aa.delete_download(temp))
			select=((GtkCList *)ListOfDownloads)->selection;
		else
			select=select->next;
	};
	gtk_clist_unselect_all(GTK_CLIST(ListOfDownloads));
	prepare_buttons();
	if (AskDelete) AskDelete->done();
};

void continue_downloads(...) {
	GList *select=((GtkCList *)ListOfDownloads)->selection;
	while (select) {
		tDownload *temp=(tDownload *)gtk_clist_get_row_data(
		                    GTK_CLIST(ListOfDownloads),GPOINTER_TO_INT(select->data));
		aa.continue_download(temp);
		select=select->next;
	};
	prepare_buttons();
};

/* ******************************************************************** */



/* ******************************************************************** */
void init_status_bar() {
	ProgressBarValues = (GtkAdjustment *) gtk_adjustment_new (0, 1, 200 , 0, 0, 0);
	ProgressOfDownload = gtk_progress_bar_new_with_adjustment( ProgressBarValues);
	/* Set the format of the string that can be displayed in the
	 * trough of the progress bar:
	 * %p - percentage
	 * %v - value
	 * %l - lower range value
	 * %u - upper range value */
	gtk_widget_set_usize(ProgressOfDownload,100,-1);
	gtk_progress_set_format_string (GTK_PROGRESS (ProgressOfDownload),
	                                "%p%%");
	gtk_progress_set_show_text(GTK_PROGRESS(ProgressOfDownload),TRUE);
	MainStatusBar=gtk_statusbar_new();
	ReadedBytesStatusBar=gtk_statusbar_new();
	StatusBarContext=gtk_statusbar_get_context_id(
	                     GTK_STATUSBAR(MainStatusBar),"Main window context");
	RBStatusBarContext=gtk_statusbar_get_context_id(
	                       GTK_STATUSBAR(ReadedBytesStatusBar),"Readed Bytes");
	gtk_statusbar_push(GTK_STATUSBAR(MainStatusBar),StatusBarContext,_("Ready to go ????"));
	gtk_statusbar_push(GTK_STATUSBAR(ReadedBytesStatusBar),RBStatusBarContext,"0");
	gtk_widget_set_usize(ReadedBytesStatusBar,150,-1);
};


void update_progress_bar() {
	tDownload *temp=list_of_downloads_last_selected();
	int percent=0;
	if (temp) percent=temp->Percent.curent;
	if (percent>100) percent=100;
	gtk_progress_bar_update((GtkProgressBar *)ProgressOfDownload,(gfloat)percent/100);
	gtk_widget_show(ProgressOfDownload);
	char data[MAX_LEN];
	char data1[MAX_LEN];
	make_number_nice(data,GVARS.READED_BYTES);
	sprintf(data1,"%s(%iB/s)",data,GlobalMeter->last_value());
	gtk_statusbar_pop(GTK_STATUSBAR(ReadedBytesStatusBar),RBStatusBarContext);
	gtk_statusbar_push(GTK_STATUSBAR(ReadedBytesStatusBar),RBStatusBarContext,data1);
};
/* ******************************************************************** */
static void cb_page_size( GtkAdjustment *get) {
	if (get==NULL) return;
	if (main_log_value==get->value && get->value<get->upper-get->page_size) {
		//added 0.01 to prevent interface lockups
		get->value=get->upper-get->page_size;
		main_log_value=get->value;
		gtk_signal_emit_by_name (GTK_OBJECT (get), "changed");
	} else
		main_log_value=get->value;
}

void init_main_window() {
	GtkWidget *hbox=gtk_hbox_new(FALSE,1);
	MainHBox=hbox;
	gtk_box_pack_start (GTK_BOX (hbox), MainStatusBar, TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (hbox), ReadedBytesStatusBar, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (hbox), ProgressOfDownload, FALSE, FALSE, 0);
	gtk_widget_set_usize(hbox,-1,20);

	MainLogList=gtk_clist_new(2);
	gtk_clist_set_column_width (GTK_CLIST(MainLogList),0,100);
	gtk_clist_set_column_width (GTK_CLIST(MainLogList),1,800);
	gtk_clist_set_column_justification (GTK_CLIST(MainLogList), 0, GTK_JUSTIFY_LEFT);
	gtk_clist_set_column_justification (GTK_CLIST(MainLogList), 1, GTK_JUSTIFY_LEFT);

	GtkWidget *hpaned=gtk_vpaned_new();
	main_log_adj = (GtkAdjustment *)gtk_adjustment_new (0.0, 0.0, 0.0, 0.1, 1.0, 1.0);
	main_log_value=0.0;
	gtk_signal_connect (GTK_OBJECT (main_log_adj), "changed",
	                    GTK_SIGNAL_FUNC (cb_page_size), NULL);
	GtkWidget *scroll_window=gtk_scrolled_window_new(NULL,main_log_adj);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll_window),
	                                GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(scroll_window),MainLogList);
	gtk_paned_add1(GTK_PANED(hpaned),ContainerForCList);
	gtk_paned_add2(GTK_PANED(hpaned),scroll_window);

	GtkWidget *TEMP=gtk_statusbar_new();
	gtk_widget_set_usize(TEMP,106,-1);
	gtk_box_pack_end (GTK_BOX (hbox), TEMP, FALSE, FALSE, 0);

	GtkWidget *vbox=gtk_vbox_new(FALSE,1);
	gtk_box_pack_start (GTK_BOX (vbox), MainMenu, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), ButtonsBar, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), hpaned, TRUE, TRUE, 0);
	gtk_box_pack_end (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
	gtk_container_add(GTK_CONTAINER(MainWindow),vbox);

	gtk_widget_show(MainStatusBar);
	gtk_widget_show(ProgressOfDownload);
	gtk_widget_show(TEMP);
	gtk_widget_show(ReadedBytesStatusBar);
	gtk_progress_bar_update((GtkProgressBar *)ProgressOfDownload,0);
	gtk_widget_show(ListOfDownloads);
	gtk_widget_show(ButtonsBar);

	gtk_widget_show(ContainerForCList);
	gtk_widget_show(MainLogList);
	gtk_widget_show(scroll_window);
	gtk_widget_show(hpaned);
	gtk_widget_show(MainMenu);
	gtk_widget_show(hbox);
	gtk_widget_show(vbox);
	gtk_widget_show(MainWindow);
	int temp,w,h;
	gdk_window_get_geometry(NULL,&temp,&temp,&w,&h,&temp);
	if (CFG.WINDOW_X_POSITION<=w || CFG.WINDOW_Y_POSITION<=h)
		gdk_window_move_resize(MainWindow->window,CFG.WINDOW_X_POSITION,CFG.WINDOW_Y_POSITION,CFG.WINDOW_WIDTH,CFG.WINDOW_HEIGHT);
	init_graph();
	//    gtk_widget_set_usize(ListOfDownloads,-1,-1);
	gtk_signal_connect(GTK_OBJECT(TEMP), "expose_event",
	                   GTK_SIGNAL_FUNC(graph_expose_event_handler),
	                   NULL);
};

/* ******************************************************************* */
static void tmp_scroll_title(char *title,int index){
	if (CFG.SCROLL_MAINWIN_TITLE){
		ScrollShift[index]+=1;
		if (ScrollShift[index]>=strlen(title))	ScrollShift[index]=0;
		scroll_string_left(title,ScrollShift[index]);
	};	
};

void update_mainwin_title() {
	if (CFG.USE_MAINWIN_TITLE) {
		tDownload *temp=list_of_downloads_last_selected();
		char data[MAX_LEN];
		if (temp && (CFG.USE_MAINWIN_TITLE2==0 || UpdateTitleCycle % 3)) {
			char data2[MAX_LEN];
			char data3[MAX_LEN];
			make_number_nice(data2,temp->Size.curent);
			if (temp->finfo.size>=0)
				make_number_nice(data3,temp->finfo.size);
			else
				sprintf(data3,"???");
			sprintf(data,"%i%c %s/%s %s ",temp->Percent.curent,'%',data2,data3,temp->info->file);
			tmp_scroll_title(data,ROLL_STAT);
			gtk_window_set_title(GTK_WINDOW (MainWindow), data);
		} else {
			if (CFG.USE_MAINWIN_TITLE2) {
				int total=RunList->count()+StopList->count()+CompleteList->count()+PausedList->count()+WaitList->count()+WaitStopList->count();
				sprintf(data,_("%i-running %i-completed %i-total "),RunList->count(),CompleteList->count(),total);
				tmp_scroll_title(data,ROLL_INFO);
				gtk_window_set_title(GTK_WINDOW (MainWindow), data);
			} else
				gtk_window_set_title(GTK_WINDOW (MainWindow), VERSION_NAME);
		};
	} else
		gtk_window_set_title(GTK_WINDOW (MainWindow), VERSION_NAME);
};

int time_for_refresh(void *a) {
	aa.main_circle();
	update_progress_bar();
	update_mainwin_title();
	UpdateTitleCycle+=1;
	tDownload *tmp=list_of_downloads_last_selected();
	if (tmp && RunList->owner(tmp))
		LocalMeter->add(tmp->NanoSpeed);
	else
		LocalMeter->add(0);
	recalc_graph();
	return 1;
};

int time_for_logs_refresh(void *a) {
	aa.redraw_logs();
	MainTimer-=1;
	dnd_trash_refresh();
	if (MainTimer==0) {
		time_for_refresh(NULL);
		MainTimer=2000/100;
		get_mainwin_sizes(MainWindow);
	};
	return 1;
};

int get_mainwin_sizes(GtkWidget *window) {
	if (FirstConfigureEvent) {
		FirstConfigureEvent=0;
		return FALSE;
	};
	if (window!=NULL && window->window!=NULL) {
		gdk_window_get_position(window->window,&CFG.WINDOW_X_POSITION,&CFG.WINDOW_Y_POSITION);
		gdk_window_get_size(window->window,&CFG.WINDOW_WIDTH,&CFG.WINDOW_HEIGHT);
	};
	return FALSE;
};

int time_for_draw_graph(void *a) {
	draw_graph();
	return 1;
};

int time_for_save_list(void *a) {
	SAVE_LIST_INTERVAL-=1;
	if (!SAVE_LIST_INTERVAL) {
		if (CFG.SAVE_LIST) {
			save_list();
		};
		SAVE_LIST_INTERVAL=CFG.SAVE_LIST_INTERVAL;
	};
	return 1;
};

void init_timeouts() {
	SAVE_LIST_INTERVAL=CFG.SAVE_LIST_INTERVAL;
	ListTimer = gtk_timeout_add (60000, time_for_save_list , NULL);
	GraphTimer = gtk_timeout_add (250, time_for_draw_graph , NULL);
	LogsTimer = gtk_timeout_add (100, time_for_logs_refresh , NULL);
	MainTimer=2000/100;
	FirstConfigureEvent=1;
	gtk_signal_connect(GTK_OBJECT(MainWindow), "configure_event",
	                   GTK_SIGNAL_FUNC(get_mainwin_sizes),
	                   MainWindow);
};


void init_face(int argc, char *argv[]) {
	gtk_set_locale();
	gtk_init(&argc, &argv);
	gdk_rgb_init();
	init_columns_info();
	MainWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW (MainWindow), VERSION_NAME);
	gtk_widget_realize(MainWindow);
	gtk_widget_set_usize( GTK_WIDGET (MainWindow), 400, 200);
	MainWindowGC=gdk_gc_new(MainWindow->window);
	init_main_menu();
	init_list_menu();
	init_status_bar();
	list_of_downloads_init_pixmaps();
	list_of_downloads_init();
	init_buttons_bar();
	init_main_window();
	init_pixmaps_for_log();
/* initing table of shifts
 */
	for (int i=0;i<ROLL_LAST;i++)
		ScrollShift[i]=0;
	prepare_buttons();
	FaceForLimits=new tFaceLimits;
#include "pixmaps/logo.xpm"
	GdkBitmap *bitmap;
	GdkPixmap *pixmap=make_pixmap_from_xpm(&bitmap,logo_xpm);
	gdk_window_set_icon(MainWindow->window,NULL,pixmap,bitmap);
	gtk_signal_connect(GTK_OBJECT(MainWindow), "delete_event",
	                   GTK_SIGNAL_FUNC(ask_exit2),
	                   NULL);
	if (CFG.DND_TRASH) dnd_trash_init();
	main_log_mask=0;
};
