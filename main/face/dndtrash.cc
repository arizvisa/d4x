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
#include <time.h>
#include "dndtrash.h"
#include "misc.h"
#include "prefs.h"
#include "lod.h"
#include "addd.h"
#include "list.h"
#include "buttons.h"
#include "colors.h"
#include "../main.h"
#include "../var.h"
#include "../ntlocale.h"


extern tMain aa;
extern GtkTargetEntry download_drop_types[];
extern gint n_download_drop_types;

enum DND_MENU_ENUM{
	DM_NEW,
	DM_PASTE,
	DM_AUTOMATED,
	DM_DELCOMPLETED,
	DM_OPTIONS,
	DM_SPEED, DM_SPEED_1, DM_SPEED_2, DM_SPEED_3,
	DM_EXIT,
	DM_SEP
};

char *dnd_menu_inames[]={
	N_("/New Download"),
	N_("/Paste Download"),
	N_("/Automated adding"),
	N_("/Delete completed"),
	N_("/General options"),
	N_("/Speed"),
	N_("/Speed/Low"),
	N_("/Speed/Medium"),
	N_("/Speed/Unlimited"),
	N_("/Exit"),
	"/-"
};

GtkItemFactory *dnd_trash_item_factory;
GtkWidget *dnd_trash_window=(GtkWidget *)NULL;
GtkWidget *dnd_trash_menu=(GtkWidget *)NULL;
GtkWidget *dnd_trash_gtk_pixmap; //used for animation/blinking
static GdkPixmap *dnd_trash_pixmap1=(GdkPixmap *)NULL,*dnd_trash_pixmap2=(GdkPixmap *)NULL;
static GdkBitmap *dnd_trash_mask1,*dnd_trash_mask2;
static int dnd_trash_raise_count=0;
static time_t dnd_trash_last_raise=0;
GtkTooltips *dnd_trash_tooltips=(GtkTooltips *)NULL;
static gint dnd_trash_moveable,dnd_trash_x,dnd_trash_y;

void dnd_trash_real_destroy(){
	if (dnd_trash_window){
		main_window_popup();
		gtk_widget_destroy(dnd_trash_window);
		dnd_trash_window=(GtkWidget *)NULL;
	};
	CFG.DND_TRASH=0;
};

void dnd_trash_destroy(){
	dnd_trash_real_destroy();
	set_dndtrash_button();
};

void dnd_trash_motion(GtkWidget *widget,GdkEventMotion *event){
	if (dnd_trash_moveable){
		motion_notify_get_coords(event);
		gint mx,my;
		GdkModifierType modmask;
		gdk_window_get_pointer((GdkWindow *)NULL, &mx, &my, &modmask);
		CFG.DND_TRASH_X+=mx-dnd_trash_x;
		CFG.DND_TRASH_Y+=my-dnd_trash_y;
		gdk_window_move(widget->window,
				CFG.DND_TRASH_X,
				CFG.DND_TRASH_Y);
		dnd_trash_x=mx;
		dnd_trash_y=my;
		gdk_flush();
	};
};

int dnd_trash_button_press(GtkWidget *widget,GdkEventButton *event){
	switch (event->button){
	case 3:{
		CFG.DND_NEED_POPUP=0;
		gtk_menu_popup(GTK_MENU(dnd_trash_menu),
			       (GtkWidget *)NULL,
			       (GtkWidget *)NULL,
			       (GtkMenuPositionFunc)NULL,
			       (gpointer)NULL,
			       event->button,event->time);
		break;
	};
	case 1:{
		if (event->type==GDK_2BUTTON_PRESS)
			main_window_toggle();
		else{
			dnd_trash_moveable=1;
//			GdkModifierType modmask;
//			gdk_window_get_pointer((GdkWindow *)NULL, &dnd_trash_x, &dnd_trash_y, &modmask);
			dnd_trash_x=gint(event->x_root);
			dnd_trash_y=gint(event->y_root);
			gtk_grab_add (widget);
			gdk_pointer_grab (widget->window,TRUE,
					  GdkEventMask(GDK_BUTTON_RELEASE_MASK |
						       GDK_BUTTON_MOTION_MASK |
						       GDK_POINTER_MOTION_HINT_MASK),
					  NULL, NULL, 0);
		};
		break;
	};
	case 2:{
		init_add_window();
		break;
	};
	};
	return 1;
}; 

