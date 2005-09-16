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
#include "colors.h"
#include <gdk-pixbuf/gdk-pixbuf.h>
#include "themes.h"

using namespace d4x;
tConfirmedDialog *AskOpening=(tConfirmedDialog *)NULL;

//GdkPixbuf *list_of_downloads_pixbufs[LPE_UNKNOWN];


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

#define _INIT_QVP_(arg,arg1) cols[arg].type=arg;cols[arg].size=arg1;cols[arg].visible=1;

d4xQVPrefs::d4xQVPrefs(){
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
		g_object_unref(G_OBJECT(ListOfDownloads));
};

void d4xQueueView::remove_wf(tDownload *what){
	d4xWFNode *node=(d4xWFNode *)(what->WFP);
	if (last_selected==what) last_selected=NULL;
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

struct D4XCellRendererProgress{
	GtkCellRenderer parent;
	gfloat percent;
	tDownload *dwn;
};

struct D4XCellRendererProgressClass{
	GtkCellRendererClass parent_class;
};

GtkType          d4x_cell_renderer_progress_get_type (void);
GtkCellRenderer *d4x_cell_renderer_progress_new      (void);

static void d4x_cell_renderer_progress_init (D4XCellRendererProgress *cellpixbuf){
};

static void
d4x_cell_renderer_progress_get_size (GtkCellRenderer *cell,
				     GtkWidget       *widget,
				     GdkRectangle    *cell_area,
				     gint            *x_offset,
				     gint            *y_offset,
				     gint            *width,
				     gint            *height){
	if (x_offset) *x_offset = 0;
	if (y_offset) *y_offset = 0;
	if (cell_area){
		if (width) *width = cell_area->width-1;
		if (height) *height = cell_area->height-1;
	};
};

static void
d4x_cell_renderer_progress_get_property (GObject        *object,
					 guint           param_id,
					 GValue         *value,
					 GParamSpec     *pspec){
//	G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
};

static void
d4x_cell_renderer_progress_set_property (GObject      *object,
					 guint         param_id,
					 const GValue *value,
					 GParamSpec   *pspec){
	D4XCellRendererProgress *renderer = (D4XCellRendererProgress *)object;
	switch (param_id){
	case 1:
		renderer->percent=g_value_get_float(value);
		break;
	case 2:
		renderer->dwn=(tDownload *)g_value_get_pointer(value);
		break;
	default:
//		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
		break;
	};
};

static void
d4x_cell_renderer_progress_render (GtkCellRenderer    *cell,
				   GdkWindow          *window,
				   GtkWidget          *widget,
				   GdkRectangle       *background_area,
				   GdkRectangle       *cell_area,
				   GdkRectangle       *expose_area,
				   GtkCellRendererState flags){
	char tmpc[100];
	float p=((D4XCellRendererProgress*)cell)->percent;

	if (p>99.0 && p<100.0)
		sprintf(tmpc,"%.1f",p);
	else
		sprintf(tmpc,"%.0f",p);
	PangoRectangle rect;
	PangoLayout *layout=gtk_widget_create_pango_layout (widget, tmpc);
	pango_layout_get_pixel_extents (layout, NULL, &rect);
	gint x=0,y=0;
	if (rect.width<cell_area->width)
		x=(cell_area->width-rect.width)/2;
	if (rect.height<cell_area->height)
		y=(cell_area->height-rect.height)/2;
	pango_layout_set_width (layout, -1);
	GtkStyle *style=gtk_widget_get_style(widget);
	if (CFG.PROGRESS_MODE==1 || CFG.PROGRESS_MODE==2){
		gtk_paint_box(style,window,
			      GTK_STATE_NORMAL,GTK_SHADOW_NONE,
			      cell_area,
			      widget,
			      "trough",
			      cell_area->x,cell_area->y,
			      cell_area->width,cell_area->height);
	};
	switch (CFG.PROGRESS_MODE){
	case 2:{
		if (p<=0) break;
		tDownload *temp=((D4XCellRendererProgress*)cell)->dwn;;
		if (temp && temp->segments && temp->finfo.size>0){
			temp->segments->lock_public();
			tSegment *tmp=temp->segments->get_first();
			while(tmp){
				gint start=int(float(tmp->begin)*float(cell_area->width)/float(temp->finfo.size));
				gint end=int(float(tmp->end)*float(cell_area->width)/float(temp->finfo.size));
				if (end-start>=2) //most themes is buggy to draw boxes with width less than 2 pixels!!!
					gtk_paint_box(style,window,
						      GTK_STATE_PRELIGHT,GTK_SHADOW_OUT,
						      cell_area,
						      widget,
						      "bar",
						      cell_area->x+start,cell_area->y,
						      end-start,cell_area->height);
				tmp=tmp->next;
			};
			temp->segments->unlock_public();
			break;
		};
	};
	case 1:
		if (p<=0) break;
		gtk_paint_box(style,window,
			      GTK_STATE_PRELIGHT,GTK_SHADOW_OUT,
			      cell_area,
			      widget,
			      "bar",
			      cell_area->x,cell_area->y,
			      int((cell_area->width*p)/100),cell_area->height);
	default:
		break;
	};
	gtk_paint_layout (widget->style,
			  window,
			  GTK_STATE_NORMAL,
			  TRUE,
			  cell_area,
			  widget,
			  "cellrenderertext",
			  cell_area->x +x + cell->xpad,
			  cell_area->y +y + cell->ypad,
			  layout);
	g_object_unref(G_OBJECT (layout));
//	printf("render: %f\n",);
};


static void d4x_cell_renderer_progress_class_init (D4XCellRendererProgressClass *klass){
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GtkCellRendererClass *cell_class = GTK_CELL_RENDERER_CLASS (klass);

	cell_class->get_size = d4x_cell_renderer_progress_get_size;
	cell_class->render = d4x_cell_renderer_progress_render;
	object_class->set_property = d4x_cell_renderer_progress_set_property;
	object_class->get_property = d4x_cell_renderer_progress_get_property;

	g_object_class_install_property (object_class, 1,
					 g_param_spec_float ("percent",
							     _("Percent"),
							     "Percentage to render",
							     0,100,0,
							     (GParamFlags)(G_PARAM_READABLE | G_PARAM_WRITABLE)));
	g_object_class_install_property (object_class, 2,
					 g_param_spec_pointer ("download",
							       _("Download"),
							       "Link to tDownload",
							       (GParamFlags)(G_PARAM_READABLE | G_PARAM_WRITABLE)));
};

GtkType d4x_cell_renderer_progress_get_type (void){
	static GtkType cell_progress_type = 0;

	if (!cell_progress_type)
	{
		static const GTypeInfo cell_progress_info =
		{
			sizeof (D4XCellRendererProgressClass),
			NULL,		/* base_init */
			NULL,		/* base_finalize */
			(GClassInitFunc) d4x_cell_renderer_progress_class_init,
			NULL,		/* class_finalize */
			NULL,		/* class_data */
			sizeof (D4XCellRendererProgress),
			0,              /* n_preallocs */
			(GInstanceInitFunc) d4x_cell_renderer_progress_init,
		};

		cell_progress_type = g_type_register_static (GTK_TYPE_CELL_RENDERER,
							     "D4XCellRendererProgress",
							     &cell_progress_info,
							     GTypeFlags(0));
	}

	return cell_progress_type;
}


GtkCellRenderer *d4x_cell_renderer_progress_new (void){
  return GTK_CELL_RENDERER (g_object_new (d4x_cell_renderer_progress_get_type (),NULL));
}

/***************************************************************/
gint lod_get_height() {
	if (!MAIN_PANED) return FALSE;
	CFG.WINDOW_CLIST_HEIGHT=GTK_PANED(MAIN_PANED)->child1_size;
	if (!MAIN_PANED1) return FALSE;
	CFG.WINDOW_TREE_WIDTH=GTK_PANED(MAIN_PANED1)->child1_size;
	return FALSE;
};

void lod_set_height() {
//	gtk_widget_set_size_request(ListOfDownloads,-1,gint(CFG.WINDOW_CLIST_HEIGHT));
	gtk_paned_set_position(GTK_PANED(MAIN_PANED),gint(CFG.WINDOW_CLIST_HEIGHT));
	gtk_paned_set_position(GTK_PANED(MAIN_PANED1),gint(CFG.WINDOW_TREE_WIDTH));
};


gboolean select_download(GtkTreeSelection *sel, GtkTreeModel *model,GtkTreePath *path,
			 gboolean is_sel, gpointer data){
	d4xQueueView *qv=(d4xQueueView *)data;
	gtk_statusbar_pop(GTK_STATUSBAR(MainStatusBar),StatusBarContext);
	GtkTreeIter iter;
	gtk_tree_model_get_iter(model,&iter,path);
	tDownload *tmp=qv->get_download(&iter);
	if (!is_sel){
		qv->last_selected=tmp;
		std::string rfile(hexed_string(tmp->info.file));
		gtk_statusbar_push(GTK_STATUSBAR(MainStatusBar),
				   StatusBarContext,
				   rfile.c_str());
	}else{
		if (qv->last_selected==tmp){
			qv->last_selected=NULL;
			gtk_statusbar_push(GTK_STATUSBAR(MainStatusBar),StatusBarContext,"");
		};
	};
	prepare_buttons();
	update_progress_bar();
// commented out to avoid WM hangs i.e. enlightenment
//	update_mainwin_title();
//	if (event && event->type==GDK_2BUTTON_PRESS && event->button==1)
	return(TRUE);
};

gboolean _lod_redraw_icons_(GtkTreeModel *model, GtkTreePath *path,
			    GtkTreeIter *iter, gpointer data){
	d4xQueueView *qv=(d4xQueueView *)data;
	qv->redraw_pixmap(iter);
	return(FALSE);
};

void d4xQueueView::redraw_pixmap(GtkTreeIter *iter){
	//nothing
	set_pixmap(get_download(iter));
};

void d4xQueueView::redraw_icons() {
	gtk_tree_model_foreach(GTK_TREE_MODEL(list_store),_lod_redraw_icons_,this);
};

void d4xQueueView::set_desc(tDownload *what){
	if (what->Description.get()){
		change_data(what->list_iter,
			    DESCRIPTION_COL,
			    what->Description.get());
	};
};

void d4xQueueView::set_filename(tDownload *what){
	const char *file_utf=what->info.file.c_str();
	if (g_utf8_validate(file_utf,-1,NULL)){
		change_data(what->list_iter,FILE_COL,file_utf);
	}else{
		char *file_rutf=g_convert_with_fallback(file_utf,-1,"UTF-8","ISO8859-1",NULL,NULL,NULL,NULL);
		change_data(what->list_iter,FILE_COL,file_rutf);
		g_free(file_rutf);
	};
};

void d4xQueueView::set_percent(GtkTreeIter *iter,float percent){
	if (percent>100) percent=100;
	if (percent<0) percent=0;
	gtk_list_store_set (list_store, iter,
			    PERCENT_COL,percent,
			    -1);
};

void d4xQueueView::change_data(GtkTreeIter *iter,int column,const gchar *data) {
	gtk_list_store_set (list_store, iter,
			    column,data,
			    -1);
};

void d4xQueueView::set_color(tDownload *what){
	//FIXME: GTK2
	gtk_list_store_set (list_store, what->list_iter,
			    NOTHING_COL+1,gboolean(what->protect),
			    -1);
	
};

void d4xQueueView::update(tDownload *what) {
	change_data(what->list_iter,URL_COL,std::string(what->info).c_str());
	set_desc(what);
	set_filename(what);
};


void d4xQueueView::get_sizes() {
	if (!ListOfDownloads) return;
	GList *tmp=gtk_tree_view_get_columns(GTK_TREE_VIEW(ListOfDownloads));
	GList *a=tmp;
	int i=0;
	while (a){
		GtkTreeViewColumn *col=GTK_TREE_VIEW_COLUMN(a->data);
		int size=gtk_tree_view_column_get_width(col);
		if (size>0) //check for columns which never be drawn
			prefs.cols[i].size=size;
		gpointer p=g_object_get_data (G_OBJECT (col),
					      "d4x_col_num");
		prefs.cols[i].type=GPOINTER_TO_INT(p);
		prefs.cols[i++].visible=gtk_tree_view_column_get_visible(col);
		a=a->next;
	};
	g_list_free(tmp);
};

void d4xQueueView::print_size(tDownload *what){
	char data1[MAX_LEN];
	int NICE_DEC_DIGITALS=what->myowner->PAPA->NICE_DEC_DIGITALS;
	if (what->finfo.size>0){
		make_number_nice(data1,what->finfo.size,NICE_DEC_DIGITALS);
		change_data(what->list_iter,
			    FULL_SIZE_COL,
			    data1);
	};
	if (what->Size.curent>0){
		make_number_nice(data1,what->Size.curent,NICE_DEC_DIGITALS);
		change_data(what->list_iter,
			    DOWNLOADED_SIZE_COL,
			    data1);
	};
	if (what->finfo.size>0 && what->Size.curent<=what->finfo.size){
		float p=(float(what->Size.curent)*float(100))/float(what->finfo.size);
		set_percent(what->list_iter,p);
		make_number_nice(data1,what->finfo.size-what->Size.curent,NICE_DEC_DIGITALS);
		change_data(what->list_iter,
			    REMAIN_SIZE_COL,
			    data1);
	};
};

void d4xQueueView::add(tDownload *what) {
	add_wf(what);
	if (CFG.WITHOUT_FACE) return;
	LoDSortFlag=NOTHING_COL;
	GtkTreeIter iter;
	gtk_list_store_append(list_store, &iter);
	gtk_list_store_set(list_store, &iter,
			   URL_COL, std::string(what->info).c_str(),
			   NOTHING_COL, what,
			   DESCRIPTION_COL,what->Description.get(),
			   NOTHING_COL+1,gboolean(what->protect),
			   -1);
	if (what->list_iter) gtk_tree_iter_free(what->list_iter);
	what->list_iter=gtk_tree_iter_copy(&iter);
	print_size(what);
	set_filename(what);
	set_pixmap(what);
};

void d4xQueueView::remove(tDownload *what){
	remove_wf(what);
	if (CFG.WITHOUT_FACE) return;
	gtk_list_store_remove(list_store,what->list_iter);
	gtk_tree_iter_free(what->list_iter);
	what->list_iter=NULL;
};

void d4xQueueView::set_run_icon(tDownload *what){
	int a=int(what->Percent * 0.09);
	if (a>9) a=9;
	if (a<0) a=0;
	switch (what->ActStatus.curent) {
	case D_QUERYING:{
		set_pixmap(what,LPE_RUN_PART+a);
		break;
	};
	default:
	case D_DOWNLOAD:{
		set_pixmap(what,LPE_RUN+a);
		break;
	};
	case D_DOWNLOAD_BAD:{
		set_pixmap(what,LPE_RUN_BAD+a);
		break;
	};
	};
};

void d4xQueueView::add_first(tDownload *what) {
	add_wf(what);
	if (CFG.WITHOUT_FACE) return;
	LoDSortFlag=NOTHING_COL;
	GtkTreeIter iter;
	gtk_list_store_prepend(list_store, &iter);
	gtk_list_store_set(list_store, &iter,
			   URL_COL, std::string(what->info).c_str(),
			   NOTHING_COL, what,
			   DESCRIPTION_COL,what->Description.get(),
			   NOTHING_COL+1,what->protect,
			   -1);
	if (what->list_iter) gtk_tree_iter_free(what->list_iter);
	what->list_iter=gtk_tree_iter_copy(&iter);
	set_filename(what);
	set_pixmap(what);
	print_size(what);
};

void d4xQueueView::move_download_up(GtkTreeIter *iter){
	GtkTreePath *prev=gtk_tree_model_get_path(GTK_TREE_MODEL(list_store),iter);
	if (gtk_tree_path_prev(prev)){
		GtkTreeIter iter_prev;
		gtk_tree_model_get_iter(GTK_TREE_MODEL(list_store),&iter_prev,prev);
		tDownload *what=get_download(&iter_prev);
		tDownload *what2=get_download(iter);
		if (what->owner()==DL_WAIT && what2->owner()==DL_WAIT)
			D4X_QUEUE->backward(what);
		gtk_tree_model_swap_rows_l(GTK_TREE_MODEL(list_store),iter,&iter_prev);
		gtk_tree_iter_free(what->list_iter);
		gtk_tree_iter_free(what2->list_iter);
		what->list_iter=gtk_tree_iter_copy(iter);
		what2->list_iter=gtk_tree_iter_copy(&iter_prev);
		GtkTreeSelection *sel=gtk_tree_view_get_selection(GTK_TREE_VIEW(ListOfDownloads));
		if (gtk_tree_selection_iter_is_selected(sel,iter)){
			gtk_tree_selection_unselect_iter(sel,iter);
			gtk_tree_selection_select_iter(sel,&iter_prev);
		};
	};
	gtk_tree_path_free(prev);
};

void d4xQueueView::move_download_down(GtkTreeIter *iter){
	GtkTreeIter *iter_next=gtk_tree_iter_copy(iter);
	if (gtk_tree_model_iter_next(GTK_TREE_MODEL(list_store),iter_next)){
		tDownload *what=get_download(iter);
		tDownload *what2=get_download(iter_next);
		if (what->owner()==DL_WAIT && what2->owner()==DL_WAIT)
			D4X_QUEUE->backward(what);
		gtk_tree_model_swap_rows_l(GTK_TREE_MODEL(list_store),iter,iter_next);
		gtk_tree_iter_free(what->list_iter);
		gtk_tree_iter_free(what2->list_iter);
		what->list_iter=gtk_tree_iter_copy(iter_next);
		what2->list_iter=gtk_tree_iter_copy(iter);
		GtkTreeSelection *sel=gtk_tree_view_get_selection(GTK_TREE_VIEW(ListOfDownloads));
		if (gtk_tree_selection_iter_is_selected(sel,iter)){
			gtk_tree_selection_unselect_iter(sel,iter);
			gtk_tree_selection_select_iter(sel,iter_next);
		};
	}else{
		move_success=0;
	};
	gtk_tree_iter_free(iter_next);
};

static void _foreach_sizequery_(GtkTreeModel *model,GtkTreePath *path,
			      GtkTreeIter *iter,gpointer p){
	d4xQueueView *qv=(d4xQueueView *)p;
	_aa_.move_to_sizequery(qv->get_download(iter));
};

void d4xQueueView::selected_sizequery(){
	GtkTreeSelection *sel=gtk_tree_view_get_selection(GTK_TREE_VIEW(ListOfDownloads));
	GtkTreeIter first;
	gtk_tree_selection_selected_foreach(sel,
					    _foreach_sizequery_,
					    this);
};

static void _foreach_move_up_(GtkTreeModel *model,GtkTreePath *path,
			      GtkTreeIter *iter,gpointer p){
	d4xQueueView *qv=(d4xQueueView *)p;
	qv->move_download_up(iter);
};


int d4xQueueView::move_selected_up(){
	// move up while first row is not selected
	GtkTreeSelection *sel=gtk_tree_view_get_selection(GTK_TREE_VIEW(ListOfDownloads));
	GtkTreeIter first;
	if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(list_store),&first) &&
	    !gtk_tree_selection_iter_is_selected(sel,&first)){
		gtk_tree_selection_selected_foreach(sel,
						    _foreach_move_up_,
						    this);
		return 1;
	};
	return 0;
};

