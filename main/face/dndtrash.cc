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
#include <stdio.h>


extern tMain aa;
extern GtkTargetEntry download_drop_types[];
extern gint n_download_drop_types;

GtkWidget *dnd_trash_window=(GtkWidget *)NULL;
GtkWidget *dnd_trash_menu=(GtkWidget *)NULL;
GtkTooltips *dnd_trash_tooltips;
int dnd_trash_moveable,dnd_trash_x,dnd_trash_y;

void dnd_trash_destroy(){
	if (dnd_trash_window)gtk_widget_destroy(dnd_trash_window);
	dnd_trash_window=NULL;
	CFG.DND_TRASH=0;
    set_dndtrash_button();
};

void dnd_trash_motion(GtkWidget *widget,GdkEventMotion *event){
	if (dnd_trash_moveable){
		motion_notify_get_coords(event);
		gint mx,my;
		GdkModifierType modmask;
		gdk_window_get_pointer(NULL, &mx, &my, &modmask);
		CFG.DND_TRASH_X+=mx-dnd_trash_x;
		CFG.DND_TRASH_Y+=my-dnd_trash_y;
		gdk_window_move(widget->window,CFG.DND_TRASH_X,CFG.DND_TRASH_Y);
		dnd_trash_x=mx;
		dnd_trash_y=my;
		gdk_flush();
	};
};

int dnd_trash_button_press(GtkWidget *widget,GdkEventButton *event){
	if (event->button==3) {
    	gtk_menu_popup(GTK_MENU(dnd_trash_menu),NULL,NULL,NULL,NULL,event->button,event->time);
	}else{
		if (event->type==GDK_2BUTTON_PRESS)
			main_window_popup();
		else{
			dnd_trash_moveable=1;
	    	GdkModifierType modmask;
	    	gdk_window_get_pointer(NULL, &dnd_trash_x, &dnd_trash_y, &modmask);
	    };
	};
	return 1;
}; 

int dnd_trash_button_release(GtkWidget *widget,GdkEventButton *event){
	gdk_pointer_ungrab(GDK_CURRENT_TIME);
        gdk_flush();
	dnd_trash_moveable=0;
	return 1;
}; 

int dnd_trash_refresh(){
	gint mx,my,newx,newy;
	GdkModifierType modmask;
	gdk_window_get_pointer(NULL, &mx, &my, &modmask);
	if (!(modmask&(GDK_BUTTON1_MASK | GDK_BUTTON2_MASK| GDK_BUTTON3_MASK)))
		dnd_trash_moveable=0;
	if (dnd_trash_moveable && dnd_trash_window){
		newx=CFG.DND_TRASH_X+mx-dnd_trash_x;
		newy=CFG.DND_TRASH_Y+my-dnd_trash_y;
		if (newx!=CFG.DND_TRASH_X ||newy!=CFG.DND_TRASH_Y){
			CFG.DND_TRASH_X=newx;
			CFG.DND_TRASH_Y=newy;
			gdk_window_move(dnd_trash_window->window,CFG.DND_TRASH_X,CFG.DND_TRASH_Y);
			dnd_trash_x=mx;
			dnd_trash_y=my;
			gdk_flush();
		};
	};
	return 1;
};

