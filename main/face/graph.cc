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


#include <string.h>
#include "list.h"
#include "../var.h"
#include "../ntlocale.h"
#include "graph.h"

/********************************************************************/
MyGtkGraph *GLOBAL_GRAPH;
MyGtkGraph *D4X_DND_GRAPH;


static GtkWidgetClass *parent_class = (GtkWidgetClass *)NULL;

static void my_gtk_graph_class_init(MyGtkGraphClass *klass);
//static void my_gtk_graph_destroy(GtkObject *widget);
static void my_gtk_graph_finalize(GObject *widget);
static void my_gtk_graph_init(MyGtkGraph *graph);
static void my_gtk_graph_draw(GtkWidget *widget,GdkRectangle *area);
static void my_gtk_graph_realize (GtkWidget *widget);
static gint my_gtk_graph_expose (GtkWidget *widget, GdkEventExpose *event);
static void my_gtk_graph_size_allocate(GtkWidget *widget,
				       GtkAllocation *allocation);
static void my_gtk_graph_reinit(MyGtkGraph *graph);

GtkType
my_gtk_graph_get_type (void){
	static GtkType graph_type = 0;
	
	if (!graph_type) {
		static const GTypeInfo graph_info={
			sizeof (MyGtkGraphClass),
			NULL,NULL,
			(GClassInitFunc) my_gtk_graph_class_init,
			NULL,NULL,
			sizeof (MyGtkGraph),
			0,
			(GInstanceInitFunc)my_gtk_graph_init
		};
		graph_type = g_type_register_static (GTK_TYPE_WIDGET,"MyGtkGraph",&graph_info,(GTypeFlags)0);
	}
	return graph_type;
}

static void my_gtk_graph_class_init(MyGtkGraphClass *klass){
	GtkWidgetClass *widget_class=(GtkWidgetClass *)klass;
	GtkObjectClass *object_class = (GtkObjectClass*) klass;
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
	
//	object_class->destroy = my_gtk_graph_destroy;
	gobject_class->finalize = my_gtk_graph_finalize;
//	widget_class->draw = my_gtk_graph_draw;
	widget_class->realize = my_gtk_graph_realize;
	widget_class->expose_event = my_gtk_graph_expose;
	widget_class->size_allocate = my_gtk_graph_size_allocate;
	parent_class=(GtkWidgetClass *)gtk_type_class(gtk_widget_get_type());
};

/*
static void my_gtk_graph_destroy(GtkObject *widget){
	g_return_if_fail(widget!=NULL);
	MyGtkGraph *graph=(MyGtkGraph *)widget;
	if (graph->rgb_data){
		g_free(graph->rgb_data);
	};
	if (graph->cmap)
		gdk_rgb_cmap_free(graph->cmap);
	graph->cmap=NULL;
	graph->rgb_data=NULL;
	if (GTK_OBJECT_CLASS (parent_class)->destroy)
		(* GTK_OBJECT_CLASS (parent_class)->destroy) (widget);
};
*/
static void my_gtk_graph_finalize (GObject *object){
	MyGtkGraph *graph;

	g_return_if_fail (object != NULL);
//  g_return_if_fail (GTK_IS_GRAPH (object));

	graph = (MyGtkGraph *)(object);

	if (graph->rgb_data){
		g_free(graph->rgb_data);
	};
	if (graph->cmap)
		gdk_rgb_cmap_free(graph->cmap);
	graph->cmap=(GdkRgbCmap *)NULL;
	graph->rgb_data=(guchar *)NULL;

	if (G_OBJECT_CLASS (parent_class)->finalize)
		G_OBJECT_CLASS (parent_class)->finalize (object);
	if (graph->font_desc)
		pango_font_description_free(graph->font_desc);
}


static void my_gtk_graph_init(MyGtkGraph *graph){
	graph->rgb_data=(guchar *)NULL;
	graph->show_speed=0;
	graph->cmap=(GdkRgbCmap *)NULL;
	graph->font_desc=pango_font_description_from_string("MiscFixed 8");
//	graph->font_desc=pango_font_description_from_string("");
};

GtkWidget *my_gtk_graph_new(){
	MyGtkGraph *graph=(MyGtkGraph *)g_object_new(my_gtk_graph_get_type(),NULL);
	my_gtk_graph_cmap_reinit(graph);
	return GTK_WIDGET(graph);
};