static void _foreach_move_down_prepare_(GtkTreeModel *model,GtkTreePath *path,
					GtkTreeIter *iter,gpointer p){
	tQueue *q=(tQueue*)p;
	tmpIterNode *i=new tmpIterNode(iter);
	q->insert(i);
};

int d4xQueueView::move_selected_down(){
	tQueue q;
	move_success=1;
	GtkTreeSelection *sel=gtk_tree_view_get_selection(GTK_TREE_VIEW(ListOfDownloads));
	gtk_tree_selection_selected_foreach(sel,
					    _foreach_move_down_prepare_,
					    &q);
	tNode *t=q.last();
	while(t){
		move_download_down(((tmpIterNode*)t)->iter);
		t=t->next;
	};
	return(move_success);
};


void d4xQueueView::move_up(){
	move_selected_up();
};

void d4xQueueView::move_down(){
	move_selected_down();
};

void d4xQueueView::move_selected_home(){
	while (move_selected_up());
};

void d4xQueueView::move_selected_end(){
	while (move_selected_down());
};

tDownload *d4xQueueView::get_download(GtkTreeIter *iter) {
	GValue val={0,};
	gtk_tree_model_get_value(GTK_TREE_MODEL(list_store),iter,
				 NOTHING_COL,&val);
	tDownload *what=(tDownload *)g_value_get_pointer(&val);
	g_value_unset(&val);
	return what;
};

