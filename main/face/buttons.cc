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
#include "lod.h"
#include "../var.h"
#include "../savelog.h"
#include "../ntlocale.h"
#include "../config.h"

enum BUTTONS_BITS{
	BBIT_ADD = 1,
	BBIT_ADD_CLIPBOARD = 1<<1,
	BBIT_DEL = 1<<2,
	BBIT_STOP = 1<<3,
	BBIT_CONTINUE = 1<<4,
	BBIT_DEL_COMPLETED = 1<<5,
	BBIT_UP = 1<<6,
	BBIT_DOWN = 1<<7,
	BBIT_LOG = 1<<8,
	BBIT_SPEED1 = 1<<9,
	BBIT_SPEED2 = 1<<10,
	BBIT_SPEED3 = 1<<11,
	BBIT_OPTIONS = 1<<12,
	BBIT_DEL_ALL = 1<<13,
	BBIT_SAVE = 1<<14,
	BBIT_LOAD = 1<<15,
	BBIT_DND_TRASH = 1<<16
};

char *BUTTONS_TEXT[]={
	N_(" Add new download "),
	N_(" Paste from clipboard "),
	N_(" Del downloads "),
	N_(" Stop downloads "),
	N_(" Continue/restart downloads "),
	N_(" Del Completed downloads "),
	N_(" Move up "),
	N_(" Move down "),
	N_(" View log "),
	N_(" Speed level one "),
	N_(" Speed level two "),
	N_(" Unlimited speed "),
	N_(" Options "),
	N_(" Delete all downloads "),
	N_(" Save list "),
	N_(" Load list "),
	N_(" DnD basket "),
	N_(" Mode of percentage "),
	N_(" Configure buttons ")
};

GtkWidget *ButtonsBar;
GtkWidget *buttons_array[BUTTON_LAST];

GtkWidget *BConfigWindow=(GtkWidget *)NULL;
GtkWidget *BConfigButtons[BUTTON_LAST];


tConfirmedDialog *AskDeleteAll=(tConfirmedDialog *)NULL;


void buttons_configure_close(){
	if (BConfigWindow)
		gtk_widget_destroy(BConfigWindow);
	BConfigWindow=(GtkWidget *)NULL;
};

void buttons_flags_init(){
	int none_visible=1;
	for (int i=0;i<BUTTON_CONFIGURE;i++){
		if (CFG.BUTTONS_FLAGS & (1<<i)){
			none_visible=0;
			gtk_widget_show(buttons_array[i]);
		}else
			gtk_widget_hide(buttons_array[i]);
	};
	if (none_visible){
		gtk_widget_hide(ButtonsBar);
	}else{
		gtk_widget_show(ButtonsBar);
	};
};

void buttons_configure_apply(){
	int old_flags=CFG.BUTTONS_FLAGS;
	CFG.BUTTONS_FLAGS=0x0FFFFFF;
	for (int i=0;i<BUTTON_CONFIGURE;i++){
		if (!(GTK_TOGGLE_BUTTON(BConfigButtons[i])->active)){
			CFG.BUTTONS_FLAGS^=(1<<i);
		};
	};
	buttons_flags_init();
	if (old_flags!=CFG.BUTTONS_FLAGS)
		save_config();
};

void buttons_configure_ok(){
	buttons_configure_apply();
	buttons_configure_close();
};

