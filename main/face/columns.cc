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
#include <string.h>
#include "columns.h"
#include "misc.h"
#include "../ntlocale.h"
#include "../main.h"
#include "../var.h"

char *columns_names[]={
	N_("status"),
	N_("file name"),
	N_("type of file"),
	N_("size of file"),
	N_("downloaded size"),
	N_("rest"),
	N_("percent"),
	N_("speed"),
	N_("elapsed time"),
	N_("estimated time"),
	N_("time of pause"),
	N_("number of attempts"),
	N_("description"),
	N_("URL"),
	"[NONE]"
};

GList *tColumnsPrefs::find_by_data(GList *where, char *what){
	GList *list=where;
	while (list){
		if (strcmp(what,(char *)(list->data))==0) break;
		list=list->next;
	};
	return list;
};

tColumnsPrefs::tColumnsPrefs(){
	tmp_apply_flag=0;
};

void tColumnsPrefs::reset(){
	tmp_apply_flag=0;
};

void tColumnsPrefs::init(){
	int vsize=(NOTHING_COL+1)/2;
	box=gtk_table_new(vsize,2,FALSE);
	for (int i=0;i<NOTHING_COL;i++){
		columns[i]=gtk_check_button_new_with_label(_(columns_names[i]));
		int temp=i<vsize?i:i-vsize;
		gtk_table_attach_defaults(GTK_TABLE(box),columns[i],i/vsize,i/vsize+1,temp,temp+1);
		if (tmp_apply_flag){
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(columns[i]),tmp_apply[i]);
		}else{
			if (ListColumns[i].enum_index<ListColumns[NOTHING_COL].enum_index)
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(columns[i]),TRUE);
			else
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(columns[i]),FALSE);
		};
	};
	frame=gtk_frame_new(_("Showed columns"));
	gtk_container_add(GTK_CONTAINER(frame),box);
	box=gtk_vbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(box),frame,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(box),gtk_hbox_new(FALSE,0),FALSE,FALSE,0);
};

static gint compare_nodes(gconstpointer a,gconstpointer b){
    gint aa=((tDownload *)(a))->GTKCListRow;
    gint bb=((tDownload *)(b))->GTKCListRow;
    if (aa>bb) return 1;
    if (aa==bb) return 0;
    return -1;
};


void tColumnsPrefs::add_to_sort(tDownload *what){
	sort_list=g_list_insert_sorted(sort_list,what,compare_nodes);
	while(sort_list && ((tDownload *)(sort_list->data))->GTKCListRow==first){
		tDownload *tmp=(tDownload *)(sort_list->data);
		list_of_downloads_add(tmp,tmp->GTKCListRow);
		sort_list=g_list_remove(sort_list,tmp);
		first+=1;
	};
};

void tColumnsPrefs::add_to_list(tDList *list){
		tDownload *temp=list->last();
		while (temp){
			add_to_sort(temp);
			temp=list->next();
		};
};

void tColumnsPrefs::apply_changes_tmp(){
	tmp_apply_flag=1;
	for (int i=0;i<NOTHING_COL;i++){
		tmp_apply[i]=GTK_TOGGLE_BUTTON(columns[i])->active;
	};
};

void tColumnsPrefs::apply_changes(){
	if (tmp_apply_flag==0) return;
	tColumn temp[NOTHING_COL+1];
	temp[NOTHING_COL].type=temp[NOTHING_COL].enum_index=NOTHING_COL;
	int a=0;
	for (int i=0;i<NOTHING_COL;i++){
		if (!tmp_apply[i]){
//move "down" NOTHING_COL
			temp[temp[NOTHING_COL].enum_index-1].type=NOTHING_COL;
			temp[NOTHING_COL].enum_index-=1;
//move curent columns before NOTHING_COL
			temp[temp[NOTHING_COL].enum_index+1].type=i;
			temp[i].enum_index=temp[NOTHING_COL].enum_index+1;
		}else{
			temp[a].type=i;
			temp[i].enum_index=a;
			a+=1;
		};
	};
//copy columns sizes from old layer
	list_of_downloads_get_sizes();
	for (int i=0;i<=NOTHING_COL;i++){
		temp[temp[i].enum_index].size=ListColumns[ListColumns[i].enum_index].size;
	};
//copy new layer to old
	int need_reinit=0;
	if (ListColumns[NOTHING_COL].enum_index!=temp[NOTHING_COL].enum_index)
		need_reinit=1;
	for (int i=0;i<=NOTHING_COL;i++){
		ListColumns[i].enum_index=temp[i].enum_index;
		ListColumns[i].size=temp[i].size;
		if (ListColumns[i].type!=temp[i].type)
			need_reinit=1;
		ListColumns[i].type=temp[i].type;
	};
//delete old list and create new
	if (need_reinit){
		for (gint row=0;;row+=1){
			tDownload *dwn=get_download_from_clist(row);
			if (dwn==NULL) break;
			dwn->GTKCListRow=row;
		};
		list_of_downloads_get_height();
		gtk_signal_handlers_destroy(GTK_OBJECT(ListOfDownloads));
		gtk_widget_destroy(ListOfDownloads);
		init_columns_info();
		list_of_downloads_init();
		first=0;
		sort_list=NULL;
		for(int i=DL_ALONE+1;i<DL_TEMP;i++)
			add_to_list(DOWNLOAD_QUEUES[i]);
		list_of_downloads_set_height();
	};
	tmp_apply_flag=0;
};

GtkWidget *tColumnsPrefs::body(){
	return box;
};
