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
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include "lod.h"
#include "buttons.h"
#include "list.h"
#include "lmenu.h"
#include "addd.h"
#include "misc.h"
#include "../ntlocale.h"
#include "../locstr.h"
#include "../main.h"
#include "../var.h"

GtkWidget *ListOfDownloads=NULL;

GdkPixmap *list_of_downloads_pixmaps[PIX_UNKNOWN];
GdkBitmap *list_of_downloads_bitmaps[PIX_UNKNOWN];

GdkBitmap *wait_mask,*stop_mask,*pause_mask,*complete_mask,*run_mask,*part_run_mask,*run_bad_mask,*stop_wait_mask;
GdkPixmap *wait_pixmap=NULL,*stop_pixmap=NULL,*pause_pixmap=NULL,*complete_pixmap=NULL;
GdkPixmap *run_pixmap=NULL,*part_run_pixmap=NULL,*run_bad_pixmap=NULL,*stop_wait_pixmap=NULL;

gint SizeListOfDownloads;
gchar *ListTitles[]={" ","File","Type","Full Size","Downloaded","Rest","%","Speed","Time","Remaining","Pause","Attempt","URL"," "};
tColumn ListColumns[]={	{STATUS_COL,STATUS_COL,NULL,25},
						{FILE_COL,FILE_COL,NULL,100},
						{FILE_TYPE_COL,FILE_TYPE_COL,NULL,40},
						{FULL_SIZE_COL,FULL_SIZE_COL,NULL,70},
						{DOWNLOADED_SIZE_COL,DOWNLOADED_SIZE_COL,NULL,70},
						{REMAIN_SIZE_COL,REMAIN_SIZE_COL,NULL,70},
						{PERCENT_COL,PERCENT_COL,NULL,30},
						{SPEED_COL,SPEED_COL,NULL,60},
						{TIME_COL,TIME_COL,NULL,60},
						{ELAPSED_TIME_COL,ELAPSED_TIME_COL,NULL,60},
						{PAUSE_COL,PAUSE_COL,NULL,40},
						{TREAT_COL,TREAT_COL,NULL,40},
						{URL_COL,URL_COL,NULL,500},
						{NOTHING_COL,NOTHING_COL,NULL,0}};

/******************************************************************
    This part of code for DnD (Drag-n-Drop) support added by
		     Justin Bradford
 ******************************************************************/

// for drag-drop support

// drop handler
// define a target entry listing the mime-types we'll acknowledge
GtkTargetEntry download_drop_types[] = {
    { "x-url/http",    0, TARGET_URL
    },
    { "x-url/ftp",     0, TARGET_URL},
    { "_NETSCAPE_URL", 0, TARGET_URL}
};

// calculate the number of mime-types listed
gint n_download_drop_types = sizeof(download_drop_types) / sizeof(download_drop_types[0]);


void init_columns_info() {
	for (int i=STATUS_COL;i<=NOTHING_COL;i++) {
		if (ListColumns[i].name) delete(ListColumns[i].name);
		ListColumns[i].name=copy_string(_(ListTitles[ListColumns[i].type]));
	};
};

void select_download(GtkWidget *clist, gint row, gint column,
                     GdkEventButton *event, gpointer data,gpointer nothing) {
	update_progress_bar();
	update_mainwin_title();
	prepare_buttons();
	gtk_statusbar_pop(GTK_STATUSBAR(MainStatusBar),StatusBarContext);
	tDownload *temp=list_of_downloads_last_selected();
	if (temp)
		gtk_statusbar_push(GTK_STATUSBAR(MainStatusBar),StatusBarContext,temp->info->file);
	else
		gtk_statusbar_push(GTK_STATUSBAR(MainStatusBar),StatusBarContext,"");
	if (event && event->type==GDK_2BUTTON_PRESS && event->button==1)
		open_log_for_selected();
};

tDownload *list_of_downloads_last_selected() {
	GList *select=((GtkCList *)ListOfDownloads)->selection_end;
	if (select) {
		tDownload *temp=(tDownload *)gtk_clist_get_row_data(
		                    GTK_CLIST(ListOfDownloads),GPOINTER_TO_INT(select->data));
		return temp;
	};
	return NULL;
};