void d4xQueueView::select_by_wildcard(GtkTreeIter *iter){
	tDownload *dwn=get_download(iter);
	if (dwn && !dwn->info.file.empty() &&
	    check_mask2(dwn->info.file.c_str(),wildcard)){
		GtkTreeSelection *sel=gtk_tree_view_get_selection(GTK_TREE_VIEW(ListOfDownloads));
		gtk_tree_selection_select_iter(sel,iter);
	};
};

void d4xQueueView::unselect_by_wildcard(GtkTreeIter *iter){
	tDownload *dwn=get_download(iter);
	if (dwn && !dwn->info.file.empty() &&
	    check_mask2(dwn->info.file.c_str(),wildcard)){
		GtkTreeSelection *sel=gtk_tree_view_get_selection(GTK_TREE_VIEW(ListOfDownloads));
		gtk_tree_selection_unselect_iter(sel,iter);
	};
};

static gboolean _real_select_by_wildcard_(GtkTreeModel *model,GtkTreePath *path,
					  GtkTreeIter *iter,gpointer data){
	d4xQueueView *qv=(d4xQueueView *)data;
	qv->select_by_wildcard(iter);
};

static gboolean _real_unselect_by_wildcard_(GtkTreeModel *model,GtkTreePath *path,
					  GtkTreeIter *iter,gpointer data){
	d4xQueueView *qv=(d4xQueueView *)data;
	qv->unselect_by_wildcard(iter);
};