void buttons_configure(){
	/* configure ALL buttons here */
	if (BConfigWindow) {
		gdk_window_show(BConfigWindow->window);
		return;
	};
	GtkWidget *vbox=gtk_vbox_new(FALSE,0);
	for (int i=BUTTON_ADD;i<BUTTON_CONFIGURE;i++){
		BConfigButtons[i]=gtk_check_button_new_with_label(_(BUTTONS_TEXT[i]));
		gtk_box_pack_start(GTK_BOX(vbox),
				   BConfigButtons[i],
				   FALSE,FALSE,0);
		GTK_TOGGLE_BUTTON(BConfigButtons[i])->active= (CFG.BUTTONS_FLAGS & (1<<i)?TRUE:FALSE);
			
	};
	BConfigWindow=gtk_window_new(GTK_WINDOW_DIALOG);
	gtk_window_set_wmclass(GTK_WINDOW(BConfigWindow),
			       "D4X_Buttons","D4X");
	gtk_window_set_title(GTK_WINDOW (BConfigWindow),
			     _(BUTTONS_TEXT[BUTTON_CONFIGURE]));
	gtk_window_set_position(GTK_WINDOW(BConfigWindow),
				GTK_WIN_POS_CENTER);
	gtk_window_set_policy (GTK_WINDOW(BConfigWindow),FALSE,FALSE,FALSE);
	gtk_widget_set_usize(BConfigWindow,-1,400);
	gtk_container_border_width(GTK_CONTAINER(BConfigWindow),5);

	GtkWidget *scroll_window=gtk_scrolled_window_new((GtkAdjustment *)NULL,
							 (GtkAdjustment *)NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll_window),
	                                GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);
	GtkWidget *viewport=gtk_viewport_new((GtkAdjustment *)NULL,
					     (GtkAdjustment *)NULL);
	gtk_container_add(GTK_CONTAINER(viewport),vbox);
	gtk_container_add(GTK_CONTAINER(scroll_window),viewport);

	vbox=gtk_vbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),scroll_window,TRUE,TRUE,0);

	GtkWidget *hbox=gtk_hbutton_box_new();
	gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);
	GtkWidget *ok_button=gtk_button_new_with_label(_("Ok"));
	GtkWidget *apply_button=gtk_button_new_with_label(_("Apply"));
	GtkWidget *cancel_button=gtk_button_new_with_label(_("Cancel"));
	GTK_WIDGET_SET_FLAGS(ok_button,GTK_CAN_DEFAULT);
	GTK_WIDGET_SET_FLAGS(apply_button,GTK_CAN_DEFAULT);
	GTK_WIDGET_SET_FLAGS(cancel_button,GTK_CAN_DEFAULT);
	gtk_box_pack_end(GTK_BOX(hbox),ok_button,FALSE,FALSE,0);
	gtk_box_pack_end(GTK_BOX(hbox),apply_button,FALSE,FALSE,0);
	gtk_box_pack_end(GTK_BOX(hbox),cancel_button,FALSE,FALSE,0);
	gtk_signal_connect(GTK_OBJECT(cancel_button),
			   "clicked",
			   GTK_SIGNAL_FUNC(buttons_configure_close), NULL);
	gtk_signal_connect(GTK_OBJECT(ok_button),
			   "clicked",
			   GTK_SIGNAL_FUNC(buttons_configure_ok), NULL);
	gtk_signal_connect(GTK_OBJECT(apply_button),
			   "clicked",
			   GTK_SIGNAL_FUNC(buttons_configure_apply), NULL);

	gtk_container_add(GTK_CONTAINER(BConfigWindow),vbox);
	gtk_window_set_default(GTK_WINDOW(BConfigWindow),cancel_button);
	gtk_signal_connect(GTK_OBJECT(BConfigWindow),
			   "delete_event",
			   GTK_SIGNAL_FUNC(buttons_configure_close), NULL);
	gtk_widget_show_all(BConfigWindow);
};


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

void buttons_progress_set_text(){
	char data[10];
	*data=0;
	sprintf(data,"%i",CFG.PROGRESS_MODE+1);
	char *text=sum_strings(_(BUTTONS_TEXT[BUTTON_PROGRESS])," ",data,NULL);
	gtk_tooltips_set_tip(GTK_TOOLBAR(ButtonsBar)->tooltips,
			     buttons_array[BUTTON_PROGRESS],
			     text,(char*) NULL);
	delete[] text;
};