void list_of_downloads_change_data(int row,int column,gchar *data) {
	int real_col=ListColumns[column].enum_index;	
	if (real_col<ListColumns[NOTHING_COL].enum_index)
		gtk_clist_set_text(GTK_CLIST(ListOfDownloads),row,real_col,data);
};

void list_of_downloads_get_sizes() {
	if (!ListOfDownloads) return;
	GtkCListColumn *tmp=GTK_CLIST(ListOfDownloads)->column;
	for (int i=0;i<ListColumns[NOTHING_COL].enum_index;i++) {
		ListColumns[i].size=int(tmp->width);
		tmp++;
	};
};

void list_of_downloads_add(tDownload *what) {
	gchar *data[NOTHING_COL+1];
	char *temp;
	char *URL=make_simply_url(what);
	for (int i=STATUS_COL;i<=NOTHING_COL;i++)
		data[ListColumns[i].enum_index]="";
	temp=what->info->file;
	what->GTKCListRow=gtk_clist_append(GTK_CLIST(ListOfDownloads),data);
	list_of_downloads_change_data(what->GTKCListRow,URL_COL,URL);
	list_of_downloads_change_data(what->GTKCListRow,FILE_COL,temp);
	gtk_clist_set_row_data(GTK_CLIST(ListOfDownloads),what->GTKCListRow,gpointer(what));
	SizeListOfDownloads+=1;
	list_of_downloads_set_pixmap(what->GTKCListRow,PIX_WAIT);
	delete URL;
};

void list_of_downloads_add(tDownload *what,int row) {
	gchar *data[NOTHING_COL+1];
	for (int i=STATUS_COL;i<=URL_COL;i++)
		data[i]=NULL;
	gtk_clist_insert(GTK_CLIST(ListOfDownloads),row,data);
	gtk_clist_set_row_data(GTK_CLIST(ListOfDownloads),row,what);
	list_of_downloads_change_data(row,FILE_COL,what->info->file);
	char *URL=make_simply_url(what);
	list_of_downloads_change_data(row,URL_COL,URL);
	delete (URL);
	switch (what->owner) {
		case DL_WAIT:
			{
				list_of_downloads_set_pixmap(row,PIX_WAIT);
				break;
			};
		case DL_STOP:
			{
				list_of_downloads_set_pixmap(row,PIX_STOP);
				break;
			};
		case DL_RUN:{
			what->update_trigers();
			switch (what->Status.curent) {
				case D_QUERYING:{
						list_of_downloads_set_pixmap(row,PIX_RUN_PART);
						break;
					};
				default:
				case D_DOWNLOAD:{
						list_of_downloads_set_pixmap(row,PIX_RUN);
						break;
					};
				case D_DOWNLOAD_BAD:{
						list_of_downloads_set_pixmap(row,PIX_RUN_BAD);
						break;
					};
				};
				break;
			};
		case DL_PAUSE:
			{
				list_of_downloads_set_pixmap(row,PIX_PAUSE);
				break;
			};
		case DL_COMPLETE:
			list_of_downloads_set_pixmap(row,PIX_COMPLETE);
	};
};

void move_download_up() {
	tDownload *what=list_of_downloads_last_selected();
	if (!what) return;
	if (what->GTKCListRow<=0) return;
	list_of_downloads_freeze();
	int row=what->GTKCListRow;
	list_of_downloads_add(what,row-1);
	what->GTKCListRow=row-1;
	char *text;
	for (int i=1;i<=ListColumns[NOTHING_COL].enum_index;i++) {
		text=NULL;
		gtk_clist_get_text(GTK_CLIST(ListOfDownloads),row+1,i,&text);
		gtk_clist_set_text(GTK_CLIST(ListOfDownloads),row-1,i,text);
	};
	gtk_clist_remove(GTK_CLIST(ListOfDownloads),row+1);

	tDownload *what2=(tDownload *)gtk_clist_get_row_data(GTK_CLIST(ListOfDownloads),row);
	if (what2) what2->GTKCListRow=row;
	gtk_clist_select_row(GTK_CLIST(ListOfDownloads),what->GTKCListRow,0);
	if (WaitList->owner(what) && WaitList->owner(what2)) WaitList->forward(what);
	list_of_downloads_unfreeze();
};

