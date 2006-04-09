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

#include <gtk/gtk.h>
#include "themes.h"
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
#include "../xml.h"
#include <gdk-pixbuf/gdk-pixbuf.h>



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
	N_(" Switch percentage mode to"),
	N_(" Configure buttons ")
};

GtkWidget *ButtonsBar;
GtkWidget *buttons_array[BUTTON_LAST];
GtkWidget *pixmaps_array[BUTTON_LAST];
GdkPixmap *progress_pixmap[3]={NULL,NULL,NULL};
GdkBitmap *progress_bitmap[3]={NULL,NULL,NULL};
GdkPixbuf *progress_pixbuf[3]={NULL,NULL,NULL};

d4x::Theme::SlaveImage *D4X_PROGRESS_MODE_BUTTON;

GtkWidget *BConfigWindow=(GtkWidget *)NULL;
GtkWidget *BConfigButtons[BUTTON_LAST];


tConfirmedDialog *AskDeleteAll=(tConfirmedDialog *)NULL;

void buttons_pixmaps_init(){
	
};

void buttons_configure_close(){
	if (BConfigWindow)
		gtk_widget_destroy(BConfigWindow);
	BConfigWindow=(GtkWidget *)NULL;
};

void buttons_cfg_init(){
	int none_visible=1;
	for (int i=0;i<BUTTON_CONFIGURE;i++){
		if (CFG.BUTTONS_FLAGS & (1<<i)){
			none_visible=0;
			gtk_widget_show(buttons_array[i]);
		}else{
			gtk_widget_hide(buttons_array[i]);
		};
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
	buttons_cfg_init();
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
	BConfigWindow=gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_wmclass(GTK_WINDOW(BConfigWindow),
			       "D4X_Buttons","D4X");
	gtk_window_set_title(GTK_WINDOW (BConfigWindow),
			     _(BUTTONS_TEXT[BUTTON_CONFIGURE]));
	gtk_window_set_position(GTK_WINDOW(BConfigWindow),
				GTK_WIN_POS_CENTER);
	gtk_window_set_resizable(GTK_WINDOW(BConfigWindow), FALSE);
	gtk_widget_set_size_request(BConfigWindow,-1,400);
	gtk_container_set_border_width(GTK_CONTAINER(BConfigWindow),5);

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
	GtkWidget *ok_button=gtk_button_new_from_stock(GTK_STOCK_OK);
	GtkWidget *apply_button=gtk_button_new_from_stock(GTK_STOCK_APPLY);
	GtkWidget *cancel_button=gtk_button_new_from_stock(GTK_STOCK_CANCEL);
	GTK_WIDGET_SET_FLAGS(ok_button,GTK_CAN_DEFAULT);
	GTK_WIDGET_SET_FLAGS(apply_button,GTK_CAN_DEFAULT);
	GTK_WIDGET_SET_FLAGS(cancel_button,GTK_CAN_DEFAULT);
	gtk_box_pack_end(GTK_BOX(hbox),ok_button,FALSE,FALSE,0);
	gtk_box_pack_end(GTK_BOX(hbox),apply_button,FALSE,FALSE,0);
	gtk_box_pack_end(GTK_BOX(hbox),cancel_button,FALSE,FALSE,0);
	g_signal_connect(G_OBJECT(cancel_button),
			   "clicked",
			   G_CALLBACK(buttons_configure_close), NULL);
	g_signal_connect(G_OBJECT(ok_button),
			   "clicked",
			   G_CALLBACK(buttons_configure_ok), NULL);
	g_signal_connect(G_OBJECT(apply_button),
			   "clicked",
			   G_CALLBACK(buttons_configure_apply), NULL);

	gtk_container_add(GTK_CONTAINER(BConfigWindow),vbox);
	gtk_window_set_default(GTK_WINDOW(BConfigWindow),cancel_button);
	g_signal_connect(G_OBJECT(BConfigWindow),
			   "delete_event",
			   G_CALLBACK(buttons_configure_close), NULL);
	d4x_eschandler_init(BConfigWindow,NULL);
	gtk_widget_show_all(BConfigWindow);
};

GtkWidget *new_pixmap(char **xpm, char *themename) {
	char *path=sum_strings("buttonsbar ",themename,">file",NULL);
	GtkWidget *rval=gtk_image_new_from_pixbuf(pixbuf_from_theme(path,(const char**)xpm));
	delete[] path;
	return (rval);
};

GtkWidget *new_pixmap_mon(char **xpm, const std::string &themename) {
	d4x::Theme::Image *img=new d4x::Theme::Image(xpm,std::string("buttonsbar ")+themename+">file");
	d4x::CUR_THEME->monitor(img);
	return (GTK_WIDGET(img->img));
};

void del_all_downloads(){
	_aa_.del_all();
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
			g_signal_connect(G_OBJECT(AskDeleteAll->ok_button),
					   "clicked",
					   G_CALLBACK(_ask_delete_all_check_),
					   AskDeleteAll);
	}else
		del_all_downloads();
};

