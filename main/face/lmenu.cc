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

#include "list.h"
#include "lmenu.h"
#include "misc.h"
#include "edit.h"
#include "../dlist.h"
#include "../locstr.h"
#include "../mainlog.h"
#include "../ntlocale.h"
#include <gdk/gdkkeysyms.h>
#include "../main.h"
#include "../fsearch.h"
#include "lod.h"


static void _open_alternates_(GtkTreeModel *model,GtkTreePath *path,
			      GtkTreeIter *iter,gpointer data){
	GValue val={0,};
	gtk_tree_model_get_value(model,iter,NOTHING_COL,&val);
	tDownload *tmp=(tDownload *)g_value_peek_pointer(&val);
	g_value_unset(&val);
	if (tmp->ALTS==NULL)
		tmp->ALTS=new d4xAltList;
	tmp->ALTS->init_edit(tmp);
};

void lmenu_alternates(){
	GtkTreeSelection *sel=gtk_tree_view_get_selection(GTK_TREE_VIEW(D4X_QUEUE->qv.ListOfDownloads));
	gtk_tree_selection_selected_foreach(sel,_open_alternates_,NULL);
};

static void _ftp_search_(GtkTreeModel *model,GtkTreePath *path,
			 GtkTreeIter *iter,gpointer data){
	GValue val={0,};
	gtk_tree_model_get_value(model,iter,NOTHING_COL,&val);
	tDownload *tmp=(tDownload *)g_value_peek_pointer(&val);
	g_value_unset(&val);
	_aa_.ftp_search(tmp);
};

void lmenu_ftp_search_go(){
	GtkTreeSelection *sel=gtk_tree_view_get_selection(GTK_TREE_VIEW(D4X_QUEUE->qv.ListOfDownloads));
	gtk_tree_selection_selected_foreach(sel,_ftp_search_,NULL);
};

extern tMLog *MainLog;

GtkWidget *ListMenu;
GtkWidget *ListMenuArray[LM_LAST];

void copy_download_to_clipboard(){
	tDownload *dwn=D4X_QUEUE->qv.last_selected;
	if (dwn->info){
		char *url=dwn->info->url();
		d4x_mw_clipboard_set(url);
		my_xclipboard_put(url);
		delete[] url;
	};
};

GtkWidget *make_menu_item(char *name,char *accel,GdkPixmap *pixmap,GdkBitmap *bitmap,GtkSizeGroup *sgroup) {
	GtkWidget *menu_item=gtk_image_menu_item_new();
	GtkWidget *hbox=gtk_hbox_new(FALSE,3);
	GtkWidget *label = gtk_label_new(name);
	gtk_size_group_add_widget(sgroup,label);
	GtkWidget *pix;
	gtk_misc_set_alignment(GTK_MISC(label),0,0.5);
	if (pixmap && bitmap){
		pix=gtk_image_new_from_pixmap(pixmap,bitmap);
		gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item),pix);
	};
	gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
	if (accel) {
		GtkWidget *accel_label=gtk_label_new(accel);
		GtkWidget *hbox1=gtk_hbox_new(FALSE,3);
		gtk_box_pack_start(GTK_BOX(hbox1),accel_label,FALSE,FALSE,0);
		gtk_box_pack_end(GTK_BOX(hbox),hbox1,FALSE,FALSE,0);
		gtk_label_set_justify(GTK_LABEL(accel_label),GTK_JUSTIFY_RIGHT);
	};
	gtk_container_add(GTK_CONTAINER(menu_item),hbox);
	gtk_widget_show(label);
	gtk_widget_show(hbox);
	gtk_widget_show(menu_item);
	return menu_item;
};

/* FIXME: rewrite next routine */


static void lmenu_move_down(){
	D4X_QUEUE->qv.move_down();
};
static void lmenu_move_up(){
	D4X_QUEUE->qv.move_up();
};

static void lmenu_open_logs(){
	D4X_QUEUE->qv.open_logs();
};

void lm_inv_protect_flag(){
	D4X_QUEUE->qv.inv_protect_flag();
};