int dnd_trash_button_release(GtkWidget *widget,GdkEventButton *event){
	gdk_pointer_ungrab(GDK_CURRENT_TIME);
        gdk_flush();
	dnd_trash_moveable=0;
	gtk_grab_remove (widget);
	gdk_pointer_ungrab (0);
	return 1;
}; 


static int dnd_trash_no_expose(){
	if (CFG.DND_NEED_POPUP){
		time_t now=time(NULL);
		if (now == dnd_trash_last_raise)
			dnd_trash_raise_count+=1;
		else{
			dnd_trash_last_raise=now;
			dnd_trash_raise_count=1;
		};
		if (dnd_trash_raise_count<15)
			gdk_window_raise(dnd_trash_window->window);
	};
	return TRUE;
};

static int dnd_trash_configure(GtkWidget *window){
//	gdk_window_get_root_origin (window->window, &(CFG.DND_TRASH_X),
//				    &(CFG.DND_TRASH_Y));
	gdk_window_get_position(window->window,&(CFG.DND_TRASH_X),
				&(CFG.DND_TRASH_Y));
	return(FALSE);
};

void dnd_trash_init(){
	CFG.DND_TRASH=1;
	CFG.DND_NEED_POPUP=1;
	if (dnd_trash_window) return;
	dnd_trash_moveable=0;
#include "pixmaps/dndtrash.xpm"
#include "pixmaps/dndtrashi.xpm"
	/* GtkWidget is the storage type for widgets */
	GtkWidget *pixmap, *fixed;
	GtkStyle *style;
	GdkGC *gc;
    
	/* Create the main window, and attach delete_event signal to terminate
	 * the application.  Note that the main window will not have a titlebar
	 * since we're making it a popup. */
	dnd_trash_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
//	dnd_trash_window = gtk_window_new( GTK_WINDOW_POPUP );
	gtk_window_set_wmclass(GTK_WINDOW(dnd_trash_window),"D4X_DnDBasket", "D4X_DnDBasket");
//	gtk_window_set_transient_for(GTK_WINDOW(dnd_trash_window), GTK_WINDOW(MainWindow));
	d4x_normalize_coords(&(CFG.DND_TRASH_X),&(CFG.DND_TRASH_Y));
	gtk_window_set_default_size(GTK_WINDOW(dnd_trash_window),50,50);
	gtk_widget_set_uposition( dnd_trash_window, gint(CFG.DND_TRASH_X),gint(CFG.DND_TRASH_Y));
//	gtk_widget_set_events(dnd_trash_window,GDK_ALL_EVENTS_MASK);
	gtk_widget_set_events(dnd_trash_window,
			      gtk_widget_get_events(dnd_trash_window) |
			      GDK_FOCUS_CHANGE_MASK |
			      GDK_BUTTON_MOTION_MASK |
			      GDK_POINTER_MOTION_MASK |
			      GDK_POINTER_MOTION_HINT_MASK |
			      GDK_BUTTON_PRESS_MASK |
			      GDK_BUTTON_RELEASE_MASK |
			      GDK_STRUCTURE_MASK |
			      GDK_VISIBILITY_NOTIFY_MASK |
			      GDK_EXPOSURE_MASK);
	gtk_signal_connect (GTK_OBJECT (dnd_trash_window), "delete_event",
			    GTK_SIGNAL_FUNC (dnd_trash_destroy), NULL);
	gtk_signal_connect (GTK_OBJECT (dnd_trash_window), "motion_notify_event",
			    GTK_SIGNAL_FUNC (dnd_trash_motion), NULL);
	gtk_signal_connect (GTK_OBJECT (dnd_trash_window), "button_press_event",
			    GTK_SIGNAL_FUNC (dnd_trash_button_press), NULL);
	gtk_signal_connect (GTK_OBJECT (dnd_trash_window), "button_release_event",
			    GTK_SIGNAL_FUNC (dnd_trash_button_release), NULL);

	gtk_signal_connect(GTK_OBJECT(dnd_trash_window),
	                   "drag_data_received",
	                   GTK_SIGNAL_FUNC(list_dnd_drop_internal),
	                   NULL);
	gtk_drag_dest_set(GTK_WIDGET(dnd_trash_window),
	                  (GtkDestDefaults)(GTK_DEST_DEFAULT_MOTION |
	                                    GTK_DEST_DEFAULT_HIGHLIGHT |
	                                    GTK_DEST_DEFAULT_DROP),
	                  download_drop_types, n_download_drop_types,
	                  (GdkDragAction)(GDK_ACTION_COPY|GDK_ACTION_MOVE));
	gtk_signal_connect(GTK_OBJECT(dnd_trash_window), "visibility_notify_event",
	                   GTK_SIGNAL_FUNC(dnd_trash_no_expose),
	                   dnd_trash_window);			  
	gtk_signal_connect(GTK_OBJECT(dnd_trash_window), "no_expose_event",
	                   GTK_SIGNAL_FUNC(dnd_trash_no_expose),
	                   dnd_trash_window);
	gtk_widget_realize(dnd_trash_window);
	gdk_window_set_decorations(dnd_trash_window->window,GdkWMDecoration(0));
	gtk_widget_show (dnd_trash_window);


	/* Now for the pixmap and the pixmap widget */
	style = gtk_widget_get_default_style();
	gc = style->black_gc;
	if (dnd_trash_pixmap1==NULL){
		dnd_trash_pixmap1 = gdk_pixmap_create_from_xpm_d( dnd_trash_window->window, &dnd_trash_mask1,
								  &style->bg[GTK_STATE_NORMAL],
								  dndtrash_xpm);
		dnd_trash_pixmap2 = gdk_pixmap_create_from_xpm_d( dnd_trash_window->window, &dnd_trash_mask2,
								  &style->bg[GTK_STATE_NORMAL],
								  dndtrashi_xpm);
		/* we will use it sometimes */
		gdk_pixmap_ref(dnd_trash_pixmap1);
		gdk_bitmap_ref(dnd_trash_mask1);
		gdk_pixmap_ref(dnd_trash_pixmap2);
		gdk_bitmap_ref(dnd_trash_mask2);
	};
	dnd_trash_gtk_pixmap = pixmap = gtk_pixmap_new( dnd_trash_pixmap1, dnd_trash_mask1 );
//		gtk_widget_show( pixmap );
	/* To display the pixmap, we use a fixed widget to place the pixmap */
	fixed = gtk_fixed_new();
	gtk_widget_set_usize( fixed, 50, 50 );
	gtk_event_box_new();
	gtk_fixed_put( GTK_FIXED(fixed), pixmap, 0, 0 );

	gtk_container_add( GTK_CONTAINER(dnd_trash_window), fixed );
	gtk_widget_show_all( fixed );

	if (dnd_trash_tooltips==NULL){
		dnd_trash_tooltips=gtk_tooltips_new();
		gtk_tooltips_force_window(dnd_trash_tooltips);
		GtkStyle *current_style =gtk_style_copy(gtk_widget_get_style(dnd_trash_tooltips->tip_window));
		current_style->bg[GTK_STATE_NORMAL] = LYELLOW;
		gdk_font_unref(current_style->font);
		current_style->font = MainWindow->style->font;
		gdk_font_ref(MainWindow->style->font);
		gtk_widget_set_style(dnd_trash_tooltips->tip_window, current_style);
	};
	gtk_tooltips_set_tip(dnd_trash_tooltips,dnd_trash_window,_("Drop link here"),(const gchar *)NULL);
	gtk_tooltips_enable(dnd_trash_tooltips);
	/* This masks out everything except for the image itself */
	gtk_widget_shape_combine_mask( dnd_trash_window, dnd_trash_mask1, 0, 0 );
    
	/* show the window */
	gtk_widget_show( dnd_trash_window );
	gdk_window_resize(dnd_trash_window->window,50,50);
	set_dndtrash_button();
	wm_skip_window(dnd_trash_window);
	gdk_window_move(dnd_trash_window->window,CFG.DND_TRASH_X,CFG.DND_TRASH_Y);
	gtk_signal_connect(GTK_OBJECT(dnd_trash_window), "configure_event",
	                   GTK_SIGNAL_FUNC(dnd_trash_configure),
	                   NULL);
};

