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
#include <string.h>
#include "list.h"
#include "../main.h"
#include "../var.h"
#include "../ntlocale.h"
#include "graph.h"

/********************************************************************/
MyGtkGraph *GLOBAL_GRAPH;


static GtkWidgetClass *parent_class = (GtkWidgetClass *)NULL;

static void my_gtk_graph_class_init(MyGtkGraphClass *klass);
//static void my_gtk_graph_destroy(GtkObject *widget);
static void my_gtk_graph_finalize(GtkObject *widget);
static void my_gtk_graph_init(MyGtkGraph *graph);
static void my_gtk_graph_draw(GtkWidget *widget,GdkRectangle *area);
static void my_gtk_graph_realize (GtkWidget *widget);
static gint my_gtk_graph_expose (GtkWidget *widget, GdkEventExpose *event);
static void my_gtk_graph_size_allocate(GtkWidget *widget,
				       GtkAllocation *allocation);
static void my_gtk_graph_reinit(MyGtkGraph *graph);

GtkType
my_gtk_graph_get_type (void)
{
  static GtkType graph_type = 0;
  
  if (!graph_type)
    {
      static const GtkTypeInfo graph_info =
      {
	"MyGtkGraph",
	sizeof (MyGtkGraph),
	sizeof (MyGtkGraphClass),
	(GtkClassInitFunc) my_gtk_graph_class_init,
	(GtkObjectInitFunc) my_gtk_graph_init,
        /* reserved_1 */ NULL,
	/* reserved_2 */ NULL,
	(GtkClassInitFunc) NULL,
      };
      
      graph_type = gtk_type_unique (gtk_widget_get_type (), &graph_info);
    }
  
  return graph_type;
}

static void my_gtk_graph_class_init(MyGtkGraphClass *klass){
	GtkWidgetClass *widget_class=(GtkWidgetClass *)klass;
	GtkObjectClass *object_class = (GtkObjectClass*) klass;
	
//	object_class->destroy = my_gtk_graph_destroy;
	object_class->finalize = my_gtk_graph_finalize;
	widget_class->draw = my_gtk_graph_draw;
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
static void my_gtk_graph_finalize (GtkObject *object){
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

	if (GTK_OBJECT_CLASS (parent_class)->finalize)
		GTK_OBJECT_CLASS (parent_class)->finalize (object);
}


static void my_gtk_graph_init(MyGtkGraph *graph){
	graph->rgb_data=(guchar *)NULL;
	graph->cmap=(GdkRgbCmap *)NULL;
};

GtkWidget *my_gtk_graph_new(){
	MyGtkGraph *graph=(MyGtkGraph *)gtk_type_new(my_gtk_graph_get_type());
	my_gtk_graph_cmap_reinit(graph);
	return GTK_WIDGET(graph);
};

static void my_gtk_graph_draw(GtkWidget *widget,GdkRectangle *area){
	g_return_if_fail(widget!=NULL);
	g_return_if_fail(area!=NULL);

	MyGtkGraph *graph=(MyGtkGraph *)widget;
	gtk_paint_box (widget->style,
		       widget->window,
		       GTK_STATE_NORMAL, GTK_SHADOW_IN,
		       (GdkRectangle *)NULL, widget, "trough",
		       0, 0,
		       widget->allocation.width,
		       widget->allocation.height);
	if (graph->rgb_data==NULL){
		my_gtk_graph_reinit(graph);
	};
	if (graph->rgb_data && graph->cmap)
		gdk_draw_indexed_image(widget->window,
				       MainWindowGC,
				       2,2,
				       widget->allocation.width-4,
				       widget->allocation.height-4,
				       GDK_RGB_DITHER_NONE,
				       graph->rgb_data,
				       widget->allocation.width-4,
				       graph->cmap);

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
//  g_return_val_if_fail (GTK_IS_PROGRESS (widget), FALSE);

	if (GTK_WIDGET_DRAWABLE (widget))
		my_gtk_graph_draw(widget,&(event->area));
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
	if (!GlobalMeter) return;
	if (!LocalMeter) return;
	int XSize=widget->allocation.width-4;//2*(METER_LENGTH);
	int YSize=widget->allocation.height-5;
	memset(graph->rgb_data,3,XSize*(YSize+1));

	int MAX=GlobalMeter->max();
	int MAX2=LocalMeter->max();
	MAX=MAX2>MAX?MAX2:MAX;
	int NUM=GlobalMeter->count();
	if (NUM>XSize) NUM=XSize;
	if (MAX>0) {
		float value=float((YSize*GlobalMeter->last_value())/float(MAX));
		float value2=float((YSize*LocalMeter->last_value())/float(MAX));
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
				value=float((YSize*GlobalMeter->next_value())/float(MAX));
				value2=float((YSize*LocalMeter->next_value())/float(MAX));
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
				value=float((YSize*GlobalMeter->next_value())/float(MAX));
				value2=float((YSize*LocalMeter->next_value())/float(MAX));
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
	if (graph->cmap)
		gdk_rgb_cmap_free(graph->cmap);
	graph->cmap=gdk_rgb_cmap_new(colors,4);
	if (GTK_WIDGET_DRAWABLE (GTK_WIDGET(graph)))
		gtk_widget_queue_draw(GTK_WIDGET(graph));
};
