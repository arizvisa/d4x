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
#include <string.h>
#include "list.h"
#include "../main.h"
#include "../var.h"
#include "../ntlocale.h"
#include "graph.h"
guchar graph_rgb_data[100*16*4];
static GdkRgbCmap *cmap;
GtkWidget *graph_menu;

gint graph_expose_event_handler(GtkWidget *widget,GdkEventExpose *event) {
	draw_graph();
	return FALSE;
};

int draw_graph() {
	int XSize=2*(METER_LENGTH);
	int YSize=16;
	int WX,WY;
	gdk_window_get_size(MainWindow->window,&WX,&WY);
	gdk_draw_indexed_image(MainWindow->window,MainWindowGC,WX-3-XSize,WY-2-YSize,XSize,YSize,
	                       GDK_RGB_DITHER_NONE,(guchar *)graph_rgb_data,XSize,cmap);
	gdk_flush();
	return FALSE;
};

void recalc_graph() {
	if (!GlobalMeter) return;
	int XSize=2*(METER_LENGTH);
	int YSize=16;
	int WX,WY;
	gdk_window_get_size(MainWindow->window,&WX,&WY);
	memset(graph_rgb_data,3,XSize*YSize);

	int MAX=GlobalMeter->max();
	int MAX2=LocalMeter->max();
	MAX=MAX2>MAX?MAX2:MAX;
	int NUM=GlobalMeter->count();
	if (NUM>XSize) NUM=XSize;
	if (MAX>0) {
		int value=(YSize*GlobalMeter->last_value())/MAX;
		int value2=(YSize*LocalMeter->last_value())/MAX;
		if (CFG.GRAPH_ORDER) {
			for (int x=XSize-1;x>0;x-=2) {
				int Y1=YSize-value;
				int Y2=YSize-value2;
				if (Y1<Y2) {
					for (int y=YSize-1;y>=Y2;y--) {
						graph_rgb_data[y*XSize+x]=graph_rgb_data[y*XSize+x-1]=2;
					};
					for (int y=Y2-1;y>Y1;y--) {
						graph_rgb_data[y*XSize+x]=graph_rgb_data[y*XSize+x-1]=1;
					};
					graph_rgb_data[Y1*XSize+x]=graph_rgb_data[Y1*XSize+x-1]=0;
				} else {
					for (int y=YSize-1;y>Y2;y--) {
						graph_rgb_data[y*XSize+x]=graph_rgb_data[y*XSize+x-1]=2;
					};
					graph_rgb_data[Y2*XSize+x]=graph_rgb_data[Y2*XSize+x-1]=0;
				};
				value=(YSize*GlobalMeter->next_value())/MAX;
				value2=(YSize*LocalMeter->next_value())/MAX;
			};
		} else {
			for (int x=1;x<XSize;x+=2) {
				int Y1=YSize-value;
				int Y2=YSize-value2;
				if (Y1<Y2) {
					for (int y=YSize-1;y>=Y2;y--) {
						graph_rgb_data[y*XSize+x]=graph_rgb_data[y*XSize+x-1]=2;
					};
					for (int y=Y2-1;y>Y1;y--) {
						graph_rgb_data[y*XSize+x]=graph_rgb_data[y*XSize+x-1]=1;
					};
					graph_rgb_data[Y1*XSize+x]=graph_rgb_data[Y1*XSize+x-1]=0;
				} else {
					for (int y=YSize-1;y>Y2;y--) {
						graph_rgb_data[y*XSize+x]=graph_rgb_data[y*XSize+x-1]=2;
					};
					graph_rgb_data[Y2*XSize+x]=graph_rgb_data[Y2*XSize+x-1]=0;
				};
				value=(YSize*GlobalMeter->next_value())/MAX;
				value2=(YSize*LocalMeter->next_value())/MAX;
			};
		};
	};
};


void init_graph() {
	guint32 colors[4];
	colors[0]=CFG.GRAPH_PICK;
	colors[1]=CFG.GRAPH_FORE1;
	colors[2]=CFG.GRAPH_FORE2;
	colors[3]=CFG.GRAPH_BACK;
	
	int XSize=2*(METER_LENGTH);
	int YSize=16;
	int WX,WY;
	gdk_window_get_size(MainWindow->window,&WX,&WY);

	cmap=gdk_rgb_cmap_new(colors,4);
	memset(graph_rgb_data,3,XSize*YSize);
	gdk_window_clear_area(MainWindow->window,WX-3-XSize,WY-3-YSize,XSize,YSize);
	gdk_draw_indexed_image(MainWindow->window,MainWindowGC,WX-3-XSize,WY-2-YSize,XSize,YSize,
	                       GDK_RGB_DITHER_NONE,(guchar *)graph_rgb_data,XSize,cmap);
	gdk_flush();
};