void init_list_menu() {
#include "pixmaps/stopmini.xpm"
#include "pixmaps/logmini.xpm"
#include "pixmaps/delmini.xpm"
#include "pixmaps/runmini.xpm"
#include "pixmaps/delcommini.xpm"
#include "pixmaps/upmini.xpm"
#include "pixmaps/downmini.xpm"
	GdkPixmap *pixmap;
	GdkBitmap *bitmap;
	GtkWidget *menu_item;
	ListMenu=gtk_menu_new();

	GtkSizeGroup *sgroup=gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	
	menu_item=make_menu_item(_("Properties"),"Alt+E",(GdkPixmap *)NULL,(GdkPixmap *)NULL,sgroup);
	gtk_menu_shell_append(GTK_MENU_SHELL(ListMenu),menu_item);
	ListMenuArray[LM_EDIT]=menu_item;
	g_signal_connect(G_OBJECT(menu_item),"activate",G_CALLBACK(open_edit_for_selected),NULL);

	pixmap=make_pixmap_from_xpm(&bitmap,logmini_xpm);
	menu_item=make_menu_item(_("View log"),(char *)NULL,pixmap,bitmap,sgroup);
	gtk_menu_shell_append(GTK_MENU_SHELL(ListMenu),menu_item);
	ListMenuArray[LM_LOG]=menu_item;
	g_signal_connect(G_OBJECT(menu_item),"activate",G_CALLBACK(lmenu_open_logs),NULL);

	pixmap=make_pixmap_from_xpm(&bitmap,stopmini_xpm);
	menu_item=make_menu_item(_("Stop"),"Alt+S",pixmap,bitmap,sgroup);
	gtk_menu_shell_append(GTK_MENU_SHELL(ListMenu),menu_item);
	ListMenuArray[LM_STOP]=menu_item;
	g_signal_connect(G_OBJECT(menu_item),"activate",G_CALLBACK(stop_downloads),NULL);

	pixmap=make_pixmap_from_xpm(&bitmap,runmini_xpm);
	menu_item=make_menu_item(_("Continue downloads"),(char *)NULL,pixmap,bitmap,sgroup);
	gtk_menu_shell_append(GTK_MENU_SHELL(ListMenu),menu_item);
	ListMenuArray[LM_CONTINUE]=menu_item;
	g_signal_connect(G_OBJECT(menu_item),"activate",G_CALLBACK(continue_downloads),NULL);

//	pixmap=make_pixmap_from_xpm(&bitmap,runmini_xpm);
	menu_item=make_menu_item(_("Copy"),(char *)NULL,(GdkPixmap *)NULL,(GdkPixmap *)NULL,sgroup);
	gtk_menu_shell_append(GTK_MENU_SHELL(ListMenu),menu_item);
	ListMenuArray[LM_COPY]=menu_item;
	g_signal_connect(G_OBJECT(menu_item),"activate",G_CALLBACK(copy_download_to_clipboard),NULL);

	menu_item=make_menu_item(_("(Un)Protect"),"Ctrl+Alt+P",(GdkPixmap *)NULL,(GdkPixmap *)NULL,sgroup);
	gtk_menu_shell_append(GTK_MENU_SHELL(ListMenu),menu_item);
	ListMenuArray[LM_PROTECT]=menu_item;
	g_signal_connect(G_OBJECT(menu_item),"activate",G_CALLBACK(lm_inv_protect_flag),NULL);
	
	menu_item=gtk_menu_item_new();
	gtk_widget_set_sensitive(menu_item,FALSE);
	gtk_menu_shell_append(GTK_MENU_SHELL(ListMenu),menu_item);

	menu_item=make_menu_item(_("Common properties"),"Ctrl+Alt+E",(GdkPixmap *)NULL,(GdkPixmap *)NULL,sgroup);
	gtk_menu_shell_append(GTK_MENU_SHELL(ListMenu),menu_item);
	ListMenuArray[LM_EDIT_COMMON]=menu_item;
	g_signal_connect(G_OBJECT(menu_item),"activate",G_CALLBACK(select_options_window_init),NULL);

	pixmap=make_pixmap_from_xpm(&bitmap,delmini_xpm);
	menu_item=make_menu_item(_("Delete downloads"),"Alt+C",pixmap,bitmap,sgroup);
	gtk_menu_shell_append(GTK_MENU_SHELL(ListMenu),menu_item);
	ListMenuArray[LM_DEL]=menu_item;
	g_signal_connect(G_OBJECT(menu_item),"activate",G_CALLBACK(ask_delete_download),NULL);

	pixmap=make_pixmap_from_xpm(&bitmap,delcommini_xpm);
	menu_item=make_menu_item(_("Delete completed"),(char *)NULL,pixmap,bitmap,sgroup);
	gtk_menu_shell_append(GTK_MENU_SHELL(ListMenu),menu_item);
	ListMenuArray[LM_DELC]=menu_item;
	g_signal_connect(G_OBJECT(menu_item),"activate",G_CALLBACK(ask_delete_completed_downloads),NULL);


	menu_item=make_menu_item(_("Delete failed"),(char *)NULL,(GdkPixmap *)NULL,(GdkPixmap *)NULL,sgroup);
	gtk_menu_shell_append(GTK_MENU_SHELL(ListMenu),menu_item);
	ListMenuArray[LM_DELF]=menu_item;
	g_signal_connect(G_OBJECT(menu_item),"activate",G_CALLBACK(ask_delete_fataled_downloads),NULL);

	menu_item=gtk_menu_item_new();
	gtk_widget_set_sensitive(menu_item,FALSE);
	gtk_menu_shell_append(GTK_MENU_SHELL(ListMenu),menu_item);

	pixmap=make_pixmap_from_xpm(&bitmap,upmini_xpm);
	menu_item=make_menu_item(_("Move up"),"Shift+Up",pixmap,bitmap,sgroup);
	gtk_menu_shell_append(GTK_MENU_SHELL(ListMenu),menu_item);
	ListMenuArray[LM_MOVEUP]=menu_item;
	g_signal_connect(G_OBJECT(menu_item),"activate",G_CALLBACK(lmenu_move_up),NULL);

	pixmap=make_pixmap_from_xpm(&bitmap,downmini_xpm);
	menu_item=make_menu_item(_("Move down"),"Shift+Down",pixmap,bitmap,sgroup);
	gtk_menu_shell_append(GTK_MENU_SHELL(ListMenu),menu_item);
	ListMenuArray[LM_MOVEDOWN]=menu_item;
	g_signal_connect(G_OBJECT(menu_item),"activate",G_CALLBACK(lmenu_move_down),NULL);

	menu_item=make_menu_item(_("Alternates"),(char *)NULL,(GdkPixmap *)NULL,(GdkPixmap *)NULL,sgroup);
	gtk_menu_shell_append(GTK_MENU_SHELL(ListMenu),menu_item);
	ListMenuArray[LM_ALT]=menu_item;
	g_signal_connect(G_OBJECT(menu_item),"activate",G_CALLBACK(lmenu_alternates),NULL);
	
	menu_item=make_menu_item(_("FTP search"),(char *)NULL,(GdkPixmap *)NULL,(GdkPixmap *)NULL,sgroup);
	gtk_menu_shell_append(GTK_MENU_SHELL(ListMenu),menu_item);
	ListMenuArray[LM_SEARCH]=menu_item;
	g_signal_connect(G_OBJECT(menu_item),"activate",G_CALLBACK(lmenu_ftp_search_go),NULL);
	
/* FIXME: GTK2.4
	GtkAccelGroup *accel_group = gtk_accel_group_new();
	gtk_accel_group_add(accel_group,GDK_E,
			    GdkModifierType(GDK_CONTROL_MASK|GDK_MOD1_MASK),
			    GtkAccelFlags(0),
			    GTK_OBJECT(ListMenuArray[LM_EDIT_COMMON]),
			    "activate");
	_gtk_accel_group_attach(accel_group,G_OBJECT(MainWindow));
*/

	gtk_widget_show_all(ListMenu);
};