static void my_gtk_graph_draw(GtkWidget *widget,GdkRectangle *area){
	g_return_if_fail(widget!=NULL);
	g_return_if_fail(area!=NULL);

	MyGtkGraph *graph=(MyGtkGraph *)widget;
	gtk_paint_shadow (widget->style,
			  widget->window,
			  GTK_STATE_NORMAL,
			  GTK_SHADOW_IN,
			  area,widget,"trough",
			  0,0,
			  widget->allocation.width,
			  widget->allocation.height);
	if (graph->rgb_data==NULL){
		my_gtk_graph_reinit(graph);
	};
	if (area->x>widget->allocation.width-2 ||
	    area->y>widget->allocation.height-2 ||
	    area->x+area->width<2 || area->y+area->height<2){
		return;
	};
	if (graph->rgb_data && graph->cmap){
		if (area->x<=2 && area->y<=2 &&
		    area->width>=widget->allocation.width-4 &&
		    area->height>=widget->allocation.height-4){
			gdk_draw_indexed_image(widget->window,
					       MainWindowGC,
					       2,2,
					       widget->allocation.width-4,
					       widget->allocation.height-4,
					       GDK_RGB_DITHER_NONE,
					       graph->rgb_data,
					       widget->allocation.width-4,
					       graph->cmap);
		}else{
			GdkRectangle a,b;
			a.x=2;
			a.y=2;
			a.width=widget->allocation.width-4;
			a.height=widget->allocation.height-4;
			if (gdk_rectangle_intersect(area,&a,&b)){
				guchar *rgb_data=new guchar[b.width*b.height];
				b.x-=2;
				b.y-=2;
				for (int x=0;x<b.width;x++)
					for (int y=0;y<b.height;y++)
						rgb_data[y*b.width+x]=graph->rgb_data[(widget->allocation.width-4)*(b.y+y)+b.x+x];
				gdk_draw_indexed_image(widget->window,
						       MainWindowGC,
						       b.x+2,b.y+2,
						       b.width,
						       b.height,
						       GDK_RGB_DITHER_NONE,
						       rgb_data,
						       b.width,
						       graph->cmap);
				delete[] rgb_data;
			};
//			graph->rgb_data
		};
	};
	if (graph->show_speed){
		char tmpc[100];
//		char tmpd[100];
		make_number_nice(tmpc,graph->GlobalM->last_speed(),2);
//		sprintf(tmpd,"%sB/s",tmpc);
		PangoLayout *layout=gtk_widget_create_pango_layout (widget, "");
		pango_layout_set_width (layout, -1);
		pango_layout_set_text (layout, tmpc, -1);
		pango_layout_set_font_description (layout,graph->font_desc);
		gdk_draw_layout_with_colors(widget->window,
					    widget->style->black_gc,
					    4, 2,
					    layout,
					    &graph->TextColor,
					    NULL);
//		gtk_paint_layout (widget->style,
//				  widget->window,
//				  GtkStateType(GTK_WIDGET_STATE (widget)),
//				  FALSE,
//				  area,widget,"label",
//				  2,
//				  1,
//				  layout);
		g_object_unref(G_OBJECT (layout));
	};
};

static void my_gtk_graph_realize (GtkWidget *widget){
	MyGtkGraph *graph;
	GdkWindowAttr attributes;
	gint attributes_mask;

	g_return_if_fail (widget != NULL);
//  g_return_if_fail (GTK_IS_GRAPH (widget));

	graph = (MyGtkGraph *)widget;
	GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);

	attributes.window_type = GDK_WINDOW_CHILD;
	attributes.x = widget->allocation.x;
	attributes.y = widget->allocation.y;
	attributes.width = widget->allocation.width;
	attributes.height = widget->allocation.height;
	attributes.wclass = GDK_INPUT_OUTPUT;
	attributes.visual = gtk_widget_get_visual (widget);
	attributes.colormap = gtk_widget_get_colormap (widget);
	attributes.event_mask = gtk_widget_get_events (widget);
	attributes.event_mask |= GDK_EXPOSURE_MASK;

	attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;

	widget->window = gdk_window_new (gtk_widget_get_parent_window (widget),
					 &attributes, attributes_mask);
	gdk_window_set_user_data (widget->window, graph);

	widget->style = gtk_style_attach (widget->style, widget->window);
	gtk_style_set_background (widget->style, widget->window, GTK_STATE_ACTIVE);

//  gtk_progress_create_pixmap (progress);
}

static gint my_gtk_graph_expose (GtkWidget *widget, GdkEventExpose *event){
	g_return_val_if_fail (widget != NULL, FALSE);
	g_return_val_if_fail (event != NULL, FALSE);

	if (GTK_WIDGET_DRAWABLE (widget)){
		my_gtk_graph_draw(widget,&(event->area));
	};
	return FALSE;
}

static void my_gtk_graph_size_allocate(GtkWidget *widget,
				       GtkAllocation *allocation){
	g_return_if_fail (widget != NULL);
	g_return_if_fail (allocation != NULL);
//  g_return_if_fail (GTK_IS_PROGRESS (widget));

	GtkAllocation old_allocation = widget->allocation;
	widget->allocation = *allocation;
	widget->allocation.width = (widget->allocation.width / 2) * 2;

	if (GTK_WIDGET_REALIZED (widget)){
		gdk_window_move_resize (widget->window,
					allocation->x, allocation->y,
					allocation->width, allocation->height);

		if (old_allocation.width != allocation->width ||
		    old_allocation.height != allocation->height)
			my_gtk_graph_reinit((MyGtkGraph *)widget);
	};
};

