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
#include "../xml.h"
#include "myclist.h"
#include "colors.h"
#include <gdk-pixbuf/gdk-pixbuf.h>

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

static GtkWidget *LoDSelectWindow=(GtkWidget *)NULL;
static GtkWidget *LoDSelectEntry;
static int LoDSelectType=0;

/* functions to store list ordering when run without interface */

#define _INIT_QVP_(arg,arg1) cols[arg].type=cols[arg].enum_index=arg;cols[arg].size=arg1;

d4xQVPrefs::d4xQVPrefs(){
	_INIT_QVP_(STATUS_COL,25);
	_INIT_QVP_(STATUS_COL,25);
	_INIT_QVP_(FILE_COL,100);
	_INIT_QVP_(FILE_TYPE_COL,40);
	_INIT_QVP_(FULL_SIZE_COL,70);
	_INIT_QVP_(DOWNLOADED_SIZE_COL,70);
	_INIT_QVP_(REMAIN_SIZE_COL,70);
	_INIT_QVP_(PERCENT_COL,30);
	_INIT_QVP_(SPEED_COL,60);
	_INIT_QVP_(TIME_COL,60);
	_INIT_QVP_(ELAPSED_TIME_COL,60);
	_INIT_QVP_(PAUSE_COL,40);
	_INIT_QVP_(TREAT_COL,40);
	_INIT_QVP_(DESCRIPTION_COL,100);
	_INIT_QVP_(URL_COL,500);
	_INIT_QVP_(NOTHING_COL,0);
};

extern d4xQVPrefs QV_PREFS;

d4xQueueView::d4xQueueView(){
	LoDSortFlag=NOTHING_COL;
/*
	for (int i=0;i<=NOTHING_COL;i++){
		prefs.cols[i].size=QV_PREFS.cols[i].size;
		prefs.cols[i].type=QV_PREFS.cols[i].type;
		prefs.cols[i].enum_index=QV_PREFS.cols[i].enum_index;
	};
*/
	ListOfDownloads=NULL;
};

d4xQueueView::~d4xQueueView(){
	if (ListOfDownloads)
		gtk_object_unref(GTK_OBJECT(ListOfDownloads));
};

void d4xQueueView::remove_wf(tDownload *what){
	d4xWFNode *node=(d4xWFNode *)(what->WFP);
	if (node){
		ALL_DOWNLOADS->lock();
		ListOfDownloadsWF.del(node);
		ALL_DOWNLOADS->unlock();
		delete(node);
	};
	what->WFP=(tNode*)NULL;
};

void d4xQueueView::add_wf(tDownload *what){
	d4xWFNode *node=new d4xWFNode;
	node->dwn=what;
	what->WFP=node;
	ALL_DOWNLOADS->lock();
	ListOfDownloadsWF.insert(node);
	ALL_DOWNLOADS->unlock();
};

/***************************************************************/

gint lod_get_height() {
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
	if (!MAIN_PANED1) return FALSE;
	CFG.WINDOW_TREE_WIDTH=GTK_PANED(MAIN_PANED1)->child1_size;
	return FALSE;
};

void lod_set_height() {
//	gtk_widget_set_usize(ListOfDownloads,-1,gint(CFG.WINDOW_CLIST_HEIGHT));
	gtk_paned_set_position(GTK_PANED(MAIN_PANED),gint(CFG.WINDOW_CLIST_HEIGHT));
	gtk_paned_set_position(GTK_PANED(MAIN_PANED1),gint(CFG.WINDOW_TREE_WIDTH));
};


