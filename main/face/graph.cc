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
#include <string.h>
#include "list.h"
#include "../main.h"
#include "../var.h"
#include "../ntlocale.h"
#include "graph.h"
guchar graph_rgb_data[100*16*4];
static GdkRgbCmap *cmap;
#define GRAPH_HEIGHT 17

/*GtkWidget *graph_widget;*/

gint graph_expose_event_handler(GtkWidget *widget,GdkEventExpose *event) {
	graph_draw();
	return TRUE;
};

int graph_draw() {
	int XSize=2*(METER_LENGTH);
	int YSize=GRAPH_HEIGHT;
	int WX,WY;
	gdk_window_get_size(MainWindow->window,&WX,&WY);
	gdk_draw_indexed_image(MainWindow->window,MainWindowGC,WX-2-XSize,WY-2-YSize,XSize,YSize,
	                       GDK_RGB_DITHER_NONE,(guchar *)graph_rgb_data,XSize,cmap);
	gdk_flush();
	return FALSE;
};


void graph_recalc() {
	if (!GlobalMeter) return;
	int XSize=2*(METER_LENGTH);
	int YSize=GRAPH_HEIGHT;
	int WX,WY;
	gdk_window_get_size(MainWindow->window,&WX,&WY);
	memset(graph_rgb_data,3,XSize*YSize);

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
							graph_rgb_data[y*XSize+x]=graph_rgb_data[y*XSize+x-1]=2;
						};
						if (value2>0)
							graph_rgb_data[y*XSize+x]=graph_rgb_data[y*XSize+x-1]=2;
						else
							graph_rgb_data[y*XSize+x]=graph_rgb_data[y*XSize+x-1]=1;
						for (y=Y2-1;y>Y1;y--) {
							graph_rgb_data[y*XSize+x]=graph_rgb_data[y*XSize+x-1]=1;
						};
						graph_rgb_data[Y1*XSize+x]=graph_rgb_data[Y1*XSize+x-1]=0;
					} else {
						for (int y=YSize;y>Y2;y--) {
							graph_rgb_data[y*XSize+x]=graph_rgb_data[y*XSize+x-1]=2;
						};
						graph_rgb_data[Y2*XSize+x]=graph_rgb_data[Y2*XSize+x-1]=0;
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
						for (y=YSize;y>=Y2;y--) {
							graph_rgb_data[y*XSize+x]=graph_rgb_data[y*XSize+x-1]=2;
						};
						if (value2>0)
							graph_rgb_data[y*XSize+x]=graph_rgb_data[y*XSize+x-1]=2;
						else
							graph_rgb_data[y*XSize+x]=graph_rgb_data[y*XSize+x-1]=1;
						for (y=Y2-1;y>Y1;y--) {
							graph_rgb_data[y*XSize+x]=graph_rgb_data[y*XSize+x-1]=1;
						};
						graph_rgb_data[Y1*XSize+x]=graph_rgb_data[Y1*XSize+x-1]=0;
					} else {
						for (int y=YSize;y>Y2;y--) {
							graph_rgb_data[y*XSize+x]=graph_rgb_data[y*XSize+x-1]=2;
						};
						graph_rgb_data[Y2*XSize+x]=graph_rgb_data[Y2*XSize+x-1]=0;
					};
				};
				value=float((YSize*GlobalMeter->next_value())/float(MAX));
				value2=float((YSize*LocalMeter->next_value())/float(MAX));
			};
		};
	};
};

/*
void graph_init(){
	graph_widget=gtk_preview_new(GTK_PREVIEW_COLOR);
	gtk_preview_size(GTK_PREVIEW(graph_widget),100,16);
	guchar buf[100*3];
	for (int i=0;i<16;i++){
		for (int j=0;j<100;j++){
			buf[j]=CFG.COLOR_BACK & 0xff;
			buf[j+1]=(CFG.COLOR_BACK>>8) & 0xff;
			buf[j+2]=(CFG.COLOR_BACK>>16) & 0xff;
		};
		gtk_preview_draw_row(GTK_PREVIEW(graph_widget),0,i,100);
	};
};
*/

void graph_init() {
	guint32 colors[4];
	colors[0]=CFG.GRAPH_PICK;
	colors[1]=CFG.GRAPH_FORE1;
	colors[2]=CFG.GRAPH_FORE2;
	colors[3]=CFG.GRAPH_BACK;
	
	int XSize=2*(METER_LENGTH);
	int YSize=GRAPH_HEIGHT;
	int WX,WY;
	gdk_window_get_size(MainWindow->window,&WX,&WY);

	cmap=gdk_rgb_cmap_new(colors,4);
	memset(graph_rgb_data,3,XSize*YSize);
	gdk_window_clear_area(MainWindow->window,WX-3-XSize,WY-2-YSize,XSize,YSize);
	gdk_draw_indexed_image(MainWindow->window,MainWindowGC,WX-3-XSize,WY-2-YSize,XSize,YSize,
	                       GDK_RGB_DITHER_NONE,(guchar *)graph_rgb_data,XSize,cmap);
	gdk_flush();
};

void graph_reinit(){
	gdk_rgb_cmap_free(cmap);
	graph_init();
	graph_recalc();
	graph_draw();
};
