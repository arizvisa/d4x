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
#include "misc.h"

static GtkWidget *fs_list_menu_to_destroy=(GtkWidget *)NULL;

void fs_list_menu_hide(GtkWidget *widget){
	if (fs_list_menu_to_destroy){
		gtk_widget_destroy(fs_list_menu_to_destroy);
	};
	fs_list_menu_to_destroy=widget;
};

extern tMain aa;

void fs_list_delete(GtkWidget *widget,tDownload *what){
	aa.ftp_search_remove(what);
};

void fs_list_reping(GtkWidget *widget,tDownload *what){
	what->Status.curent=0;
	aa.ftp_search_reping(what);
};

void fs_list_cumulative_reping(GtkWidget *widget,tDownload *what){
	what->Status.curent=1;
	aa.ftp_search_reping(what);
};

void fs_list_add_download(GtkWidget *widget,tDownload *what){
	char *url=what->info->url();
	init_add_dnd_window(url,what->info->host.get());
	delete[] url;
};

void fs_list_prepare_menu(tDownload *what,GdkEventButton *bevent){
	GtkWidget *menu=gtk_menu_new();
	GtkWidget *menu_item;
	g_signal_connect(G_OBJECT(menu),"hide",G_CALLBACK(fs_list_menu_hide),NULL);

	if (what->status==DOWNLOAD_COMPLETE){
		tDownload *tmp=what->DIR==NULL?(tDownload *)NULL:what->DIR->last();
		if (tmp){
			while (tmp){
				char a[MAX_LEN];
				char b[100];
				float p=tmp->Percent/tmp->Attempt.curent;
				d4x_percent_str(p,b,sizeof(b));
				if (what->finfo.size>0){
					sprintf(a,"%s%% %s",b,tmp->info->host.get());
				}else{
					char size[100];
					make_number_nice(size,tmp->finfo.size,D4X_QUEUE->NICE_DEC_DIGITALS);
					sprintf(a,"%s%% %s [%s %s]",b,tmp->info->host.get(),
						tmp->finfo.size>0?size:"???",_("bytes"));
				};
				menu_item=gtk_menu_item_new_with_label(a);
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

gint fs_list_event_callback(GtkWidget *widget,GdkEvent *event){
	GtkTreeView *view=GTK_TREE_VIEW(widget);
	GdkEventButton *bevent=(GdkEventButton *)event;
	GtkTreeSelection *sel=gtk_tree_view_get_selection(view);
	GtkTreePath *path=NULL;
	GtkTreeIter iter;
	if (event->type==GDK_BUTTON_PRESS && bevent->button==3) {
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
			fs_list_prepare_menu(what,bevent);
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
						      G_TYPE_POINTER);
	GtkTreeView *view = (GtkTreeView *)gtk_tree_view_new_with_model(GTK_TREE_MODEL(list_store));
	gtk_tree_view_set_headers_visible(view,FALSE);
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *col;
	renderer = gtk_cell_renderer_pixbuf_new();
	col=gtk_tree_view_column_new_with_attributes ("Status",
						      renderer,
						      "pixbuf",FS_COL_ICON,
						      NULL);
	gtk_tree_view_append_column(view,col);
	for (int i=FS_COL_NAME;i<FS_COL_LAST;i++){
		renderer = gtk_cell_renderer_text_new ();
		col=gtk_tree_view_column_new_with_attributes ("Tittle",
							      renderer,
							      "text",i,
							      NULL);
		gtk_tree_view_column_set_resizable(col,FALSE);
		gtk_tree_view_append_column(view,col);
	};
/*
	GtkCList *clist=(GtkCList *)gtk_clist_new(FS_COL_LAST);
	gtk_clist_set_row_height(clist,16);
	gtk_clist_set_column_width (clist,FS_COL_ICON,20);
	gtk_clist_set_column_width (clist,FS_COL_NAME,180);
	gtk_clist_set_column_width (clist,FS_COL_SIZE,100);
	gtk_clist_set_column_auto_resize(clist,FS_COL_NAME,TRUE);
	gtk_clist_set_column_auto_resize(clist,FS_COL_SIZE,TRUE);
*/
	g_signal_connect(G_OBJECT(view), "event",
			 G_CALLBACK(fs_list_event_callback),NULL);
	return(view);
};

void fs_list_set_icon(GtkTreeView *view,tDownload *what,int icon){
	GtkListStore *store=(GtkListStore *)gtk_tree_view_get_model(view);
	gtk_list_store_set(store,what->list_iter,
			   FS_COL_ICON,list_of_downloads_pixbufs[icon],
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
			   FS_COL_ICON, list_of_downloads_pixbufs[PIX_WAIT],
			   FS_COL_SIZE, data,
			   FS_COL_NAME, what->info->file.get(),
			   FS_COL_LAST, what,
			   -1);
	if (what->list_iter) gtk_tree_iter_free(what->list_iter);
	what->list_iter=gtk_tree_iter_copy(&iter);
};

void fs_list_remove(GtkTreeView *view,tDownload *what){
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
