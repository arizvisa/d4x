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

#include "fsface.h"
#include "lod.h"
#include "list.h"
#include "log.h"
#include "addd.h"
#include "../ntlocale.h"
#include "../var.h"
#include "../main.h"
#include "misc.h"
#include "themes.h"

using namespace d4x;


enum FS_FACE2_COLUMNS{
	FS2_COL_PING,
	FS2_COL_SIZE,
	FS2_COL_URL
};

tDownload *FS_CUR_SELECTED=NULL;
static GtkWidget *fs_list_menu_to_destroy=(GtkWidget *)NULL;

void fs_list_menu_hide(GtkWidget *widget){
	if (fs_list_menu_to_destroy){
		gtk_widget_destroy(fs_list_menu_to_destroy);
	};
	fs_list_menu_to_destroy=widget;
};

void fs_list_delete(GtkWidget *widget,tDownload *what){
	_aa_.ftp_search_remove(what);
};

void fs_list_reping(GtkWidget *widget,tDownload *what){
	what->ActStatus=0;
	_aa_.ftp_search_reping(what);
};

void fs_list_cumulative_reping(GtkWidget *widget,tDownload *what){
	what->ActStatus=1;
	_aa_.ftp_search_reping(what);
};

void fs_list_add_download(GtkWidget *widget,tDownload *what){
	init_add_dnd_window(std::string(what->info).c_str(),what->info.host.c_str());
};

void fs_list_prepare_menu(tDownload *what,GdkEventButton *bevent){
	GtkWidget *menu=gtk_menu_new();
	GtkWidget *menu_item;
	g_signal_connect(G_OBJECT(menu),"hide",G_CALLBACK(fs_list_menu_hide),NULL);

	if (what->status==DOWNLOAD_COMPLETE){
		tDownload *tmp=what->DIR==NULL?(tDownload *)NULL:what->DIR->last();
		if (tmp){
			std::string a;
			while (tmp){
				char b[100];
				float p=tmp->Percent/fsize_t(tmp->Attempt);
				d4x_percent_str(p,b,sizeof(b));
				a=std::string(b)+"% "+tmp->info.host;
				if (what->finfo.size<=0){
					a+=std::string(" [")+
						(tmp->finfo.size>0?make_number_nice(tmp->finfo.size,D4X_QUEUE->NICE_DEC_DIGITALS):std::string("???"))+
						_("bytes");
				};
				menu_item=gtk_menu_item_new_with_label(a.c_str());
//				menu_item=gtk_menu_item_new_with_label(tmp->info->host.get());
				gtk_menu_shell_append(GTK_MENU_SHELL(menu),menu_item);
				g_signal_connect(G_OBJECT(menu_item),
						 "activate",
						 G_CALLBACK(fs_list_add_download),
						 tmp);
				tmp=what->DIR->next();
			};
		}else{
			menu_item=gtk_menu_item_new_with_label(_("nothing found"));
			gtk_menu_shell_append(GTK_MENU_SHELL(menu),menu_item);
		};
	}else{
		menu_item=gtk_menu_item_new_with_label(_("searching"));
		gtk_menu_shell_append(GTK_MENU_SHELL(menu),menu_item);
	};
	
	menu_item=gtk_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),menu_item);

	if (what->status==DOWNLOAD_COMPLETE && what->owner()==DL_FS_STOP
	    && what->DIR && what->DIR->count()>0){
		menu_item=gtk_menu_item_new_with_label(_("reping"));
		gtk_menu_shell_append(GTK_MENU_SHELL(menu),menu_item);
		g_signal_connect(G_OBJECT(menu_item),"activate",
				 G_CALLBACK(fs_list_reping),what);
		menu_item=gtk_menu_item_new_with_label(_("cumulative reping"));
		gtk_menu_shell_append(GTK_MENU_SHELL(menu),menu_item);
		g_signal_connect(G_OBJECT(menu_item),"activate",
				 G_CALLBACK(fs_list_cumulative_reping),what);
	};

	menu_item=gtk_menu_item_new_with_label(_("remove"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),menu_item);
	g_signal_connect(G_OBJECT(menu_item),"activate",
			 G_CALLBACK(fs_list_delete),what);
		
	gtk_widget_show_all(menu);
	
	gtk_menu_popup(GTK_MENU(menu),(GtkWidget *)NULL,
		       (GtkWidget *)NULL,(GtkMenuPositionFunc)NULL,
		       (gpointer)NULL,bevent->button,bevent->time);
};

