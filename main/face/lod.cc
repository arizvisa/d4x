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
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "lod.h"
#include "log.h"
#include "buttons.h"
#include "list.h"
#include "lmenu.h"
#include "addd.h"
#include "misc.h"
#include "about.h"
#include "dndtrash.h"
#include "../ntlocale.h"
#include "../locstr.h"
#include "../main.h"
#include "../var.h"
#include "myclist.h"
#include "../sndserv.h"
#include "colors.h"

GtkWidget *ListOfDownloads=(GtkWidget *)NULL;
tConfirmedDialog *AskOpening=(tConfirmedDialog *)NULL;

GdkPixmap *list_of_downloads_pixmaps[PIX_UNKNOWN];
GdkBitmap *list_of_downloads_bitmaps[PIX_UNKNOWN];

GdkBitmap *wait_mask,*stop_mask,*pause_mask,*complete_mask,*run_mask,*part_run_mask,*run_bad_mask,*stop_wait_mask;
GdkPixmap *wait_pixmap=(GdkPixmap *)NULL,*stop_pixmap=(GdkPixmap *)NULL,*pause_pixmap=(GdkPixmap *)NULL,*complete_pixmap=(GdkPixmap *)NULL;
GdkPixmap *run_pixmap=(GdkPixmap *)NULL,*part_run_pixmap=(GdkPixmap *)NULL,*run_bad_pixmap=(GdkPixmap *)NULL,*stop_wait_pixmap=(GdkPixmap *)NULL;

static gchar *ListTitles[]={
	" ",
	N_("File"),
	N_("Type"),
	N_("Full Size"),
	N_("Downloaded"),
	N_("Rest"),
	"%",
	N_("Speed"),
	N_("Time"),
	N_("Remaining"),
	N_("Pause"),
	N_("Attempt"),
	N_("Description"),
	N_("URL"),
	" "
};

static int LoDSortFlag=NOTHING_COL;
static GtkWidget *LoDSelectWindow=(GtkWidget *)NULL;
static GtkWidget *LoDSelectEntry;

tColumn ListColumns[]={
	{STATUS_COL,STATUS_COL,			(char *)NULL,25},
	{FILE_COL,FILE_COL,			(char *)NULL,100},
	{FILE_TYPE_COL,FILE_TYPE_COL,		(char *)NULL,40},
	{FULL_SIZE_COL,FULL_SIZE_COL,		(char *)NULL,70},
	{DOWNLOADED_SIZE_COL,DOWNLOADED_SIZE_COL,(char *)NULL,70},
	{REMAIN_SIZE_COL,REMAIN_SIZE_COL,	(char *)NULL,70},
	{PERCENT_COL,PERCENT_COL,		(char *)NULL,30},
	{SPEED_COL,SPEED_COL,			(char *)NULL,60},
	{TIME_COL,TIME_COL,			(char *)NULL,60},
	{ELAPSED_TIME_COL,ELAPSED_TIME_COL,	(char *)NULL,60},
	{PAUSE_COL,PAUSE_COL,			(char *)NULL,40},
	{TREAT_COL,TREAT_COL,			(char *)NULL,40},
	{DESCRIPTION_COL,DESCRIPTION_COL,	(char *)NULL,100},
	{URL_COL,URL_COL,			(char *)NULL,500},
	{NOTHING_COL,NOTHING_COL,		(char *)NULL,0}};

/******************************************************************
    This part of code for DnD (Drag-n-Drop) support added by
		     Justin Bradford
 ******************************************************************/

// for drag-drop support

// drop handler
// define a target entry listing the mime-types we'll acknowledge
GtkTargetEntry download_drop_types[] = {
	{ "x-url/http",		0, TARGET_URL},
	{ "x-url/ftp",		0, TARGET_URL},
	{ "_NETSCAPE_URL",	0, TARGET_URL},
	{ "x-url/*",		0, TARGET_URL},
	{ "text/uri-list",	0, TARGET_URL},
	{ "text/plain",		0, TARGET_DND_TEXT },
	{ "text/html", 		0, TARGET_DND_TEXT }
};

// calculate the number of mime-types listed
gint n_download_drop_types = sizeof(download_drop_types) / sizeof(download_drop_types[0]);

/*********************************************************************
    End of first part of DnD's code
 *********************************************************************/


void init_columns_info() {
	for (int i=STATUS_COL;i<=NOTHING_COL;i++) {
		if (ListColumns[i].name) delete[] ListColumns[i].name;
		ListColumns[i].name=copy_string(_(ListTitles[ListColumns[i].type]));
	};
};

void select_download(GtkWidget *clist, gint row, gint column,
                     GdkEventButton *event, gpointer data,gpointer nothing) {
	update_progress_bar();
	/* commented to avoid wm hangs (e.g. enl-nt)
	 */
//	update_mainwin_title();
	prepare_buttons();
	gtk_statusbar_pop(GTK_STATUSBAR(MainStatusBar),StatusBarContext);
	tDownload *temp=list_of_downloads_last_selected();
	if (temp)
		gtk_statusbar_push(GTK_STATUSBAR(MainStatusBar),StatusBarContext,temp->info->file.get());
	else
		gtk_statusbar_push(GTK_STATUSBAR(MainStatusBar),StatusBarContext,"");
	if (event && event->type==GDK_2BUTTON_PRESS && event->button==1)
		list_of_downloads_open_logs();
};

tDownload *list_of_downloads_last_selected() {
	GList *select=((GtkCList *)ListOfDownloads)->selection_end;
	if (select) {
		tDownload *temp=(tDownload *)gtk_clist_get_row_data(
		                    GTK_CLIST(ListOfDownloads),GPOINTER_TO_INT(select->data));
		return temp;
	};
	return((tDownload *)NULL);
};