void d4xQueueView::real_select(int type,char *w){
	if (w==NULL || *w==0) return;
	wildcard=w;
	if (type){
		gtk_tree_model_foreach(GTK_TREE_MODEL(list_store),
				       _real_unselect_by_wildcard_,
				       this);
	}else{
		gtk_tree_model_foreach(GTK_TREE_MODEL(list_store),
				       _real_select_by_wildcard_,
				       this);
	};
};

static void _select_ok_(GtkButton *button,d4xQueueView *qv){
	char *w=text_from_combo(LoDSelectEntry);
	qv->real_select(LoDSelectType,w);
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
	LoDSelectWindow= gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_wmclass(GTK_WINDOW(LoDSelectWindow),
			       "D4X_SelectDialog","D4X");
	gtk_window_set_title(GTK_WINDOW (LoDSelectWindow),
			     _("Enter wildcard"));
	gtk_window_set_position(GTK_WINDOW(LoDSelectWindow),
				GTK_WIN_POS_CENTER);
	gtk_container_set_border_width(GTK_CONTAINER(LoDSelectWindow),5);
	GtkWidget *vbox=gtk_vbox_new(FALSE,5);
	GtkWidget *hbox=gtk_hbutton_box_new();
	gtk_box_set_spacing(GTK_BOX(hbox),5);
	LoDSelectEntry=gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(vbox),LoDSelectEntry,FALSE,FALSE,0);
	GtkWidget *ok_button=gtk_button_new_from_stock(GTK_STOCK_OK);
	GtkWidget *cancel_button=gtk_button_new_from_stock(GTK_STOCK_CANCEL);
	GTK_WIDGET_SET_FLAGS(ok_button,GTK_CAN_DEFAULT);
	GTK_WIDGET_SET_FLAGS(cancel_button,GTK_CAN_DEFAULT);
	gtk_box_pack_start(GTK_BOX(hbox),ok_button,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(hbox),cancel_button,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);
	g_signal_connect(G_OBJECT(ok_button),"clicked",
			 G_CALLBACK(_select_ok_),this);
	g_signal_connect(G_OBJECT(cancel_button),"clicked",
			 G_CALLBACK(_select_cancel_),NULL);
	g_signal_connect(G_OBJECT(LoDSelectWindow),"delete_event",
			 G_CALLBACK(_select_delete_),NULL);
	g_signal_connect(G_OBJECT(LoDSelectEntry), "activate",
			 G_CALLBACK(_select_ok_), this);
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