void buttons_speed_set_text(){
	char text[MAX_LEN];
	sprintf(text,"%s (%i B/s)",_(" Speed level one "),CFG.SPEED_LIMIT_1);
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(buttons_array[BUTTON_SPEED1]),
				  GTK_TOOLBAR(ButtonsBar)->tooltips,text,NULL);
	sprintf(text,"%s (%i B/s)",_(" Speed level two "),CFG.SPEED_LIMIT_2);
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(buttons_array[BUTTON_SPEED2]),
				  GTK_TOOLBAR(ButtonsBar)->tooltips,text,NULL);
};


void set_speed_buttons() {
	if (CFG.WITHOUT_FACE) return;
	switch (CFG.SPEED_LIMIT) {
	case 1:	{
		gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(buttons_array[BUTTON_SPEED1]),TRUE);
		break;
	};
	case 2:	{
		gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(buttons_array[BUTTON_SPEED2]),TRUE);
		break;
	};
	case 3:
	default:{
		gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(buttons_array[BUTTON_SPEED3]),TRUE);
		break;
	};
	};
	dnd_trash_menu_prepare();
};

static void set_speed_limit() {
	if (gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(buttons_array[BUTTON_SPEED1])))
		CFG.SPEED_LIMIT=1;
	else {
		if (gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(buttons_array[BUTTON_SPEED2])))
			CFG.SPEED_LIMIT=2;
		else
			CFG.SPEED_LIMIT=3;
	};
	main_menu_speed_prepare();
	dnd_trash_menu_prepare();
};

static void dnd_trash_toggle(){
//	if (GTK_TOGGLE_BUTTON(buttons_array[BUTTON_DND_TRASH])->active)
	if (gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(buttons_array[BUTTON_DND_TRASH])))
		dnd_trash_init();
	else
		dnd_trash_destroy();
};

void set_dndtrash_button(){
	if (CFG.DND_TRASH) {
		if (!gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(buttons_array[BUTTON_DND_TRASH]))){
			gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(buttons_array[BUTTON_DND_TRASH]),TRUE);
		};
	}else{
		if (gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(buttons_array[BUTTON_DND_TRASH]))){
			gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(buttons_array[BUTTON_DND_TRASH]),FALSE);
		};
	};
};

gint buttons_save_press(GtkWidget *widget,GdkEventButton *event, gint code){
	if (event->button==3){
		g_signal_emit_by_name(G_OBJECT(widget),"pressed",NULL);
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
	sprintf(data,"%i ",CFG.PROGRESS_MODE>=2?1:CFG.PROGRESS_MODE+2);
	char *text=sum_strings(_(BUTTONS_TEXT[BUTTON_PROGRESS])," ",data,NULL);
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(buttons_array[BUTTON_PROGRESS]),
				  GTK_TOOLBAR(ButtonsBar)->tooltips,text,NULL);
	D4X_PROGRESS_MODE_BUTTON->reinit(d4x::PBM_ONLY_TEXT+CFG.PROGRESS_MODE);
	
	delete[] text;
};

void buttons_progress_toggle(){
	CFG.PROGRESS_MODE+=1;
	if (CFG.PROGRESS_MODE>2)
		CFG.PROGRESS_MODE=0;
	gtk_widget_queue_draw(D4X_QUEUE->qv.ListOfDownloads);
	buttons_progress_set_text();
};

static void bb_move_up(){
	D4X_QUEUE->qv.move_up();
};

static void bb_move_down(){
	D4X_QUEUE->qv.move_down();
};

static void bb_open_logs(){
	D4X_QUEUE->qv.open_logs();
};


GtkWidget *d4x_gtk_toolbar_append_button(GtkWidget *toolbar,GtkWidget *icon,gchar *text,GCallback handler){
	GtkWidget *button=GTK_WIDGET(gtk_tool_button_new(icon,text));
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar),GTK_TOOL_ITEM(button),-1);
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(button),GTK_TOOLBAR(ButtonsBar)->tooltips,text,NULL);
	g_signal_connect(G_OBJECT(button),"clicked",handler, NULL);
	return(button);
};