void list_menu_prepare() {
	tDownload *dwn=D4X_QUEUE->qv.last_selected;
	if (dwn==NULL) {
		for (int i=0;i<=LM_DELF;i++)
			gtk_widget_set_sensitive(ListMenuArray[i],FALSE);
		
		gtk_widget_set_sensitive(ListMenuArray[LM_SEARCH],FALSE);
		gtk_widget_set_sensitive(ListMenuArray[LM_ALT],FALSE);
	} else {
		for (int i=0;i<=LM_DELF;i++)
			gtk_widget_set_sensitive(ListMenuArray[i],TRUE);
		if (dwn->info->file.get()){
			gtk_widget_set_sensitive(ListMenuArray[LM_SEARCH],TRUE);
			gtk_widget_set_sensitive(ListMenuArray[LM_ALT],TRUE);
		}else{
			gtk_widget_set_sensitive(ListMenuArray[LM_SEARCH],FALSE);
			gtk_widget_set_sensitive(ListMenuArray[LM_ALT],FALSE);
		};
//		gtk_widget_set_sensitive(ListMenuArray[LM_EDIT_COMMON],FALSE);
		if (D4X_SEARCH_ENGINES.count()==0)
			gtk_widget_set_sensitive(ListMenuArray[LM_SEARCH],FALSE);
	};
};
