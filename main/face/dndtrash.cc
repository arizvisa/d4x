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

#include <stdio.h>
#include <time.h>
#include "dndtrash.h"
#include "misc.h"
#include "prefs.h"
#include "lod.h"
#include "addd.h"
#include "list.h"
#include "graph.h"
#include "buttons.h"
#include "colors.h"
#include "../main.h"
#include "../var.h"
#include "../ntlocale.h"
#include "../xml.h"
#include <themes.h>
#include <gdk-pixbuf/gdk-pixbuf.h>


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

//GtkItemFactory *dnd_trash_item_factory;
GtkUIManager *dnd_trash_ui_manager;
GtkWidget *dnd_trash_window=(GtkWidget *)NULL;
GtkWidget *dnd_trash_menu=(GtkWidget *)NULL;
GtkWidget *dnd_trash_gtk_pixmap; //used for animation/blinking
GtkWidget *dnd_trash_fixed;
GtkWidget *dnd_basket_graph=(GtkWidget *)NULL;;
d4xDownloadQueue *dnd_trash_target_queue=(d4xDownloadQueue *)NULL;
static GdkPixbuf *dnd_trash_pixbuf1=(GdkPixbuf *)NULL,*dnd_trash_pixbuf2=(GdkPixbuf *)NULL;
static GdkBitmap *dnd_trash_mask1=(GdkBitmap*)NULL,*dnd_trash_mask2=(GdkBitmap*)NULL;
static int dnd_trash_raise_count=0;
static time_t dnd_trash_last_raise=0;
GtkTooltips *dnd_trash_tooltips=(GtkTooltips *)NULL;
static gint dnd_trash_moveable,dnd_trash_x,dnd_trash_y,dnd_trash_move_x,dnd_trash_move_y;
static GtkTooltips *dnd_trash_speed_tooltips[2];
static GtkWidget *dnd_trash_speed_menu[2];
char *dnd_trash_tooltip_text=NULL;
float dnd_trash_tooltip_percent=0;

void dnd_trash_set_speed_text(){
	char text[MAX_LEN];
	if (dnd_trash_speed_menu[0]){
		sprintf(text,"%i B/s",CFG.SPEED_LIMIT_1);
		gtk_tooltips_set_tip(dnd_trash_speed_tooltips[0],
				     dnd_trash_speed_menu[0],
				     text,
				     (const gchar *)NULL);
	};
	if (dnd_trash_speed_menu[1]){
		sprintf(text,"%i B/s",CFG.SPEED_LIMIT_2);
		gtk_tooltips_set_tip(dnd_trash_speed_tooltips[1],
				     dnd_trash_speed_menu[1],
				     text,
				     (const gchar *)NULL);
	};
};

/*
static gint dnd_trash_tooltips_draw(GtkWidget *window,GtkTooltips *tooltip){
	if (dnd_trash_tooltip_percent<=0.001) return(FALSE);
	GtkStyle *style=dnd_trash_tooltips->tip_window->style;
	
	gint y, baseline_skip, gap,pwidth;
	GtkTooltipsData *data = dnd_trash_tooltips->active_tips_data;
	if (!data) return FALSE;
	GdkFont *font=gtk_style_get_font(style);
	gap = (font->ascent + font->descent) / 4;
	if (gap < 2) gap = 2;
	baseline_skip = font->ascent + font->descent + gap;
	y = font->ascent + 4;
	pwidth=gint(window->allocation.width*(dnd_trash_tooltip_percent/100));
	gtk_paint_flat_box(style, window->window,
			   GTK_STATE_ACTIVE, GTK_SHADOW_NONE,
			   NULL, window, "tooltip",
			   0, 0, pwidth, window->allocation.height);
	GdkGC *pr_gc=style->fg_gc[GTK_STATE_ACTIVE];
	GdkRectangle clip_rectangle;
	clip_rectangle.x=0;
	clip_rectangle.y=0;
	clip_rectangle.height=window->allocation.height;
	clip_rectangle.width=pwidth;
	gdk_gc_set_clip_rectangle (pr_gc, &clip_rectangle);
	gdk_draw_string (window->window, font, pr_gc,
			 4,y,(char*)(data->row->data));
	gdk_gc_set_clip_rectangle (pr_gc, (GdkRectangle *)NULL);
	return(FALSE);
};

*/