GtkWidget *d4x_gtk_toolbar_append_radio_button(GtkWidget *toolbar,GtkWidget *icon,gchar *text,GCallback handler,GtkWidget *group){
	GtkWidget *button = group==NULL?GTK_WIDGET(gtk_radio_tool_button_new(NULL)):GTK_WIDGET(gtk_radio_tool_button_new_from_widget(GTK_RADIO_TOOL_BUTTON(group)));
	gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(button),icon);
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(button),GTK_TOOLBAR(ButtonsBar)->tooltips,text,NULL);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar),GTK_TOOL_ITEM(button),-1);
	g_signal_connect(G_OBJECT(button),"clicked",handler, NULL);
	return(button);
};

GtkWidget *d4x_gtk_toolbar_append_toggle_button(GtkWidget *toolbar,GtkWidget *icon,gchar *text,GCallback handler){
	GtkWidget *button = GTK_WIDGET(gtk_toggle_tool_button_new());
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(button),GTK_TOOLBAR(ButtonsBar)->tooltips,text,NULL);
	gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(button),icon);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar),GTK_TOOL_ITEM(button),-1);
	g_signal_connect(G_OBJECT(button),"clicked",handler, NULL);
	return(button);
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
#include "pixmaps/load.xpm"
	ButtonsBar=gtk_toolbar_new ();
	gtk_toolbar_set_orientation(GTK_TOOLBAR(ButtonsBar),GTK_ORIENTATION_HORIZONTAL);
	gtk_toolbar_set_style(GTK_TOOLBAR(ButtonsBar),GTK_TOOLBAR_ICONS);
	gtk_container_set_border_width(GTK_CONTAINER(ButtonsBar),0);
	gtk_toolbar_set_tooltips(GTK_TOOLBAR(ButtonsBar),TRUE);
	gtk_toolbar_set_show_arrow (GTK_TOOLBAR(ButtonsBar),FALSE);
//	gtk_toolbar_set_space_style(GTK_TOOLBAR(ButtonsBar), GTK_TOOLBAR_SPACE_LINE);
//	gtk_toolbar_set_button_relief(GTK_TOOLBAR(ButtonsBar), GTK_RELIEF_NONE);

	pixmaps_array[BUTTON_ADD]=new_pixmap_mon(add_xpm,"add");
	buttons_array[BUTTON_ADD]=d4x_gtk_toolbar_append_button(ButtonsBar,pixmaps_array[BUTTON_ADD],_(BUTTONS_TEXT[BUTTON_ADD]),G_CALLBACK(init_add_window));
	
	pixmaps_array[BUTTON_ADD_CLIPBOARD]=new_pixmap_mon(add_clipboard_xpm,"clipboardadd");
	buttons_array[BUTTON_ADD_CLIPBOARD]=d4x_gtk_toolbar_append_button(ButtonsBar,pixmaps_array[BUTTON_ADD_CLIPBOARD],_(BUTTONS_TEXT[BUTTON_ADD_CLIPBOARD]),G_CALLBACK(init_add_clipboard_window));
	
	pixmaps_array[BUTTON_DEL]=new_pixmap_mon(del_xpm,"del");
	buttons_array[BUTTON_DEL]=d4x_gtk_toolbar_append_button(ButtonsBar,pixmaps_array[BUTTON_DEL],_(BUTTONS_TEXT[BUTTON_DEL]),G_CALLBACK(ask_delete_download));
	
	gtk_toolbar_insert(GTK_TOOLBAR(ButtonsBar),gtk_separator_tool_item_new(),-1);
	
	pixmaps_array[BUTTON_CONTINUE]=new_pixmap_mon(continue_xpm,"continue");
	buttons_array[BUTTON_CONTINUE]=d4x_gtk_toolbar_append_button(ButtonsBar,pixmaps_array[BUTTON_CONTINUE],_(BUTTONS_TEXT[BUTTON_CONTINUE]),G_CALLBACK(continue_downloads));
	
	pixmaps_array[BUTTON_STOP]=new_pixmap_mon(stop_bar_xpm,"stop");
	buttons_array[BUTTON_STOP]=d4x_gtk_toolbar_append_button(ButtonsBar,pixmaps_array[BUTTON_STOP],_(BUTTONS_TEXT[BUTTON_STOP]),G_CALLBACK(stop_downloads));
	
	pixmaps_array[BUTTON_DEL_COMPLETED]=new_pixmap_mon(del_com_xpm,"delcompleted");
	buttons_array[BUTTON_DEL_COMPLETED]=d4x_gtk_toolbar_append_button(ButtonsBar,pixmaps_array[BUTTON_DEL_COMPLETED],_(BUTTONS_TEXT[BUTTON_DEL_COMPLETED]),G_CALLBACK(ask_delete_completed_downloads));
	
	pixmaps_array[BUTTON_UP]=new_pixmap_mon(up_bar_xpm,"moveup");
	buttons_array[BUTTON_UP]=d4x_gtk_toolbar_append_button(ButtonsBar,pixmaps_array[BUTTON_UP],_(BUTTONS_TEXT[BUTTON_UP]),G_CALLBACK(bb_move_up));
	
	pixmaps_array[BUTTON_DOWN]=new_pixmap_mon(down_bar_xpm,"movedown");
	buttons_array[BUTTON_DOWN]=d4x_gtk_toolbar_append_button(ButtonsBar,pixmaps_array[BUTTON_DOWN],_(BUTTONS_TEXT[BUTTON_DOWN]),G_CALLBACK(bb_move_down));
	
	pixmaps_array[BUTTON_LOG]=new_pixmap_mon(openlog_xpm,"openlog");
	buttons_array[BUTTON_LOG]=d4x_gtk_toolbar_append_button(ButtonsBar,pixmaps_array[BUTTON_LOG],_(BUTTONS_TEXT[BUTTON_LOG]),G_CALLBACK(bb_open_logs));
	
	gtk_toolbar_insert(GTK_TOOLBAR(ButtonsBar),gtk_separator_tool_item_new(),-1);
	
	pixmaps_array[BUTTON_OPTIONS]=new_pixmap_mon(prefs_xpm,"preferences");
	buttons_array[BUTTON_OPTIONS]=d4x_gtk_toolbar_append_button(ButtonsBar,pixmaps_array[BUTTON_OPTIONS],_(BUTTONS_TEXT[BUTTON_OPTIONS]),G_CALLBACK(d4x_prefs_init));
	
	gtk_toolbar_insert(GTK_TOOLBAR(ButtonsBar),gtk_separator_tool_item_new(),-1);
	
	pixmaps_array[BUTTON_SPEED1]=new_pixmap_mon(speed1_xpm,"speedlow");
	pixmaps_array[BUTTON_SPEED2]=new_pixmap_mon(speed2_xpm,"speedmedium");
	pixmaps_array[BUTTON_SPEED3]=new_pixmap_mon(speed3_xpm,"speedhigh");
