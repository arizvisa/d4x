/*	WebDownloader for X-Window
 *	Copyright (C) 1999-2000 Koshelev Maxim
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
#include "lod.h"

extern tMain aa;

void lmenu_ftp_search_go(){
	tDownload *tmp=list_of_downloads_last_selected();
	if (tmp)
		aa.ftp_search(tmp);
};

extern tMLog *MainLog;

GtkWidget *ListMenu;
GtkWidget *ListMenuArray[LM_LAST];

void copy_download_to_clipboard(){
	tDownload *dwn=list_of_downloads_last_selected();
	if (dwn->info){
		char *url=dwn->info->url();
		d4x_mw_clipboard_set(url);
		my_xclipboard_put(url);
		delete(url);
	};
};

GtkWidget *make_menu_item(char *name,char *accel,GdkPixmap *pixmap,GdkBitmap *bitmap,int size) {
	GtkWidget *menu_item=gtk_menu_item_new();
	GtkWidget *hbox=gtk_hbox_new(FALSE,3);
	GtkWidget *label = gtk_label_new(name);
	gtk_widget_set_usize(label,size,-1);
	GtkWidget *pix;
	gtk_misc_set_alignment(GTK_MISC(label),0,0.5);
	if (pixmap && bitmap)
		pix=gtk_pixmap_new(pixmap,bitmap);
	else {
		pix=gtk_hbox_new(FALSE,0);
		gtk_widget_set_usize(pix,10,-1);
	};
	gtk_box_pack_start(GTK_BOX(hbox),pix,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
	if (accel) {
		GtkWidget *accel_label=gtk_label_new(accel);
		GtkWidget *hbox1=gtk_hbox_new(FALSE,3);
		gtk_box_pack_start(GTK_BOX(hbox1),accel_label,FALSE,FALSE,0);
		gtk_box_pack_end(GTK_BOX(hbox),hbox1,FALSE,FALSE,0);
		gtk_label_set_justify(GTK_LABEL(accel_label),GTK_JUSTIFY_RIGHT);
	};
	gtk_container_add(GTK_CONTAINER(menu_item),hbox);
	gtk_widget_show(pix);
	gtk_widget_show(label);
	gtk_widget_show(hbox);
	gtk_widget_show(menu_item);
	return menu_item;
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
	int MAX_STR_LENGTH=0;
	GtkStyle *style =  gtk_widget_get_default_style();
	char *names[]={"View log",
		       "Stop",
		       "Continue downloads",
		       "Properties",
		       "Common properties",
		       "Delete downloads",
		       "Delete completed",
		       "Delete failed",
		       "Move up",
		       "Move down",
		       "Set limitation",
		       "FTP search"
	};
	for(unsigned int i=0;i<sizeof(names)/sizeof(char *);i++){
		int size=gdk_string_width(style->font,_(names[i]));
		if (size>MAX_STR_LENGTH)
			MAX_STR_LENGTH=size;
	};
	MAX_STR_LENGTH+=10;
		
	ListMenu=gtk_menu_new();
	pixmap=make_pixmap_from_xpm(&bitmap,logmini_xpm);
	menu_item=make_menu_item(_("View log"),(char *)NULL,pixmap,bitmap,MAX_STR_LENGTH);
	gtk_menu_append(GTK_MENU(ListMenu),menu_item);
	ListMenuArray[LM_LOG]=menu_item;
	gtk_signal_connect(GTK_OBJECT(menu_item),"activate",GTK_SIGNAL_FUNC(list_of_downloads_open_logs),NULL);

	pixmap=make_pixmap_from_xpm(&bitmap,stopmini_xpm);
	menu_item=make_menu_item(_("Stop"),"Alt+S",pixmap,bitmap,MAX_STR_LENGTH);
	gtk_widget_set_usize(menu_item,130,-1);
	gtk_menu_append(GTK_MENU(ListMenu),menu_item);
	ListMenuArray[LM_STOP]=menu_item;
	gtk_signal_connect(GTK_OBJECT(menu_item),"activate",GTK_SIGNAL_FUNC(stop_downloads),NULL);

	pixmap=make_pixmap_from_xpm(&bitmap,runmini_xpm);
	menu_item=make_menu_item(_("Continue downloads"),(char *)NULL,pixmap,bitmap,MAX_STR_LENGTH);
	gtk_menu_append(GTK_MENU(ListMenu),menu_item);
	ListMenuArray[LM_CONTINUE]=menu_item;
	gtk_signal_connect(GTK_OBJECT(menu_item),"activate",GTK_SIGNAL_FUNC(continue_downloads),NULL);

	pixmap=make_pixmap_from_xpm(&bitmap,runmini_xpm);
	menu_item=make_menu_item(_("Copy"),(char *)NULL,(GdkPixmap *)NULL,(GdkPixmap *)NULL,MAX_STR_LENGTH);
	gtk_menu_append(GTK_MENU(ListMenu),menu_item);
	ListMenuArray[LM_COPY]=menu_item;
	gtk_signal_connect(GTK_OBJECT(menu_item),"activate",GTK_SIGNAL_FUNC(copy_download_to_clipboard),NULL);

	menu_item=gtk_menu_item_new();
	gtk_widget_set_sensitive(menu_item,FALSE);
	gtk_menu_append(GTK_MENU(ListMenu),menu_item);

	menu_item=make_menu_item(_("Properties"),"Alt+E",(GdkPixmap *)NULL,(GdkPixmap *)NULL,MAX_STR_LENGTH);
	gtk_menu_append(GTK_MENU(ListMenu),menu_item);
	ListMenuArray[LM_EDIT]=menu_item;
	gtk_signal_connect(GTK_OBJECT(menu_item),"activate",GTK_SIGNAL_FUNC(open_edit_for_selected),NULL);

	menu_item=make_menu_item(_("Common properties"),"Ctrl+Alt+E",(GdkPixmap *)NULL,(GdkPixmap *)NULL,MAX_STR_LENGTH);
	gtk_menu_append(GTK_MENU(ListMenu),menu_item);
	ListMenuArray[LM_EDIT_COMMON]=menu_item;
	gtk_signal_connect(GTK_OBJECT(menu_item),"activate",GTK_SIGNAL_FUNC(select_options_window_init),NULL);

	pixmap=make_pixmap_from_xpm(&bitmap,delmini_xpm);
	menu_item=make_menu_item(_("Delete downloads"),"Alt+C",pixmap,bitmap,MAX_STR_LENGTH);
	gtk_menu_append(GTK_MENU(ListMenu),menu_item);
	ListMenuArray[LM_DEL]=menu_item;
	gtk_signal_connect(GTK_OBJECT(menu_item),"activate",GTK_SIGNAL_FUNC(ask_delete_download),NULL);

	pixmap=make_pixmap_from_xpm(&bitmap,delcommini_xpm);
	menu_item=make_menu_item(_("Delete completed"),(char *)NULL,pixmap,bitmap,MAX_STR_LENGTH);
	gtk_menu_append(GTK_MENU(ListMenu),menu_item);
	ListMenuArray[LM_DELC]=menu_item;
	gtk_signal_connect(GTK_OBJECT(menu_item),"activate",GTK_SIGNAL_FUNC(ask_delete_completed_downloads),NULL);


	menu_item=make_menu_item(_("Delete failed"),(char *)NULL,(GdkPixmap *)NULL,(GdkPixmap *)NULL,MAX_STR_LENGTH);
	gtk_menu_append(GTK_MENU(ListMenu),menu_item);
	ListMenuArray[LM_DELF]=menu_item;
	gtk_signal_connect(GTK_OBJECT(menu_item),"activate",GTK_SIGNAL_FUNC(ask_delete_fataled_downloads),NULL);

	menu_item=gtk_menu_item_new();
	gtk_widget_set_sensitive(menu_item,FALSE);
	gtk_menu_append(GTK_MENU(ListMenu),menu_item);

	pixmap=make_pixmap_from_xpm(&bitmap,upmini_xpm);
	menu_item=make_menu_item(_("Move up"),"Shift+Up",pixmap,bitmap,MAX_STR_LENGTH);
	gtk_menu_append(GTK_MENU(ListMenu),menu_item);
	ListMenuArray[LM_MOVEUP]=menu_item;
	gtk_signal_connect(GTK_OBJECT(menu_item),"activate",GTK_SIGNAL_FUNC(list_of_downloads_move_up),NULL);

	pixmap=make_pixmap_from_xpm(&bitmap,downmini_xpm);
	menu_item=make_menu_item(_("Move down"),"Shift+Down",pixmap,bitmap,MAX_STR_LENGTH);
	gtk_widget_set_usize(menu_item,200,-1);
	gtk_menu_append(GTK_MENU(ListMenu),menu_item);
	ListMenuArray[LM_MOVEDOWN]=menu_item;
	gtk_signal_connect(GTK_OBJECT(menu_item),"activate",GTK_SIGNAL_FUNC(list_of_downloads_move_down),NULL);

	menu_item=make_menu_item(_("Set limitation"),(char *)NULL,(GdkPixmap *)NULL,(GdkPixmap *)NULL,MAX_STR_LENGTH);
	gtk_menu_append(GTK_MENU(ListMenu),menu_item);
	ListMenuArray[LM_SET_LIMIT]=menu_item;
	gtk_signal_connect(GTK_OBJECT(menu_item),"activate",GTK_SIGNAL_FUNC(set_limit_to_download),NULL);

	menu_item=make_menu_item(_("FTP search"),(char *)NULL,(GdkPixmap *)NULL,(GdkPixmap *)NULL,MAX_STR_LENGTH);
	gtk_menu_append(GTK_MENU(ListMenu),menu_item);
	ListMenuArray[LM_SEARCH]=menu_item;
	gtk_signal_connect(GTK_OBJECT(menu_item),"activate",GTK_SIGNAL_FUNC(lmenu_ftp_search_go),NULL);

	GtkAccelGroup *accel_group = gtk_accel_group_new();
	gtk_accel_group_add(accel_group,GDK_E,
			    GdkModifierType(GDK_CONTROL_MASK|GDK_MOD1_MASK),
			    GtkAccelFlags(0),
			    GTK_OBJECT(ListMenuArray[LM_EDIT_COMMON]),
			    "activate");
	gtk_accel_group_attach(accel_group,GTK_OBJECT(MainWindow));

	gtk_widget_show_all(ListMenu);
};

void list_menu_prepare() {
	tDownload *dwn=list_of_downloads_last_selected();
	if (dwn==NULL) {
		for (int i=0;i<=LM_SET_LIMIT;i++)
			gtk_widget_set_sensitive(ListMenuArray[i],FALSE);
		
		gtk_widget_set_sensitive(ListMenuArray[LM_SEARCH],FALSE);
	} else {
		for (int i=0;i<=LM_SET_LIMIT;i++)
			gtk_widget_set_sensitive(ListMenuArray[i],TRUE);
		if (dwn->finfo.size>0 && dwn->info->file.get())
			gtk_widget_set_sensitive(ListMenuArray[LM_SEARCH],TRUE);
		else
			gtk_widget_set_sensitive(ListMenuArray[LM_SEARCH],FALSE);
	};
};
