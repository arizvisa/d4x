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
#include "fsface.h"
#include "lod.h"
#include "list.h"
#include "log.h"
#include "addd.h"
#include "../ntlocale.h"
#include "../var.h"

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
	gtk_signal_connect(GTK_OBJECT(menu),"hide",GTK_SIGNAL_FUNC(fs_list_menu_hide),NULL);

	if (what->status==DOWNLOAD_COMPLETE){
		tDownload *tmp=what->DIR==NULL?(tDownload *)NULL:what->DIR->last();
		if (tmp){
			while (tmp){
				char a[MAX_LEN];
				float p=tmp->Percent/tmp->Attempt.curent;
				sprintf(a,"%2.1f%% %s",p,tmp->info->host.get());
				menu_item=gtk_menu_item_new_with_label(a);
//				menu_item=gtk_menu_item_new_with_label(tmp->info->host.get());
				gtk_menu_append(GTK_MENU(menu),menu_item);
				gtk_signal_connect(GTK_OBJECT(menu_item),
						   "activate",
						   GTK_SIGNAL_FUNC(fs_list_add_download),
						   tmp);
				tmp=what->DIR->next();
			};
		}else{
			menu_item=gtk_menu_item_new_with_label(_("nothing found"));
			gtk_menu_append(GTK_MENU(menu),menu_item);
		};
	}else{
		menu_item=gtk_menu_item_new_with_label(_("searching"));
		gtk_menu_append(GTK_MENU(menu),menu_item);
	};
	
	menu_item=gtk_menu_item_new();
	gtk_menu_append(GTK_MENU(menu),menu_item);

	if (what->status==DOWNLOAD_COMPLETE && what->owner==DL_FS_STOP
	    && what->DIR && what->DIR->count()>0){
		menu_item=gtk_menu_item_new_with_label(_("reping"));
		gtk_menu_append(GTK_MENU(menu),menu_item);
		gtk_signal_connect(GTK_OBJECT(menu_item),"activate",
				   GTK_SIGNAL_FUNC(fs_list_reping),what);
		menu_item=gtk_menu_item_new_with_label(_("cumulative reping"));
		gtk_menu_append(GTK_MENU(menu),menu_item);
		gtk_signal_connect(GTK_OBJECT(menu_item),"activate",
				   GTK_SIGNAL_FUNC(fs_list_cumulative_reping),what);
	};

	menu_item=gtk_menu_item_new_with_label(_("remove"));
	gtk_menu_append(GTK_MENU(menu),menu_item);
	gtk_signal_connect(GTK_OBJECT(menu_item),"activate",
			   GTK_SIGNAL_FUNC(fs_list_delete),what);
		
	gtk_widget_show_all(menu);
	
	gtk_menu_popup(GTK_MENU(menu),(GtkWidget *)NULL,
		       (GtkWidget *)NULL,(GtkMenuPositionFunc)NULL,
		       (gpointer)NULL,bevent->button,bevent->time);
};

gint fs_list_event_callback(GtkWidget *widget,GdkEvent *event){
	GtkCList *clist=GTK_CLIST(widget);
	GdkEventButton *bevent=(GdkEventButton *)event;
	if (event->type==GDK_BUTTON_PRESS && bevent->button==3) {
		int row;
		if (gtk_clist_get_selection_info(GTK_CLIST(widget),int(bevent->x),int(bevent->y),&row,(gint *)NULL)) {
			gtk_clist_unselect_all(GTK_CLIST(widget));
			gtk_clist_select_row(GTK_CLIST(widget),row,-1);
			GList *select=clist->selection;
			if (select){
				tDownload *temp=(tDownload *)gtk_clist_get_row_data(clist,
										    GPOINTER_TO_INT(select->data));
				if (temp){
					fs_list_prepare_menu(temp,bevent);
				};
			};
		}else
			gtk_clist_unselect_all(GTK_CLIST(widget));
		return(TRUE);
	};
	
	return(FALSE);
};

