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
#include <gtk/gtk.h>
#include "list.h"
#include "saveload.h"
#include "addd.h"
#include "prefs.h"
#include "buttons.h"
#include "about.h"
#include "misc.h"
#include "../var.h"
#include "../ntlocale.h"

GtkWidget *ButtonsBar;
GtkWidget *AddButton;
GtkWidget *AddClipboardButton;
GtkWidget *DelButton;
GtkWidget *StopButton;
GtkWidget *ContinueButton;
GtkWidget *DelCompleteButton;
GtkWidget *OptionsButton;
GtkWidget *LogButton;
GtkWidget *DelallButton;
GtkWidget *UpButton;
GtkWidget *DownButton;
GtkWidget *Speed1Button;
GtkWidget *Speed2Button;
GtkWidget *Speed3Button;
GtkWidget *SaveButton;

GtkWidget *HandleBox;
extern gint SelectedRow;
extern gint SizeListOfDownloads;

tDialogWidget *AskDeleteAll=NULL;


GtkWidget *new_pixmap(char **xpm) {
	GdkBitmap *mask;
	GdkPixmap *pixmap;
	pixmap=make_pixmap_from_xpm(&mask,xpm);
	return (gtk_pixmap_new(pixmap,mask));
};

void del_all_downloads() {
	aa.del_all();
	if (AskDeleteAll)
		AskDeleteAll->done();
};

void ask_delete_all(...) {
	if (!AskDeleteAll) AskDeleteAll=new tDialogWidget;
	if (AskDeleteAll->init(_("Delete ALL downloads?"),_("Delete all?")))
		gtk_signal_connect(GTK_OBJECT(AskDeleteAll->ok_button),"clicked",GTK_SIGNAL_FUNC(del_all_downloads),NULL);
};

static void set_speed_buttons() {
	switch (CFG.SPEED_LIMIT) {
		case 1:
			{
				gtk_signal_emit_by_name (GTK_OBJECT (Speed1Button), "clicked");
				break;
			};
		case 2:
			{
				gtk_signal_emit_by_name (GTK_OBJECT (Speed2Button), "clicked");
				break;
			};
		case 3:
		default:
			{
				gtk_signal_emit_by_name (GTK_OBJECT (Speed3Button), "clicked");
				break;
			};
	};
};

static void set_speed_limit() {
	if (GTK_TOGGLE_BUTTON(Speed1Button)->active)
		CFG.SPEED_LIMIT=1;
	else {
		if (GTK_TOGGLE_BUTTON(Speed2Button)->active)
			CFG.SPEED_LIMIT=2;
		else
			CFG.SPEED_LIMIT=3;
	};
};