gint list_of_downloads_row(tDownload *what){
	return(gtk_clist_find_row_from_data (GTK_CLIST (ListOfDownloads),what));
};

void list_of_downloads_set_desc(gint row,tDownload *what){
	if (what->Description.get()){
		list_of_downloads_change_data(row,
					      DESCRIPTION_COL,
					      what->Description.get());
	};
};

void list_of_downloads_set_filename(gint row,tDownload *what){
	list_of_downloads_change_data(row,FILE_COL,what->info->file.get());
};

void list_of_downloads_set_percent(int row,int column,float percent){
	int real_col=ListColumns[column].enum_index;	
	if (real_col<ListColumns[NOTHING_COL].enum_index)
		my_gtk_clist_set_progress(GTK_CLIST(ListOfDownloads),row,real_col,percent);
};

void list_of_downloads_change_data(int row,int column,gchar *data) {
	int real_col=ListColumns[column].enum_index;	
	if (real_col<ListColumns[NOTHING_COL].enum_index)
		gtk_clist_set_text(GTK_CLIST(ListOfDownloads),row,real_col,data);
};

void list_of_downloads_set_color(tDownload *what,int row){
	if (what->protect)
		gtk_clist_set_foreground(GTK_CLIST(ListOfDownloads),row,gdk_color_copy(&RED));
	else{
		GtkStyle *style=gtk_widget_get_style(ListOfDownloads);
		if (style)
			gtk_clist_set_foreground(GTK_CLIST(ListOfDownloads),row,
						 gdk_color_copy(&(style->fg[GTK_STATE_NORMAL])));
//		GdkGCValues values;
//		gtk_gc_get_values(style->fg_gc,&values);
//		gtk_clist_set_foreground(GTK_CLIST(ListOfDownloads),row,
//					 values->foreground);
	};
};

void list_of_downloads_update(tDownload *what) {
	char *URL=what->info->url();
	gint row=list_of_downloads_row(what);
	list_of_downloads_change_data(row,URL_COL,URL);
	delete[] URL;
	list_of_downloads_set_desc(row,what);
	list_of_downloads_set_filename(row,what);
};


void list_of_downloads_get_sizes() {
	if (!ListOfDownloads) return;
	GtkCListColumn *tmp=GTK_CLIST(ListOfDownloads)->column;
	for (int i=0;i<ListColumns[NOTHING_COL].enum_index;i++) {
		ListColumns[i].size=int(tmp->width);
		tmp++;
	};
};

void list_of_downloads_print_size(gint row,tDownload *what){
	char data1[MAX_LEN];
	if (what->finfo.size>0){
		make_number_nice(data1,what->finfo.size);
		list_of_downloads_change_data(row,
					      FULL_SIZE_COL,
					      data1);
	};
	if (what->Size.curent>0){
		make_number_nice(data1,what->Size.curent);
		list_of_downloads_change_data(row,
					      DOWNLOADED_SIZE_COL,
					      data1);
	};
	if (what->finfo.size>0 && what->Size.curent<=what->finfo.size){
		float p=(float(what->Size.curent)*float(100))/float(what->finfo.size);
		list_of_downloads_set_percent(row,
					      PERCENT_COL,
					      p);
		make_number_nice(data1,what->finfo.size-what->Size.curent);
		list_of_downloads_change_data(row,
					      REMAIN_SIZE_COL,
					      data1);
	};
};

void list_of_downloads_add(tDownload *what) {
	LoDSortFlag=NOTHING_COL;
	gchar *data[NOTHING_COL+1];
	char *URL=what->info->url();
	for (int i=STATUS_COL;i<=NOTHING_COL;i++)
		data[ListColumns[i].enum_index]="";
	gint row=gtk_clist_append(GTK_CLIST(ListOfDownloads),data);
	gtk_clist_set_row_data(GTK_CLIST(ListOfDownloads),row,gpointer(what));
	list_of_downloads_change_data(row,URL_COL,URL);
	list_of_downloads_set_filename(row,what);
	list_of_downloads_print_size(row,what);
	list_of_downloads_set_desc(row,what);

	list_of_downloads_set_pixmap(row,PIX_WAIT);
	list_of_downloads_set_color(what,row);
	if (what->protect)
	if (row==0) gtk_clist_select_row(GTK_CLIST(ListOfDownloads),0,-1);
	delete[] URL;
};

void list_of_downloads_remove(tDownload *what){
	gint row=list_of_downloads_row(what);
	gtk_clist_remove(GTK_CLIST(ListOfDownloads),row);
};

void list_of_downloads_set_run_icon(tDownload *what){
	switch (what->Status.curent) {
	case D_QUERYING:{
		list_of_downloads_set_pixmap(what,PIX_RUN_PART);
		break;
	};
	default:
	case D_DOWNLOAD:{
		list_of_downloads_set_pixmap(what,PIX_RUN);
		break;
	};
	case D_DOWNLOAD_BAD:{
		list_of_downloads_set_pixmap(what,PIX_RUN_BAD);
		break;
	};
	};
};