void dnd_trash_menu_prepare(){
	GtkWidget *menu_item=gtk_item_factory_get_widget(dnd_trash_item_factory,_(dnd_menu_inames[DM_SPEED_1]));
	GTK_CHECK_MENU_ITEM(menu_item)->active=CFG.SPEED_LIMIT==1?TRUE:FALSE;
	if (GTK_WIDGET_VISIBLE(menu_item)) gtk_widget_queue_draw(menu_item);
	menu_item=gtk_item_factory_get_widget(dnd_trash_item_factory,_(dnd_menu_inames[DM_SPEED_2]));
	GTK_CHECK_MENU_ITEM(menu_item)->active=CFG.SPEED_LIMIT==2?TRUE:FALSE;
	if (GTK_WIDGET_VISIBLE(menu_item)) gtk_widget_queue_draw(menu_item);
	menu_item=gtk_item_factory_get_widget(dnd_trash_item_factory,_(dnd_menu_inames[DM_SPEED_3]));
	GTK_CHECK_MENU_ITEM(menu_item)->active=CFG.SPEED_LIMIT==3?TRUE:FALSE;
	if (GTK_WIDGET_VISIBLE(menu_item)) gtk_widget_queue_draw(menu_item);
};

void dnd_trash_menu_calback(gpointer *data, guint action, GtkWidget *widget){
	if (action>0 && action<4){
		CFG.SPEED_LIMIT=action;
		set_speed_buttons();
	};
};

