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
#include "mywidget.h"
#include "edit.h"
#include "misc.h"
#include "../ntlocale.h"

static GtkWidgetClass *parent_class = NULL;
static GtkWidgetClass *color_parent_class = NULL;

static void my_gtk_filesel_destroy_browser(MyGtkFilesel *filesel){
	if (filesel->browser){
		gtk_widget_destroy(filesel->browser);
		filesel->browser=(GtkWidget *)NULL;
	};
};

static gint my_gtk_filesel_ok(GtkButton *button,MyGtkFilesel *filesel){
	g_return_val_if_fail(filesel!=NULL,FALSE);

	if (filesel->browser){
		text_to_combo(filesel->combo,
			      gtk_file_selection_get_filename(GTK_FILE_SELECTION(filesel->browser)));
		my_gtk_filesel_destroy_browser(filesel);
	};

	return TRUE;
};

static gint my_gtk_filesel_cancel(GtkButton *button,MyGtkFilesel *filesel){
	g_return_val_if_fail(filesel!=NULL,FALSE);
	my_gtk_filesel_destroy_browser(filesel);
	return TRUE;
};

static gint my_gtk_filesel_delete(GtkWidget *window,GdkEvent *event, MyGtkFilesel *filesel) {
	my_gtk_filesel_cancel(NULL,filesel);
	return TRUE;
};

static gint my_gtk_filesel_init_browser(GtkButton *button, MyGtkFilesel *filesel){
	g_return_val_if_fail(filesel!=NULL,FALSE);
	if (filesel->browser){
		gdk_window_show(filesel->browser->window);
	}else{
		if (filesel->only_dirs){
			filesel->browser=gtk_file_selection_new(_("Select directory"));
			gtk_widget_set_sensitive(GTK_FILE_SELECTION(filesel->browser)->file_list,FALSE);
		}else
			filesel->browser=gtk_file_selection_new(_("Select file"));
		char *tmp=text_from_combo(filesel->combo);
		if (tmp && *tmp)
			gtk_file_selection_set_filename(GTK_FILE_SELECTION(filesel->browser),tmp);
		gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(filesel->browser)->ok_button),
				   "clicked",GTK_SIGNAL_FUNC(my_gtk_filesel_ok),filesel);
		gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(filesel->browser)->cancel_button),
				   "clicked",GTK_SIGNAL_FUNC(my_gtk_filesel_cancel),filesel);
		gtk_signal_connect(GTK_OBJECT(&(GTK_FILE_SELECTION(filesel->browser)->window)),
				   "delete_event",GTK_SIGNAL_FUNC(my_gtk_filesel_delete),filesel);
		gtk_widget_show(filesel->browser);
		if (filesel->modal){
			gtk_window_set_modal (GTK_WINDOW(filesel->browser),TRUE);
			gtk_window_set_transient_for (GTK_WINDOW (filesel->browser), filesel->modal);
		};
		
	};
	return TRUE;
};

static void my_gtk_filsel_destroy(GtkObject *widget){
	g_return_if_fail(widget!=NULL);
	MyGtkFilesel *filesel=(MyGtkFilesel *)widget;
	my_gtk_filesel_destroy_browser(filesel);
	
	if (GTK_OBJECT_CLASS (parent_class)->destroy)
		(* GTK_OBJECT_CLASS (parent_class)->destroy) (widget);
};

static void my_gtk_filesel_class_init(MyGtkFileselClass *klass){
	GtkObjectClass *object_class=(GtkObjectClass *)klass;
	
	object_class->destroy=my_gtk_filsel_destroy;
	parent_class=(GtkWidgetClass *)gtk_type_class(gtk_box_get_type());
};

static void my_gtk_filesel_init(MyGtkFilesel *filesel){
	filesel->browser=(GtkWidget *)NULL;
	filesel->combo=gtk_combo_new();
	filesel->only_dirs=0;
	filesel->modal=NULL;
	GtkWidget *button=gtk_button_new_with_label(_("Browse"));
	gtk_signal_connect(GTK_OBJECT(button),"clicked",GTK_SIGNAL_FUNC(my_gtk_filesel_init_browser),filesel);
	gtk_combo_set_case_sensitive(GTK_COMBO(filesel->combo),TRUE);
	gtk_box_set_spacing(GTK_BOX(filesel),5);
	gtk_box_pack_start(GTK_BOX(filesel),filesel->combo,TRUE,TRUE,0);
	gtk_box_pack_start(GTK_BOX(filesel),button,FALSE,FALSE,0);
};