void fs_list_prepare_list(tDownload *what){
	FS_CUR_SELECTED=NULL;
	gtk_list_store_clear(GTK_LIST_STORE(gtk_tree_view_get_model(FSearchView2)));
	if (what->status==DOWNLOAD_COMPLETE && what->owner()==DL_FS_STOP &&
	    what->DIR && what->DIR->count()>0){
		GtkTreeIter iter;
		GtkListStore *store=(GtkListStore *)gtk_tree_view_get_model(FSearchView2);
		tDownload *tmp=what->DIR==NULL?(tDownload *)NULL:what->DIR->last();
		while (tmp){
			std::string size;
			char b[100];
			float p=tmp->Percent/fsize_t(tmp->Attempt);
			d4x_percent_str(p,b,sizeof(b));
			if (tmp->finfo.size>0)
				size=make_number_nice(tmp->finfo.size,D4X_QUEUE->NICE_DEC_DIGITALS);
			else
				size="????";
			gtk_list_store_append(store, &iter);
			gtk_list_store_set(store, &iter,
					   FS2_COL_PING,b,
					   FS2_COL_SIZE,size.c_str(),
					   FS2_COL_URL, std::string(tmp->info).c_str(),
					   -1);
			tmp=what->DIR->next();
		};
		FS_CUR_SELECTED=what;
	};
};

gint fs_list_event_callback(GtkWidget *widget,GdkEvent *event){
	GtkTreeView *view=GTK_TREE_VIEW(widget);
	GdkEventButton *bevent=(GdkEventButton *)event;
	GtkTreeSelection *sel=gtk_tree_view_get_selection(view);
	GtkTreePath *path=NULL;
	GtkTreeIter iter;
	if (event->type==GDK_BUTTON_PRESS && (bevent->button==3 || bevent->button==1)) {
		gtk_tree_selection_unselect_all(sel);
		int selected=0;
		if (gtk_tree_view_get_path_at_pos(view,gint(bevent->x),gint(bevent->y),&path,NULL,NULL,NULL)){
			selected=0;
			gtk_tree_selection_select_path(sel,path);
			GtkTreeModel *model=gtk_tree_view_get_model(view);
			gtk_tree_model_get_iter(model,&iter,path);
			gtk_tree_path_free(path);
			GValue val={0,};
			gtk_tree_model_get_value(model,&iter,
						 FS_COL_LAST,&val);
			tDownload *what=(tDownload *)g_value_peek_pointer(&val);
			g_value_unset(&val);
			if (bevent->button==3)
				fs_list_prepare_menu(what,bevent);
			else
				fs_list_prepare_list(what);
		};
		return(TRUE);
	};
	if (event->type==GDK_2BUTTON_PRESS && bevent->button==1){
		gtk_tree_selection_unselect_all(sel);
		if (gtk_tree_view_get_path_at_pos(view,gint(bevent->x),gint(bevent->y),&path,NULL,NULL,NULL)){
			gtk_tree_selection_select_path(sel,path);
			GtkTreeModel *model=gtk_tree_view_get_model(view);
			gtk_tree_model_get_iter(model,&iter,path);
			GValue val={0,};
			gtk_tree_model_get_value(model,&iter,
						 FS_COL_LAST,&val);
			tDownload *what=(tDownload *)g_value_peek_pointer(&val);
			g_value_unset(&val);
			if (what) log_window_init(what);
		};
	};
	return(FALSE);
};