void lod_init_pixmaps(){
#include "pixmaps/wait_xpm.xpm"
#include "pixmaps/run_xpm.xpm"
#include "pixmaps/run1.xpm"
#include "pixmaps/run2.xpm"
#include "pixmaps/run3.xpm"
#include "pixmaps/run4.xpm"
#include "pixmaps/run5.xpm"
#include "pixmaps/run6.xpm"
#include "pixmaps/run7.xpm"
#include "pixmaps/run8.xpm"
#include "pixmaps/run_bad.xpm"
#include "pixmaps/run_bad1.xpm"
#include "pixmaps/run_bad2.xpm"
#include "pixmaps/run_bad3.xpm"
#include "pixmaps/run_bad4.xpm"
#include "pixmaps/run_bad5.xpm"
#include "pixmaps/run_bad6.xpm"
#include "pixmaps/run_bad7.xpm"
#include "pixmaps/run_bad8.xpm"
#include "pixmaps/run_part.xpm"
#include "pixmaps/run_part1.xpm"
#include "pixmaps/run_part2.xpm"
#include "pixmaps/run_part3.xpm"
#include "pixmaps/run_part4.xpm"
#include "pixmaps/run_part5.xpm"
#include "pixmaps/run_part6.xpm"
#include "pixmaps/run_part7.xpm"
#include "pixmaps/run_part8.xpm"
#include "pixmaps/stop_xpm.xpm"
#include "pixmaps/stop_wait.xpm"
#include "pixmaps/paused.xpm"
#include "pixmaps/complete.xpm"
	char *xml_names[]={
		"waitpix",
		"failedpix",
		"stopwaitpix",
		"runpix",
		"runpix1",
		"runpix2",
		"runpix3",
		"runpix4",
		"runpix5",
		"runpix6",
		"runpix7",
		"runpix8",
		"runbadpix",
		"runbadpix1",
		"runbadpix2",
		"runbadpix3",
		"runbadpix4",
		"runbadpix5",
		"runbadpix6",
		"runbadpix7",
		"runbadpix8",
		"runpartpix",
		"runpartpix1",
		"runpartpix2",
		"runpartpix3",
		"runpartpix4",
		"runpartpix5",
		"runpartpix6",
		"runpartpix7",
		"runpartpix8",
		"completepix",
		"pausedpix"
	};
	char **xpm_table[]={
		wait_xpm,
		stop_xpm,
		stop_wait_xpm,
		run_xpm,
		run1_xpm,
		run2_xpm,
		run3_xpm,
		run4_xpm,
		run5_xpm,
		run6_xpm,
		run7_xpm,
		run8_xpm,
		run_bad_xpm,
		run_bad1_xpm,
		run_bad2_xpm,
		run_bad3_xpm,
		run_bad4_xpm,
		run_bad5_xpm,
		run_bad6_xpm,
		run_bad7_xpm,
		run_bad8_xpm,
		run_part_xpm,
		run_part1_xpm,
		run_part2_xpm,
		run_part3_xpm,
		run_part4_xpm,
		run_part5_xpm,
		run_part6_xpm,
		run_part7_xpm,
		run_part8_xpm,
		complete_xpm,
		paused_xpm
	};
	d4xXmlObject *xmlobj=d4x_xml_find_obj(D4X_THEME_DATA,"queue");
	for (unsigned int i=0;i<sizeof(xpm_table)/sizeof(char*);i++){
		char *file=NULL;
		d4xXmlObject *icon=xmlobj?xmlobj->find_obj(xml_names[i]):NULL;
		d4xXmlField *fld=icon?icon->get_attr("file"):NULL;
		if (fld){
			file=sum_strings(CFG.THEMES_DIR,"/",fld->value.get(),NULL);
		};
		GdkPixbuf *pixbuf;
		if (file && (pixbuf=gdk_pixbuf_new_from_file(file))){
			gdk_pixbuf_render_pixmap_and_mask(pixbuf,
							  &(list_of_downloads_pixmaps[i]),
							  &(list_of_downloads_bitmaps[i]),1);
			gdk_pixbuf_unref(pixbuf);
		}else
			list_of_downloads_pixmaps[i]=make_pixmap_from_xpm(&(list_of_downloads_bitmaps[i]),xpm_table[i]);
		if (file) delete[] file;
	};
	/* we will use these pixmaps many times */
};

void lod_all_redraw(d4xDownloadQueue *q,void *a){
	q->qv.redraw_icons();
};

void lod_theme_changed(){
	for (int i=0;i<PIX_UNKNOWN;i++){
		gdk_pixmap_unref(list_of_downloads_pixmaps[i]);
		gdk_bitmap_unref(list_of_downloads_bitmaps[i]);
		list_of_downloads_pixmaps[i]=NULL;
		list_of_downloads_bitmaps[i]=NULL;
	};
	lod_init_pixmaps();
	d4x_qtree_for_each(lod_all_redraw,NULL);
};

void select_download(GtkWidget *clist, gint row, gint column,
                     GdkEventButton *event, gpointer nothing) {
	d4xQueueView *qv=(d4xQueueView *)nothing;
	update_progress_bar();
	/* commented to avoid wm hangs (e.g. enl-nt)
	 */
//	update_mainwin_title();
	prepare_buttons();
	gtk_statusbar_pop(GTK_STATUSBAR(MainStatusBar),StatusBarContext);
	tDownload *temp=qv->last_selected();
	if (temp)
		gtk_statusbar_push(GTK_STATUSBAR(MainStatusBar),StatusBarContext,temp->info->file.get());
	else
		gtk_statusbar_push(GTK_STATUSBAR(MainStatusBar),StatusBarContext,"");
	if (event && event->type==GDK_2BUTTON_PRESS && event->button==1)
		qv->open_logs();
};

void d4xQueueView::redraw_icons() {
	for (int i=0;i<((GtkCList *)ListOfDownloads)->rows;i++){
		tDownload *temp=(tDownload *)gtk_clist_get_row_data(GTK_CLIST(ListOfDownloads),i);
		set_pixmap(i,temp);
	};
};

tDownload *d4xQueueView::last_selected() {
	GList *select=((GtkCList *)ListOfDownloads)->selection_end;
	if (select) {
		tDownload *temp=(tDownload *)gtk_clist_get_row_data(
		                    GTK_CLIST(ListOfDownloads),GPOINTER_TO_INT(select->data));
		return temp;
	};
	return((tDownload *)NULL);
};

gint d4xQueueView::get_row(tDownload *what){
	return(gtk_clist_find_row_from_data (GTK_CLIST (ListOfDownloads),what));
};

void d4xQueueView::set_desc(gint row,tDownload *what){
	if (what->Description.get()){
		change_data(row,
			    DESCRIPTION_COL,
			    what->Description.get());
	};
};