//	buttons_array[BUTTON_]=d4x_gtk_toolbar_append_button(ButtonsBar,pixmaps_array[BUTTON_],_(BUTTONS_TEXT[BUTTON_]),G_CALLBACK());
	buttons_array[BUTTON_SPEED1]=d4x_gtk_toolbar_append_radio_button(ButtonsBar,pixmaps_array[BUTTON_SPEED1],_(BUTTONS_TEXT[BUTTON_SPEED1]),
									 G_CALLBACK(set_speed_limit),NULL);
	buttons_array[BUTTON_SPEED2]=d4x_gtk_toolbar_append_radio_button(ButtonsBar,pixmaps_array[BUTTON_SPEED2],_(BUTTONS_TEXT[BUTTON_SPEED2]),
									 G_CALLBACK(set_speed_limit),buttons_array[BUTTON_SPEED1]);
	buttons_array[BUTTON_SPEED3]=d4x_gtk_toolbar_append_radio_button(ButtonsBar,pixmaps_array[BUTTON_SPEED3],_(BUTTONS_TEXT[BUTTON_SPEED3]),
									 G_CALLBACK(set_speed_limit),buttons_array[BUTTON_SPEED2]);

	gtk_toolbar_insert(GTK_TOOLBAR(ButtonsBar),gtk_separator_tool_item_new(),-1);
	
	pixmaps_array[BUTTON_DEL_ALL]=new_pixmap_mon(del_all_xpm,"clearlist");
	buttons_array[BUTTON_DEL_ALL]=d4x_gtk_toolbar_append_button(ButtonsBar,pixmaps_array[BUTTON_DEL_ALL],_(BUTTONS_TEXT[BUTTON_DEL_ALL]),G_CALLBACK(ask_delete_all));

	pixmaps_array[BUTTON_SAVE]=new_pixmap_mon(save_xpm,"save");
	buttons_array[BUTTON_SAVE]=d4x_gtk_toolbar_append_button(ButtonsBar,pixmaps_array[BUTTON_SAVE],_(BUTTONS_TEXT[BUTTON_SAVE]),G_CALLBACK(init_save_list));

	pixmaps_array[BUTTON_LOAD]=new_pixmap_mon(load_xpm,"load");
	buttons_array[BUTTON_LOAD]=d4x_gtk_toolbar_append_button(ButtonsBar,pixmaps_array[BUTTON_LOAD],_(BUTTONS_TEXT[BUTTON_LOAD]),G_CALLBACK(init_load_list));

	D4X_PROGRESS_MODE_BUTTON=new d4x::Theme::SlaveImage(d4x::PBM_ONLY_TEXT);
	d4x::CUR_THEME->monitor(D4X_PROGRESS_MODE_BUTTON);
	
	pixmaps_array[BUTTON_PROGRESS]=GTK_WIDGET(D4X_PROGRESS_MODE_BUTTON->img);
	buttons_array[BUTTON_PROGRESS]=d4x_gtk_toolbar_append_button(ButtonsBar,pixmaps_array[BUTTON_PROGRESS],
								     _(BUTTONS_TEXT[BUTTON_PROGRESS]),G_CALLBACK(buttons_progress_toggle));
	pixmaps_array[BUTTON_DND_TRASH]=new_pixmap_mon (dndtrash_bar_xpm,"dnd");
	buttons_array[BUTTON_DND_TRASH]=d4x_gtk_toolbar_append_toggle_button(ButtonsBar,pixmaps_array[BUTTON_DND_TRASH],
									     _(BUTTONS_TEXT[BUTTON_DND_TRASH]),G_CALLBACK(dnd_trash_toggle));
	pixmaps_array[BUTTON_CONFIGURE] = new_pixmap (cfgbt_xpm,NULL);
	buttons_array[BUTTON_CONFIGURE]=d4x_gtk_toolbar_append_button(ButtonsBar,pixmaps_array[BUTTON_CONFIGURE],
								      _(BUTTONS_TEXT[BUTTON_CONFIGURE]),G_CALLBACK(buttons_configure));
	gtk_tool_item_set_homogeneous(GTK_TOOL_ITEM(buttons_array[BUTTON_CONFIGURE]),FALSE);