GtkTreeView *fs_list_init(){
	GtkListStore *list_store = gtk_list_store_new(FS_COL_LAST+1,
						      GDK_TYPE_PIXBUF,
						      G_TYPE_STRING,
						      G_TYPE_STRING,
						      G_TYPE_STRING,
						      G_TYPE_POINTER);
	GtkTreeView *view = (GtkTreeView *)gtk_tree_view_new_with_model(GTK_TREE_MODEL(list_store));
	char *titles[]={
		"",
		N_("Filename"),
		N_("Size"),
		N_("Count")
	};
	gtk_tree_view_set_headers_visible(view,TRUE);
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *col;
	renderer = gtk_cell_renderer_pixbuf_new();
	col=gtk_tree_view_column_new_with_attributes ("",
						      renderer,
						      "pixbuf",FS_COL_ICON,
						      NULL);
	gtk_tree_view_append_column(view,col);
	for (int i=FS_COL_NAME;i<FS_COL_LAST;i++){
		renderer = gtk_cell_renderer_text_new ();
		col=gtk_tree_view_column_new_with_attributes (_(titles[i]),
							      renderer,
							      "text",i,
							      NULL);
		gtk_tree_view_column_set_resizable(col,FALSE);
		gtk_tree_view_append_column(view,col);
	};
	g_signal_connect(G_OBJECT(view), "event",
			 G_CALLBACK(fs_list_event_callback),NULL);
	return(view);
};

gint fs_sublist_event_callback(GtkWidget *widget,GdkEvent *event){
	GtkTreeView *view=GTK_TREE_VIEW(widget);
	GdkEventButton *bevent=(GdkEventButton *)event;
	GtkTreeSelection *sel=gtk_tree_view_get_selection(view);
	GtkTreePath *path=NULL;
	GtkTreeIter iter;
	if (event->type==GDK_2BUTTON_PRESS && bevent->button==1){
		gtk_tree_selection_unselect_all(sel);
		if (gtk_tree_view_get_path_at_pos(view,gint(bevent->x),gint(bevent->y),&path,NULL,NULL,NULL)){
			gtk_tree_selection_select_path(sel,path);
			GtkTreeModel *model=gtk_tree_view_get_model(view);
			gtk_tree_model_get_iter(model,&iter,path);
			GValue val={0,};
			gtk_tree_model_get_value(model,&iter,
						 FS2_COL_URL,&val);
			gchar *url=(gchar*)g_value_get_string(&val);
			if (url) init_add_dnd_window(url,_("Found during FTP-search"));
			g_value_unset(&val);
		};
	};
	return(FALSE);
};

GtkTreeView *fs_list_init_sublist(){
	GtkListStore *list_store = gtk_list_store_new(3,
						      G_TYPE_STRING,
						      G_TYPE_STRING,
						      G_TYPE_STRING);
	GtkTreeView *view = (GtkTreeView *)gtk_tree_view_new_with_model(GTK_TREE_MODEL(list_store));
	g_signal_connect(G_OBJECT(view),"event",
			 G_CALLBACK(fs_sublist_event_callback),NULL);
	gtk_tree_view_set_headers_visible(view,TRUE);
	GtkTreeSelection *sel=gtk_tree_view_get_selection(view);
	gtk_tree_selection_set_mode (sel,GTK_SELECTION_SINGLE);
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *col;
	
	renderer = gtk_cell_renderer_text_new ();
	col=gtk_tree_view_column_new_with_attributes (_("Ping"),
						      renderer,
						      "text",FS2_COL_PING,
						      NULL);
	gtk_tree_view_column_set_resizable(col,TRUE);
	gtk_tree_view_append_column(view,col);
	gtk_tree_view_column_set_sizing(col,
					GTK_TREE_VIEW_COLUMN_AUTOSIZE);

	renderer = gtk_cell_renderer_text_new ();
	col=gtk_tree_view_column_new_with_attributes (_("Size"),
						      renderer,
						      "text",FS2_COL_SIZE,
						      NULL);
	gtk_tree_view_column_set_resizable(col,TRUE);
	gtk_tree_view_append_column(view,col);
	gtk_tree_view_column_set_sizing(col,
					GTK_TREE_VIEW_COLUMN_AUTOSIZE);

	renderer = gtk_cell_renderer_text_new ();
	col=gtk_tree_view_column_new_with_attributes (_("Url"),
						      renderer,
						      "text",FS2_COL_URL,
						      NULL);
	gtk_tree_view_column_set_resizable(col,TRUE);
	gtk_tree_view_append_column(view,col);
	return(view);
};