void d4xQueueView::set_filename(gint row,tDownload *what){
	change_data(row,FILE_COL,what->info->file.get());
};

void d4xQueueView::set_percent(int row,int column,float percent){
	int real_col=prefs.cols[column].enum_index;	
	if (real_col<prefs.cols[NOTHING_COL].enum_index)
		my_gtk_clist_set_progress(GTK_CLIST(ListOfDownloads),row,real_col,percent);
};

void d4xQueueView::change_data(int row,int column,gchar *data) {
	int real_col=prefs.cols[column].enum_index;	
	if (real_col<prefs.cols[NOTHING_COL].enum_index)
		gtk_clist_set_text(GTK_CLIST(ListOfDownloads),row,real_col,data);
};

void d4xQueueView::set_color(tDownload *what,int row){
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

void d4xQueueView::update(tDownload *what) {
	char *URL=what->info->url();
	gint row=get_row(what);
	change_data(row,URL_COL,URL);
	delete[] URL;
	set_desc(row,what);
	set_filename(row,what);
};


void d4xQueueView::get_sizes() {
	if (!ListOfDownloads) return;
	GtkCListColumn *tmp=GTK_CLIST(ListOfDownloads)->column;
	for (int i=0;i<prefs.cols[NOTHING_COL].enum_index;i++) {
		prefs.cols[i].size=int(tmp->width);
		tmp++;
	};
};

void d4xQueueView::print_size(gint row,tDownload *what){
	char data1[MAX_LEN];
	int NICE_DEC_DIGITALS=what->myowner->PAPA->NICE_DEC_DIGITALS;
	if (what->finfo.size>0){
		make_number_nice(data1,what->finfo.size,NICE_DEC_DIGITALS);
		change_data(row,
			    FULL_SIZE_COL,
			    data1);
	};
	if (what->Size.curent>0){
		make_number_nice(data1,what->Size.curent,NICE_DEC_DIGITALS);
		change_data(row,
			    DOWNLOADED_SIZE_COL,
			    data1);
	};
	if (what->finfo.size>0 && what->Size.curent<=what->finfo.size){
		float p=(float(what->Size.curent)*float(100))/float(what->finfo.size);
		set_percent(row,
			    PERCENT_COL,
			    p);
		make_number_nice(data1,what->finfo.size-what->Size.curent,NICE_DEC_DIGITALS);
		change_data(row,
			    REMAIN_SIZE_COL,
			    data1);
	};
};

void d4xQueueView::add(tDownload *what) {
	add_wf(what);
	if (CFG.WITHOUT_FACE) return;
	LoDSortFlag=NOTHING_COL;
	gchar *data[NOTHING_COL+1];
	char *URL=what->info->url();
	for (int i=STATUS_COL;i<=NOTHING_COL;i++)
		data[prefs.cols[i].enum_index]="";
	gint row=gtk_clist_append(GTK_CLIST(ListOfDownloads),data);
	gtk_clist_set_row_data(GTK_CLIST(ListOfDownloads),row,gpointer(what));
	change_data(row,URL_COL,URL);
	set_filename(row,what);
	print_size(row,what);
	set_desc(row,what);

	set_pixmap(row,what);
	set_color(what,row);
	if (what->protect)
	if (row==0) gtk_clist_select_row(GTK_CLIST(ListOfDownloads),0,-1);
	delete[] URL;
};

void d4xQueueView::remove(tDownload *what){
	remove_wf(what);
	if (CFG.WITHOUT_FACE) return;
	gint row=get_row(what);
	gtk_clist_remove(GTK_CLIST(ListOfDownloads),row);
};

void d4xQueueView::set_run_icon(tDownload *what){
	int a=int(what->Percent * 0.09);
	if (a>9) a=9;
	if (a<0) a=0;
	switch (what->Status.curent) {
	case D_QUERYING:{
		set_pixmap(what,PIX_RUN_PART+a);
		break;
	};
	default:
	case D_DOWNLOAD:{
		set_pixmap(what,PIX_RUN+a);
		break;
	};
	case D_DOWNLOAD_BAD:{
		set_pixmap(what,PIX_RUN_BAD+a);
		break;
	};
	};
};

void d4xQueueView::add(tDownload *what,int row) {
	add_wf(what);
	if (CFG.WITHOUT_FACE) return;
	LoDSortFlag=NOTHING_COL;
	gchar *data[NOTHING_COL+1];
	for (int i=STATUS_COL;i<=URL_COL;i++)
		data[i]=(gchar *)NULL;
	gtk_clist_insert(GTK_CLIST(ListOfDownloads),row,data);
	gtk_clist_set_row_data(GTK_CLIST(ListOfDownloads),row,what);
	set_color(what,row);
	char *URL=what->info->url();
	change_data(row,URL_COL,URL);
	delete[] URL;
	set_pixmap(row,what);
	print_size(row,what);
	set_filename(row,what);
	set_desc(row,what);
};

void d4xQueueView::move_download_up(int row){
	gtk_clist_swap_rows(GTK_CLIST(ListOfDownloads),row,row-1);
	tDownload *what=get_download(row-1);
	tDownload *what2=get_download(row);
	if (what->owner()==DL_WAIT && what2->owner()==DL_WAIT)
		D4X_QUEUE->forward(what);
};

void d4xQueueView::move_download_down(int row){
	gtk_clist_swap_rows(GTK_CLIST(ListOfDownloads),row,row+1);
	tDownload *what=get_download(row+1);
	tDownload *what2=get_download(row);
	if (what->owner()==DL_WAIT && what2->owner()==DL_WAIT)
		D4X_QUEUE->backward(what);
};
static gint compare_nodes1(gconstpointer a,gconstpointer b){
    gint a1=GPOINTER_TO_INT(a);
    gint b1=GPOINTER_TO_INT(b);
    if (a1>b1) return 1;
    if (a1==b1) return 0;
    return -1;
};

static gint compare_nodes2(gconstpointer a,gconstpointer b){
    gint a1=GPOINTER_TO_INT(a);
    gint bb=GPOINTER_TO_INT(b);
    if (a1>bb) return -1;
    if (a1==bb) return 0;
    return 1;
};

int d4xQueueView::move_selected_up(){
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

int d4xQueueView::move_selected_down(){
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


void d4xQueueView::move_up(){
	freeze();
	move_selected_up();
	unfreeze();
};

void d4xQueueView::move_down(){
	freeze();
	move_selected_down();
	unfreeze();
};

void d4xQueueView::move_selected_home(){
	freeze();
	while (move_selected_up());
	unfreeze();
};

void d4xQueueView::move_selected_end(){
	freeze();
	while (move_selected_down());
	unfreeze();
};

tDownload *d4xQueueView::get_download(int row) {
	tDownload *what=(tDownload *)gtk_clist_get_row_data(GTK_CLIST(ListOfDownloads),row);
	return what;
};

void d4xQueueView::freeze() {
	gtk_clist_freeze(GTK_CLIST(ListOfDownloads));
};

void d4xQueueView::unfreeze() {
	gtk_clist_thaw(GTK_CLIST(ListOfDownloads));
	gtk_widget_show(ListOfDownloads);
};

void d4xQueueView::real_select(int type,char *wildcard){
	if (wildcard==NULL || *wildcard==0) return;
	if (type){
		for (int i=0;i<GTK_CLIST(ListOfDownloads)->rows;i++){
			tDownload *dwn=get_download(i);
			if (dwn && dwn->info->file.get() &&
			    check_mask2(dwn->info->file.get(),wildcard))
				gtk_clist_unselect_row(GTK_CLIST(ListOfDownloads),i,-1);
		};
	}else{
		for (int i=0;i<GTK_CLIST(ListOfDownloads)->rows;i++){
			tDownload *dwn=get_download(i);
			if (dwn && dwn->info->file.get() &&
			    check_mask2(dwn->info->file.get(),wildcard))
				gtk_clist_select_row(GTK_CLIST(ListOfDownloads),i,-1);
		};
	};
};

static void _select_ok_(GtkButton *button,d4xQueueView *qv){
	char *wildcard=text_from_combo(LoDSelectEntry);
	qv->real_select(LoDSelectType,wildcard);
	gtk_widget_destroy(LoDSelectWindow);
	LoDSelectWindow=(GtkWidget*)NULL;
};

static void _select_cancel_(GtkButton *button,gpointer unused){
	gtk_widget_destroy(LoDSelectWindow);
	LoDSelectWindow=(GtkWidget*)NULL;
};

static gint _select_delete_(GtkWidget *window,GdkEvent *event,
			    gpointer unused) {
	gtk_widget_destroy(LoDSelectWindow);
	LoDSelectWindow=(GtkWidget*)NULL;
	return(TRUE);
};

/*
  void list_of_downloads_select()

  this routine bring up a dialog for selecting
  items in queue of downloads by wildcart
*/

void d4xQueueView::init_select_window(int type){
	if (LoDSelectWindow){
		gdk_window_show(LoDSelectWindow->window);
		return;
	};
	LoDSelectType=type;
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
			   GTK_SIGNAL_FUNC(_select_ok_),this);
	gtk_signal_connect(GTK_OBJECT(cancel_button),"clicked",
			   GTK_SIGNAL_FUNC(_select_cancel_),NULL);
	gtk_signal_connect(GTK_OBJECT(LoDSelectWindow),"delete_event",
			   GTK_SIGNAL_FUNC(_select_delete_),NULL);
	gtk_signal_connect(GTK_OBJECT(LoDSelectEntry), "activate",
			   GTK_SIGNAL_FUNC (_select_ok_), this);
	d4x_eschandler_init(LoDSelectWindow,this);
	GtkWidget *frame=(GtkWidget*)NULL;
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

int list_event_callback(GtkWidget *widget,GdkEvent *event,d4xQueueView *qv) {
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
			qv->open_logs();
			return TRUE;
		};
		case GDK_KP_Add:{
			qv->init_select_window();
			return TRUE;
		};
		case GDK_KP_Subtract:{
			qv->init_select_window(1);
			return TRUE;
		};
		};
		if (kevent->state & GDK_SHIFT_MASK) {
			switch (kevent->keyval) {
			case GDK_KP_Up:
			case GDK_Up:{
				qv->move_up();
				return TRUE;
			};
			case GDK_KP_Down:
			case GDK_Down:{
				qv->move_down();
				return TRUE;
			};
			case GDK_KP_Page_Up:
			case GDK_Page_Up:{
				qv->move_selected_home();
				return TRUE;
			};
			case GDK_KP_Page_Down:
			case GDK_Page_Down:{
				qv->move_selected_end();
				return TRUE;
			};
			default:
				break;
			};
		};
	};
	return FALSE;
};