static void _foreach_changelog_(GtkTreeModel *model,GtkTreePath *path,
			      GtkTreeIter *iter,gpointer p){
	d4xQueueView *qv=(d4xQueueView *)p;
	tDownload *tmp=qv->get_download(iter);
	d4x_main_switch_log(tmp);
};

int list_event_callback(GtkTreeView *view,GdkEvent *event,d4xQueueView *qv) {
	GtkTreeSelection *sel=gtk_tree_view_get_selection(view);
	GdkEventButton *bevent=(GdkEventButton *)event;
	if (event->type == GDK_BUTTON_PRESS) {
		if (bevent->button==3) {
			GtkTreePath *path=NULL;
			if (gtk_tree_view_get_path_at_pos(view,int(bevent->x),int(bevent->y),&path,NULL,NULL,NULL)){
				GtkTreeIter iter;
				GtkTreeModel *model=gtk_tree_view_get_model(view);
				gtk_tree_model_get_iter(model,&iter,path);
				if (!gtk_tree_selection_iter_is_selected(sel,&iter))
					gtk_tree_selection_unselect_all(sel);
				gtk_tree_selection_select_iter(sel,&iter);
				gtk_tree_path_free(path);
			} else {
				gtk_tree_selection_unselect_all(sel);
			};
			gint x,y;
			GdkModifierType modmask;
			gdk_window_get_pointer((GdkWindow *)NULL,&x,&y, &modmask);
			list_menu_prepare();
			gtk_menu_popup(GTK_MENU(ListMenu),(GtkWidget *)NULL,(GtkWidget *)NULL,(GtkMenuPositionFunc)NULL,(gpointer)NULL,bevent->button,bevent->time);
			return TRUE;
		};

	};
	if (event->type==GDK_BUTTON_PRESS && bevent->button==1){
		GtkTreePath *path=NULL;
		if (gtk_tree_view_get_path_at_pos(view,int(bevent->x),int(bevent->y),&path,NULL,NULL,NULL)){
			GtkTreeIter iter;
			GtkTreeModel *model=gtk_tree_view_get_model(view);
			gtk_tree_model_get_iter(model,&iter,path);
			tDownload *tmp=qv->get_download(&iter);
			d4x_main_switch_log(tmp);
			gtk_tree_path_free(path);
		};
	};
	if (event->type==GDK_2BUTTON_PRESS && bevent->button==1){
		GtkTreePath *path=NULL;
		if (gtk_tree_view_get_path_at_pos(view,int(bevent->x),int(bevent->y),&path,NULL,NULL,NULL)){
			gtk_tree_selection_unselect_all(sel);
			gtk_tree_selection_select_path(sel,path);
			switch (CFG.DBLCLK_ACT){
			case DBCLA_OPENLOG:
				qv->open_logs();
				gtk_tree_path_free(path);
				break;
			case DBCLA_EDIT:
				open_edit_for_selected();
				break;
			case DBCLA_OPENFILE:
				lm_open_file();
				break;
			};
		};
	};
	if (event->type == GDK_KEY_PRESS) {
		GdkEventKey *kevent=(GdkEventKey *)event;
		switch(kevent->keyval) {
		case GDK_F5:{
			qv->selected_sizequery();
			return TRUE;
		};
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
		}else{
		};
	};
	return FALSE;
};


int list_event_callback_first(GtkTreeView *view,GdkEvent *event,d4xQueueView *qv) {
	GdkEventKey *kevent=(GdkEventKey *)event;
	if (event->type == GDK_KEY_PRESS && (kevent->state & GDK_SHIFT_MASK)==0) {
		switch (kevent->keyval) {
		case GDK_KP_Page_Up:
		case GDK_Page_Up:
		case GDK_KP_Page_Down:
		case GDK_Page_Down:
		case GDK_KP_Up:
		case GDK_Up:
		case GDK_KP_Down:
		case GDK_Down:{
			GtkTreeSelection *sel=gtk_tree_view_get_selection(view);
		        GtkWidgetClass *tree_view_klass=(GtkWidgetClass *)gtk_type_class(gtk_tree_view_get_type());
			tree_view_klass->key_press_event(GTK_WIDGET(view),kevent);
			GtkTreeIter first;
			gtk_tree_selection_selected_foreach(sel,
							    _foreach_changelog_,
							    qv);
			return TRUE;
		};
		};
	};
	return FALSE;
};