void move_download_down() {
	tDownload *what=list_of_downloads_last_selected();
	if (!what) return;
	if (what->GTKCListRow>=SizeListOfDownloads-1) return;
	list_of_downloads_freeze();
	int row=what->GTKCListRow;
	list_of_downloads_add(what,row+2);
	what->GTKCListRow=row+1;
	char *text;
	for (int i=1;i<=ListColumns[NOTHING_COL].enum_index;i++) {
		text=NULL;
		gtk_clist_get_text(GTK_CLIST(ListOfDownloads),row,i,&text);
		gtk_clist_set_text(GTK_CLIST(ListOfDownloads),row+2,i,text);
	};
	gtk_clist_remove(GTK_CLIST(ListOfDownloads),row);

	tDownload *what2=(tDownload *)gtk_clist_get_row_data(GTK_CLIST(ListOfDownloads),row);
	if (what2) what2->GTKCListRow=row;
	gtk_clist_select_row(GTK_CLIST(ListOfDownloads),what->GTKCListRow,0);
	if (WaitList->owner(what) && WaitList->owner(what2)) WaitList->backward(what);
	list_of_downloads_unfreeze();
};

void list_of_downloads_del(tDownload *what) {
	gtk_clist_remove(GTK_CLIST(ListOfDownloads),what->GTKCListRow);
	SizeListOfDownloads-=1;
	for (int i=what->GTKCListRow;i<SizeListOfDownloads;i++) {
		tDownload *temp=(tDownload *)gtk_clist_get_row_data(
		                    GTK_CLIST(ListOfDownloads),i);
		if (temp) temp->GTKCListRow=i;
	};
	prepare_buttons();
	update_mainwin_title();
};

tDownload *get_download_from_clist(int row) {
	tDownload *what=(tDownload *)gtk_clist_get_row_data(GTK_CLIST(ListOfDownloads),row);
	return what;
};

void list_of_downloads_freeze() {
	gtk_clist_freeze(GTK_CLIST(ListOfDownloads));
};

void list_of_downloads_unfreeze() {
	gtk_clist_thaw(GTK_CLIST(ListOfDownloads));
	gtk_widget_show(ListOfDownloads);
};

int list_event_callback(GtkWidget *widget,GdkEvent *event) {
	if (event->type == GDK_BUTTON_PRESS) {
		GdkEventButton *bevent=(GdkEventButton *)event;
		if (bevent->button==3) {
			int row;
			if (gtk_clist_get_selection_info(GTK_CLIST(widget),int(bevent->x),int(bevent->y),&row,NULL)) {
				GList *select=((GtkCList *)widget)->selection;
				gint sel_row=-1;
				/*
				 * If row is not selected yet wee need to unselect all list
				 */
				int need_unselect=1;
				while (select) {
					sel_row=GPOINTER_TO_INT(select->data);
					if (row==sel_row) {
						need_unselect=0;
						break;
					};
					select=select->next;
				};
				if (need_unselect)
					gtk_clist_unselect_all(GTK_CLIST(widget));
				gtk_clist_select_row(GTK_CLIST(widget),row,-1);
			} else {
				gtk_clist_unselect_all(GTK_CLIST(widget));
				prepare_buttons();
				update_progress_bar();
				update_mainwin_title();
			};
			gint x,y;
			GdkModifierType modmask;
			gdk_window_get_pointer(NULL,&x,&y, &modmask);
			//          util_item_factory_popup(list_menu_itemfact,x,y,3,GDK_CURRENT_TIME);
			list_menu_prepare();
			gtk_menu_popup(GTK_MENU(ListMenu),NULL,NULL,NULL,NULL,bevent->button,bevent->time);
			return TRUE;
		};
	};
	if (event->type == GDK_KEY_PRESS) {
		GdkEventKey *kevent=(GdkEventKey *)event;
		switch(kevent->keyval) {
			case GDK_Delete:
			case GDK_KP_Delete:
				{
					ask_delete_download();
					return TRUE;
				};
		};
		if (kevent->state & GDK_SHIFT_MASK) {
			switch (kevent->keyval) {
				case GDK_KP_Up:
				case GDK_Up:
					{
						move_download_up();
						return TRUE;
					};
				case GDK_KP_Down:
				case GDK_Down:
					{
						move_download_down();
						return TRUE;
					};
				default:
					break;
			};
		};
	};
	return FALSE;
};