void list_of_downloads_add(tDownload *what,int row) {
	LoDSortFlag=NOTHING_COL;
	gchar *data[NOTHING_COL+1];
	for (int i=STATUS_COL;i<=URL_COL;i++)
		data[i]=(gchar *)NULL;
	gtk_clist_insert(GTK_CLIST(ListOfDownloads),row,data);
	gtk_clist_set_row_data(GTK_CLIST(ListOfDownloads),row,what);
	list_of_downloads_set_color(what,row);
	char *URL=what->info->url();
	list_of_downloads_change_data(row,URL_COL,URL);
	delete[] URL;
	switch (what->owner) {
	case DL_WAIT:{
		list_of_downloads_set_pixmap(row,PIX_WAIT);
		break;
	};
	case DL_STOP:{
		list_of_downloads_set_pixmap(row,PIX_STOP);
		break;
	};
	case DL_RUN:{
		what->update_trigers();
		list_of_downloads_set_run_icon(what);
		break;
	};
	case DL_PAUSE:{
		list_of_downloads_set_pixmap(row,PIX_PAUSE);
		break;
	};
	case DL_COMPLETE:{
		list_of_downloads_set_pixmap(row,PIX_COMPLETE);
	};
	};
	list_of_downloads_print_size(row,what);
	list_of_downloads_set_filename(row,what);
	list_of_downloads_set_desc(row,what);
};

void move_download_up(int row){
	gtk_clist_swap_rows(GTK_CLIST(ListOfDownloads),row,row-1);
	tDownload *what=get_download_from_clist(row-1);
	tDownload *what2=get_download_from_clist(row);
	if (DOWNLOAD_QUEUES[DL_WAIT]->owner(what) && DOWNLOAD_QUEUES[DL_WAIT]->owner(what2))
		DOWNLOAD_QUEUES[DL_WAIT]->forward(what);
};

void move_download_down(int row){
	gtk_clist_swap_rows(GTK_CLIST(ListOfDownloads),row,row+1);
	tDownload *what=get_download_from_clist(row+1);
	tDownload *what2=get_download_from_clist(row);
	if (DOWNLOAD_QUEUES[DL_WAIT]->owner(what) && DOWNLOAD_QUEUES[DL_WAIT]->owner(what2))
		DOWNLOAD_QUEUES[DL_WAIT]->backward(what);
};
static gint compare_nodes1(gconstpointer a,gconstpointer b){
    gint aa=GPOINTER_TO_INT(a);
    gint bb=GPOINTER_TO_INT(b);
    if (aa>bb) return 1;
    if (aa==bb) return 0;
    return -1;
};

static gint compare_nodes2(gconstpointer a,gconstpointer b){
    gint aa=GPOINTER_TO_INT(a);
    gint bb=GPOINTER_TO_INT(b);
    if (aa>bb) return -1;
    if (aa==bb) return 0;
    return 1;
};

int list_of_downloads_move_selected_up(){
	GList *select=((GtkCList *)ListOfDownloads)->selection;
	if (select==NULL) return 0;
	select=((GtkCList *)ListOfDownloads)->selection;
	GList *sorted_select=g_list_copy(select);
	sorted_select=g_list_sort(sorted_select,compare_nodes1);
	select=sorted_select;
	if (GPOINTER_TO_INT(select->data)<=0) return 0;
	while (select) {
		move_download_up(GPOINTER_TO_INT(select->data));
		select=select->next;
	};
	g_list_free(sorted_select);
	return 1;
};

int list_of_downloads_move_selected_down(){
	GList *select=((GtkCList *)ListOfDownloads)->selection;
	if (select==NULL) return 0;
	select=((GtkCList *)ListOfDownloads)->selection;
	GList *sorted_select=g_list_copy(select);
	sorted_select=g_list_sort(sorted_select,compare_nodes2);
	select=sorted_select;
	if (GPOINTER_TO_INT(select->data)>=GTK_CLIST(ListOfDownloads)->rows-1) return 0;
	while (select) {
		move_download_down(GPOINTER_TO_INT(select->data));
		select=select->next;
	};
	g_list_free(sorted_select);
	return 1;
};


void list_of_downloads_move_up(){
	list_of_downloads_freeze();
	list_of_downloads_move_selected_up();
	list_of_downloads_unfreeze();
};

void list_of_downloads_move_down(){
	list_of_downloads_freeze();
	list_of_downloads_move_selected_down();
	list_of_downloads_unfreeze();
};

void list_of_downloads_move_selected_home(){
	list_of_downloads_freeze();
	while (list_of_downloads_move_selected_up());
	list_of_downloads_unfreeze();
};