/*

void d4xQueueView::rebuild_wait(){
	if (D4X_QUEUE->count(DL_WAIT)==0) return;
	int i=0;
	tDList *dlist=new tDList(DL_WAIT);
	dlist->init_pixmap(LPE_WAIT);
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
*/
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
	if (info == 0)
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

void d4xQueueView::toggle_column_visibility(int b){
	GList *tmp=gtk_tree_view_get_columns(GTK_TREE_VIEW(ListOfDownloads));
	GList *a=tmp;
	int i=0;
	while (a){
		GtkTreeViewColumn *col=GTK_TREE_VIEW_COLUMN(a->data);
		gpointer p=g_object_get_data (G_OBJECT (col),
					      "d4x_col_num");
		if (b==GPOINTER_TO_INT(p)){
			if (gtk_tree_view_column_get_visible(col))
				gtk_tree_view_column_set_visible(col,FALSE);
			else
				gtk_tree_view_column_set_visible(col,TRUE);
			break;
		};
		a=a->next;
	};
};

static void _tmp_activate_(GtkWidget *widget,d4xQueueView *qv){
	gint a=GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget),"d4x_col_num"));
	if (a>=0){
		qv->toggle_column_visibility(a);
	};
};

void d4xQueueView::popup_columns_visibility_menu(GdkEventButton *event){
	GtkWidget *popup_menu=gtk_menu_new();
	get_sizes();
	GtkWidget *open_row_item=gtk_check_menu_item_new_with_label(_("Status"));
	GtkWidget *arr[NOTHING_COL];
	gtk_menu_shell_append(GTK_MENU_SHELL(popup_menu),open_row_item);
	g_object_set_data(G_OBJECT(open_row_item),
			  "d4x_col_num",GINT_TO_POINTER(0));
	arr[0]=open_row_item;
	for (int i=1;i<NOTHING_COL;i++){
		open_row_item=gtk_check_menu_item_new_with_label(_(ListTitles[i]));
		g_object_set_data(G_OBJECT(open_row_item),"d4x_col_num",GINT_TO_POINTER(i));
		gtk_menu_shell_append(GTK_MENU_SHELL(popup_menu),open_row_item);
		arr[i]=open_row_item;
	};
	for (int i=0;i<NOTHING_COL;i++){
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(arr[prefs.cols[i].type]),
					       prefs.cols[i].visible);
		g_signal_connect(G_OBJECT(arr[prefs.cols[i].type]),
				 "activate",
				 G_CALLBACK(_tmp_activate_),this);
	};;
	gtk_widget_show_all(popup_menu);
	gtk_menu_popup(GTK_MENU(popup_menu),NULL,NULL,NULL,NULL,event->button,event->time);
};

static GdkPixmap *d4x_dnd_icon_pix=NULL;
static GdkBitmap *d4x_dnd_icon_bit=NULL;

static gboolean _tmp_handler_(GtkWidget *widget,GdkEventButton *event,d4xQueueView *qv){
	if (event->type==GDK_BUTTON_RELEASE && event->button==3){
		qv->popup_columns_visibility_menu(event);
		return(TRUE);
	};
	return FALSE;
};

void d4xQueueView::init() {
	last_selected=NULL;
	list_store = gtk_list_store_new(NOTHING_COL+2,
					GDK_TYPE_PIXBUF, //STATUS
					G_TYPE_STRING,   // FILE
					G_TYPE_STRING,   // FILE_TYPE
					G_TYPE_STRING,   // FULL_SIZE
					G_TYPE_STRING,   // DOWNLOADED_SIZE
					G_TYPE_STRING,   // REMAIN_SIZE
					G_TYPE_FLOAT,    // PERCENT
					G_TYPE_STRING,   // SPEED
					G_TYPE_STRING,   // TIME
					G_TYPE_STRING,   // ELAPSED_TIME
					G_TYPE_STRING,   // PAUSE
					G_TYPE_STRING,   // TREAT
					G_TYPE_STRING,   // DESRIPTION
					G_TYPE_STRING,   // URL
					G_TYPE_POINTER,  // pointer to tDownload
					G_TYPE_BOOLEAN); // style changer
	ListOfDownloads = gtk_tree_view_new_with_model(GTK_TREE_MODEL(list_store));
	gtk_widget_ref(ListOfDownloads);
	for (int i=0;i<NOTHING_COL;i++){
		GtkCellRenderer *renderer;
		GtkTreeViewColumn *col;
//		if (prefs.cols[i].type==PERCENT_COL) continue; //FIXME: GTK2
//		printf("%i\n",prefs.cols[i].type);
		switch (prefs.cols[i].type){
		case STATUS_COL:
			renderer = gtk_cell_renderer_pixbuf_new();
			col=gtk_tree_view_column_new_with_attributes (_(ListTitles[prefs.cols[i].type]),
								      renderer,
								      "pixbuf",prefs.cols[i].type,
								      NULL);
			break;
		case PERCENT_COL:
			renderer = d4x_cell_renderer_progress_new ();
			col=gtk_tree_view_column_new_with_attributes (_(ListTitles[prefs.cols[i].type]),
								      renderer,
								      "percent",prefs.cols[i].type,
								      "download",NOTHING_COL,
								      NULL);
			break;
		default:
			renderer = gtk_cell_renderer_text_new ();
			if (prefs.cols[i].type== DESCRIPTION_COL)
				g_object_set(G_OBJECT(renderer),"ellipsize",PANGO_ELLIPSIZE_END,0);
			g_object_set (G_OBJECT (renderer),
				      "foreground-gdk", &LRED,
				      NULL);
			col=gtk_tree_view_column_new_with_attributes (_(ListTitles[prefs.cols[i].type]),
								      renderer,
								      "text",prefs.cols[i].type,
								      "foreground_set", NOTHING_COL+1,
								      NULL);
			break;
		};
		g_object_set_data(G_OBJECT(col),"d4x_col_num",GINT_TO_POINTER(prefs.cols[i].type));
		gtk_tree_view_column_set_reorderable(col,TRUE);
		gtk_tree_view_column_set_sizing(col,
						GTK_TREE_VIEW_COLUMN_FIXED);
		gtk_tree_view_column_set_fixed_width(col,prefs.cols[i].size);
		gtk_tree_view_column_set_resizable(col,TRUE);
		gtk_tree_view_column_set_visible(col,prefs.cols[i].visible);
		gtk_tree_view_column_set_clickable(col,TRUE);
		gtk_tree_view_append_column (GTK_TREE_VIEW(ListOfDownloads), col);
		g_signal_connect(G_OBJECT(col->button), "event",
				 G_CALLBACK(_tmp_handler_),this);
	};
	GtkTreeSelection *sel=gtk_tree_view_get_selection(GTK_TREE_VIEW(ListOfDownloads));
	gtk_tree_selection_set_mode (sel,GTK_SELECTION_MULTIPLE);
	gtk_tree_selection_set_select_function(sel,select_download,this,NULL);
	g_signal_connect(G_OBJECT(ListOfDownloads), "event",
			 G_CALLBACK(list_event_callback_first),this);
	g_signal_connect(G_OBJECT(ListOfDownloads), "event",
			 G_CALLBACK(list_event_callback),this);
//	ListOfDownloads = my_gtk_clist_new_with_titles( prefs.cols[NOTHING_COL].enum_index, RealListTitles);
#include "pixmaps/dnd.xpm"
	if (!d4x_dnd_icon_pix)
		d4x_dnd_icon_pix=gdk_pixmap_colormap_create_from_xpm_d(NULL,
								       gtk_widget_get_colormap (MainWindow),
								       &d4x_dnd_icon_bit,
								       NULL,
								       dnd_xpm);
	gtk_drag_source_set (ListOfDownloads, GdkModifierType(GDK_BUTTON1_MASK | GDK_BUTTON3_MASK),
				ltarget_table, ln_targets,
				GdkDragAction(GDK_ACTION_COPY | GDK_ACTION_MOVE));
	gtk_drag_source_set_icon(ListOfDownloads,gtk_widget_get_colormap (MainWindow),
				 d4x_dnd_icon_pix,
				 d4x_dnd_icon_bit);
	g_signal_connect(G_OBJECT (ListOfDownloads), "drag_data_get",
			 G_CALLBACK(source_drag_data_get), NULL);

// No need to unref cos it will be freed at exiting
//	gdk_pixmap_unref (d4x_dnd_icon_pix);
//	gdk_pixmap_unref (d4x_dnd_icon_bit);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (ContainerForCList),
	                                GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW (ContainerForCList),
					    GTK_SHADOW_IN);