static void dnd_trash_delete_completed(){
	aa.del_completed();
};

void dnd_trash_set_del_completed(gint how){
	GtkWidget *menu_item=gtk_item_factory_get_widget(dnd_trash_item_factory,
					      _(dnd_menu_inames[DM_DELCOMPLETED]));
	if (menu_item)
		gtk_widget_set_sensitive(menu_item,how);
};

static gint dnd_trash_menu_umap(){
	CFG.DND_NEED_POPUP=1;
	dnd_trash_no_expose();
	return(FALSE);
};
void dnd_trash_init_menu() {
	GtkItemFactoryEntry menu_items[] = {
		{_(dnd_menu_inames[DM_NEW]),		(gchar *)NULL,	(GtkItemFactoryCallback)init_add_window,		0, (gchar *)NULL},
		{_(dnd_menu_inames[DM_PASTE]),		(gchar *)NULL,	(GtkItemFactoryCallback)init_add_clipboard_window,	0, (gchar *)NULL},
		{_(dnd_menu_inames[DM_AUTOMATED]),	(gchar *)NULL,	(GtkItemFactoryCallback)d4x_automated_add,		0,(gchar *)NULL},
		{_(dnd_menu_inames[DM_DELCOMPLETED]),	(gchar *)NULL,	(GtkItemFactoryCallback)dnd_trash_delete_completed,	0, (gchar *)NULL},
		{_(dnd_menu_inames[DM_SEP]),		(gchar *)NULL,	(GtkItemFactoryCallback)NULL,				0, "<Separator>"},
		{_(dnd_menu_inames[DM_OPTIONS]),	(gchar *)NULL,	(GtkItemFactoryCallback)d4x_prefs_init,			0, (gchar *)NULL},
		{_(dnd_menu_inames[DM_SPEED]),		(gchar *)NULL,	(GtkItemFactoryCallback)NULL,				0, "<Branch>"},
		{_(dnd_menu_inames[DM_SPEED_1]),	(gchar *)NULL,	(GtkItemFactoryCallback)dnd_trash_menu_calback,		1, "<RadioItem>"},
		{_(dnd_menu_inames[DM_SPEED_2]),	(gchar *)NULL,	(GtkItemFactoryCallback)dnd_trash_menu_calback,		2, _(dnd_menu_inames[DM_SPEED_1])},
		{_(dnd_menu_inames[DM_SPEED_3]),	(gchar *)NULL,	(GtkItemFactoryCallback)dnd_trash_menu_calback,		3, _(dnd_menu_inames[DM_SPEED_2])},
		{_(dnd_menu_inames[DM_SEP]),		(gchar *)NULL,	(GtkItemFactoryCallback)NULL,				0, "<Separator>"},
		{_(dnd_menu_inames[DM_EXIT]),		(gchar *)NULL,	(GtkItemFactoryCallback)ask_exit,			0, (gchar *)NULL}
	};
	int nmenu_items = sizeof(menu_items) / sizeof(menu_items[0]);
	GtkAccelGroup *accel_group;
	accel_group = gtk_accel_group_new();
	dnd_trash_item_factory = gtk_item_factory_new(GTK_TYPE_MENU, "<main>",accel_group);
	gtk_item_factory_create_items(dnd_trash_item_factory, nmenu_items, menu_items, NULL);
	dnd_trash_menu = gtk_item_factory_get_widget(dnd_trash_item_factory, "<main>");
	gtk_signal_connect (GTK_OBJECT (dnd_trash_menu), "unmap_event",
			    GTK_SIGNAL_FUNC (dnd_trash_menu_umap), NULL);
};