gint fs_list_select_callback(GtkWidget *widget, gint row, gint column,
			     GdkEventButton *event, gpointer data,
			     gpointer nothing){
	GtkCList *clist=GTK_CLIST(widget);
	if (event && event->type==GDK_2BUTTON_PRESS && event->button==1){
		GList *select=clist->selection;
		if (select){
			tDownload *temp=(tDownload *)gtk_clist_get_row_data(clist,
									    GPOINTER_TO_INT(select->data));
			if (temp)
				log_window_init(temp);
		};
	};
	return(TRUE);
};

GtkCList *fs_list_init(){
	GtkCList *clist=(GtkCList *)gtk_clist_new(FS_COL_LAST);
	gtk_clist_set_column_width (clist,FS_COL_ICON,20);
	gtk_clist_set_column_width (clist,FS_COL_NAME,180);
	gtk_clist_set_column_width (clist,FS_COL_SIZE,100);
	gtk_clist_set_column_auto_resize(clist,FS_COL_NAME,TRUE);
	gtk_clist_set_column_auto_resize(clist,FS_COL_SIZE,TRUE);
	gtk_signal_connect(GTK_OBJECT(clist), "select_row",
	                   GTK_SIGNAL_FUNC(fs_list_select_callback),NULL);
	gtk_signal_connect(GTK_OBJECT(clist), "event",
	                   GTK_SIGNAL_FUNC(fs_list_event_callback),NULL);
	return(clist);
};

void fs_list_set_icon(GtkCList *clist,tDownload *what,int icon){
	gint row=gtk_clist_find_row_from_data (clist,what);
	if (row>=0)
		gtk_clist_set_pixmap (clist, row,FS_COL_ICON,
				      list_of_downloads_pixmaps[icon],
				      list_of_downloads_bitmaps[icon]);
};

void fs_list_add(GtkCList* clist,tDownload *what){
	char data[10];
	sprintf(data,"%li",what->finfo.size);
	char *text[FS_COL_LAST]={(char*)NULL,what->info->file.get(),data};
	gint row=gtk_clist_append(clist,text);
	gtk_clist_set_row_data (clist,row,what);
	gtk_clist_set_pixmap (clist, row,FS_COL_ICON,
			      list_of_downloads_pixmaps[PIX_WAIT],
			      list_of_downloads_bitmaps[PIX_WAIT]);
};

void fs_list_remove(GtkCList *clist,tDownload *what){
	gint row=gtk_clist_find_row_from_data (clist,what);
	if (row>=0)
		gtk_clist_remove(clist,row);
};

static gint fs_list_status=0;
static gint MAIN_PANED_WIDTH=0;

void fs_list_hide(){
	fs_list_status=0;
	if (MAIN_PANED2 &&
	    MAIN_PANED2->allocation.width-GTK_PANED(MAIN_PANED2)->gutter_size > GTK_PANED(MAIN_PANED2)->child1_size){
		gtk_paned_set_position(GTK_PANED(MAIN_PANED2),
				       MAIN_PANED2->allocation.width-GTK_PANED(MAIN_PANED2)->gutter_size);
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
					       gint(CFG.WINDOW_CLIST_WIDTH)-
					       GTK_PANED(MAIN_PANED2)->gutter_size);
		else{
			fs_list_hide();
		};
	};
};

void fs_list_get_size(){
	if (MAIN_PANED2 && fs_list_status)
		CFG.WINDOW_CLIST_WIDTH=MAIN_PANED2->allocation.width-
			GTK_PANED(MAIN_PANED2)->child1_size-
			GTK_PANED(MAIN_PANED2)->gutter_size;
};


void fs_list_allocation(GtkWidget *paned,GtkAllocation *allocation){
	if (MAIN_PANED_WIDTH && allocation->width!=MAIN_PANED_WIDTH){
		int temp=CFG.WINDOW_CLIST_WIDTH;
		float ratio=(float)CFG.WINDOW_CLIST_WIDTH/(float)(MAIN_PANED_WIDTH-GTK_PANED(MAIN_PANED2)->gutter_size);
		CFG.WINDOW_CLIST_WIDTH=int(ratio*(float)(allocation->width-GTK_PANED(MAIN_PANED2)->gutter_size));
		fs_list_set_size();
		if (!fs_list_status)
			CFG.WINDOW_CLIST_WIDTH=temp;
	};
	MAIN_PANED_WIDTH=allocation->width;
	if (fs_list_status){
		fs_list_get_size();
	};
};