guint my_gtk_filesel_get_type(){
	static guint my_filesel_type=0;
	if (!my_filesel_type){
		GtkTypeInfo my_filesel_info={
			"MyGtkFilesel",
			sizeof(MyGtkFilesel),
			sizeof(MyGtkFileselClass),
			(GtkClassInitFunc) my_gtk_filesel_class_init,
			(GtkObjectInitFunc) my_gtk_filesel_init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL
		};
		my_filesel_type = gtk_type_unique (gtk_hbox_get_type (), &my_filesel_info);
	};
	return my_filesel_type;
};

GtkWidget *my_gtk_filesel_new(tHistory *history){
	MyGtkFilesel *filesel=(MyGtkFilesel *)gtk_type_new(my_gtk_filesel_get_type());
	GList *list=make_glist_from_mylist(history);
	if (list)
		gtk_combo_set_popdown_strings (GTK_COMBO (filesel->combo), list);
	return GTK_WIDGET(filesel);
};

/* ------------------ Color selection widget --------------------*/

static void my_gtk_colorsel_destroy_browser(MyGtkColorsel *colsel){
	if (colsel->browser){
		gtk_widget_destroy(colsel->browser);
		colsel->browser=(GtkWidget *)NULL;
	};
};

static void my_gtk_colorsel_update(MyGtkColorsel *colsel){
	guchar buf[50*3];
	
	for (int i=0;i<20;i++){
		for (int j=0,k=0;j<50;j++){
			buf[k]=guchar(colsel->color[0] * 0xff);
			buf[k+1]=guchar(colsel->color[1] * 0xff);
			buf[k+2]=guchar(colsel->color[2] * 0xff);
			k+=3;
		};
		gtk_preview_draw_row (GTK_PREVIEW (colsel->preview),
				      buf, 0, i, 50);
	};
};

static gint my_gtk_colorsel_ok(GtkButton *button,MyGtkColorsel *colsel){
	g_return_val_if_fail(colsel!=NULL,FALSE);

	if (colsel->browser){
		gtk_color_selection_get_color(GTK_COLOR_SELECTION(GTK_COLOR_SELECTION_DIALOG(colsel->browser)->colorsel),
					      colsel->color);
		my_gtk_colorsel_update(colsel);
		gtk_widget_draw (colsel->preview, NULL); 
		my_gtk_colorsel_destroy_browser(colsel);
	};

	return TRUE;
};

static gint my_gtk_colorsel_cancel(GtkButton *button,MyGtkColorsel *colsel){
	g_return_val_if_fail(colsel!=NULL,FALSE);
	my_gtk_colorsel_destroy_browser(colsel);
	return TRUE;
};

static gint my_gtk_colorsel_delete(GtkWidget *window,GdkEvent *event,  MyGtkColorsel *colsel) {
	my_gtk_colorsel_cancel(NULL,colsel);
	return TRUE;
};

static gint my_gtk_colorsel_init_browser(GtkButton *button, MyGtkColorsel *colsel){
	g_return_val_if_fail(colsel!=NULL,FALSE);
	if (colsel->browser){
		gdk_window_show(colsel->browser->window);
	}else{
		colsel->browser=gtk_color_selection_dialog_new(_("Select color"));
		gtk_color_selection_set_color(GTK_COLOR_SELECTION(GTK_COLOR_SELECTION_DIALOG(colsel->browser)->colorsel),
					      colsel->color);
		gtk_signal_connect(GTK_OBJECT(GTK_COLOR_SELECTION_DIALOG(colsel->browser)->ok_button),
				   "clicked",GTK_SIGNAL_FUNC(my_gtk_colorsel_ok),colsel);
		gtk_signal_connect(GTK_OBJECT(GTK_COLOR_SELECTION_DIALOG(colsel->browser)->cancel_button),
				   "clicked",GTK_SIGNAL_FUNC(my_gtk_colorsel_cancel),colsel);
		gtk_signal_connect(GTK_OBJECT(&(GTK_COLOR_SELECTION_DIALOG(colsel->browser)->window)),
				   "delete_event",GTK_SIGNAL_FUNC(my_gtk_colorsel_delete),colsel);
		gtk_widget_show(colsel->browser);
		if (colsel->modal){
			gtk_window_set_modal (GTK_WINDOW(colsel->browser),TRUE);
			gtk_window_set_transient_for (GTK_WINDOW (colsel->browser), colsel->modal);
		};
		
	};
	return TRUE;
};