static void dnd_trash_init_speed_tips(){
	char *menu_path[]={
		"/DndTrash/Speed/speedlow",
		"/DndTrash/Speed/speedmedium"};
	for(int i=0;i<sizeof(menu_path)/sizeof(char*);i++){
		GtkWidget *menu_item=gtk_ui_manager_get_widget(dnd_trash_ui_manager,menu_path[i]);
		dnd_trash_speed_menu[i]=menu_item;
		if (menu_item){
			GtkTooltips *tooltip=gtk_tooltips_new();
			dnd_trash_speed_tooltips[i]=tooltip;
			gtk_tooltips_force_window(tooltip);
			GtkStyle *current_style=gtk_style_copy(gtk_widget_get_style(tooltip->tip_window));
			current_style->bg[GTK_STATE_NORMAL] = LYELLOW;
			gtk_widget_set_style(tooltip->tip_window, current_style);
			g_object_unref(G_OBJECT(current_style));
			gtk_tooltips_enable(tooltip);
		};
	};
	dnd_trash_set_speed_text();
};

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
static GtkWidget *dnd_trash_current_item=NULL;
static int dnd_drag_motion_first=1;

static void dnd_trash_overmenu_populate(tQueue *q,GtkWidget *box,int depth=0){
	d4xDownloadQueue *dq=(d4xDownloadQueue *)(q->first());
	while(dq){
		int len=2*depth;
		char *space=new char[len+1];
		space[len]=0;
		for (int i=0;i<len;i++) space[i]=' ';
		char *str=sum_strings(space,dq->name.get(),NULL);
		delete[] space;
		GtkWidget *menu_item=gtk_menu_item_new_with_label(str);
		delete[] str;
		g_object_set_data(G_OBJECT(menu_item),"d4x_user_data",dq);
		gtk_box_pack_start(GTK_BOX(box),menu_item,FALSE,FALSE,0);
		gtk_widget_show(menu_item);
		dnd_trash_overmenu_populate(&(dq->child),box,depth+1);
		dq=(d4xDownloadQueue *)(dq->prev);
	};
};

static GtkWidget *dnd_trash_overmenu=NULL;
void dnd_trash_switch_to_menu(){
	dnd_trash_current_item=NULL;
	gtk_widget_shape_combine_mask(dnd_trash_window,NULL,0,0);
	gtk_widget_ref(dnd_trash_fixed);
	gtk_container_remove(GTK_CONTAINER(dnd_trash_window),dnd_trash_fixed);
	dnd_trash_overmenu=gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(dnd_trash_window),dnd_trash_overmenu);
	gtk_container_set_border_width(GTK_CONTAINER(dnd_trash_window),2);
	gtk_widget_show(dnd_trash_overmenu);
	dnd_trash_overmenu_populate(&D4X_QTREE,dnd_trash_overmenu);
};

void dnd_trash_switch_to_icon(){
	gint width,height;
	gtk_widget_destroy(dnd_trash_overmenu);
	gtk_container_add(GTK_CONTAINER(dnd_trash_window), dnd_trash_fixed);
	gtk_container_set_border_width(GTK_CONTAINER(dnd_trash_window),0);
	if (CFG.GRAPH_ON_BASKET==0){
		gtk_image_set_from_pixbuf(GTK_IMAGE(dnd_trash_gtk_pixmap),
					  dnd_trash_pixbuf1);
		width=gdk_pixbuf_get_width(dnd_trash_pixbuf1);
		height=gdk_pixbuf_get_width(dnd_trash_pixbuf1);
		gtk_widget_shape_combine_mask(dnd_trash_window, dnd_trash_mask1, 0, 0 );
		gtk_window_resize(GTK_WINDOW(dnd_trash_window),width,height);
	};
	gtk_widget_unref(dnd_trash_fixed);
};