/*
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
*/
};

void d4xQueueView::set_pixmap(tDownload *what){
	switch (what->owner()) {
	default:
	case DL_WAIT:{
		set_pixmap(what->list_iter,LPE_WAIT);
		break;
	};
	case DL_STOP:{
		set_pixmap(what->list_iter,LPE_STOP);
		break;
	};
	case DL_RUN:{
		what->update_trigers();
		set_run_icon(what);
		break;
	};
	case DL_PAUSE:{
		set_pixmap(what->list_iter,LPE_PAUSE);
		break;
	};
	case DL_COMPLETE:{
		set_pixmap(what->list_iter,LPE_COMPLETE);
	};
	};
};

void d4xQueueView::set_pixmap(GtkTreeIter *iter,int type){
	if (type>=LPE_UNKNOWN || iter==NULL) return;
	gtk_list_store_set(list_store,iter,STATUS_COL,CUR_THEME->lodpix[type],-1);
};

void d4xQueueView::set_pixmap(tDownload *dwn,int type){
	if (type>=LPE_UNKNOWN) return;
	set_pixmap(dwn->list_iter,type);
};

void d4xQueueView::unselect_all(){
	GtkTreeSelection *sel=gtk_tree_view_get_selection(GTK_TREE_VIEW(ListOfDownloads));
	gtk_tree_selection_unselect_all(sel);
};

void d4xQueueView::select_all(){
	GtkTreeSelection *sel=gtk_tree_view_get_selection(GTK_TREE_VIEW(ListOfDownloads));
	gtk_tree_selection_select_all(sel);
};


void d4xQueueView::invert_sel(GtkTreeIter *iter){
	GtkTreeSelection *sel=gtk_tree_view_get_selection(GTK_TREE_VIEW(ListOfDownloads));
	if (gtk_tree_selection_iter_is_selected(sel,iter))
		gtk_tree_selection_unselect_iter(sel,iter);
	else
		gtk_tree_selection_select_iter(sel,iter);
};

static gboolean _foreach_invert_selection_(GtkTreeModel *model,GtkTreePath *path,
					   GtkTreeIter *iter,gpointer data){
	d4xQueueView *qv=(d4xQueueView *)data;
	qv->invert_sel(iter);
};

void d4xQueueView::invert_selection(){
	gtk_tree_model_foreach(GTK_TREE_MODEL(list_store),_foreach_invert_selection_,this);
};

void d4xQueueView::select(tDownload *dwn){
	GtkTreeSelection *sel=gtk_tree_view_get_selection(GTK_TREE_VIEW(ListOfDownloads));
	gtk_tree_selection_unselect_all(sel);
	gtk_tree_selection_select_iter(sel,dwn->list_iter);
};

int d4xQueueView::rows(){
	return(ListOfDownloadsWF.count());
};

/*
void d4xQueueView::swap(tDownload *a,tDownload *b){
	gint rowa=get_row(a);
	gint rowb=get_row(b);
	gtk_clist_swap_rows(GTK_CLIST(ListOfDownloads),rowa,rowb);
};
*/

/* Various additional functions
 */

static void _foreach_continue_logs_(GtkTreeModel *model,GtkTreePath *path,
			     GtkTreeIter *iter,gpointer p){
	GValue val={0,};
	gtk_tree_model_get_value(model,iter,NOTHING_COL,&val);
	tDownload *temp=(tDownload *)g_value_peek_pointer(&val);
	g_value_unset(&val);
	if (temp) log_window_init(temp);
};

