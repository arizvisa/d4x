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
#include "../savelog.h"
#include "../ntlocale.h"

GtkWidget *ButtonsBar;
GtkWidget *buttons_array[BUTTON_LAST];

GtkWidget *HandleBox;

tConfirmedDialog *AskDeleteAll=(tConfirmedDialog *)NULL;


GtkWidget *new_pixmap(char **xpm) {
	GdkBitmap *mask;
	GdkPixmap *pixmap;
	pixmap=make_pixmap_from_xpm(&mask,xpm);
	return (gtk_pixmap_new(pixmap,mask));
};

void del_all_downloads(){
	aa.del_all();
	if (AskDeleteAll)
		AskDeleteAll->done();
};

static void _ask_delete_all_check_(GtkWidget *widget, tConfirmedDialog *parent){
	CFG.CONFIRM_DELETE_ALL=!(GTK_TOGGLE_BUTTON(parent->check)->active);
	del_all_downloads();
};

void ask_delete_all(...) {
	if (!AskDeleteAll) AskDeleteAll=new tConfirmedDialog;
	if (CFG.CONFIRM_DELETE_ALL){
		if (AskDeleteAll->init(_("Delete ALL downloads?"),_("Delete all?")))
			gtk_signal_connect(GTK_OBJECT(AskDeleteAll->ok_button),
					   "clicked",
					   GTK_SIGNAL_FUNC(_ask_delete_all_check_),
					   AskDeleteAll);
	}else
		del_all_downloads();
};

void buttons_speed_set_text(){
	char text[MAX_LEN];
	sprintf(text,"%s (%i B/s)",_(" Speed level one "),CFG.SPEED_LIMIT_1);
	gtk_tooltips_set_tip(GTK_TOOLBAR(ButtonsBar)->tooltips,
			     buttons_array[BUTTON_SPEED1],
			     text,(char*) NULL);
	sprintf(text,"%s (%i B/s)",_(" Speed level two "),CFG.SPEED_LIMIT_2);
	gtk_tooltips_set_tip(GTK_TOOLBAR(ButtonsBar)->tooltips,
			     buttons_array[BUTTON_SPEED2],
			     text,(char *)NULL);
};