void init_buttons_bar() {
#include "pixmaps/add.xpm"
#include "pixmaps/add_clipboard.xpm"
#include "pixmaps/del.xpm"
#include "pixmaps/stop_bar.xpm"
#include "pixmaps/continue.xpm"
#include "pixmaps/del_com.xpm"
#include "pixmaps/prefs.xpm"
#include "pixmaps/openlog.xpm"
#include "pixmaps/del_all.xpm"
#include "pixmaps/up_bar.xpm"
#include "pixmaps/down_bar.xpm"
#include "pixmaps/speed1.xpm"
#include "pixmaps/speed2.xpm"
#include "pixmaps/speed3.xpm"
#include "pixmaps/save.xpm"

	//	HandleBox = gtk_handle_box_new ();
	ButtonsBar=gtk_toolbar_new (GTK_ORIENTATION_HORIZONTAL, GTK_TOOLBAR_ICONS);
	//	gtk_container_add (GTK_CONTAINER (HandleBox), ButtonsBar);
	AddButton=gtk_toolbar_append_item (GTK_TOOLBAR (ButtonsBar), "", _(" Add new download "), "", new_pixmap (add_xpm),
	                                   GTK_SIGNAL_FUNC (init_add_window), NULL);
	AddClipboardButton=gtk_toolbar_append_item (GTK_TOOLBAR (ButtonsBar), "", _(" Paste from clipboard "), "", new_pixmap (add_clipboard_xpm),
	                   GTK_SIGNAL_FUNC (init_add_clipboard_window), NULL);
	DelButton=gtk_toolbar_append_item (GTK_TOOLBAR (ButtonsBar), "", _(" Del downloads "), "", new_pixmap (del_xpm),
	                                   GTK_SIGNAL_FUNC (ask_delete_download), NULL);
	gtk_toolbar_append_space (GTK_TOOLBAR (ButtonsBar));
	ContinueButton=gtk_toolbar_append_item (GTK_TOOLBAR (ButtonsBar), "", _(" Continue/restart downloads "), "", new_pixmap (continue_xpm),
	                                        GTK_SIGNAL_FUNC (continue_downloads), NULL);
	StopButton=gtk_toolbar_append_item (GTK_TOOLBAR (ButtonsBar), "", _(" Stop downloads "), "", new_pixmap (stop_bar_xpm),
	                                    GTK_SIGNAL_FUNC (stop_downloads), NULL);
	DelCompleteButton=gtk_toolbar_append_item (GTK_TOOLBAR (ButtonsBar), "", _(" Del Completed downloads "), "", new_pixmap (del_com_xpm),
	                  GTK_SIGNAL_FUNC (ask_delete_completed_downloads), NULL);
	UpButton=gtk_toolbar_append_item (GTK_TOOLBAR (ButtonsBar), "", _(" Move up "), "", new_pixmap (up_bar_xpm),
	                                  GTK_SIGNAL_FUNC (move_download_up), NULL);
	DownButton=gtk_toolbar_append_item (GTK_TOOLBAR (ButtonsBar), "", _(" Move down "), "", new_pixmap (down_bar_xpm),
	                                    GTK_SIGNAL_FUNC (move_download_down), NULL);
	LogButton=gtk_toolbar_append_item (GTK_TOOLBAR (ButtonsBar), "", _(" View log "), "", new_pixmap (openlog_xpm),
	                                   GTK_SIGNAL_FUNC (open_log_for_selected), NULL);
	gtk_toolbar_append_space (GTK_TOOLBAR (ButtonsBar));
	OptionsButton=gtk_toolbar_append_item (GTK_TOOLBAR (ButtonsBar), "", _(" Options "), "", new_pixmap (prefs_xpm),
	                                       GTK_SIGNAL_FUNC (init_options_window), NULL);
	gtk_toolbar_append_space (GTK_TOOLBAR (ButtonsBar));
	Speed1Button=gtk_toolbar_append_element (GTK_TOOLBAR (ButtonsBar),
	             GTK_TOOLBAR_CHILD_RADIOBUTTON,
	             NULL,
	             "",
	             _(" Speed level one "),
	             "",
	             new_pixmap (speed1_xpm),
	             GTK_SIGNAL_FUNC (set_speed_limit),
	             NULL);
	Speed2Button=gtk_toolbar_append_element (GTK_TOOLBAR (ButtonsBar),
	             GTK_TOOLBAR_CHILD_RADIOBUTTON,
	             Speed1Button,
	             "",
	             _(" Speed level two "),
	             "",
	             new_pixmap (speed2_xpm),
	             GTK_SIGNAL_FUNC (set_speed_limit),
	             NULL);
	Speed3Button=gtk_toolbar_append_element (GTK_TOOLBAR (ButtonsBar),
	             GTK_TOOLBAR_CHILD_RADIOBUTTON,
	             Speed2Button,
	             "",
	             _(" Unlimited speed "),
	             "",
	             new_pixmap (speed3_xpm),
	             GTK_SIGNAL_FUNC (set_speed_limit),
	             NULL);
	gtk_toolbar_append_space (GTK_TOOLBAR (ButtonsBar));
	gtk_toolbar_append_space (GTK_TOOLBAR (ButtonsBar));
	gtk_toolbar_append_space (GTK_TOOLBAR (ButtonsBar));
	DelallButton=gtk_toolbar_append_item (GTK_TOOLBAR (ButtonsBar), "", _(" Delete all downloads "), "", new_pixmap (del_all_xpm),
	                                      GTK_SIGNAL_FUNC (ask_delete_all), NULL);
	SaveButton=gtk_toolbar_append_item (GTK_TOOLBAR (ButtonsBar), "", _(" Save list "), "", new_pixmap (save_xpm),
	                                      GTK_SIGNAL_FUNC (init_save_list), NULL);
	set_speed_buttons();
};

void prepare_buttons() {
	if (PausedList->count() || WaitList->count() || RunList->count() ||StopList->count() || CompleteList->count())
		gtk_widget_set_sensitive(DelallButton,TRUE);
	else
		gtk_widget_set_sensitive(DelallButton,FALSE);
	if (CompleteList->count())
		gtk_widget_set_sensitive(DelCompleteButton,TRUE);
	else
		gtk_widget_set_sensitive(DelCompleteButton,FALSE);
	GList *select=((GtkCList *)ListOfDownloads)->selection;
	if (select==NULL) {
		gtk_widget_set_sensitive(DelButton,FALSE);
		gtk_widget_set_sensitive(StopButton,FALSE);
		gtk_widget_set_sensitive(ContinueButton,FALSE);
		gtk_widget_set_sensitive(LogButton,FALSE);
		gtk_widget_set_sensitive(UpButton,FALSE);
		gtk_widget_set_sensitive(DownButton,FALSE);
	} else {
		gtk_widget_set_sensitive(DelButton,TRUE);
		gtk_widget_set_sensitive(StopButton,TRUE);
		gtk_widget_set_sensitive(ContinueButton,TRUE);
		gtk_widget_set_sensitive(LogButton,TRUE);
		gtk_widget_set_sensitive(UpButton,TRUE);
		gtk_widget_set_sensitive(DownButton,TRUE);
	};
};