void fs_list_set_icon(GtkTreeView *view,tDownload *what,int icon){
	GtkListStore *store=(GtkListStore *)gtk_tree_view_get_model(view);
	gtk_list_store_set(store,what->list_iter,
			   FS_COL_ICON,CUR_THEME->get_pixbuf(icon),
			   -1);
};

void fs_list_set_count(GtkTreeView *view,tDownload *what){
	GtkListStore *store=(GtkListStore *)gtk_tree_view_get_model(view);
	gtk_list_store_set(store,what->list_iter,
			   FS_COL_COUNT,boost::lexical_cast<std::string>(fsize_t(what->Size)).c_str(),
			   -1);
};

void fs_list_add(GtkTreeView *view,tDownload *what){
	char data[10];
	if (what->finfo.size>0)
		sprintf(data,"%li",what->finfo.size);
	else
		sprintf(data,"???");
	GtkTreeIter iter;
	GtkListStore *store=(GtkListStore *)gtk_tree_view_get_model(view);
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter,
			   FS_COL_ICON, CUR_THEME->get_pixbuf(LPE_WAIT),
			   FS_COL_SIZE, data,
			   FS_COL_NAME, what->info.file.c_str(),
			   FS_COL_LAST, what,
			   -1);
	if (what->list_iter) gtk_tree_iter_free(what->list_iter);
	what->list_iter=gtk_tree_iter_copy(&iter);
};

void fs_list_remove(GtkTreeView *view,tDownload *what){
	if (FS_CUR_SELECTED==what){
		FS_CUR_SELECTED=NULL;
		gtk_list_store_clear(GTK_LIST_STORE(gtk_tree_view_get_model(FSearchView2)));
	};
	GtkListStore *store=(GtkListStore *)gtk_tree_view_get_model(view);
	gtk_list_store_remove(store,what->list_iter);
	gtk_tree_iter_free(what->list_iter);
	what->list_iter=NULL;
};

static gint fs_list_status=0;
static gint MAIN_PANED_WIDTH=0;

void fs_list_hide(){
	fs_list_status=0;
	if (MAIN_PANED2 &&
	    MAIN_PANED2->allocation.width > GTK_PANED(MAIN_PANED2)->child1_size){
		gtk_paned_set_position(GTK_PANED(MAIN_PANED2),
				       MAIN_PANED2->allocation.width);
	};
};

void fs_list_show(){
	fs_list_status=1;
	fs_list_set_size();
};

void fs_list_set_size(){
	if (MAIN_PANED2){
		if (fs_list_status)
			gtk_paned_set_position(GTK_PANED(MAIN_PANED2),
					       MAIN_PANED2->allocation.width-
					       gint(CFG.WINDOW_CLIST_WIDTH));
		else{
			fs_list_hide();
		};
	};
};

void fs_list_get_size(){
	if (MAIN_PANED2 && fs_list_status)
		CFG.WINDOW_CLIST_WIDTH=MAIN_PANED2->allocation.width-
			GTK_PANED(MAIN_PANED2)->child1_size;
};


void fs_list_allocation(GtkWidget *paned,GtkAllocation *allocation){
	if (MAIN_PANED_WIDTH && allocation->width!=MAIN_PANED_WIDTH){
		int temp=CFG.WINDOW_CLIST_WIDTH;
		float ratio=(float)CFG.WINDOW_CLIST_WIDTH/(float)(MAIN_PANED_WIDTH);
		CFG.WINDOW_CLIST_WIDTH=int(ratio*(float)(allocation->width));
		fs_list_set_size();
		if (!fs_list_status)
			CFG.WINDOW_CLIST_WIDTH=temp;
	};
	MAIN_PANED_WIDTH=allocation->width;
	if (fs_list_status){
		fs_list_get_size();
	};
};