/*

	g_signal_connect(G_OBJECT (buttons_array[BUTTON_SAVE]), "button_press_event",
			   G_CALLBACK (buttons_save_press), GINT_TO_POINTER(BUTTON_SAVE));
	g_signal_connect(G_OBJECT (buttons_array[BUTTON_SAVE]), "button_release_event",
			   G_CALLBACK (buttons_save_release),GINT_TO_POINTER(BUTTON_SAVE));
	g_signal_connect(G_OBJECT (buttons_array[BUTTON_OPTIONS]), "button_press_event",
			   G_CALLBACK (buttons_save_press), GINT_TO_POINTER(BUTTON_OPTIONS));
	g_signal_connect(G_OBJECT (buttons_array[BUTTON_OPTIONS]), "button_release_event",
			   G_CALLBACK (buttons_save_release),GINT_TO_POINTER(BUTTON_OPTIONS));
*/
	set_speed_buttons();
	set_dndtrash_button();
	
	GtkTooltips *tooltips=((GtkToolbar *)(ButtonsBar))->tooltips;
	gtk_tooltips_force_window(tooltips);
	GtkStyle *current_style =gtk_style_copy(gtk_widget_get_style(tooltips->tip_window));
	char *tcolor=d4x_xml_find_obj_value(D4X_THEME_DATA,"buttonsbar tipcolor");
	if (tcolor){
		gdk_color_parse(tcolor,&(current_style->bg[GTK_STATE_NORMAL]));
	}else{
		current_style->bg[GTK_STATE_NORMAL] = LYELLOW;
	};
	
	gtk_widget_set_style(tooltips->tip_window, current_style);
	gtk_widget_show(ButtonsBar);
	buttons_speed_set_text();
	buttons_progress_set_text();
};

void buttons_theme_changed(){
	GtkTooltips *tooltips=((GtkToolbar *)(ButtonsBar))->tooltips;
	GtkStyle *current_style =gtk_style_copy(gtk_widget_get_style(tooltips->tip_window));
	char *tcolor=d4x_xml_find_obj_value(D4X_THEME_DATA,"buttonsbar tipcolor");
	if (tcolor){
		gdk_color_parse(tcolor,&(current_style->bg[GTK_STATE_NORMAL]));
	}else{
		current_style->bg[GTK_STATE_NORMAL] = LYELLOW;
	};
	gtk_widget_set_style(tooltips->tip_window, current_style);
};

void prepare_buttons() {
	if (!D4X_QUEUE) return;
	if (D4X_QUEUE->count())
		gtk_widget_set_sensitive(buttons_array[BUTTON_DEL_ALL],TRUE);
	else
		gtk_widget_set_sensitive(buttons_array[BUTTON_DEL_ALL],FALSE);
	if (D4X_QUEUE->qv.last_selected==NULL) {
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