void dnd_trash_init(){
	CFG.DND_TRASH=1;
	if (dnd_trash_window) return;
	dnd_trash_moveable=0;
#include "pixmaps/dndtrash.xpm"
    /* GtkWidget is the storage type for widgets */
    GtkWidget *pixmap, *fixed;
    GdkPixmap *gdk_pixmap;
    GdkBitmap *mask;
    GtkStyle *style;
    GdkGC *gc;
    
    /* Create the main window, and attach delete_event signal to terminate
     * the application.  Note that the main window will not have a titlebar
     * since we're making it a popup. */
    dnd_trash_window = gtk_window_new( GTK_WINDOW_POPUP );
    if (CFG.DND_TRASH_X>gdk_screen_width()) CFG.DND_TRASH_X=0;
    if (CFG.DND_TRASH_Y>gdk_screen_height()) CFG.DND_TRASH_Y=0;
	gtk_window_set_default_size(GTK_WINDOW(dnd_trash_window),50,50);
    gtk_widget_set_uposition( dnd_trash_window, gint(CFG.DND_TRASH_X),gint(CFG.DND_TRASH_Y));

    gtk_widget_set_events(dnd_trash_window, GDK_FOCUS_CHANGE_MASK | GDK_BUTTON_MOTION_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_STRUCTURE_MASK);
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
	                  (GdkDragAction)GDK_ACTION_COPY);

    gtk_widget_show (dnd_trash_window);

    /* Now for the pixmap and the pixmap widget */
    style = gtk_widget_get_default_style();
    gc = style->black_gc;
    gdk_pixmap = gdk_pixmap_create_from_xpm_d( dnd_trash_window->window, &mask,
                                             &style->bg[GTK_STATE_NORMAL],
                                             dndtrash_xpm);
    pixmap = gtk_pixmap_new( gdk_pixmap, mask );
    gtk_widget_show( pixmap );

    /* To display the pixmap, we use a fixed widget to place the pixmap */
    fixed = gtk_fixed_new();
    gtk_widget_set_usize( fixed, 50, 50 );
    gtk_fixed_put( GTK_FIXED(fixed), pixmap, 0, 0 );
    gtk_container_add( GTK_CONTAINER(dnd_trash_window), fixed );
    gtk_widget_show( fixed );

	dnd_trash_tooltips=gtk_tooltips_new();
    gtk_tooltips_force_window(dnd_trash_tooltips);
	GtkStyle *current_style =gtk_style_copy(gtk_widget_get_style(dnd_trash_tooltips->tip_window));
	current_style->bg[GTK_STATE_NORMAL] = LYELLOW;
	gtk_widget_set_style(dnd_trash_tooltips->tip_window, current_style);

	gtk_tooltips_set_tip(dnd_trash_tooltips,dnd_trash_window,_("Drop link here"),NULL);
	gtk_tooltips_enable(dnd_trash_tooltips);
    /* This masks out everything except for the image itself */
    gtk_widget_shape_combine_mask( dnd_trash_window, mask, 0, 0 );
    
    /* show the window */
    gtk_widget_show( dnd_trash_window );
    gdk_window_resize(dnd_trash_window->window,50,50);
    set_dndtrash_button();
    dnd_trash_init_menu();
};

void dnd_trash_init_menu() {
	GtkWidget *popup_menu=gtk_menu_new();
	GtkWidget *item=gtk_menu_item_new_with_label(_("New Download"));
	gtk_menu_append(GTK_MENU(popup_menu),item);
	gtk_signal_connect(GTK_OBJECT(item),"activate",GTK_SIGNAL_FUNC(init_add_window),NULL);

	item=gtk_menu_item_new_with_label(_("Paste Download"));
	gtk_menu_append(GTK_MENU(popup_menu),item);
	gtk_signal_connect(GTK_OBJECT(item),"activate",GTK_SIGNAL_FUNC(init_add_clipboard_window),NULL);

	item=gtk_menu_item_new();
	gtk_menu_append(GTK_MENU(popup_menu),item);

	item=gtk_menu_item_new_with_label(_("Common options"));
	gtk_menu_append(GTK_MENU(popup_menu),item);
	gtk_signal_connect(GTK_OBJECT(item),"activate",GTK_SIGNAL_FUNC(init_options_window),NULL);

	item=gtk_menu_item_new();
	gtk_menu_append(GTK_MENU(popup_menu),item);

	item=gtk_menu_item_new_with_label(_("Exit"));
	gtk_menu_append(GTK_MENU(popup_menu),item);
	gtk_signal_connect(GTK_OBJECT(item),"activate",GTK_SIGNAL_FUNC(ask_exit),NULL);

	gtk_widget_show_all(popup_menu);
	dnd_trash_menu= popup_menu;
};