void buttons_progress_toggle(){
	CFG.PROGRESS_MODE+=1;
	if (CFG.PROGRESS_MODE>2)
		CFG.PROGRESS_MODE=0;
	gtk_widget_queue_draw(ListOfDownloads);
	buttons_progress_set_text();
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
#include "pixmaps/cfgbt.xpm"
#include "pixmaps/percent.xpm"
#include "pixmaps/load.xpm"

	ButtonsBar=gtk_toolbar_new (GTK_ORIENTATION_HORIZONTAL, GTK_TOOLBAR_ICONS);
//	gtk_toolbar_set_space_style(GTK_TOOLBAR(ButtonsBar), GTK_TOOLBAR_SPACE_LINE);
//	gtk_toolbar_set_button_relief(GTK_TOOLBAR(ButtonsBar), GTK_RELIEF_NONE);

	buttons_array[BUTTON_ADD]=gtk_toolbar_append_item (GTK_TOOLBAR (ButtonsBar), "", _(BUTTONS_TEXT[BUTTON_ADD]), "", new_pixmap (add_xpm),
							   GTK_SIGNAL_FUNC (init_add_window), NULL);
	buttons_array[BUTTON_ADD_CLIPBOARD]=gtk_toolbar_append_item (GTK_TOOLBAR (ButtonsBar), "",_(BUTTONS_TEXT[BUTTON_ADD_CLIPBOARD]), "", new_pixmap (add_clipboard_xpm),
	                   GTK_SIGNAL_FUNC (init_add_clipboard_window), NULL);
	buttons_array[BUTTON_DEL]=gtk_toolbar_append_item (GTK_TOOLBAR (ButtonsBar), "",_(BUTTONS_TEXT[BUTTON_DEL]), "", new_pixmap (del_xpm),
	                                   GTK_SIGNAL_FUNC (ask_delete_download), NULL);
	gtk_toolbar_append_space (GTK_TOOLBAR (ButtonsBar));
	buttons_array[BUTTON_CONTINUE]=gtk_toolbar_append_item (GTK_TOOLBAR (ButtonsBar), "",_(BUTTONS_TEXT[BUTTON_CONTINUE]), "", new_pixmap (continue_xpm),
	                                        GTK_SIGNAL_FUNC (continue_downloads), NULL);
	buttons_array[BUTTON_STOP]=gtk_toolbar_append_item (GTK_TOOLBAR (ButtonsBar), "",_(BUTTONS_TEXT[BUTTON_STOP]) , "", new_pixmap (stop_bar_xpm),
	                                    GTK_SIGNAL_FUNC (stop_downloads), NULL);
	buttons_array[BUTTON_DEL_COMPLETED]=gtk_toolbar_append_item (GTK_TOOLBAR (ButtonsBar), "",_(BUTTONS_TEXT[BUTTON_DEL_COMPLETED]) , "", new_pixmap (del_com_xpm),
	                  GTK_SIGNAL_FUNC (ask_delete_completed_downloads), NULL);
	buttons_array[BUTTON_UP]=gtk_toolbar_append_item (GTK_TOOLBAR (ButtonsBar), "",_(BUTTONS_TEXT[BUTTON_UP]) , "", new_pixmap (up_bar_xpm),
	                                  GTK_SIGNAL_FUNC (list_of_downloads_move_up), NULL);
	buttons_array[BUTTON_DOWN]=gtk_toolbar_append_item (GTK_TOOLBAR (ButtonsBar), "",_(BUTTONS_TEXT[BUTTON_DOWN]) , "", new_pixmap (down_bar_xpm),
	                                    GTK_SIGNAL_FUNC (list_of_downloads_move_down), NULL);
	buttons_array[BUTTON_LOG]=gtk_toolbar_append_item (GTK_TOOLBAR (ButtonsBar), "",_(BUTTONS_TEXT[BUTTON_LOG]) , "", new_pixmap (openlog_xpm),
	                                   GTK_SIGNAL_FUNC (list_of_downloads_open_logs), NULL);
	gtk_toolbar_append_space (GTK_TOOLBAR (ButtonsBar));
	buttons_array[BUTTON_OPTIONS]=gtk_toolbar_append_item (GTK_TOOLBAR (ButtonsBar), "",_(BUTTONS_TEXT[BUTTON_OPTIONS]) , "", new_pixmap (prefs_xpm),
	                                       GTK_SIGNAL_FUNC (d4x_prefs_init), NULL);
	gtk_toolbar_append_space (GTK_TOOLBAR (ButtonsBar));
	buttons_array[BUTTON_SPEED1]=gtk_toolbar_append_element (GTK_TOOLBAR (ButtonsBar),
	             GTK_TOOLBAR_CHILD_RADIOBUTTON,
	             (GtkWidget *)NULL,
	             "",
	             _(BUTTONS_TEXT[BUTTON_SPEED1]),
	             "",
	             new_pixmap (speed1_xpm),
	             GTK_SIGNAL_FUNC (set_speed_limit),
	             (GtkWidget *)NULL);
	buttons_array[BUTTON_SPEED2]=gtk_toolbar_append_element (GTK_TOOLBAR (ButtonsBar),
	             GTK_TOOLBAR_CHILD_RADIOBUTTON,
	             buttons_array[BUTTON_SPEED1],
	             "",
	             _(BUTTONS_TEXT[BUTTON_SPEED2]),
	             "",
	             new_pixmap (speed2_xpm),
	             GTK_SIGNAL_FUNC (set_speed_limit),
	             (GtkWidget *)NULL);
	buttons_array[BUTTON_SPEED3]=gtk_toolbar_append_element (GTK_TOOLBAR (ButtonsBar),
	             GTK_TOOLBAR_CHILD_RADIOBUTTON,
	             buttons_array[BUTTON_SPEED2],
	             "",
	             _(BUTTONS_TEXT[BUTTON_SPEED3]),
	             "",
	             new_pixmap (speed3_xpm),
	             GTK_SIGNAL_FUNC (set_speed_limit),
	             (GtkWidget *)NULL);
	gtk_toolbar_append_space (GTK_TOOLBAR (ButtonsBar));
//	gtk_toolbar_append_space (GTK_TOOLBAR (ButtonsBar));
//	gtk_toolbar_append_space (GTK_TOOLBAR (ButtonsBar));
	buttons_array[BUTTON_DEL_ALL]=gtk_toolbar_append_item (GTK_TOOLBAR (ButtonsBar), "",_(BUTTONS_TEXT[BUTTON_DEL_ALL]) , "", new_pixmap (del_all_xpm),
	                                      GTK_SIGNAL_FUNC (ask_delete_all), NULL);
	buttons_array[BUTTON_SAVE]=gtk_toolbar_append_item (GTK_TOOLBAR (ButtonsBar), "",_(BUTTONS_TEXT[BUTTON_SAVE]) , "", new_pixmap (save_xpm),
	                                      GTK_SIGNAL_FUNC (init_save_list), NULL);
	buttons_array[BUTTON_LOAD]=gtk_toolbar_append_item (GTK_TOOLBAR (ButtonsBar), "",_(BUTTONS_TEXT[BUTTON_LOAD]) , "", new_pixmap (load_xpm),
							    GTK_SIGNAL_FUNC (init_load_list), NULL);
//	gtk_toolbar_append_space (GTK_TOOLBAR (ButtonsBar));
	buttons_array[BUTTON_PROGRESS]=gtk_toolbar_append_item (GTK_TOOLBAR (ButtonsBar),
								"",
								_(BUTTONS_TEXT[BUTTON_PROGRESS]),
								"",
								new_pixmap (percent_xpm),
								GTK_SIGNAL_FUNC (buttons_progress_toggle),
								NULL);
	buttons_array[BUTTON_DND_TRASH]=gtk_toolbar_append_element (GTK_TOOLBAR (ButtonsBar),
	             GTK_TOOLBAR_CHILD_TOGGLEBUTTON,
	             (GtkWidget *)NULL,
	             "",
	             _(BUTTONS_TEXT[BUTTON_DND_TRASH]),
	             "",
	             new_pixmap (dndtrash_bar_xpm),
	             GTK_SIGNAL_FUNC (dnd_trash_toggle),
	             (GtkWidget *)NULL);

	buttons_array[BUTTON_CONFIGURE] = gtk_button_new ();
	gtk_button_set_relief (GTK_BUTTON (buttons_array[BUTTON_CONFIGURE]), GTK_RELIEF_NONE);
	GTK_WIDGET_UNSET_FLAGS(GTK_WIDGET(buttons_array[BUTTON_CONFIGURE]), GTK_CAN_FOCUS);
	gtk_container_add(GTK_CONTAINER(buttons_array[BUTTON_CONFIGURE]),new_pixmap (cfgbt_xpm));
	gtk_widget_show_all(buttons_array[BUTTON_CONFIGURE]);
	gtk_toolbar_append_widget(GTK_TOOLBAR (ButtonsBar),
				  buttons_array[BUTTON_CONFIGURE],
				  _(BUTTONS_TEXT[BUTTON_CONFIGURE]),
				  "");
	gtk_signal_connect(GTK_OBJECT(buttons_array[BUTTON_CONFIGURE]),
			   "clicked",GTK_SIGNAL_FUNC(buttons_configure), NULL);

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
	buttons_progress_set_text();
	buttons_cfg_init();
};

void buttons_cfg_init(){
	buttons_flags_init();
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