void d4xQueueView::continue_opening_logs(){
	GtkTreeSelection *sel=gtk_tree_view_get_selection(GTK_TREE_VIEW(ListOfDownloads));
	gtk_tree_selection_selected_foreach(sel,_foreach_continue_logs_,NULL);
};

void d4xQueueView::open_logs() {
	GtkTreeSelection *sel=gtk_tree_view_get_selection(GTK_TREE_VIEW(ListOfDownloads));
	gtk_tree_selection_selected_foreach(sel,_foreach_continue_logs_,NULL);
};

void d4xQueueView::set_shift(float shift){
//	GtkAdjustment *adj=gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(ContainerForCList));
	GtkAdjustment *adj=gtk_tree_view_get_vadjustment(GTK_TREE_VIEW(ListOfDownloads));
	if (adj) gtk_adjustment_set_value(adj,shift);
};

/* move_to(tDownload *dwn)
   shift current view to display dwn
 */

void d4xQueueView::move_to(tDownload *dwn){
	GtkTreePath *path=gtk_tree_model_get_path(GTK_TREE_MODEL(list_store),dwn->list_iter);
	gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(ListOfDownloads),path,NULL,FALSE,0,0);
	gtk_tree_path_free(path);
};

void d4xQueueView::get_adj(){
	GtkAdjustment *adj=gtk_tree_view_get_vadjustment(GTK_TREE_VIEW(ListOfDownloads));
	if (adj) current_shift=adj->value;
};

// manipulating with downloads

static void _foreach_stop_(GtkTreeModel *model,GtkTreePath *path,
			   GtkTreeIter *iter,gpointer p){
	GValue val={0,};
	gtk_tree_model_get_value(model,iter,NOTHING_COL,&val);
	tDownload *temp=(tDownload *)g_value_peek_pointer(&val);
	g_value_unset(&val);
	_aa_.stop_download(temp);
};

int d4xQueueView::get_row_num(tDownload *dwn){
	GtkTreePath *path=gtk_tree_model_get_path(GTK_TREE_MODEL(list_store),dwn->list_iter);
	int *a=gtk_tree_path_get_indices(path);
	int rval=*a;
	gtk_tree_path_free(path);
	return(rval);
};

void d4xQueueView::stop_downloads(){
	int olda=_aa_.set_auto_run(1);
	GtkTreeSelection *sel=gtk_tree_view_get_selection(GTK_TREE_VIEW(ListOfDownloads));
	gtk_tree_selection_selected_foreach(sel,_foreach_stop_,NULL);
	_aa_.set_auto_run(olda);
};


void d4xQueueView::delete_downloads(int flag){
	tQueue q;
	GtkTreeSelection *sel=gtk_tree_view_get_selection(GTK_TREE_VIEW(ListOfDownloads));
	gtk_tree_selection_selected_foreach(sel,
					    _foreach_move_down_prepare_,
					    &q);
	tNode *t=q.last();
	while(t){
		tDownload *temp=get_download(((tmpIterNode*)t)->iter);
		_aa_.delete_download(temp,flag);
		t=t->next;
	};
};

static void _foreach_continue_(GtkTreeModel *model,GtkTreePath *path,
			       GtkTreeIter *iter,gpointer p){
	GValue val={0,};
	gtk_tree_model_get_value(model,iter,NOTHING_COL,&val);
	tDownload *temp=(tDownload *)g_value_peek_pointer(&val);
	g_value_unset(&val);
	if (GPOINTER_TO_INT(p))
		temp->restart_from_begin=1;
	_aa_.continue_download(temp);
};

void d4xQueueView::continue_downloads(int from_begin){
	GtkTreeSelection *sel=gtk_tree_view_get_selection(GTK_TREE_VIEW(ListOfDownloads));
	gtk_tree_selection_selected_foreach(sel,_foreach_continue_,GINT_TO_POINTER(from_begin));
};

static void _foreach_invprot_(GtkTreeModel *model,GtkTreePath *path,
			      GtkTreeIter *iter,gpointer p){
	GValue val={0,};
	gtk_tree_model_get_value(model,iter,NOTHING_COL,&val);
	tDownload *temp=(tDownload *)g_value_peek_pointer(&val);
	g_value_unset(&val);
	temp->protect=!temp->protect;
	d4xQueueView *qv=(d4xQueueView *)p;
	qv->set_color(temp);
};

void d4xQueueView::inv_protect_flag(){
	GtkTreeSelection *sel=gtk_tree_view_get_selection(GTK_TREE_VIEW(ListOfDownloads));
	gtk_tree_selection_selected_foreach(sel,_foreach_invprot_,this);
};

void d4xQueueView::inherit_settings(d4xQueueView *papa){
	for (int i=STATUS_COL;i<=NOTHING_COL;i++){
		prefs.cols[i].type=papa->prefs.cols[i].type;
		prefs.cols[i].size=papa->prefs.cols[i].size;
		prefs.cols[i].visible=papa->prefs.cols[i].visible;
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
		sprintf(data,"%i",prefs.cols[i].visible);
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
		if (tmp) sscanf(tmp,"%i",&prefs.cols[i].visible);
		else prefs.cols[i].visible=i;
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
	int check[NOTHING_COL]={0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	for (int i=0;i<NOTHING_COL;i++){
		check[prefs.cols[i].type]=1;
	};
	int need_to_reset=0;
	for (int i=0;i<NOTHING_COL;i++){
		if (check[i]==0){
			need_to_reset=1;
			break;
		};
	};
	if (need_to_reset){
		for (int i=0;i<NOTHING_COL;i++){
			prefs.cols[i].type=i;
			prefs.cols[i].visible=1;
			prefs.cols[i].size=100;
		};
	};
		
	return(0);
};
	