static void d4x_menuitem_foreach1(GtkWidget *widget,gpointer data){
	if (GTK_IS_MENU_ITEM (widget)){
		GtkWidget **w=(GtkWidget **)data;
		gint x,y;
		gtk_widget_get_pointer(widget,&x,&y);
		if (y>0 && y<=widget->allocation.height)
			*w=widget;
	};
};

static GtkWidget *d4x_dnd_get_item(GtkWidget *dnd_trash_overmenu){
	GtkWidget *rval=NULL;
	gtk_container_foreach(GTK_CONTAINER (dnd_trash_overmenu),d4x_menuitem_foreach1,&rval);
	return(rval);
};

static gboolean dnd_drag_motion (GtkWidget *widget,
				 GdkDragContext *context,
				 gint x,gint y,
				 guint time,
				 gpointer data){
	if (dnd_drag_motion_first){
		if (d4x_only_one_queue()) return(TRUE);
		dnd_drag_motion_first=0;
		dnd_trash_target_queue=NULL;
		dnd_trash_switch_to_menu();
		return(TRUE);
	};
	GtkWidget *menu_item=d4x_dnd_get_item(dnd_trash_overmenu);
	if (dnd_trash_current_item!=menu_item &&
	    dnd_trash_current_item)
		gtk_menu_item_deselect(GTK_MENU_ITEM(dnd_trash_current_item));
	if (menu_item){
		gtk_menu_item_select(GTK_MENU_ITEM(menu_item));
		dnd_trash_target_queue=(d4xDownloadQueue *)g_object_get_data(G_OBJECT(menu_item),"d4x_user_data");
		dnd_trash_current_item=menu_item;
	};
	return(TRUE);
};

static gboolean dnd_drag_leave (GtkWidget *widget,
				GdkDragContext *context,
				gint x,gint y,
				guint time,
				gpointer data){
	if (!dnd_drag_motion_first){
		dnd_drag_motion_first=1;
		dnd_trash_switch_to_icon();
		return(TRUE);
	};
	return(TRUE);
};

void dnd_trash_motion(GtkWidget *widget,GdkEventMotion *event){
	if (dnd_trash_moveable){
		gint mx,my;
		GdkModifierType modmask;
		gdk_window_get_pointer((GdkWindow *)NULL, &mx, &my, &modmask);
		dnd_trash_move_x+=mx-dnd_trash_x;
		dnd_trash_move_y+=my-dnd_trash_y;
		gdk_window_move(widget->window,
				dnd_trash_move_x,
				dnd_trash_move_y);
		dnd_trash_x=mx;
		dnd_trash_y=my;
		gdk_flush();
	};
};

void dnd_trash_menu_popdown(GdkEventButton *event){
	gtk_menu_popup(GTK_MENU(dnd_trash_menu),
		       (GtkWidget *)NULL,
		       (GtkWidget *)NULL,
		       (GtkMenuPositionFunc)NULL,
		       (gpointer)NULL,
		       event->button,event->time);
};