/**********************************************************
    Handler for DnD event
 **********************************************************/
// this the drag-drop even handler
// just add the url, and assume default download location
void list_dnd_drop_internal(GtkWidget *widget,
                                   GdkDragContext *context,
                                   gint x, gint y,
                                   GtkSelectionData *selection_data,
                                   guint info, guint time) {
	g_return_if_fail(widget!=NULL);

	switch (info) {
			// covers all single URLs
			// a uri-list mime-type will need special handling
		case TARGET_URL:
			{
				// make sure our url (in selection_data->data) is good
				if (selection_data->data != NULL) {
					int len = strlen((char*)selection_data->data);
					if (len && selection_data->data[len-1] == '\n')
						selection_data->data[len-1] = 0;
					// add the new download
					if (CFG.NEED_DIALOG_FOR_DND)
						init_add_dnd_window((char*)selection_data->data);
					else
						aa.add_downloading((char*)selection_data->data, (char*)CFG.GLOBAL_SAVE_PATH,(char*)NULL);
				}
			}
	}
}

/**********************************************************
    End of handler for DnD 
 **********************************************************/

static void my_gtk_clist_set_column_justification (GtkWidget *clist, int col, GtkJustification justify){
	if (ListColumns[col].enum_index<NOTHING_COL)
		gtk_clist_set_column_justification (GTK_CLIST(clist), ListColumns[col].enum_index, justify);
};

void list_of_downloads_init() {
	char *RealListTitles[NOTHING_COL+1];
	for (int i=0;i<ListColumns[NOTHING_COL].enum_index;i++)
		RealListTitles[i]=ListColumns[i].name;
	ListOfDownloads = gtk_clist_new_with_titles( ListColumns[NOTHING_COL].enum_index, RealListTitles);
	gtk_signal_connect(GTK_OBJECT(ListOfDownloads), "select_row",
	                   GTK_SIGNAL_FUNC(select_download),NULL);
	gtk_signal_connect(GTK_OBJECT(ListOfDownloads), "event",
	                   GTK_SIGNAL_FUNC(list_event_callback),NULL);

	gtk_clist_set_row_height(GTK_CLIST(ListOfDownloads),16);
	for(int i=STATUS_COL;i<ListColumns[NOTHING_COL].enum_index;i++)
		gtk_clist_set_column_width (GTK_CLIST(ListOfDownloads),ListColumns[ListColumns[i].type].enum_index,gint(ListColumns[i].size));
	gtk_clist_set_shadow_type (GTK_CLIST(ListOfDownloads), GTK_SHADOW_IN);
	gtk_clist_set_selection_mode(GTK_CLIST(ListOfDownloads),GTK_SELECTION_EXTENDED);
	if (ContainerForCList==NULL) ContainerForCList=gtk_scrolled_window_new(NULL,NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (ContainerForCList),
	                                GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);

	/****************************************************************
	  Initing signals' handlers for DnD support (added by Justin Bradford)
	 ****************************************************************/
	// connect the drag-drop signal
	gtk_signal_connect(GTK_OBJECT(ContainerForCList),
	                   "drag_data_received",
	                   GTK_SIGNAL_FUNC(list_dnd_drop_internal),
	                   NULL);
	// set the list container as a drop destination
	gtk_drag_dest_set(GTK_WIDGET(ContainerForCList),
	                  (GtkDestDefaults)(GTK_DEST_DEFAULT_MOTION |
	                                    GTK_DEST_DEFAULT_HIGHLIGHT |
	                                    GTK_DEST_DEFAULT_DROP),
	                  download_drop_types, n_download_drop_types,
	                  (GdkDragAction)GDK_ACTION_COPY);

	/****************************************************************
	    End of second part of DnD code
	 ****************************************************************/

	gtk_clist_set_hadjustment(GTK_CLIST(ListOfDownloads),NULL);
	gtk_clist_set_vadjustment(GTK_CLIST(ListOfDownloads),NULL);

	my_gtk_clist_set_column_justification (ListOfDownloads, FULL_SIZE_COL, GTK_JUSTIFY_RIGHT);
	my_gtk_clist_set_column_justification (ListOfDownloads, PERCENT_COL, GTK_JUSTIFY_RIGHT);
	my_gtk_clist_set_column_justification (ListOfDownloads, DOWNLOADED_SIZE_COL, GTK_JUSTIFY_RIGHT);
	my_gtk_clist_set_column_justification (ListOfDownloads, REMAIN_SIZE_COL, GTK_JUSTIFY_RIGHT);
	my_gtk_clist_set_column_justification (ListOfDownloads, TREAT_COL, GTK_JUSTIFY_RIGHT);
	my_gtk_clist_set_column_justification (ListOfDownloads, SPEED_COL, GTK_JUSTIFY_RIGHT);
	my_gtk_clist_set_column_justification (ListOfDownloads, ELAPSED_TIME_COL, GTK_JUSTIFY_RIGHT);
	my_gtk_clist_set_column_justification (ListOfDownloads, TIME_COL, GTK_JUSTIFY_RIGHT);
	my_gtk_clist_set_column_justification (ListOfDownloads, PAUSE_COL, GTK_JUSTIFY_RIGHT);
	gtk_clist_set_column_auto_resize(GTK_CLIST(ListOfDownloads),ListColumns[URL_COL].enum_index,TRUE);

	gtk_widget_show(ListOfDownloads);
	gtk_container_add(GTK_CONTAINER(ContainerForCList),ListOfDownloads);
};