static void my_gtk_colorsel_destroy(GtkObject *widget){
	g_return_if_fail(widget!=NULL);
	MyGtkColorsel *colsel=(MyGtkColorsel *)widget;
	my_gtk_colorsel_destroy_browser(colsel);
	
	if (GTK_OBJECT_CLASS (color_parent_class)->destroy)
		(* GTK_OBJECT_CLASS (color_parent_class)->destroy) (widget);
};

static void my_gtk_colorsel_class_init(MyGtkColorselClass *klass){
	GtkObjectClass *object_class=(GtkObjectClass *)klass;
	
	object_class->destroy=my_gtk_colorsel_destroy;
	color_parent_class=(GtkWidgetClass *)gtk_type_class(gtk_box_get_type());
};

static void my_gtk_colorsel_init(MyGtkColorsel *colsel){
	colsel->browser=(GtkWidget *)NULL;
	colsel->modal=(GtkWindow *)NULL;
	GtkWidget *button=gtk_button_new_with_label(_("Select"));
	gtk_signal_connect(GTK_OBJECT(button),"clicked",GTK_SIGNAL_FUNC(my_gtk_colorsel_init_browser),colsel);
	gtk_box_set_spacing(GTK_BOX(colsel),5);
	colsel->preview=gtk_preview_new(GTK_PREVIEW_COLOR);
	gtk_preview_size (GTK_PREVIEW (colsel->preview), 50, 20);
	gtk_box_pack_start(GTK_BOX(colsel),colsel->preview,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(colsel),button,FALSE,FALSE,0);
};

guint my_gtk_colorsel_get_type(){
	static guint my_filesel_type=0;
	if (!my_filesel_type){
		GtkTypeInfo my_filesel_info={
			"MyGtkColorsel",
			sizeof(MyGtkColorsel),
			sizeof(MyGtkColorselClass),
			(GtkClassInitFunc) my_gtk_colorsel_class_init,
			(GtkObjectInitFunc) my_gtk_colorsel_init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL
		};
		my_filesel_type = gtk_type_unique (gtk_hbox_get_type (), &my_filesel_info);
	};
	return my_filesel_type;
};

gint my_gtk_colorsel_get_color(MyGtkColorsel *colsel){
	gint color=0;
	guchar *a=(guchar *)&color;
	g_return_val_if_fail(colsel!=NULL,0);

	a[0]=guchar(colsel->color[2]*0xff);
	a[1]=guchar(colsel->color[1]*0xff);
	a[2]=guchar(colsel->color[0]*0xff);
	return color;
};

void my_gtk_colorsel_set_color(MyGtkColorsel *colsel, gint color){
	g_return_if_fail(colsel!=NULL);
	colsel->color[2]=double(color&0xff)/double(0xff);
	colsel->color[1]=double((color>>8)&0xff)/double(0xff);
	colsel->color[0]=double((color>>16)&0xff)/double(0xff);
	my_gtk_colorsel_update(colsel);
	if (GTK_WIDGET_VISIBLE(colsel))
		gtk_widget_draw (colsel->preview, NULL); 
};

GtkWidget *my_gtk_colorsel_new(gint color,gchar *title){
	MyGtkColorsel *colsel=(MyGtkColorsel *)gtk_type_new(my_gtk_colorsel_get_type());
	if (title){
		GtkWidget *label=gtk_label_new(title);
		gtk_box_pack_start(GTK_BOX(colsel),label,FALSE,FALSE,0);
	};
	my_gtk_colorsel_set_color(colsel,color);
	return GTK_WIDGET(colsel);
};