void list_of_downloads_move_selected_end(){
	list_of_downloads_freeze();
	while (list_of_downloads_move_selected_down());
	list_of_downloads_unfreeze();
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

void list_of_downloads_real_select(int type){
	char *wildcard=text_from_combo(LoDSelectEntry);
	if (wildcard==NULL || *wildcard==0) return;
	if (type){
		for (int i=0;i<GTK_CLIST(ListOfDownloads)->rows;i++){
			tDownload *dwn=get_download_from_clist(i);
			if (dwn && dwn->info->file.get() &&
			    check_mask2(dwn->info->file.get(),wildcard))
				gtk_clist_unselect_row(GTK_CLIST(ListOfDownloads),i,-1);
		};
	}else{
		for (int i=0;i<GTK_CLIST(ListOfDownloads)->rows;i++){
			tDownload *dwn=get_download_from_clist(i);
			if (dwn && dwn->info->file.get() &&
			    check_mask2(dwn->info->file.get(),wildcard))
				gtk_clist_select_row(GTK_CLIST(ListOfDownloads),i,-1);
		};
	};
};

static void _select_ok_(GtkButton *button,gint type){
	list_of_downloads_real_select(type);
	gtk_widget_destroy(LoDSelectWindow);
	LoDSelectWindow=NULL;
};

static void _select_cancel_(GtkButton *button,gpointer unused){
	gtk_widget_destroy(LoDSelectWindow);
	LoDSelectWindow=NULL;
};

static gint _select_delete_(GtkWidget *window,GdkEvent *event,
			    gpointer unused) {
	gtk_widget_destroy(LoDSelectWindow);
	LoDSelectWindow=NULL;
	return(TRUE);
};

/*
  void list_of_downloads_select()

  this routine bring up a dialog for selecting
  items in queue of downloads by wildcart
*/

void list_of_downloads_select(int type=0){
	if (LoDSelectWindow){
		gdk_window_show(LoDSelectWindow->window);
		return;
	};
	LoDSelectWindow= gtk_window_new(GTK_WINDOW_DIALOG);
	gtk_window_set_wmclass(GTK_WINDOW(LoDSelectWindow),
			       "D4X_SelectDialog","D4X");
	gtk_window_set_title(GTK_WINDOW (LoDSelectWindow),
			     _("Enter wildcard"));
	gtk_window_set_position(GTK_WINDOW(LoDSelectWindow),
				GTK_WIN_POS_CENTER);
	gtk_container_border_width(GTK_CONTAINER(LoDSelectWindow),5);
	GtkWidget *vbox=gtk_vbox_new(FALSE,0);
	GtkWidget *hbox=gtk_hbutton_box_new();
	gtk_box_set_spacing(GTK_BOX(vbox),5);
	LoDSelectEntry=gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(vbox),LoDSelectEntry,FALSE,FALSE,0);
	gtk_box_set_spacing(GTK_BOX(hbox),5);
	GtkWidget *ok_button=gtk_button_new_with_label(_("Ok"));
	GtkWidget *cancel_button=gtk_button_new_with_label(_("Cancel"));
	GTK_WIDGET_SET_FLAGS(ok_button,GTK_CAN_DEFAULT);
	GTK_WIDGET_SET_FLAGS(cancel_button,GTK_CAN_DEFAULT);
	gtk_box_pack_start(GTK_BOX(hbox),ok_button,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(hbox),cancel_button,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);
	gtk_signal_connect(GTK_OBJECT(ok_button),"clicked",
			   GTK_SIGNAL_FUNC(_select_ok_),GINT_TO_POINTER(type));
	gtk_signal_connect(GTK_OBJECT(cancel_button),"clicked",
			   GTK_SIGNAL_FUNC(_select_cancel_),NULL);
	gtk_signal_connect(GTK_OBJECT(LoDSelectWindow),"delete_event",
			   GTK_SIGNAL_FUNC(_select_delete_),NULL);
	gtk_signal_connect(GTK_OBJECT(LoDSelectEntry), "activate",
			   GTK_SIGNAL_FUNC (_select_ok_), GINT_TO_POINTER(type));
	GtkWidget *frame=NULL;
	if (type)
		frame=gtk_frame_new(_("Unselect"));
	else
		frame=gtk_frame_new(_("Select"));
	gtk_container_add(GTK_CONTAINER(frame),vbox);
	gtk_container_add(GTK_CONTAINER(LoDSelectWindow),frame);
	gtk_widget_show_all(LoDSelectWindow);
	gtk_window_set_default(GTK_WINDOW(LoDSelectWindow),ok_button);
	gtk_widget_grab_focus(LoDSelectEntry);
	gtk_window_set_modal (GTK_WINDOW(LoDSelectWindow),TRUE);
	gtk_window_set_transient_for (GTK_WINDOW (LoDSelectWindow),
				      GTK_WINDOW (MainWindow));
};

