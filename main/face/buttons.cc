/*	WebDownloader for X-Window
 *	Copyright (C) 1999 Koshelev Maxim
 *	This Program is free but not GPL!!! You can't modify it
 *	without agreement with author. You can't distribute modified
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
#include "colors.h"
#include "dndtrash.h"
#include "../var.h"
#include "../ntlocale.h"

GtkWidget *ButtonsBar;
GtkWidget *buttons_array[BUTTON_DND_TRASH+1];

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

void set_speed_buttons() {
	switch (CFG.SPEED_LIMIT) {
		case 1:
			{
				gtk_signal_emit_by_name (GTK_OBJECT (buttons_array[BUTTON_SPEED1]), "clicked");
				break;
			};
		case 2:
			{
				gtk_signal_emit_by_name (GTK_OBJECT (buttons_array[BUTTON_SPEED2]), "clicked");
				break;
			};
		case 3:
		default:
			{
				gtk_signal_emit_by_name (GTK_OBJECT (buttons_array[BUTTON_SPEED3]), "clicked");
				break;
			};
	};
};

static void set_speed_limit() {
	if (GTK_TOGGLE_BUTTON(buttons_array[BUTTON_SPEED1])->active)
		CFG.SPEED_LIMIT=1;
	else {
		if (GTK_TOGGLE_BUTTON(buttons_array[BUTTON_SPEED2])->active)
			CFG.SPEED_LIMIT=2;
		else
			CFG.SPEED_LIMIT=3;
	};
};

static void dnd_trash_toggle(){
	if (GTK_TOGGLE_BUTTON(buttons_array[BUTTON_DND_TRASH])->active)
		dnd_trash_init();
	else
		dnd_trash_destroy();
};

void set_dndtrash_button(){
	if (CFG.DND_TRASH ) {
		if (!(GTK_TOGGLE_BUTTON(buttons_array[BUTTON_DND_TRASH])->active)){
			GTK_TOGGLE_BUTTON(buttons_array[BUTTON_DND_TRASH])->active=FALSE;
			gtk_signal_emit_by_name (GTK_OBJECT (buttons_array[BUTTON_DND_TRASH]), "clicked");
		};
	}else{
		if (GTK_TOGGLE_BUTTON(buttons_array[BUTTON_DND_TRASH])->active){
			GTK_TOGGLE_BUTTON(buttons_array[BUTTON_DND_TRASH])->active=TRUE;
			gtk_signal_emit_by_name (GTK_OBJECT (buttons_array[BUTTON_DND_TRASH]), "clicked");
		};
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
#include "pixmaps/dndtrash_bar.xpm"

	//	HandleBox = gtk_handle_box_new ();
	ButtonsBar=gtk_toolbar_new (GTK_ORIENTATION_HORIZONTAL, GTK_TOOLBAR_ICONS);
	//	gtk_container_add (GTK_CONTAINER (HandleBox), ButtonsBar);
	buttons_array[BUTTON_ADD]=gtk_toolbar_append_item (GTK_TOOLBAR (ButtonsBar), "", _(" Add new download "), "", new_pixmap (add_xpm),
	                                   GTK_SIGNAL_FUNC (init_add_window), NULL);
	buttons_array[BUTTON_ADD_CLIPBOARD]=gtk_toolbar_append_item (GTK_TOOLBAR (ButtonsBar), "", _(" Paste from clipboard "), "", new_pixmap (add_clipboard_xpm),
	                   GTK_SIGNAL_FUNC (init_add_clipboard_window), NULL);
	buttons_array[BUTTON_DEL]=gtk_toolbar_append_item (GTK_TOOLBAR (ButtonsBar), "", _(" Del downloads "), "", new_pixmap (del_xpm),
	                                   GTK_SIGNAL_FUNC (ask_delete_download), NULL);
	gtk_toolbar_append_space (GTK_TOOLBAR (ButtonsBar));
	buttons_array[BUTTON_CONTINUE]=gtk_toolbar_append_item (GTK_TOOLBAR (ButtonsBar), "", _(" Continue/restart downloads "), "", new_pixmap (continue_xpm),
	                                        GTK_SIGNAL_FUNC (continue_downloads), NULL);
	buttons_array[BUTTON_STOP]=gtk_toolbar_append_item (GTK_TOOLBAR (ButtonsBar), "", _(" Stop downloads "), "", new_pixmap (stop_bar_xpm),
	                                    GTK_SIGNAL_FUNC (stop_downloads), NULL);
	buttons_array[BUTTON_DEL_COMPLETED]=gtk_toolbar_append_item (GTK_TOOLBAR (ButtonsBar), "", _(" Del Completed downloads "), "", new_pixmap (del_com_xpm),
	                  GTK_SIGNAL_FUNC (ask_delete_completed_downloads), NULL);
	buttons_array[BUTTON_UP]=gtk_toolbar_append_item (GTK_TOOLBAR (ButtonsBar), "", _(" Move up "), "", new_pixmap (up_bar_xpm),
	                                  GTK_SIGNAL_FUNC (list_of_downloads_move_selected_up), NULL);
	buttons_array[BUTTON_DOWN]=gtk_toolbar_append_item (GTK_TOOLBAR (ButtonsBar), "", _(" Move down "), "", new_pixmap (down_bar_xpm),
	                                    GTK_SIGNAL_FUNC (list_of_downloads_move_selected_down), NULL);
	buttons_array[BUTTON_LOG]=gtk_toolbar_append_item (GTK_TOOLBAR (ButtonsBar), "", _(" View log "), "", new_pixmap (openlog_xpm),
	                                   GTK_SIGNAL_FUNC (open_log_for_selected), NULL);
	gtk_toolbar_append_space (GTK_TOOLBAR (ButtonsBar));
	buttons_array[BUTTON_OPTIONS]=gtk_toolbar_append_item (GTK_TOOLBAR (ButtonsBar), "", _(" Options "), "", new_pixmap (prefs_xpm),
	                                       GTK_SIGNAL_FUNC (init_options_window), NULL);
	gtk_toolbar_append_space (GTK_TOOLBAR (ButtonsBar));
	buttons_array[BUTTON_SPEED1]=gtk_toolbar_append_element (GTK_TOOLBAR (ButtonsBar),
	             GTK_TOOLBAR_CHILD_RADIOBUTTON,
	             NULL,
	             "",
	             _(" Speed level one "),
	             "",
	             new_pixmap (speed1_xpm),
	             GTK_SIGNAL_FUNC (set_speed_limit),
	             NULL);
	buttons_array[BUTTON_SPEED2]=gtk_toolbar_append_element (GTK_TOOLBAR (ButtonsBar),
	             GTK_TOOLBAR_CHILD_RADIOBUTTON,
	             buttons_array[BUTTON_SPEED1],
	             "",
	             _(" Speed level two "),
	             "",
	             new_pixmap (speed2_xpm),
	             GTK_SIGNAL_FUNC (set_speed_limit),
	             NULL);
	buttons_array[BUTTON_SPEED3]=gtk_toolbar_append_element (GTK_TOOLBAR (ButtonsBar),
	             GTK_TOOLBAR_CHILD_RADIOBUTTON,
	             buttons_array[BUTTON_SPEED2],
	             "",
	             _(" Unlimited speed "),
	             "",
	             new_pixmap (speed3_xpm),
	             GTK_SIGNAL_FUNC (set_speed_limit),
	             NULL);
	gtk_toolbar_append_space (GTK_TOOLBAR (ButtonsBar));
	gtk_toolbar_append_space (GTK_TOOLBAR (ButtonsBar));
	gtk_toolbar_append_space (GTK_TOOLBAR (ButtonsBar));
	buttons_array[BUTTON_DEL_ALL]=gtk_toolbar_append_item (GTK_TOOLBAR (ButtonsBar), "", _(" Delete all downloads "), "", new_pixmap (del_all_xpm),
	                                      GTK_SIGNAL_FUNC (ask_delete_all), NULL);
	buttons_array[BUTTON_SAVE]=gtk_toolbar_append_item (GTK_TOOLBAR (ButtonsBar), "", _(" Save list "), "", new_pixmap (save_xpm),
	                                      GTK_SIGNAL_FUNC (init_save_list), NULL);
	gtk_toolbar_append_space (GTK_TOOLBAR (ButtonsBar));
	buttons_array[BUTTON_DND_TRASH]=gtk_toolbar_append_element (GTK_TOOLBAR (ButtonsBar),
	             GTK_TOOLBAR_CHILD_TOGGLEBUTTON,
	             NULL,
	             "",
	             _(" DnD trash "),
	             "",
	             new_pixmap (dndtrash_bar_xpm),
	             GTK_SIGNAL_FUNC (dnd_trash_toggle),
	             NULL);
	set_speed_buttons();
	set_dndtrash_button();
	GtkTooltips *tooltips=((GtkToolbar *)(ButtonsBar))->tooltips;
    gtk_tooltips_force_window(tooltips);
	GtkStyle *current_style =gtk_style_copy(gtk_widget_get_style(tooltips->tip_window));
	current_style->bg[GTK_STATE_NORMAL] = LYELLOW;
	gtk_widget_set_style(tooltips->tip_window, current_style);

};

void prepare_buttons() {
	if (PausedList->count() || WaitList->count() || RunList->count() ||StopList->count() || CompleteList->count())
		gtk_widget_set_sensitive(buttons_array[BUTTON_DEL_ALL],TRUE);
	else
		gtk_widget_set_sensitive(buttons_array[BUTTON_DEL_ALL],FALSE);
	if (CompleteList->count())
		gtk_widget_set_sensitive(buttons_array[BUTTON_DEL_COMPLETED],TRUE);
	else
		gtk_widget_set_sensitive(buttons_array[BUTTON_DEL_COMPLETED],FALSE);
	GList *select=((GtkCList *)ListOfDownloads)->selection;
	if (select==NULL) {
		gtk_widget_set_sensitive(buttons_array[BUTTON_DEL],FALSE);
		gtk_widget_set_sensitive(buttons_array[BUTTON_STOP],FALSE);
		gtk_widget_set_sensitive(buttons_array[BUTTON_CONTINUE],FALSE);
		gtk_widget_set_sensitive(buttons_array[BUTTON_LOG],FALSE);
		gtk_widget_set_sensitive(buttons_array[BUTTON_UP],FALSE);
		gtk_widget_set_sensitive(buttons_array[BUTTON_DOWN],FALSE);
	} else {
		gtk_widget_set_sensitive(buttons_array[BUTTON_DEL],TRUE);
		gtk_widget_set_sensitive(buttons_array[BUTTON_STOP],TRUE);
		gtk_widget_set_sensitive(buttons_array[BUTTON_CONTINUE],TRUE);
		gtk_widget_set_sensitive(buttons_array[BUTTON_LOG],TRUE);
		gtk_widget_set_sensitive(buttons_array[BUTTON_UP],TRUE);
		gtk_widget_set_sensitive(buttons_array[BUTTON_DOWN],TRUE);
	};
};