static void my_gtk_graph_reinit(MyGtkGraph *graph){
	g_return_if_fail (graph != NULL);
	
	GtkWidget *widget = GTK_WIDGET(graph);
	GtkAllocation *allocation = &(widget->allocation);
	if (graph->rgb_data){
		g_free(graph->rgb_data);
	};
	if (allocation->width<=4 || allocation->height<=4)
		graph->rgb_data=(guchar *)NULL;
	else{
		graph->rgb_data = (guchar *)g_malloc((allocation->width-4)*(allocation->height-4));
		my_gtk_graph_recalc(graph);
	};
};

void my_gtk_graph_recalc(MyGtkGraph *graph){
	g_return_if_fail (graph != NULL);
	
	GtkWidget *widget = GTK_WIDGET(graph);
	if (graph->rgb_data==NULL) return;
	if (!graph->GlobalM) return;
	if (!graph->LocalM) return;
	int XSize=widget->allocation.width-4;//2*(METER_LENGTH);
	int YSize=widget->allocation.height-5;
	memset(graph->rgb_data,3,XSize*(YSize+1));

	int MAX=graph->GlobalM->max();
	int MAX2=graph->LocalM->max();
	MAX=MAX2>MAX?MAX2:MAX;
	int NUM=graph->GlobalM->count();
	if (NUM>XSize) NUM=XSize;
	if (MAX>0) {
		float value=float((YSize*graph->GlobalM->last_value())/float(MAX));
		float value2=float((YSize*graph->LocalM->last_value())/float(MAX));
		if (CFG.GRAPH_ORDER) {
			for (int x=XSize-1;x>0;x-=2) {
				int Y1=YSize-int(value);
				int Y2=YSize-int(value2);
				if (value>0 || value2>0){
					if (value>value2) {
						int y=YSize;
						for (y=YSize;y>Y2;y--) {
							graph->rgb_data[y*XSize+x]=graph->rgb_data[y*XSize+x-1]=2;
						};
						if (value2>0)
							graph->rgb_data[y*XSize+x]=graph->rgb_data[y*XSize+x-1]=2;
						else
							graph->rgb_data[y*XSize+x]=graph->rgb_data[y*XSize+x-1]=1;
						for (y=Y2-1;y>Y1;y--) {
							graph->rgb_data[y*XSize+x]=graph->rgb_data[y*XSize+x-1]=1;
						};
						graph->rgb_data[Y1*XSize+x]=graph->rgb_data[Y1*XSize+x-1]=0;
					} else {
						for (int y=YSize;y>Y2;y--) {
							graph->rgb_data[y*XSize+x]=graph->rgb_data[y*XSize+x-1]=2;
						};
						graph->rgb_data[Y2*XSize+x]=graph->rgb_data[Y2*XSize+x-1]=0;
					};
				};
				value=float((YSize*graph->GlobalM->next_value())/float(MAX));
				value2=float((YSize*graph->LocalM->next_value())/float(MAX));
			};
		} else {
			for (int x=1;x<XSize;x+=2) {
				int Y1=YSize-int(value);
				int Y2=YSize-int(value2);
				if (value>0 || value2>0){
					if (value>value2) {
						int y=YSize;
						for (y=YSize;y>Y2;y--) {
							graph->rgb_data[y*XSize+x]=graph->rgb_data[y*XSize+x-1]=2;
						};
						if (value2>0)
							graph->rgb_data[y*XSize+x]=graph->rgb_data[y*XSize+x-1]=2;
						else
							graph->rgb_data[y*XSize+x]=graph->rgb_data[y*XSize+x-1]=1;
						for (y=Y2-1;y>Y1;y--) {
							graph->rgb_data[y*XSize+x]=graph->rgb_data[y*XSize+x-1]=1;
						};
						graph->rgb_data[Y1*XSize+x]=graph->rgb_data[Y1*XSize+x-1]=0;
					} else {
						for (int y=YSize;y>Y2;y--) {
							graph->rgb_data[y*XSize+x]=graph->rgb_data[y*XSize+x-1]=2;
						};
						graph->rgb_data[Y2*XSize+x]=graph->rgb_data[Y2*XSize+x-1]=0;
					};
				};
				value=float((YSize*graph->GlobalM->next_value())/float(MAX));
				value2=float((YSize*graph->LocalM->next_value())/float(MAX));
			};
		};
	};
	gtk_widget_queue_draw(GTK_WIDGET(graph));
};

void my_gtk_graph_cmap_reinit(MyGtkGraph *graph){
	g_return_if_fail (graph != NULL);
       	guint32 colors[4];
	colors[0]=CFG.GRAPH_PICK;
	colors[1]=CFG.GRAPH_FORE1;
	colors[2]=CFG.GRAPH_FORE2;
	colors[3]=CFG.GRAPH_BACK;
	graph->TextColor.red=((colors[2]>>16)&0xff)<<8;
	graph->TextColor.green=((colors[2]>>8)&0xff)<<8;
	graph->TextColor.blue=(colors[2]&0xff)<<8;
	if (graph->cmap)
		gdk_rgb_cmap_free(graph->cmap);
	graph->cmap=gdk_rgb_cmap_new(colors,4);
	if (GTK_WIDGET_DRAWABLE (GTK_WIDGET(graph)))
		gtk_widget_queue_draw(GTK_WIDGET(graph));
};