void list_of_downloads_get_height() {
	if (!ListOfDownloads) return;
	gint x=0;
	gint y=0;
	gdk_window_get_size(ListOfDownloads->window,&x,&y);
	CFG.WINDOW_CLIST_HEIGHT=int(y);
};

void list_of_downloads_set_height() {
	gtk_widget_set_usize(ListOfDownloads,-1,gint(CFG.WINDOW_CLIST_HEIGHT));
};

/* Setting pixmaps functions;
 */
void list_of_downloads_init_pixmaps(){
#include "pixmaps/wait_xpm.xpm"
#include "pixmaps/run_xpm.xpm"
#include "pixmaps/run_bad.xpm"
#include "pixmaps/run_part.xpm"
#include "pixmaps/stop_xpm.xpm"
#include "pixmaps/stop_wait.xpm"
#include "pixmaps/paused.xpm"
#include "pixmaps/complete.xpm"
		list_of_downloads_pixmaps[PIX_WAIT]=make_pixmap_from_xpm(&(list_of_downloads_bitmaps[PIX_WAIT]),wait_xpm);
		list_of_downloads_pixmaps[PIX_RUN]=make_pixmap_from_xpm(&(list_of_downloads_bitmaps[PIX_RUN]),run_xpm);
		list_of_downloads_pixmaps[PIX_RUN_PART]=make_pixmap_from_xpm(&(list_of_downloads_bitmaps[PIX_RUN_PART]),run_part_xpm);
		list_of_downloads_pixmaps[PIX_RUN_BAD]=make_pixmap_from_xpm(&(list_of_downloads_bitmaps[PIX_RUN_BAD]),run_bad_xpm);
		list_of_downloads_pixmaps[PIX_STOP]=make_pixmap_from_xpm(&(list_of_downloads_bitmaps[PIX_STOP]),stop_xpm);
		list_of_downloads_pixmaps[PIX_STOP_WAIT]=make_pixmap_from_xpm(&(list_of_downloads_bitmaps[PIX_STOP_WAIT]),stop_wait_xpm);
		list_of_downloads_pixmaps[PIX_PAUSE]=make_pixmap_from_xpm(&(list_of_downloads_bitmaps[PIX_PAUSE]),paused_xpm);
		list_of_downloads_pixmaps[PIX_COMPLETE]=make_pixmap_from_xpm(&(list_of_downloads_bitmaps[PIX_COMPLETE]),complete_xpm);
};

void list_of_downloads_set_pixmap(int row,int type){
	if (type>=PIX_UNKNOWN) return;
	if (ListColumns[STATUS_COL].enum_index<ListColumns[NOTHING_COL].enum_index)
		gtk_clist_set_pixmap (GTK_CLIST (ListOfDownloads), row,
	                      ListColumns[STATUS_COL].enum_index, list_of_downloads_pixmaps[type], list_of_downloads_bitmaps[type]);
};