void d4xQueueView::rebuild_wait(){
	if (D4X_QUEUE->count(DL_WAIT)==0) return;
	int i=0;
	tDList *dlist=new tDList(DL_WAIT);
	dlist->init_pixmap(PIX_WAIT);
	dlist->init(0);
	tDownload *tmp=(tDownload *)gtk_clist_get_row_data(GTK_CLIST(ListOfDownloads),i);
	while(tmp){
		if (tmp->owner()==DL_WAIT){
			D4X_QUEUE->del(tmp);
			dlist->insert(tmp);
			if (D4X_QUEUE->count(DL_WAIT)==0)
				break;
		};
		i+=1;
		tmp=(tDownload *)gtk_clist_get_row_data(GTK_CLIST(ListOfDownloads),i);
	};
	D4X_QUEUE->replace_list(dlist,DL_WAIT);
	if (CFG.WITHOUT_FACE==0)
		D4X_QVT->update(D4X_QUEUE);
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

static int _cmp_whole_status_(GtkCList *clist,
			      gconstpointer ptr1,
			      gconstpointer ptr2){
	GtkCListRow *row1=(GtkCListRow *)ptr1;
	GtkCListRow *row2=(GtkCListRow *)ptr2;
	if (row1->data && row2->data){
		int smap[]={0,10,6,8,9,11,7,0};
		tDownload *d1=(tDownload *)(row1->data);
		tDownload *d2=(tDownload *)(row2->data);
		return(smap[d2->owner()]-smap[d1->owner()]);
	};
	if (row1->data) return(1);
	if (row2->data) return(0);
	return(-1);
};

CLIST_CMP_I(_cmp_whole_status_);

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

void d4xQueueView::sort(int how){
	int count=D4X_QUEUE->count(DL_RUN);
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
			rebuild_wait();
			LoDSortFlag=LoDSortFlag==FILE_COL?-FILE_COL:FILE_COL;
			return;
		};
		cmp_func=_cmp_byfile;
	};
	case STATUS_COL:
		if (whole_list){
			gtk_clist_set_compare_func(GTK_CLIST(ListOfDownloads),
						   LoDSortFlag==PERCENT_COL?_cmp_whole_status_:_cmp_whole_status_i_);
			gtk_clist_sort(GTK_CLIST(ListOfDownloads));
			rebuild_wait();
			LoDSortFlag=LoDSortFlag==PERCENT_COL?-PERCENT_COL:PERCENT_COL;
			return;
		};
		cmp_func=NULL;
		break;
	case PERCENT_COL:
		if (whole_list){
			gtk_clist_set_compare_func(GTK_CLIST(ListOfDownloads),
						   LoDSortFlag==PERCENT_COL?_cmp_whole_percent_:_cmp_whole_percent_i_);
			gtk_clist_sort(GTK_CLIST(ListOfDownloads));
			rebuild_wait();
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
			rebuild_wait();
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
	freeze();
	tDownload *tmp=D4X_QUEUE->last(DL_RUN);
	tDownload *cur=(tDownload*)(tmp->next);
	int changed;
	do{
		changed=0;
		gint row1=get_row(tmp);
		while (cur){
			gint row=get_row(cur);
			if ((row<row1 && cmp_func(cur,tmp)<0) ||
			    (row>row1 && cmp_func(cur,tmp)>0)){
				swap(tmp,cur);
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
	unfreeze();
	LoDSortFlag=NOTHING_COL;
};

static void list_of_downloads_sort(GtkWidget *widget,d4xQueueView *qv){
	int how=GPOINTER_TO_INT(gtk_object_get_user_data(GTK_OBJECT(widget)));
	qv->sort(how);
};

void d4xQueueView::set_column_justification (int col, GtkJustification justify){
	if (prefs.cols[col].enum_index<prefs.cols[NOTHING_COL].enum_index)
		gtk_clist_set_column_justification (GTK_CLIST(ListOfDownloads),
						    prefs.cols[col].enum_index,
						    justify);
};

GtkWidget *d4xQueueView::get_column_widget(int col){
	if (prefs.cols[col].enum_index<prefs.cols[NOTHING_COL].enum_index)
		return (GTK_CLIST(ListOfDownloads)->column[prefs.cols[col].enum_index].button);
	return((GtkWidget *)NULL);
};

void d4xQueueView::init_sort_buttons(){
	GtkWidget *button=(GtkWidget *)NULL;
	int a[]={PERCENT_COL,
		 FILE_COL,
		 STATUS_COL,
		 DOWNLOADED_SIZE_COL,
		 SPEED_COL,
		 REMAIN_SIZE_COL,
		 FULL_SIZE_COL
	};
	for (unsigned int i=0;i<sizeof(a)/sizeof(int);i++){
		button=get_column_widget(a[i]);
		if (button){
			gtk_object_set_user_data(GTK_OBJECT(button),
						 GINT_TO_POINTER(a[i]));
			gtk_signal_connect(GTK_OBJECT(button),
					   "clicked",
					   GTK_SIGNAL_FUNC(list_of_downloads_sort),
					   this);
		};
	};
	
};

static GtkTargetEntry ltarget_table[] = {
	{ "d4x/dpointer",     0, 0 }
};

static guint ln_targets = sizeof(ltarget_table) / sizeof(ltarget_table[0]);

static void source_drag_data_get  (GtkWidget          *widget,
				   GdkDragContext     *context,
				   GtkSelectionData   *selection_data,
				   guint               info,
				   guint               time,
				   gpointer            data){
	if (info == 0 && GTK_CLIST(widget)->selection)
		gtk_selection_data_set (selection_data,
					selection_data->target,
					8, (guchar*)(&widget) ,sizeof(GtkWidget*));
	else
		gtk_drag_finish (context, TRUE, FALSE, time);
}


/*
static void source_drag_begin(GtkWidget *widget,
			      GdkDragContext *context){
	if (GTK_CLIST(widget)->selection==NULL){
		gdk_drag_status (context, context->suggested_action, time(NULL)); 
//		gtk_drag_finish (context, FALSE, FALSE, time(NULL));
	};
};
*/
static GdkPixmap *d4x_dnd_icon_pix=NULL;
static GdkBitmap *d4x_dnd_icon_bit=NULL;

void d4xQueueView::init() {
	char *RealListTitles[NOTHING_COL+1];
	for (int i=0;i<prefs.cols[NOTHING_COL].enum_index;i++)
		RealListTitles[i]=_(ListTitles[prefs.cols[i].type]);
	ListOfDownloads = my_gtk_clist_new_with_titles( prefs.cols[NOTHING_COL].enum_index, RealListTitles);
#include "pixmaps/dnd.xpm"
	if (!d4x_dnd_icon_pix)
		d4x_dnd_icon_pix=gdk_pixmap_colormap_create_from_xpm_d(NULL,
								       gtk_widget_get_colormap (MainWindow),
								       &d4x_dnd_icon_bit,
								       NULL,
								       dnd_xpm);
	gtk_clist_set_use_drag_icons(GTK_CLIST(ListOfDownloads),FALSE);
	gtk_drag_source_set (ListOfDownloads, GdkModifierType(GDK_BUTTON1_MASK | GDK_BUTTON3_MASK),
				ltarget_table, ln_targets,
				GdkDragAction(GDK_ACTION_COPY | GDK_ACTION_MOVE));
	gtk_drag_source_set_icon(ListOfDownloads,gtk_widget_get_colormap (MainWindow),
				 d4x_dnd_icon_pix,
				 d4x_dnd_icon_bit);
// No need to unref cos it will be freed at exiting
//	gdk_pixmap_unref (d4x_dnd_icon_pix);
//	gdk_pixmap_unref (d4x_dnd_icon_bit);
	gtk_signal_connect (GTK_OBJECT (ListOfDownloads), "drag_data_get",
			    GTK_SIGNAL_FUNC (source_drag_data_get), NULL);
	gtk_object_ref(GTK_OBJECT(ListOfDownloads));
	gtk_signal_connect(GTK_OBJECT(ListOfDownloads), "select_row",
	                   GTK_SIGNAL_FUNC(select_download),this);
	gtk_signal_connect(GTK_OBJECT(ListOfDownloads), "event",
	                   GTK_SIGNAL_FUNC(list_event_callback),this);

	gtk_clist_set_row_height(GTK_CLIST(ListOfDownloads),16);
	for(int i=STATUS_COL;i<prefs.cols[NOTHING_COL].enum_index;i++)
		gtk_clist_set_column_width (GTK_CLIST(ListOfDownloads),prefs.cols[prefs.cols[i].type].enum_index,gint(prefs.cols[i].size));
	gtk_clist_set_shadow_type (GTK_CLIST(ListOfDownloads), GTK_SHADOW_IN);
	gtk_clist_set_selection_mode(GTK_CLIST(ListOfDownloads),GTK_SELECTION_EXTENDED);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (ContainerForCList),
	                                GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);

	gtk_clist_set_hadjustment(GTK_CLIST(ListOfDownloads),(GtkAdjustment *)NULL);
	gtk_clist_set_vadjustment(GTK_CLIST(ListOfDownloads),(GtkAdjustment *)NULL);

	set_column_justification (FULL_SIZE_COL, GTK_JUSTIFY_RIGHT);
	set_column_justification (PERCENT_COL, GTK_JUSTIFY_CENTER);
	set_column_justification (DOWNLOADED_SIZE_COL, GTK_JUSTIFY_RIGHT);
	set_column_justification (REMAIN_SIZE_COL, GTK_JUSTIFY_RIGHT);
	set_column_justification (TREAT_COL, GTK_JUSTIFY_RIGHT);
	set_column_justification (SPEED_COL, GTK_JUSTIFY_RIGHT);
	set_column_justification (ELAPSED_TIME_COL, GTK_JUSTIFY_RIGHT);
	set_column_justification (TIME_COL, GTK_JUSTIFY_RIGHT);
	set_column_justification (PAUSE_COL, GTK_JUSTIFY_RIGHT);
	gtk_clist_set_column_auto_resize(GTK_CLIST(ListOfDownloads),prefs.cols[URL_COL].enum_index,TRUE);

	init_sort_buttons();
};

void d4xQueueView::set_pixmap(gint row, tDownload *what){
	switch (what->owner()) {
	default:
	case DL_WAIT:{
		set_pixmap(row,PIX_WAIT);
		break;
	};
	case DL_STOP:{
		set_pixmap(row,PIX_STOP);
		break;
	};
	case DL_RUN:{
		what->update_trigers();
		set_run_icon(what);
		break;
	};
	case DL_PAUSE:{
		set_pixmap(row,PIX_PAUSE);
		break;
	};
	case DL_COMPLETE:{
		set_pixmap(row,PIX_COMPLETE);
	};
	};
};

void d4xQueueView::set_pixmap(gint row,int type){
	if (type>=PIX_UNKNOWN || row<0) return;
	if (prefs.cols[STATUS_COL].enum_index<prefs.cols[NOTHING_COL].enum_index){
/*
		GdkPixmap *pixmap;
		GdkBitmap *bitmap;
		gtk_clist_get_pixmap(GTK_CLIST (ListOfDownloads), row,
				     prefs.cols[STATUS_COL].enum_index,
				     &pixmap,&bitmap);
		if (pixmap!=list_of_downloads_pixmaps[type])
*/
			gtk_clist_set_pixmap (GTK_CLIST (ListOfDownloads), row,
					      prefs.cols[STATUS_COL].enum_index,
					      list_of_downloads_pixmaps[type],
					      list_of_downloads_bitmaps[type]);
	};
};

void d4xQueueView::set_pixmap(tDownload *dwn,int type){
	if (type>=PIX_UNKNOWN) return;
	set_pixmap(get_row(dwn),type);
};

void d4xQueueView::unselect_all(){
	if (GTK_CLIST(ListOfDownloads)->rows)
		gtk_clist_unselect_all(GTK_CLIST(ListOfDownloads));
};

void d4xQueueView::select_all(){
	if (GTK_CLIST(ListOfDownloads)->rows)
		gtk_clist_select_all(GTK_CLIST(ListOfDownloads));
};

void d4xQueueView::invert_selection(){
	if (GTK_CLIST(ListOfDownloads)->rows==0) return;
	freeze();
	GList *select=g_list_copy(((GtkCList *)ListOfDownloads)->selection);
	gtk_clist_select_all(GTK_CLIST(ListOfDownloads));
	while(select!=NULL){
		gtk_clist_unselect_row(GTK_CLIST(ListOfDownloads),GPOINTER_TO_INT(select->data),-1);
		select=g_list_remove_link(select,select);
	};
	unfreeze();
};

void d4xQueueView::select(tDownload *dwn){
	gint row=get_row(dwn);
	unselect_all();
	gtk_clist_select_row(GTK_CLIST(ListOfDownloads),row,-1);
};

int d4xQueueView::sel(){
	return(GTK_CLIST(ListOfDownloads)->selection==NULL?1:0);
};

int d4xQueueView::rows(){
	return(GTK_CLIST(ListOfDownloads)->rows);
};

void d4xQueueView::swap(tDownload *a,tDownload *b){
	gint rowa=get_row(a);
	gint rowb=get_row(b);
	gtk_clist_swap_rows(GTK_CLIST(ListOfDownloads),rowa,rowb);
};

/* Various additional functions
 */


void d4xQueueView::continue_opening_logs(){
	GList *select=GTK_CLIST(ListOfDownloads)->selection;
	while (select) {
		tDownload *temp=(tDownload *)gtk_clist_get_row_data(
		                    GTK_CLIST(ListOfDownloads),
				    GPOINTER_TO_INT(select->data));
		if (temp && (temp->LOG==NULL || temp->LOG->Window==NULL))
			log_window_init(temp);
		select=select->next;
	};
};

static void _continue_opening_logs_(GtkWidget *widget,d4xQueueView *qv){
	CFG.CONFIRM_OPENING_MANY=!(GTK_TOGGLE_BUTTON(AskOpening->check)->active);
	qv->continue_opening_logs();
	if (AskOpening)
		AskOpening->done();
};

void d4xQueueView::open_logs() {
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
						   this);
			AskOpening->set_modal(MainWindow);
			break;
		};
	};
};