static gint dnd_trash_animation_end(gpointer unused){
	gtk_pixmap_set(GTK_PIXMAP(dnd_trash_gtk_pixmap),
		       dnd_trash_pixmap1,dnd_trash_mask1);
	return 0;
};

void dnd_trash_animation(){
	if (dnd_trash_window){
		if (GTK_PIXMAP(dnd_trash_gtk_pixmap)->pixmap==dnd_trash_pixmap1){
			gtk_pixmap_set(GTK_PIXMAP(dnd_trash_gtk_pixmap),
				       dnd_trash_pixmap2,dnd_trash_mask2);
//			gtk_widget_queue_draw(dnd_trash_gtk_pixmap);
			GdkRectangle rect;
			rect.x=dnd_trash_gtk_pixmap->allocation.x;
			rect.y=dnd_trash_gtk_pixmap->allocation.y;
			rect.width=dnd_trash_gtk_pixmap->allocation.width;
			rect.height=dnd_trash_gtk_pixmap->allocation.height;
			gtk_widget_draw(dnd_trash_gtk_pixmap,&rect);
			gtk_timeout_add (500, dnd_trash_animation_end , NULL);
		};
	};
};

void dnd_trash_set_tooltip(char *str){
	if (str==NULL || dnd_trash_window==NULL) return;
	if (dnd_trash_tooltips->tip_window){
		GtkTooltipsData *data=dnd_trash_tooltips->active_tips_data;
		if (data==NULL && dnd_trash_tooltips->tips_data_list)
			data=(GtkTooltipsData *)(dnd_trash_tooltips->tips_data_list->data);
		/* only if data changed */
		if (data && data->row && !equal((char*)(data->row->data),str)){
			/* we need only one row so other
			   rows should be freed */
			GList *tmp=data->row->next;
			data->row->next=NULL;
			while (tmp){
				GList *tmp1=tmp->next;
				if (tmp->data) g_free(tmp->data);
				g_free(tmp);
				tmp=tmp1;
			};
			g_free(data->row->data);
			data->row->data=g_strdup(str);
			GtkWidget *window=GTK_WIDGET(dnd_trash_tooltips->tip_window);
			GtkStyle *style = gtk_widget_get_style(window);
/*			
			gint gap = (style->font->ascent + style->font->descent) / 4;
			if (gap < 2)
				gap = 2;
*/
			gint new_width=data->width=gdk_string_width(style->font,str)+8;
			if (GTK_WIDGET_VISIBLE(window)){
				gint scr_w = gdk_screen_width();
				gint x,y;
				gdk_window_get_position(window->window,&x,&y);
				if (x+new_width>scr_w)
					gdk_window_move (window->window, scr_w-new_width,y);
				else{
					if (new_width<window->allocation.width)
						gdk_window_move (window->window, x+(window->allocation.width-new_width)/2,y);
				};
				gtk_widget_set_usize(window,new_width,
						     window->allocation.height);
				gtk_widget_queue_draw(window);
			};
		};
/*
	}else{
		gtk_tooltips_set_tip(dnd_trash_tooltips,
				     dnd_trash_window,
				     str,NULL);	
*/
	};

};