int list_event_callback(GtkWidget *widget,GdkEvent *event) {
	if (event->type == GDK_BUTTON_PRESS) {
		GdkEventButton *bevent=(GdkEventButton *)event;
		if (bevent->button==3) {
			int row;
			if (gtk_clist_get_selection_info(GTK_CLIST(widget),int(bevent->x),int(bevent->y),&row,(gint *)NULL)) {
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
			gdk_window_get_pointer((GdkWindow *)NULL,&x,&y, &modmask);
			//          util_item_factory_popup(list_menu_itemfact,x,y,3,GDK_CURRENT_TIME);
			list_menu_prepare();
			gtk_menu_popup(GTK_MENU(ListMenu),(GtkWidget *)NULL,(GtkWidget *)NULL,(GtkMenuPositionFunc)NULL,(gpointer)NULL,bevent->button,bevent->time);
			return TRUE;
		};
	};
	if (event->type == GDK_KEY_PRESS) {
		GdkEventKey *kevent=(GdkEventKey *)event;
		switch(kevent->keyval) {
		case GDK_Delete:
		case GDK_KP_Delete:{
			ask_delete_download();
			return TRUE;
		};
		case GDK_KP_Enter:
		case GDK_Return:{
			list_of_downloads_open_logs();
			return TRUE;
		};
		case GDK_KP_Add:{
			list_of_downloads_select();
			return TRUE;
		};
		case GDK_KP_Subtract:{
			list_of_downloads_select(1);
			return TRUE;
		};
		};
		if (kevent->state & GDK_SHIFT_MASK) {
			switch (kevent->keyval) {
			case GDK_KP_Up:
			case GDK_Up:{
				list_of_downloads_move_up();
				return TRUE;
			};
			case GDK_KP_Down:
			case GDK_Down:{
				list_of_downloads_move_down();
				return TRUE;
			};
			case GDK_KP_Page_Up:
			case GDK_Page_Up:{
				list_of_downloads_move_selected_home();
				return TRUE;
			};
			case GDK_KP_Page_Down:
			case GDK_Page_Down:{
				list_of_downloads_move_selected_end();
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
	case TARGET_DND_TEXT:
	case TARGET_URL:{
		// make sure our url (in selection_data->data) is good
		/*
		printf("%s\n",gdk_atom_name(selection_data->type));
		printf("%s\n",gdk_atom_name(selection_data->target));
		printf("%s\n",gdk_atom_name(selection_data->selection));
		*/
		if (selection_data->data != NULL) {
			if (!GTK_IS_SCROLLED_WINDOW(widget))
				dnd_trash_animation();
			SOUND_SERVER->add_event(SND_DND_DROP);
			int len = strlen((char*)selection_data->data);
			if (len && selection_data->data[len-1] == '\n')
				selection_data->data[len-1] = 0;
			// add the new download
			char *str = (char*)selection_data->data;
			int sbd=0;//should be deleted flag
			char *ent=index(str,'\n');
			if (ent) *ent=0;
			unsigned char *a=(unsigned char *)str;
			while (*a){ // to avoid invalid signs
				if (*a<' '){
					*a=0;
					break;
				};
				a++;
			};
			/* check for gmc style URL */
			const char *gmc_url="file:/#ftp:";
			if (begin_string((char*)selection_data->data,gmc_url)){
				str = sum_strings("ftp://",
						  (char*)selection_data->data + strlen(gmc_url),
						  (char*)NULL);
				sbd=1;
			};
			char *desc=ent?ent+1:(char *)NULL;
			if (CFG.NEED_DIALOG_FOR_DND){
				init_add_dnd_window(str,desc);
			}else{
				aa.add_downloading(str, (char*)CFG.GLOBAL_SAVE_PATH,(char*)NULL,desc);
			};
			if (sbd) delete[] str;
		}
	}
	}
}

/**********************************************************
    End of handler for DnD 
 **********************************************************/

void list_of_downloads_rebuild_wait(){
	if (DOWNLOAD_QUEUES[DL_WAIT]->count()==0) return;
	int i=0;
	tDList *dlist=new tDList(DL_WAIT);
	dlist->init(0);
	tDownload *tmp=(tDownload *)gtk_clist_get_row_data(GTK_CLIST(ListOfDownloads),i);
	while(tmp){
		if (tmp->owner==DL_WAIT){
			DOWNLOAD_QUEUES[DL_WAIT]->del(tmp);
			dlist->insert(tmp);
			if (DOWNLOAD_QUEUES[DL_WAIT]->count()==0)
				break;
		};
		i+=1;
		tmp=(tDownload *)gtk_clist_get_row_data(GTK_CLIST(ListOfDownloads),i);
	};
	delete(DOWNLOAD_QUEUES[DL_WAIT]);
	DOWNLOAD_QUEUES[DL_WAIT]=dlist;
};

static int _cmp_bypercent(tDownload *a,tDownload *b){
	return( b->Percent>a->Percent?1:-1);
};

static int _cmp_bysize(tDownload *a,tDownload *b){
	return( b->finfo.size - a->finfo.size );
};

static int _cmp_bydsize(tDownload *a,tDownload *b){
	return( b->Size.curent - a->Size.curent );
};

static int _cmp_byremain(tDownload *a,tDownload *b){
	return( b->Remain.curent - a->Remain.curent );
};

static int _cmp_byspeed(tDownload *a,tDownload *b){
	return( b->Speed.curent - a->Speed.curent);
};

static int _cmp_byfile(tDownload *a,tDownload *b){
	if (a->info->file.get() && b->info->file.get())
		return(strcmp(a->info->file.get(),
			      b->info->file.get()));
	if (a->info->file.get()) return(1);
	if (b->info->file.get()) return(0);
	return(-1);
};

static int _cmp_whole_size_(GtkCList *clist,
			    gconstpointer ptr1,
			    gconstpointer ptr2){
	GtkCListRow *row1=(GtkCListRow *)ptr1;
	GtkCListRow *row2=(GtkCListRow *)ptr2;
	if (row1->data && row2->data){
		return(_cmp_bysize((tDownload *)(row1->data),
				   (tDownload *)(row2->data)));
	};
	if (row1->data)	return(1);
	if (row2->data)	return(0);
	return(-1);
};

#define CLIST_CMP_I(arg) static int arg##i_(GtkCList *clist, \
					     gconstpointer ptr1, \
					     gconstpointer ptr2){ \
   return (arg(clist,ptr2,ptr1)); \
}

CLIST_CMP_I(_cmp_whole_size_);

static int _cmp_whole_percent_(GtkCList *clist,
			    gconstpointer ptr1,
			    gconstpointer ptr2){
	GtkCListRow *row1=(GtkCListRow *)ptr1;
	GtkCListRow *row2=(GtkCListRow *)ptr2;
	if (row1->data && row2->data){
		return(_cmp_bypercent((tDownload *)(row1->data),
				      (tDownload *)(row2->data)));
	};
	if (row1->data)	return(1);
	if (row2->data)	return(0);
	return(-1);
};

CLIST_CMP_I(_cmp_whole_percent_);

static int _cmp_whole_file_(GtkCList *clist,
			    gconstpointer ptr1,
			    gconstpointer ptr2){
	GtkCListRow *row1=(GtkCListRow *)ptr1;
	GtkCListRow *row2=(GtkCListRow *)ptr2;
	if (row1->data && row2->data){
		return(_cmp_byfile((tDownload *)(row1->data),
				      (tDownload *)(row2->data)));
	};
	if (row1->data) return(1);
	if (row2->data)	return(0);
	return(-1);
};

CLIST_CMP_I(_cmp_whole_file_);

static void list_of_downloads_sort(GtkWidget *widget,int how){
	int count=DOWNLOAD_QUEUES[DL_RUN]->count();
	int (*cmp_func)(tDownload *,tDownload *)=(int)NULL;
	GdkModifierType mask;
	gint x,y;
	gdk_window_get_pointer(MainWindow->window,&x,&y,&mask);
	int whole_list=mask & GDK_SHIFT_MASK;
	switch(how){
	case FILE_COL:{
		if (whole_list){
			gtk_clist_set_compare_func(GTK_CLIST(ListOfDownloads),
						   LoDSortFlag==FILE_COL?_cmp_whole_file_:_cmp_whole_file_i_);
			gtk_clist_sort(GTK_CLIST(ListOfDownloads));
			list_of_downloads_rebuild_wait();
			LoDSortFlag=LoDSortFlag==FILE_COL?-FILE_COL:FILE_COL;
			return;
		};
		cmp_func=_cmp_byfile;
	};
	case PERCENT_COL:
		if (whole_list){
			gtk_clist_set_compare_func(GTK_CLIST(ListOfDownloads),
						   LoDSortFlag==PERCENT_COL?_cmp_whole_percent_:_cmp_whole_percent_i_);
			gtk_clist_sort(GTK_CLIST(ListOfDownloads));
			list_of_downloads_rebuild_wait();
			LoDSortFlag=LoDSortFlag==PERCENT_COL?-PERCENT_COL:PERCENT_COL;
			return;
		};
		cmp_func=_cmp_bypercent;
		break;
	case FULL_SIZE_COL:{
		if (whole_list){
			gtk_clist_set_compare_func(GTK_CLIST(ListOfDownloads),
						   LoDSortFlag==FULL_SIZE_COL?_cmp_whole_size_:_cmp_whole_size_i_);
			gtk_clist_sort(GTK_CLIST(ListOfDownloads));
			list_of_downloads_rebuild_wait();
			LoDSortFlag=LoDSortFlag==FULL_SIZE_COL?-FULL_SIZE_COL:FULL_SIZE_COL;
			return;
		};
		cmp_func=_cmp_bysize;
		break;
	};
	case DOWNLOADED_SIZE_COL:
		cmp_func=_cmp_bydsize;		
		break;
	case REMAIN_SIZE_COL:
		cmp_func=_cmp_byremain;		
		break;
	case SPEED_COL:
		cmp_func=_cmp_byspeed;		
		break;
	};
	if (count<2) return; //nothing todo
	if (cmp_func==NULL) return;
	list_of_downloads_freeze();
	tDownload *tmp=DOWNLOAD_QUEUES[DL_RUN]->last();
	tDownload *cur=(tDownload*)(tmp->next);
	int changed;
	do{
		changed=0;
		gint row1=list_of_downloads_row(tmp);
		while (cur){
			gint row=list_of_downloads_row(cur);
			if ((row<row1 && cmp_func(cur,tmp)<0) ||
			    (row>row1 && cmp_func(cur,tmp)>0)){
				list_of_downloads_swap(tmp,cur);
				row1=row;
				changed=1;
			};
			cur=(tDownload*)(cur->next);
		};
		if (changed==0){
			tmp=(tDownload*)(tmp->next);
			changed=1;
		};
		cur=(tDownload*)(tmp->next);
	}while(changed && cur);
	list_of_downloads_unfreeze();
	LoDSortFlag=NOTHING_COL;
};

static int _cmp_whole_status_(GtkCList *clist,
			      gconstpointer ptr1,
			      gconstpointer ptr2){
	GtkCListRow *row1=(GtkCListRow *)ptr1;
	GtkCListRow *row2=(GtkCListRow *)ptr2;
	if (row1->data && row2->data){
		int smap[]={0,10,6,8,9,11,7,0};
		tDownload *d1=(tDownload *)(row1->data);
		tDownload *d2=(tDownload *)(row2->data);
		return(smap[d2->owner]-smap[d1->owner]);
	};
	if (row1->data)	return(1);
	if (row2->data)	return(0);
	return(-1);
};

CLIST_CMP_I(_cmp_whole_status_);

static void list_of_downloads_sort_status(GtkWidget *button){
	GdkModifierType mask;
	gint x,y;
	gdk_window_get_pointer(MainWindow->window,&x,&y,&mask);
	if (mask & GDK_SHIFT_MASK){
		gtk_clist_set_compare_func(GTK_CLIST(ListOfDownloads),
					   LoDSortFlag==NOTHING_COL+1?_cmp_whole_status_:_cmp_whole_status_i_);
		gtk_clist_sort(GTK_CLIST(ListOfDownloads));
		list_of_downloads_rebuild_wait();
		LoDSortFlag=LoDSortFlag==NOTHING_COL+1?-NOTHING_COL:NOTHING_COL+1;
	};
};

static void my_gtk_clist_set_column_justification (GtkWidget *clist, int col, GtkJustification justify){
	if (ListColumns[col].enum_index<ListColumns[NOTHING_COL].enum_index)
		gtk_clist_set_column_justification (GTK_CLIST(clist), ListColumns[col].enum_index, justify);
};

static GtkWidget *my_gtk_clist_get_column_widget(GtkWidget *clist, int col){
	if (ListColumns[col].enum_index<ListColumns[NOTHING_COL].enum_index)
		return (GTK_CLIST(clist)->column[ListColumns[col].enum_index].button);
	return((GtkWidget *)NULL);
};

void list_of_downloads_init() {
	char *RealListTitles[NOTHING_COL+1];
	for (int i=0;i<ListColumns[NOTHING_COL].enum_index;i++)
		RealListTitles[i]=ListColumns[i].name;
	ListOfDownloads = my_gtk_clist_new_with_titles( ListColumns[NOTHING_COL].enum_index, RealListTitles);
	gtk_signal_connect(GTK_OBJECT(ListOfDownloads), "select_row",
	                   GTK_SIGNAL_FUNC(select_download),NULL);
	gtk_signal_connect(GTK_OBJECT(ListOfDownloads), "event",
	                   GTK_SIGNAL_FUNC(list_event_callback),NULL);

	gtk_clist_set_row_height(GTK_CLIST(ListOfDownloads),16);
	for(int i=STATUS_COL;i<ListColumns[NOTHING_COL].enum_index;i++)
		gtk_clist_set_column_width (GTK_CLIST(ListOfDownloads),ListColumns[ListColumns[i].type].enum_index,gint(ListColumns[i].size));
	gtk_clist_set_shadow_type (GTK_CLIST(ListOfDownloads), GTK_SHADOW_IN);
	gtk_clist_set_selection_mode(GTK_CLIST(ListOfDownloads),GTK_SELECTION_EXTENDED);
	if (ContainerForCList==NULL) ContainerForCList=gtk_scrolled_window_new((GtkAdjustment *)NULL,(GtkAdjustment *)NULL);
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
	                  (GdkDragAction)(GDK_ACTION_COPY|GDK_ACTION_MOVE));

	/****************************************************************
	    End of second part of DnD code
	 ****************************************************************/

	gtk_clist_set_hadjustment(GTK_CLIST(ListOfDownloads),(GtkAdjustment *)NULL);
	gtk_clist_set_vadjustment(GTK_CLIST(ListOfDownloads),(GtkAdjustment *)NULL);

	my_gtk_clist_set_column_justification (ListOfDownloads, FULL_SIZE_COL, GTK_JUSTIFY_RIGHT);
	my_gtk_clist_set_column_justification (ListOfDownloads, PERCENT_COL, GTK_JUSTIFY_CENTER);
	my_gtk_clist_set_column_justification (ListOfDownloads, DOWNLOADED_SIZE_COL, GTK_JUSTIFY_RIGHT);
	my_gtk_clist_set_column_justification (ListOfDownloads, REMAIN_SIZE_COL, GTK_JUSTIFY_RIGHT);
	my_gtk_clist_set_column_justification (ListOfDownloads, TREAT_COL, GTK_JUSTIFY_RIGHT);
	my_gtk_clist_set_column_justification (ListOfDownloads, SPEED_COL, GTK_JUSTIFY_RIGHT);
	my_gtk_clist_set_column_justification (ListOfDownloads, ELAPSED_TIME_COL, GTK_JUSTIFY_RIGHT);
	my_gtk_clist_set_column_justification (ListOfDownloads, TIME_COL, GTK_JUSTIFY_RIGHT);
	my_gtk_clist_set_column_justification (ListOfDownloads, PAUSE_COL, GTK_JUSTIFY_RIGHT);
	gtk_clist_set_column_auto_resize(GTK_CLIST(ListOfDownloads),ListColumns[URL_COL].enum_index,TRUE);
	GtkWidget *button=my_gtk_clist_get_column_widget(ListOfDownloads,PERCENT_COL);
	if (button)
		gtk_signal_connect(GTK_OBJECT(button),
				   "clicked",
				   GTK_SIGNAL_FUNC(list_of_downloads_sort),
				   GINT_TO_POINTER(PERCENT_COL));
	button=my_gtk_clist_get_column_widget(ListOfDownloads,FILE_COL);
	if (button)
		gtk_signal_connect(GTK_OBJECT(button),
				   "clicked",
				   GTK_SIGNAL_FUNC(list_of_downloads_sort),
				   GINT_TO_POINTER(FILE_COL));
	button=my_gtk_clist_get_column_widget(ListOfDownloads,STATUS_COL);
	if (button)
		gtk_signal_connect(GTK_OBJECT(button),
				   "clicked",
				   GTK_SIGNAL_FUNC(list_of_downloads_sort_status),
				   NULL);
	button=my_gtk_clist_get_column_widget(ListOfDownloads,DOWNLOADED_SIZE_COL);
	if (button)
		gtk_signal_connect(GTK_OBJECT(button),
				   "clicked",
				   GTK_SIGNAL_FUNC(list_of_downloads_sort),
				   GINT_TO_POINTER(DOWNLOADED_SIZE_COL));
	button=my_gtk_clist_get_column_widget(ListOfDownloads,SPEED_COL);
	if (button)
		gtk_signal_connect(GTK_OBJECT(button),
				   "clicked",
				   GTK_SIGNAL_FUNC(list_of_downloads_sort),
				   GINT_TO_POINTER(SPEED_COL));
	button=my_gtk_clist_get_column_widget(ListOfDownloads,REMAIN_SIZE_COL);
	if (button)
		gtk_signal_connect(GTK_OBJECT(button),
				   "clicked",
				   GTK_SIGNAL_FUNC(list_of_downloads_sort),
				   GINT_TO_POINTER(REMAIN_SIZE_COL));
	button=my_gtk_clist_get_column_widget(ListOfDownloads,FULL_SIZE_COL);
	if (button)
		gtk_signal_connect(GTK_OBJECT(button),
				   "clicked",
				   GTK_SIGNAL_FUNC(list_of_downloads_sort),
				   GINT_TO_POINTER(FULL_SIZE_COL));

	gtk_widget_show(ListOfDownloads);
	gtk_container_add(GTK_CONTAINER(ContainerForCList),ListOfDownloads);
};

gint list_of_downloads_get_height() {
/*	if (!ListOfDownloads) return;
	gint x=0;
	gint y=0;
	gdk_window_get_size(ListOfDownloads->window,&x,&y);
	CFG.WINDOW_CLIST_HEIGHT=int(y);
	if (ContainerForCList){
		y=0;
		if (GTK_SCROLLED_WINDOW(ContainerForCList)->hscrollbar &&
		    GTK_SCROLLED_WINDOW(ContainerForCList)->hscrollbar->window){
			gdk_window_get_size(GTK_SCROLLED_WINDOW(ContainerForCList)->hscrollbar->window,&x,&y);
		};
		CFG.WINDOW_CLIST_HEIGHT+=int(y)+3;
	};
*/
	if (!MAIN_PANED) return FALSE;
	CFG.WINDOW_CLIST_HEIGHT=GTK_PANED(MAIN_PANED)->child1_size;
	return FALSE;
};

void list_of_downloads_set_height() {
//	gtk_widget_set_usize(ListOfDownloads,-1,gint(CFG.WINDOW_CLIST_HEIGHT));
	gtk_paned_set_position(GTK_PANED(MAIN_PANED),gint(CFG.WINDOW_CLIST_HEIGHT));
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
		/* we will use these pixmaps many times */
		for (int i=0;i<PIX_UNKNOWN;i++){
			gdk_pixmap_ref(list_of_downloads_pixmaps[i]);
			gdk_bitmap_ref(list_of_downloads_bitmaps[i]);
		};
};

void list_of_downloads_set_pixmap(gint row,int type){
	if (type>=PIX_UNKNOWN) return;
	if (ListColumns[STATUS_COL].enum_index<ListColumns[NOTHING_COL].enum_index)
		gtk_clist_set_pixmap (GTK_CLIST (ListOfDownloads), row,
	                      ListColumns[STATUS_COL].enum_index, list_of_downloads_pixmaps[type], list_of_downloads_bitmaps[type]);
};

void list_of_downloads_set_pixmap(tDownload *dwn,int type){
	if (type>=PIX_UNKNOWN) return;
	list_of_downloads_set_pixmap(list_of_downloads_row(dwn),type);
};

void list_of_downloads_unselect_all(){
	if (GTK_CLIST(ListOfDownloads)->rows)
		gtk_clist_unselect_all(GTK_CLIST(ListOfDownloads));
};

void list_of_downloads_select_all(){
	if (GTK_CLIST(ListOfDownloads)->rows)
		gtk_clist_select_all(GTK_CLIST(ListOfDownloads));
};

void list_of_downloads_invert_selection(){
	if (GTK_CLIST(ListOfDownloads)->rows==0) return;
	list_of_downloads_freeze();
	GList *select=g_list_copy(((GtkCList *)ListOfDownloads)->selection);
	gtk_clist_select_all(GTK_CLIST(ListOfDownloads));
	while(select!=NULL){
		gtk_clist_unselect_row(GTK_CLIST(ListOfDownloads),GPOINTER_TO_INT(select->data),-1);
		select=g_list_remove_link(select,select);
	};
	list_of_downloads_unfreeze();
};

int list_of_downloads_sel(){
	return(GTK_CLIST(ListOfDownloads)->selection==NULL?1:0);
};

void list_of_downloads_swap(tDownload *a,tDownload *b){
	gint rowa=list_of_downloads_row(a);
	gint rowb=list_of_downloads_row(b);
	gtk_clist_swap_rows(GTK_CLIST(ListOfDownloads),rowa,rowb);
};

/* Various additional functions
 */


static void _continue_opening_logs_(GtkWidget *widget,tConfirmedDialog *parent){
	CFG.CONFIRM_OPENING_MANY=!(GTK_TOGGLE_BUTTON(parent->check)->active);	
	GList *select=GTK_CLIST(ListOfDownloads)->selection;
	while (select) {
		tDownload *temp=(tDownload *)gtk_clist_get_row_data(
		                    GTK_CLIST(ListOfDownloads),GPOINTER_TO_INT(select->data));
		if (temp && (temp->LOG==NULL || temp->LOG->Window==NULL))
			log_window_init(temp);
		select=select->next;
	};
	if (AskOpening)
		AskOpening->done();
};

void list_of_downloads_open_logs(...) {
	GList *select=GTK_CLIST(ListOfDownloads)->selection;
	int a=5;
	while (select) {
		tDownload *temp=(tDownload *)gtk_clist_get_row_data(
		                    GTK_CLIST(ListOfDownloads),GPOINTER_TO_INT(select->data));
		if (temp && (temp->LOG==NULL || temp->LOG->Window==NULL))
			a-=1;
		log_window_init(temp);
		select=select->next;
		if (a<0 && select && CFG.CONFIRM_OPENING_MANY){
			if (!AskOpening) AskOpening=new tConfirmedDialog;
			if (AskOpening->init(_("Continue open log windows?"),_("Open logs?")))
				gtk_signal_connect(GTK_OBJECT(AskOpening->ok_button),
						   "clicked",
						   GTK_SIGNAL_FUNC(_continue_opening_logs_),
						   AskOpening);
			AskOpening->set_modal(MainWindow);
			break;
		};
	};
};

void list_of_downloads_set_shift(float shift){
	GtkAdjustment *adj=gtk_clist_get_vadjustment(GTK_CLIST(ListOfDownloads));
	if (adj->upper>shift){
		adj->value=shift;
		gtk_signal_emit_by_name (GTK_OBJECT (adj), "value_changed");
	};
};