void d4xQueueView::set_shift(float shift){
	GtkAdjustment *adj=gtk_clist_get_vadjustment(GTK_CLIST(ListOfDownloads));
	if (adj->upper>shift){
		adj->value=shift;
		gtk_signal_emit_by_name (GTK_OBJECT (adj), "value_changed");
	};
};

void d4xQueueView::move_to(tDownload *dwn){
	gint row=get_row(dwn);
	gtk_clist_moveto(GTK_CLIST(ListOfDownloads),row,0,0,0);
};

void d4xQueueView::get_adj(){
	GtkAdjustment *adj=gtk_clist_get_vadjustment(GTK_CLIST(ListOfDownloads));
	if (adj)
		CFG.CLIST_SHIFT=adj->value;
};

// manipulating with downloads

void d4xQueueView::stop_downloads(){
	GList *select=GTK_CLIST(ListOfDownloads)->selection;
	int olda=aa.set_auto_run(1);
	while (select) {
		tDownload *temp=(tDownload *)gtk_clist_get_row_data(
			GTK_CLIST(ListOfDownloads),GPOINTER_TO_INT(select->data));
		aa.stop_download(temp);
		select=select->next;
	};
	aa.set_auto_run(olda);
};

void d4xQueueView::delete_downloads(int flag){
	GList *select=((GtkCList *)ListOfDownloads)->selection;
	freeze();
	while (select) {
		GList *next=select->next;
		gint row=GPOINTER_TO_INT(select->data);
		gtk_clist_unselect_row(GTK_CLIST(ListOfDownloads),row,-1);
		tDownload *temp=(tDownload *)gtk_clist_get_row_data(
			GTK_CLIST(ListOfDownloads),row);
		aa.delete_download(temp,flag);
//		select=((GtkCList *)ListOfDownloads)->selection;
		select=next;
	};
	gtk_clist_unselect_all(GTK_CLIST(ListOfDownloads));
	unfreeze();
};