int dnd_trash_button_press(GtkWidget *widget,GdkEventButton *event){
	switch (event->button){
	case 3:{
		CFG.DND_NEED_POPUP=0;
		dnd_trash_menu_popdown(event);
		break;
	};
	case 1:{
		if (event->type==GDK_2BUTTON_PRESS)
			main_window_toggle();
		else{
			if (!dnd_trash_moveable){
				dnd_trash_move_x=CFG.DND_TRASH_X;
				dnd_trash_move_y=CFG.DND_TRASH_Y;
				dnd_trash_moveable=1;
				dnd_trash_x=gint(event->x_root);
				dnd_trash_y=gint(event->y_root);
				gtk_grab_add (widget);
				gdk_pointer_grab (widget->window,TRUE,
						  GdkEventMask(GDK_BUTTON_RELEASE_MASK |
							       GDK_BUTTON_MOTION_MASK |
							       GDK_POINTER_MOTION_HINT_MASK),
						  NULL, NULL, 0);
			};
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
	if (CFG.DND_NEED_POPUP && dnd_trash_window){
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

/*
static GdkPixbuf *dnd_trash_scale(GdkPixbuf *pixbuf){
	int temp,w,h;
	gdk_window_get_geometry((GdkWindow *)NULL,&temp,&temp,&w,&h,&temp);
	if (w<2048){
		float ratio=float(w)/2048.0;
		w=gdk_pixbuf_get_width(pixbuf);
		h=gdk_pixbuf_get_height(pixbuf);
		GdkPixbuf *rval=gdk_pixbuf_scale_simple(pixbuf,gint(float(w)*ratio),gint(float(h)*ratio),GDK_INTERP_HYPER);
		gdk_pixbuf_unref(pixbuf);
		return(rval);
	};
	return(pixbuf);
};
*/

void dnd_trash_destroy_theme(){
	if (dnd_trash_pixbuf1){
		gdk_pixbuf_unref(dnd_trash_pixbuf1);
		dnd_trash_pixbuf1=NULL;
	};
	if (dnd_trash_pixbuf2){
		gdk_pixbuf_unref(dnd_trash_pixbuf2);
		dnd_trash_pixbuf2=NULL;
	};
	dnd_trash_tooltip_text=NULL;
	if (dnd_trash_tooltips){
		gtk_object_destroy(GTK_OBJECT(dnd_trash_tooltips));
		dnd_trash_tooltips=NULL;
	};
};

void dnd_trash_init(){
	CFG.DND_TRASH=1;
	CFG.DND_NEED_POPUP=1;
	if (dnd_trash_window) return;
	dnd_trash_moveable=0;
#include "pixmaps/dndtrash.xpm"
#include "pixmaps/dndtrashi.xpm"
	/* GtkWidget is the storage type for widgets */
	GtkWidget *pixmap;
	GtkStyle *style;
    
//	dnd_trash_window = gtk_window_new( GTK_WINDOW_POPUP );
	dnd_trash_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_stick(GTK_WINDOW(dnd_trash_window));
//	gtk_window_set_type_hint (GTK_WINDOW(dnd_trash_window),
//				  GDK_WINDOW_TYPE_HINT_DIALOG);
//	gtk_window_set_type_hint (GTK_WINDOW(dnd_trash_window),
//				  GDK_WINDOW_TYPE_HINT_NORMAL);
	gtk_window_set_resizable(GTK_WINDOW(dnd_trash_window),FALSE);
	gtk_window_set_wmclass(GTK_WINDOW(dnd_trash_window),"D4X_DnDBasket","D4X");
	gtk_window_set_title(GTK_WINDOW (dnd_trash_window), _("DnD basket"));
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
	/* create pixmaps first
	 */
	gint width=0,height=0;
	gtk_widget_realize(dnd_trash_window);
	gtk_window_set_keep_above (GTK_WINDOW(dnd_trash_window),TRUE);
	gtk_window_set_skip_taskbar_hint (GTK_WINDOW(dnd_trash_window),TRUE);
	gtk_window_set_skip_pager_hint(GTK_WINDOW(dnd_trash_window),TRUE);
	gtk_window_set_accept_focus(GTK_WINDOW(dnd_trash_window),FALSE);
//	wm_skip_window(dnd_trash_window);
	if (dnd_trash_pixbuf1==NULL){
		dnd_trash_pixbuf1=pixbuf_from_theme("dndbasket icon>file",(const char**)dndtrash_xpm);
		dnd_trash_pixbuf2=pixbuf_from_theme("dndbasket dropicon>file",(const char**)dndtrashi_xpm);
		gdk_pixbuf_ref(dnd_trash_pixbuf2);
		gdk_pixbuf_render_pixmap_and_mask(dnd_trash_pixbuf1,NULL,&dnd_trash_mask1,1);
		gdk_bitmap_ref(dnd_trash_mask1);
		gdk_pixbuf_render_pixmap_and_mask(dnd_trash_pixbuf2,NULL,&dnd_trash_mask2,1);
		gdk_bitmap_ref(dnd_trash_mask2);
	};
	width=gdk_pixbuf_get_width(dnd_trash_pixbuf1);
	height=gdk_pixbuf_get_width(dnd_trash_pixbuf1);
	d4x_normalize_coords(&(CFG.DND_TRASH_X),&(CFG.DND_TRASH_Y),width,height);
	gtk_window_move(GTK_WINDOW(dnd_trash_window), gint(CFG.DND_TRASH_X),gint(CFG.DND_TRASH_Y));
	/* Create the main window, and attach delete_event signal to terminate
	 * the application.  Note that the main window will not have a titlebar
	 * since we're making it a popup. */
//	gtk_window_set_transient_for(GTK_WINDOW(dnd_trash_window), GTK_WINDOW(MainWindow));
	gtk_window_set_default_size(GTK_WINDOW(dnd_trash_window),width,height);
//	gtk_widget_set_events(dnd_trash_window,GDK_ALL_EVENTS_MASK);
	g_signal_connect(G_OBJECT (dnd_trash_window), "delete_event",
			 G_CALLBACK (dnd_trash_destroy), NULL);
	g_signal_connect(G_OBJECT (dnd_trash_window), "motion_notify_event",
			 G_CALLBACK (dnd_trash_motion), NULL);
	g_signal_connect(G_OBJECT (dnd_trash_window), "button_press_event",
			 G_CALLBACK (dnd_trash_button_press), NULL);
	g_signal_connect(G_OBJECT (dnd_trash_window), "button_release_event",
			 G_CALLBACK (dnd_trash_button_release), NULL);
	
	g_signal_connect(G_OBJECT(dnd_trash_window),
			 "drag_data_received",
			 G_CALLBACK(list_dnd_drop_internal),
			 NULL);
	g_signal_connect(G_OBJECT (dnd_trash_window), "drag_leave",
			 G_CALLBACK (dnd_drag_leave), NULL);
	g_signal_connect(G_OBJECT (dnd_trash_window),"drag_motion",
			 G_CALLBACK (dnd_drag_motion),NULL);
	gtk_drag_dest_set(GTK_WIDGET(dnd_trash_window),
	                  (GtkDestDefaults)(GTK_DEST_DEFAULT_MOTION |
	                                    GTK_DEST_DEFAULT_HIGHLIGHT |
	                                    GTK_DEST_DEFAULT_DROP),
	                  download_drop_types, n_download_drop_types,
	                  (GdkDragAction)(GDK_ACTION_COPY|GDK_ACTION_MOVE));
	g_signal_connect(G_OBJECT(dnd_trash_window), "visibility_notify_event",
			 G_CALLBACK(dnd_trash_no_expose),
			 dnd_trash_window);			  
	g_signal_connect(G_OBJECT(dnd_trash_window), "no_expose_event",
			 G_CALLBACK(dnd_trash_no_expose),
			 dnd_trash_window);
	gtk_widget_realize(dnd_trash_window);
	gdk_window_set_decorations(dnd_trash_window->window,GdkWMDecoration(0));
	gtk_widget_show (dnd_trash_window);


	/* Now for the pixmap and the pixmap widget */
//		gtk_widget_show( pixmap );
	/* To display the pixmap, we use a fixed widget to place the pixmap */
	dnd_trash_fixed = gtk_fixed_new();
//	gtk_widget_set_usize( dnd_trash_fixed, width, height );
//	gtk_event_box_new();

	if (dnd_basket_graph==NULL){
		dnd_basket_graph=my_gtk_graph_new();
		D4X_DND_GRAPH=(MyGtkGraph *)dnd_basket_graph;
		D4X_DND_GRAPH->show_speed=CFG.SHOW_SPEED_ON_BASKET;
		D4X_DND_GRAPH->show_offline=1;
		D4X_DND_GRAPH->GlobalM=GraphMeter;
		D4X_DND_GRAPH->LocalM=GraphLMeter;
		gtk_widget_ref(dnd_basket_graph);
		gtk_widget_set_size_request(dnd_basket_graph,50,50);
	};
	if (CFG.GRAPH_ON_BASKET)
		gtk_fixed_put( GTK_FIXED(dnd_trash_fixed), dnd_basket_graph, 0, 0);
	else{
		dnd_trash_gtk_pixmap = pixmap = gtk_image_new_from_pixbuf(dnd_trash_pixbuf1);
		gtk_fixed_put( GTK_FIXED(dnd_trash_fixed), pixmap, 0, 0 );
	};

	gtk_container_add( GTK_CONTAINER(dnd_trash_window), dnd_trash_fixed );
	gtk_widget_show_all( dnd_trash_fixed );

	d4xXmlObject *xmltip=d4x_xml_find_obj(D4X_THEME_DATA,"dndbasket tooltip");
	if (dnd_trash_tooltips==NULL){
		dnd_trash_tooltips=gtk_tooltips_new();
		gtk_tooltips_force_window(dnd_trash_tooltips);
		GtkStyle *current_style =gtk_style_copy(gtk_widget_get_style(dnd_trash_tooltips->tip_window));
/* FIXME: GTK2
		g_signal_connect(G_OBJECT(dnd_trash_tooltips->tip_window), "draw",
				   G_CALLBACK(dnd_trash_tooltips_draw),NULL);
		g_signal_connect(G_OBJECT(dnd_trash_tooltips->tip_window), "expose_event",
				   G_CALLBACK(dnd_trash_tooltips_draw),NULL);
*/
		char *bgcolor=NULL,*fgcolor=NULL;
		if (xmltip){
			d4xXmlField *fld=NULL;
			fld=xmltip->get_attr("bgcolor");
			bgcolor=fld?fld->value.get():NULL;
			fld=xmltip->get_attr("fgcolor");
			fgcolor=fld?fld->value.get():NULL;
		};
		if (bgcolor){
			gdk_color_parse(bgcolor,&(current_style->bg[GTK_STATE_NORMAL]));
			current_style->fg[GTK_STATE_ACTIVE]=current_style->bg[GTK_STATE_NORMAL];
		}else
			current_style->bg[GTK_STATE_NORMAL]=current_style->fg[GTK_STATE_ACTIVE]=LYELLOW;
		if (fgcolor)
			gdk_color_parse(fgcolor,&(current_style->fg[GTK_STATE_NORMAL]));
		current_style->bg[GTK_STATE_ACTIVE]=current_style->fg[GTK_STATE_NORMAL];
		gtk_widget_set_style(dnd_trash_tooltips->tip_window, current_style);
	};
	if (xmltip){
		dnd_trash_tooltip_text=xmltip->value.get();
	}else
		dnd_trash_tooltip_text=_("Drop link here");
	gtk_tooltips_set_tip(dnd_trash_tooltips,
			     dnd_trash_window,
			     dnd_trash_tooltip_text,
			     (const gchar *)NULL);
	gtk_tooltips_enable(dnd_trash_tooltips);
	/* This mask cut out everything except the image */
	if (dnd_trash_mask1 && CFG.GRAPH_ON_BASKET==0)
		gtk_widget_shape_combine_mask( dnd_trash_window, dnd_trash_mask1,0,0);
	/* show the window */
	gtk_widget_show( dnd_trash_window );
	gdk_window_resize(dnd_trash_window->window,width,height);
	set_dndtrash_button();
	gdk_window_move(dnd_trash_window->window,CFG.DND_TRASH_X,CFG.DND_TRASH_Y);
	g_signal_connect(G_OBJECT(dnd_trash_window), "configure_event",
			 G_CALLBACK(dnd_trash_configure),
			 NULL);
	gdk_window_set_functions(dnd_trash_window->window,GdkWMFunction(GDK_FUNC_MOVE|GDK_FUNC_CLOSE));
	x_opacity_set(dnd_trash_window,(unsigned int)(0xffffffff*0.6));
};

static bool _no_speed_callback_dnd_=false;

void dnd_trash_menu_prepare(){
	if (_no_speed_callback_dnd_) return;
	_no_speed_callback_dnd_=true;
	switch (CFG.SPEED_LIMIT){
	case 1:{
		GtkAction *action1=gtk_ui_manager_get_action(dnd_trash_ui_manager,"/DndTrash/Speed/speedlow");
		gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(action1),TRUE);
		break;
	};
	case 2:{
		GtkAction *action2=gtk_ui_manager_get_action(dnd_trash_ui_manager,"/DndTrash/Speed/speedmedium");
		gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(action2),TRUE);
		break;
	};
	default:{
		GtkAction *action3=gtk_ui_manager_get_action(dnd_trash_ui_manager,"/DndTrash/Speed/speedunlim");
		gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(action3),TRUE);
	};
	};
	GtkAction *action=gtk_ui_manager_get_action(dnd_trash_ui_manager,"/DndTrash/offline");
	gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(action),CFG.OFFLINE_MODE?TRUE:FALSE);
	g_object_set(G_OBJECT(action),"label",CFG.OFFLINE_MODE?_("Offline"):_("Online"),NULL);
	_no_speed_callback_dnd_=false;
};

void dnd_trash_menu_callback(GtkAction *action){
	if (_no_speed_callback_dnd_) return;
	_no_speed_callback_dnd_=true;
	CFG.SPEED_LIMIT=gtk_radio_action_get_current_value (GTK_RADIO_ACTION (action))+1;
	set_speed_buttons();
	_no_speed_callback_dnd_=false;
};

static void dnd_trash_delete_completed(){
	_aa_.del_completed();
};

void dnd_trash_set_del_completed(gint how){
	GtkWidget *menu_item=gtk_ui_manager_get_widget(dnd_trash_ui_manager,"/DndTrash/delc");
	if (menu_item)
		gtk_widget_set_sensitive(menu_item,how);
};

void dnd_trash_offline(){
	if (_no_speed_callback_dnd_) return;
	_aa_.switch_offline_mode();
};

static gint dnd_trash_menu_umap(){
	CFG.DND_NEED_POPUP=1;
	dnd_trash_no_expose();
	return(FALSE);
};
void dnd_trash_init_menu() {
	static char *ui_info=
		"<popup name='DndTrash'>"
		"   <menuitem action='new' />"
		"   <menuitem action='paste' />"
		"   <menuitem action='auto' />"
		"   <menuitem action='delc' />"
		"   <separator name='sep1' />"
		"   <menuitem action='props' />"
		"   <menu action='Speed'>"
		"        <menuitem action='speedlow' />"
		"        <menuitem action='speedmedium' />"
		"        <menuitem action='speedunlim' />"
		"   </menu>"
		"   <menuitem action='offline' />"
		"   <separator name='sep2' />"
		"   <menuitem action='exit' />"
		"</popup>";
	static GtkActionEntry entries[] = {
		{ "DndTrash", NULL, "DnDTrash" },
		{ "Speed", NULL, N_("Speed") },
		{ "new", GTK_STOCK_NEW, N_("New Download"), NULL, "add new file to download",  G_CALLBACK(init_add_window)},
		{ "paste",GTK_STOCK_PASTE,N_("Paste Download"), NULL, "add new from clipboard",  G_CALLBACK(init_add_clipboard_window)},
		{ "auto",NULL,N_("Automated adding"), NULL, "automated adding many downloads",  G_CALLBACK(d4x_automated_add)},
		{ "delc",NULL,N_("Delete completed"), NULL, "remove completed downloads",  G_CALLBACK(dnd_trash_delete_completed)},
		{ "props",GTK_STOCK_PREFERENCES,N_("General options"), NULL, "open main preferences dialog",  G_CALLBACK(d4x_prefs_init)},
		{ "exit",GTK_STOCK_QUIT,N_("Exit"), NULL, "open main preferences dialog",  G_CALLBACK(ask_exit)}
	};
	static GtkToggleActionEntry toggle_entries[]={
		{ "offline",NULL,N_("Online"), NULL, "switch to or from offline mode",  G_CALLBACK(dnd_trash_offline),FALSE}
	};
	static GtkRadioActionEntry radio_entries[] = {
		{ "speedlow", NULL, N_("Low"), NULL, "set low traffic limitation", 0 },
		{ "speedmedium", NULL, N_("Medium"), NULL, "set medium traffic limitation", 1 },
		{ "speedunlim", NULL, N_("Unlimited"), NULL, "set unlimited speed", 2 }
	};
	
	GtkActionGroup *action_group = gtk_action_group_new ("QueueActions");
	gtk_action_group_set_translate_func(action_group,d4x_menu_translate_func,NULL,NULL);
	dnd_trash_ui_manager=gtk_ui_manager_new ();
	gtk_action_group_add_actions (action_group, entries, G_N_ELEMENTS (entries), NULL);
	gtk_action_group_add_toggle_actions (action_group, toggle_entries, G_N_ELEMENTS (toggle_entries), NULL);
	gtk_action_group_add_radio_actions (action_group, radio_entries, G_N_ELEMENTS (radio_entries), CFG.SPEED_LIMIT-1, G_CALLBACK(dnd_trash_menu_callback), NULL);
	gtk_ui_manager_insert_action_group (dnd_trash_ui_manager, action_group, 0);
	GError *error = NULL;
	if (!gtk_ui_manager_add_ui_from_string (dnd_trash_ui_manager, ui_info, -1, &error))
	{
		g_message ("building menus failed: %s", error->message);
		g_error_free (error);
		exit (EXIT_FAILURE);
	}
	
	dnd_trash_menu = gtk_ui_manager_get_widget (dnd_trash_ui_manager,"/DndTrash");
	g_signal_connect(G_OBJECT (dnd_trash_menu), "unmap_event",
			 G_CALLBACK(dnd_trash_menu_umap), NULL);
	dnd_trash_init_speed_tips();
};

static gint dnd_trash_animation_end(gpointer unused){
	if (CFG.GRAPH_ON_BASKET) return 0;
	gint width,height;
	gtk_image_set_from_pixbuf(GTK_IMAGE(dnd_trash_gtk_pixmap),
				  dnd_trash_pixbuf1);
	width=gdk_pixbuf_get_width(dnd_trash_pixbuf1);
	height=gdk_pixbuf_get_height(dnd_trash_pixbuf1);
	gdk_window_resize(dnd_trash_window->window,width,height);
	gtk_widget_shape_combine_mask(dnd_trash_window, dnd_trash_mask1, 0, 0 );
	/* draw NOW */
	GdkRectangle rect;
	rect.x=dnd_trash_gtk_pixmap->allocation.x;
	rect.y=dnd_trash_gtk_pixmap->allocation.y;
	rect.width=dnd_trash_gtk_pixmap->allocation.width;
	rect.height=dnd_trash_gtk_pixmap->allocation.height;
	gtk_widget_queue_draw(dnd_trash_gtk_pixmap);
	return 0;
};

void dnd_trash_animation(){
	if (CFG.GRAPH_ON_BASKET==0 && dnd_trash_window){
		gint width,height;
		gtk_image_set_from_pixbuf(GTK_IMAGE(dnd_trash_gtk_pixmap),
					  dnd_trash_pixbuf2);
		width=gdk_pixbuf_get_width(dnd_trash_pixbuf2);
		height=gdk_pixbuf_get_height(dnd_trash_pixbuf2);
		gdk_window_resize(dnd_trash_window->window,width,height);
		gtk_widget_shape_combine_mask(dnd_trash_window, dnd_trash_mask2,0, 0 );
		GdkRectangle rect;
		rect.x=dnd_trash_gtk_pixmap->allocation.x;
		rect.y=dnd_trash_gtk_pixmap->allocation.y;
		rect.width=dnd_trash_gtk_pixmap->allocation.width;
		rect.height=dnd_trash_gtk_pixmap->allocation.height;
		gtk_widget_queue_draw(dnd_trash_gtk_pixmap);
//		gtk_widget_draw(dnd_trash_gtk_pixmap,&rect);
		g_timeout_add (500, dnd_trash_animation_end , NULL);
	};
};

void dnd_trash_set_tooltip(const char *str,float percent){
	dnd_trash_tooltip_percent=percent;
	if (str==NULL)
		str=dnd_trash_tooltip_text;
	if (str==NULL || dnd_trash_window==NULL) return;
	gtk_tooltips_set_tip(dnd_trash_tooltips,
			     dnd_trash_window,
			     str,NULL);

};


void dnd_trash_redraw(){
	gtk_widget_queue_draw(GTK_WIDGET(dnd_trash_window));
};