void set_speed_buttons() {
	switch (CFG.SPEED_LIMIT) {
		case 1:	{
				gtk_signal_emit_by_name (GTK_OBJECT (buttons_array[BUTTON_SPEED1]), "clicked");
				break;
			};
		case 2:	{
				gtk_signal_emit_by_name (GTK_OBJECT (buttons_array[BUTTON_SPEED2]), "clicked");
				break;
			};
		case 3:
		default:{
				gtk_signal_emit_by_name (GTK_OBJECT (buttons_array[BUTTON_SPEED3]), "clicked");
				break;
			};
	};
	main_menu_speed_prepare();
	dnd_trash_menu_prepare();
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
	main_menu_speed_prepare();
	dnd_trash_menu_prepare();
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

gint buttons_save_press(GtkWidget *widget,GdkEventButton *event, gint code){
	if (event->button==3){
		gtk_signal_emit_by_name(GTK_OBJECT(widget),"pressed",NULL);
		return TRUE;
	};
	return FALSE;
};

gint buttons_save_release(GtkButton *button,GdkEventButton *event,gint code){
	if (event->button==3){
		button->button_down=FALSE;		
		GtkStateType new_state = (button->in_button ? GTK_STATE_PRELIGHT : GTK_STATE_NORMAL);
		if (GTK_WIDGET_STATE(button)!=new_state){
			gtk_widget_set_state(GTK_WIDGET(button),new_state);
			gtk_widget_queue_draw(GTK_WIDGET(button));
		};
		switch (code){
		case BUTTON_SAVE:
			save_list();
			break;
		case BUTTON_OPTIONS:
			open_edit_for_selected();
			break;
		};
		return TRUE;
	};
	return FALSE;
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
	                                  GTK_SIGNAL_FUNC (list_of_downloads_move_up), NULL);
	buttons_array[BUTTON_DOWN]=gtk_toolbar_append_item (GTK_TOOLBAR (ButtonsBar), "", _(" Move down "), "", new_pixmap (down_bar_xpm),
	                                    GTK_SIGNAL_FUNC (list_of_downloads_move_down), NULL);
	buttons_array[BUTTON_LOG]=gtk_toolbar_append_item (GTK_TOOLBAR (ButtonsBar), "", _(" View log "), "", new_pixmap (openlog_xpm),
	                                   GTK_SIGNAL_FUNC (list_of_downloads_open_logs), NULL);
	gtk_toolbar_append_space (GTK_TOOLBAR (ButtonsBar));
	buttons_array[BUTTON_OPTIONS]=gtk_toolbar_append_item (GTK_TOOLBAR (ButtonsBar), "", _(" Options "), "", new_pixmap (prefs_xpm),
	                                       GTK_SIGNAL_FUNC (d4x_prefs_init), NULL);
	gtk_toolbar_append_space (GTK_TOOLBAR (ButtonsBar));
	buttons_array[BUTTON_SPEED1]=gtk_toolbar_append_element (GTK_TOOLBAR (ButtonsBar),
	             GTK_TOOLBAR_CHILD_RADIOBUTTON,
	             (GtkWidget *)NULL,
	             "",
	             _(" Speed level one "),
	             "",
	             new_pixmap (speed1_xpm),
	             GTK_SIGNAL_FUNC (set_speed_limit),
	             (GtkWidget *)NULL);
	buttons_array[BUTTON_SPEED2]=gtk_toolbar_append_element (GTK_TOOLBAR (ButtonsBar),
	             GTK_TOOLBAR_CHILD_RADIOBUTTON,
	             buttons_array[BUTTON_SPEED1],
	             "",
	             _(" Speed level two "),
	             "",
	             new_pixmap (speed2_xpm),
	             GTK_SIGNAL_FUNC (set_speed_limit),
	             (GtkWidget *)NULL);
	buttons_array[BUTTON_SPEED3]=gtk_toolbar_append_element (GTK_TOOLBAR (ButtonsBar),
	             GTK_TOOLBAR_CHILD_RADIOBUTTON,
	             buttons_array[BUTTON_SPEED2],
	             "",
	             _(" Unlimited speed "),
	             "",
	             new_pixmap (speed3_xpm),
	             GTK_SIGNAL_FUNC (set_speed_limit),
	             (GtkWidget *)NULL);
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
	             (GtkWidget *)NULL,
	             "",
	             _(" DnD basket "),
	             "",
	             new_pixmap (dndtrash_bar_xpm),
	             GTK_SIGNAL_FUNC (dnd_trash_toggle),
	             (GtkWidget *)NULL);
	gtk_signal_connect (GTK_OBJECT (buttons_array[BUTTON_SAVE]), "button_press_event",
			    GTK_SIGNAL_FUNC (buttons_save_press), GINT_TO_POINTER(BUTTON_SAVE));
	gtk_signal_connect (GTK_OBJECT (buttons_array[BUTTON_SAVE]), "button_release_event",
			    GTK_SIGNAL_FUNC (buttons_save_release),GINT_TO_POINTER(BUTTON_SAVE));
	gtk_signal_connect (GTK_OBJECT (buttons_array[BUTTON_OPTIONS]), "button_press_event",
			    GTK_SIGNAL_FUNC (buttons_save_press), GINT_TO_POINTER(BUTTON_OPTIONS));
	gtk_signal_connect (GTK_OBJECT (buttons_array[BUTTON_OPTIONS]), "button_release_event",
			    GTK_SIGNAL_FUNC (buttons_save_release),GINT_TO_POINTER(BUTTON_OPTIONS));
	set_speed_buttons();
	set_dndtrash_button();
	GtkTooltips *tooltips=((GtkToolbar *)(ButtonsBar))->tooltips;
	gtk_tooltips_force_window(tooltips);
	GtkStyle *current_style =gtk_style_copy(gtk_widget_get_style(tooltips->tip_window));
	gdk_font_unref(current_style->font);
	current_style->font = MainWindow->style->font;
	current_style->bg[GTK_STATE_NORMAL] = LYELLOW;
	gtk_widget_set_style(tooltips->tip_window, current_style);
	gtk_widget_show(ButtonsBar);
	buttons_speed_set_text();
	buttons_cfg_init();
};

void buttons_cfg_init(){
	if (CFG.BUTTONS_ADD){
		for (int i=BUTTON_ADD;i<=BUTTON_DEL;i++)
			gtk_widget_show(buttons_array[i]);
	}else{
		for (int i=BUTTON_ADD;i<=BUTTON_DEL;i++)
			gtk_widget_hide(buttons_array[i]);
	};
	if (CFG.BUTTONS_MAN){
		for (int i=BUTTON_STOP;i<=BUTTON_LOG;i++)
			gtk_widget_show(buttons_array[i]);
	}else{
		for (int i=BUTTON_STOP;i<=BUTTON_LOG;i++)
			gtk_widget_hide(buttons_array[i]);
	};

	if (CFG.BUTTONS_SPEED){
		for (int i=BUTTON_SPEED1;i<=BUTTON_SPEED3;i++)
			gtk_widget_show(buttons_array[i]);
	}else{
		for (int i=BUTTON_SPEED1;i<=BUTTON_SPEED3;i++)
			gtk_widget_hide(buttons_array[i]);
	};
	if (CFG.BUTTONS_MISC){
		for (int i=BUTTON_OPTIONS;i<=BUTTON_DND_TRASH;i++)
			gtk_widget_show(buttons_array[i]);
	}else{
		for (int i=BUTTON_OPTIONS;i<=BUTTON_DND_TRASH;i++)
			gtk_widget_hide(buttons_array[i]);
	};
	if (CFG.BUTTONS_ADD || CFG.BUTTONS_SPEED ||
	    CFG.BUTTONS_MAN || CFG.BUTTONS_MISC){
		gtk_widget_show(ButtonsBar);
	}else{
		gtk_widget_hide(ButtonsBar);
	};
};

void prepare_buttons() {
	if (amount_of_downloads_in_queues())
		gtk_widget_set_sensitive(buttons_array[BUTTON_DEL_ALL],TRUE);
	else
		gtk_widget_set_sensitive(buttons_array[BUTTON_DEL_ALL],FALSE);
	if (list_of_downloads_sel()) {
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