void d4xQueueView::continue_downloads(int from_begin){
	GList *select=((GtkCList *)ListOfDownloads)->selection;
	while (select) {
		tDownload *temp=(tDownload *)gtk_clist_get_row_data(
		                    GTK_CLIST(ListOfDownloads),GPOINTER_TO_INT(select->data));
		if (from_begin)
			temp->restart_from_begin=1;
		aa.continue_download(temp);
		select=select->next;
	};
};

void d4xQueueView::inv_protect_flag(){
	GList *select=((GtkCList *)ListOfDownloads)->selection;
	while (select) {
		int row=GPOINTER_TO_INT(select->data);
		tDownload *temp=(tDownload *)gtk_clist_get_row_data(
			GTK_CLIST(ListOfDownloads),row);
		temp->protect=!temp->protect;
		set_color(temp,row);
		select=select->next;
	};
};

void d4xQueueView::inherit_settings(d4xQueueView *papa){
	for (int i=STATUS_COL;i<=NOTHING_COL;i++){
		prefs.cols[i].type=papa->prefs.cols[i].type;
		prefs.cols[i].size=papa->prefs.cols[i].size;
		prefs.cols[i].enum_index=papa->prefs.cols[i].enum_index;
	};
};

void d4xQueueView::save_to_config(int fd){
	if (CFG.WITHOUT_FACE==0)
		get_sizes();
	f_wstr_lf(fd,"QV:");
	for (int i=STATUS_COL;i<=NOTHING_COL;i++){
		char data[10];
		sprintf(data,"%i",prefs.cols[i].type);
		f_wstr(fd,data);
		if (i!=NOTHING_COL)
			f_wstr(fd,",");
		else
			f_wstr_lf(fd,"");
	};
	for (int i=STATUS_COL;i<=NOTHING_COL;i++){
		char data[10];
		sprintf(data,"%i",prefs.cols[i].enum_index);
		f_wstr(fd,data);
		if (i!=NOTHING_COL)
			f_wstr(fd,",");
		else
			f_wstr_lf(fd,"");
	};		
	for (int i=STATUS_COL;i<=NOTHING_COL;i++){
		char data[10];
		sprintf(data,"%i",prefs.cols[i].size);
		f_wstr(fd,data);
		if (i!=NOTHING_COL)
			f_wstr(fd,",");
		else
			f_wstr_lf(fd,"");
	};		
};

int d4xQueueView::load_from_config(int fd){
	char data[1000];
	f_rstr(fd,data,1000);
	char *tmp=data;
	for (int i=STATUS_COL;i<=NOTHING_COL;i++){
		if (tmp) sscanf(tmp,"%i",&prefs.cols[i].type);
		else prefs.cols[i].type=i;
		tmp=tmp?index(tmp,','):NULL;
		if (tmp) tmp++;
	};
	f_rstr(fd,data,1000);
	tmp=data;
	for (int i=STATUS_COL;i<=NOTHING_COL;i++){
		if (tmp) sscanf(tmp,"%i",&prefs.cols[i].enum_index);
		else prefs.cols[i].enum_index=i;
		tmp=tmp?index(tmp,','):NULL;
		if (tmp) tmp++;
	};
	f_rstr(fd,data,1000);
	tmp=data;
	for (int i=STATUS_COL;i<=NOTHING_COL;i++){
		if (tmp) sscanf(tmp,"%i",&prefs.cols[i].size);
		else prefs.cols[i].size=10;
		tmp=tmp?index(tmp,','):NULL;
		if (tmp) tmp++;
	};
	return(0);
};
	
