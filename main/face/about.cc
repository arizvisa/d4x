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
#include <gdk/gdkkeysyms.h>
#include <stdio.h>
#include "list.h"
#include "about.h"
#include "misc.h"
#include "../var.h"
#include "../ntlocale.h"

GtkWidget *AboutWindow=(GtkWidget *)NULL;
GtkWidget *AboutTLabel,*AboutSLabel;

char *TRANSLATORS[]={
	"Emil (emil5@go2.pl) [pl.po]",
	"Sa'ndor Pinte'r [hu.po]",
	"Brane Jovanovic [sr.po]",
	"Liu Songhe [zh_CN.po]",
	"Utumi Hirosi [ja.po]",
	"Jerome Couderc [fr.po]",
	"Zdenko Podobny [sk.po]",
	"Brane Jovanovic [sr.po]",
	"Thomas R. Koll [de.po]",
	"Gustavo D. Vranjes [es.po]",
	"Olexander Kunytsa [uk.po]",
	"Marco Martin [es.po]",
	"Vittorio Rebecchi [es.po]",
	"Iordan Pavlov [bg.po]",
	"Legnar WinShadow [pt_BR.po]",
	"Eduardo Jorge [pt_BR.po]",
	"Felix Knecht",
	"Oren Held [he.po,iw.po]",
	"Cawko Xakep",
	"Petteri Wirkkala [fi.po]",
	"Pavel Janousek [cs.po]",
	"Gorkem Cetin [tr.po]",
	"Kei Kodera [ja.po]",
	"Priyadi Iman Nurcahyo [id.po]",
	"Mario Sergio Fujikawa Ferreira",
	"Seung-young Oh [ko.po]",
	"Grzegorz Kowal [pl.po]",
	"DJ Art",
	"Enrico Manfredini",
	"Lubosh Holichka",
	"Kyritsis Athanasios",
	"Vicente Aguilar",
	"Robin Verduijn",
	"Dirk Moebius",
	"Paulo Henrique",
	"A.J.",
	"Josef Jahn",
	"Marlin [TLC-ML]",
	"Philippe Rigaux",
	"Eric Seigne",
	"Guiliano Rangel Alves"
};

char *SPECIAL_THANKS[]={
	"Gene Schiavone",
	"Brian Trapp"
};
int ABOUT_CURRENT_NAME;

static gint about_window_change_names(void *a){
	if (AboutWindow){
		GtkScrolledWindow *w=(GtkScrolledWindow *)a;
		GtkAdjustment *adj=gtk_scrolled_window_get_vadjustment(w);
		gdouble val=adj->value;
		switch (ABOUT_CURRENT_NAME){
		case 1:{
			val=adj->value+1;
			if (val>adj->upper-adj->page_size){
				val=adj->upper-adj->page_size;
				ABOUT_CURRENT_NAME=2;
			};
			break;
		};
		case 0:{
			val=adj->value-1;
			if (val<adj->lower){
				val=adj->lower;
				ABOUT_CURRENT_NAME=15;
			};
			break;
		};
		default:{
			if (ABOUT_CURRENT_NAME++==14) ABOUT_CURRENT_NAME=0;
			if (ABOUT_CURRENT_NAME>28) ABOUT_CURRENT_NAME=1;
		};
		};
		gtk_adjustment_set_value (adj,val);
		return 1;
	};
	return 0;
};

static gint about_window_esc_handler(GtkWidget *window,GdkEvent *event){
	if (event && event->type == GDK_KEY_PRESS) {
		GdkEventKey *kevent=(GdkEventKey *)event;
		switch(kevent->keyval) {
		case GDK_Escape:{
			destroy_about_window();
			return TRUE;
			break;
		};
		};
	};
	return FALSE;
};

void destroy_about_window() {
	if (AboutWindow){
		gtk_widget_destroy(AboutWindow);
		AboutWindow=(GtkWidget *)NULL;
		gtk_widget_set_sensitive(MainWindow,TRUE);
	};
};

void init_about_window(...) {
#include "pixmaps/dndtrash.xpm"
	if (AboutWindow) {
		gdk_window_show(AboutWindow->window);
		return;
	};
	ABOUT_CURRENT_NAME=0;
	AboutWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_wmclass(GTK_WINDOW(AboutWindow),
			       "D4X_About","D4X");
	//    gtk_widget_set_usize( GTK_WIDGET (AboutWindow), 400, 105);
	gtk_window_set_resizable(GTK_WINDOW(AboutWindow), FALSE);
	gtk_window_set_position(GTK_WINDOW(AboutWindow),GTK_WIN_POS_CENTER);
	gtk_window_set_title(GTK_WINDOW (AboutWindow), _("About"));
	gtk_container_set_border_width(GTK_CONTAINER(AboutWindow),5);
	GtkWidget *box=gtk_vbox_new(FALSE,0);
	GtkWidget *label1=gtk_label_new(VERSION_NAME);
	GtkWidget *label2=gtk_entry_new();
/*
	GtkStyle *style = gtk_widget_get_style(label2);
	gint real_size=gdk_string_width(gtk_style_get_font(style),HOME_PAGE);
	gtk_widget_set_size_request(label2,real_size+10,-1);
*/
	gtk_editable_set_editable(GTK_EDITABLE(label2),FALSE);
	gtk_entry_set_text(GTK_ENTRY(label2),HOME_PAGE);
	GtkWidget *label3=gtk_label_new(_("Author: Maxim Koshelev"));
	GtkWidget *label4=gtk_label_new("e-mail: chuchelo@krasu.ru");
	GtkWidget *label5=gtk_label_new(_("Autoconf: Zaufi"));
	GtkWidget *frame=gtk_frame_new(_("Translators team"));
	gtk_frame_set_shadow_type(GTK_FRAME(frame),GTK_SHADOW_IN);
	GtkWidget *scroll_window=gtk_scrolled_window_new((GtkAdjustment *)NULL,
							 (GtkAdjustment *)NULL);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW (scroll_window),GTK_SHADOW_NONE);
	gtk_widget_set_size_request(scroll_window,-1,100);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll_window),
	                                GTK_POLICY_NEVER,
					GTK_POLICY_NEVER);
	GtkWidget *box1=gtk_vbox_new(FALSE,0);

	GtkWidget *viewport=gtk_viewport_new((GtkAdjustment *)NULL,
					     (GtkAdjustment *)NULL);
	gtk_viewport_set_shadow_type(GTK_VIEWPORT(viewport),GTK_SHADOW_NONE);
	gtk_container_add(GTK_CONTAINER(viewport),box1);
	gtk_container_add(GTK_CONTAINER(scroll_window),viewport);

//	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW (scroll_window),box1);
	gtk_container_add(GTK_CONTAINER(frame),scroll_window);
	for (int i=0;i<sizeof(TRANSLATORS)/sizeof(char*);i++){
		AboutTLabel=gtk_label_new(TRANSLATORS[i]);
		gtk_box_pack_start(GTK_BOX(box1),AboutTLabel,FALSE,FALSE,0);
	}

	GtkWidget *frame1=gtk_frame_new(_("Special thanks to"));
	gtk_frame_set_shadow_type(GTK_FRAME(frame1),GTK_SHADOW_IN);
	box1=gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(frame1),box1);
	for (int j=0;j<sizeof(SPECIAL_THANKS)/sizeof(char*);j++){
		AboutSLabel=gtk_label_new(SPECIAL_THANKS[j]);
		gtk_box_pack_start(GTK_BOX(box1),AboutSLabel,FALSE,FALSE,0);
	};

	GtkWidget *Button=gtk_button_new_from_stock(GTK_STOCK_OK);
	GtkWidget *box2=gtk_hbox_new(FALSE,0);
	GtkWidget *box3=gtk_vbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(box),label1,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(box3),label2,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(box3),label3,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(box3),label4,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(box3),label5,FALSE,FALSE,0);

	GdkBitmap *pixmask=NULL;
	gtk_widget_realize(AboutWindow);
	GdkPixmap *pixmap=make_pixmap_from_xpm(&pixmask,dndtrash_xpm,AboutWindow);
	GtkWidget *image = gtk_image_new_from_pixmap(pixmap,pixmask);
	gtk_box_pack_start(GTK_BOX(box2),image,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(box2),box3,FALSE,FALSE,0);

	gtk_box_pack_start(GTK_BOX(box),box2,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(box),frame,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(box),frame1,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(box),Button,FALSE,FALSE,0);
	gtk_container_add(GTK_CONTAINER(AboutWindow),box);
	g_signal_connect(G_OBJECT(Button),"clicked",
	                   (GtkSignalFunc)destroy_about_window,NULL);
	g_signal_connect(G_OBJECT(AboutWindow), "key_press_event",
			   (GtkSignalFunc)about_window_esc_handler, NULL);
	g_signal_connect(G_OBJECT(AboutWindow), "delete_event",
	                   (GtkSignalFunc)destroy_about_window,NULL);
	d4x_eschandler_init(AboutWindow,NULL);
	GTK_WIDGET_SET_FLAGS(Button,GTK_CAN_DEFAULT);
	gtk_window_set_default(GTK_WINDOW(AboutWindow),Button);
	gtk_widget_show_all(AboutWindow);
	gtk_window_set_modal (GTK_WINDOW(AboutWindow),TRUE);
	gtk_window_set_transient_for (GTK_WINDOW (AboutWindow), GTK_WINDOW (MainWindow));
//	gtk_widget_show(AboutWindow);
	gtk_widget_set_sensitive(MainWindow,FALSE);
	g_timeout_add (80, about_window_change_names , scroll_window);
};

/* ------------------------------------------------------------
 * Dialogs...
 * ------------------------------------------------------------
 */
tDialog::tDialog() {
	window=(GtkWidget *)NULL;
};

void tDialog::done() {
	if (!window) return;
	gtk_widget_destroy(window);
	window=(GtkWidget *)NULL;
};

void tDialog::set_modal(GtkWidget *widget){
	if (window && widget){
		gtk_window_set_modal (GTK_WINDOW(window),TRUE);
		gtk_window_set_transient_for (GTK_WINDOW (window), GTK_WINDOW (widget));
	};
};

tDialog::~tDialog() {
	done();
};


static void dialog_delete(GtkWidget *widget, GdkEvent *event,tDialogWidget *parent) {
	parent->done();
};

static void dialog_delete2(GtkWidget *widget,tDialogWidget *parent) {
	parent->done();
};

tDialogWidget::tDialogWidget():tDialog(){
};

void tDialogWidget::create(char *ask,char *title){
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW (window),title);
	gtk_window_set_position(GTK_WINDOW(window),GTK_WIN_POS_CENTER);
	gtk_container_set_border_width(GTK_CONTAINER(window),5);
	label=gtk_label_new(ask);
	ok_button=gtk_button_new_from_stock(GTK_STOCK_OK);
	cancel_button=gtk_button_new_from_stock(GTK_STOCK_CANCEL);
	GTK_WIDGET_SET_FLAGS(ok_button,GTK_CAN_DEFAULT);
	GTK_WIDGET_SET_FLAGS(cancel_button,GTK_CAN_DEFAULT);
	mainvbox=gtk_vbox_new(FALSE,5);
	GtkWidget *hbox=gtk_hbutton_box_new();
	gtk_box_set_spacing(GTK_BOX(hbox),5);
	gtk_box_pack_start(GTK_BOX(mainvbox),label,FALSE,FALSE,0);
	gtk_box_pack_end(GTK_BOX(mainvbox),hbox,FALSE,FALSE,0);
	gtk_box_pack_end(GTK_BOX(hbox),ok_button,FALSE,FALSE,0);
	gtk_box_pack_end(GTK_BOX(hbox),cancel_button,FALSE,FALSE,0);
	gtk_container_add(GTK_CONTAINER(window),mainvbox);
	gtk_window_set_default(GTK_WINDOW(window),cancel_button);
	g_signal_connect(G_OBJECT(cancel_button),"clicked",G_CALLBACK(dialog_delete2),this);
	g_signal_connect(G_OBJECT(window),"delete_event",G_CALLBACK(dialog_delete), this);
	d4x_eschandler_init(window,this);
};

int tDialogWidget::init(char *ask,char *title) {
	if (window) {
		gdk_window_show(window->window);
		return 0;
	};
	create(ask,title);
	gtk_widget_show_all(window);
	return 1;
};


tDialogWidget::~tDialogWidget() {
};
/* -----------------------------------------------*/
tConfirmedDialog::tConfirmedDialog():tDialogWidget(){
};

int tConfirmedDialog::init(char *ask,char *title){
	if (window) {
		gdk_window_show(window->window);
		return 0;
	};
	create(ask,title);
	check=gtk_check_button_new_with_label(_("Don't ask next time"));
	gtk_box_pack_start(GTK_BOX(mainvbox),check,FALSE,FALSE,0);
	gtk_widget_show_all(window);
	return 1;
};

tConfirmedDialog::~tConfirmedDialog(){
};

/* -----------------------------------------------*/

static gint string_dialog_delete_event(GtkWidget *widget, GdkEvent *event,tStringDialog *parent) {
	parent->done();
	return TRUE;
};

static void string_dialog_ok_clicked(GtkWidget *widget,tStringDialog *parent) {
	parent->done();
};

tStringDialog::tStringDialog():tDialog(){
};

int tStringDialog::init(char *str,char *title,char *frame_title) {
	if (window) {
		gtk_entry_set_text(GTK_ENTRY(entry),str);
		gtk_window_set_title(GTK_WINDOW (window), title);
		gtk_window_set_default(GTK_WINDOW(window),ok_button);
		gtk_frame_set_label(GTK_FRAME(frame),frame_title?frame_title:"");
		gdk_window_show(window->window);
		return 0;
	};
	window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(G_OBJECT(window),"delete_event",G_CALLBACK(string_dialog_delete_event), this);
	d4x_eschandler_init(window,this);
	gtk_window_set_title(GTK_WINDOW (window), title);
	gtk_container_set_border_width(GTK_CONTAINER(window),1);
	gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
	GtkWidget *hbox=gtk_hbutton_box_new();
	GtkWidget *vbox=gtk_vbox_new(FALSE,5);
	gtk_button_box_set_layout(GTK_BUTTON_BOX(hbox),GTK_BUTTONBOX_END);
	ok_button=gtk_button_new_from_stock(GTK_STOCK_OK);
	g_signal_connect(G_OBJECT(ok_button),"clicked",G_CALLBACK(string_dialog_ok_clicked),this);
	GTK_WIDGET_SET_FLAGS(ok_button,GTK_CAN_DEFAULT);
	entry=gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entry),str);
	gtk_editable_set_editable(GTK_EDITABLE(entry),FALSE);
	gtk_widget_set_size_request(GTK_WIDGET (window), 500,-1);
	gtk_box_pack_start(GTK_BOX(vbox),entry,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);
	gtk_box_pack_end(GTK_BOX(hbox),ok_button,FALSE,FALSE,0);
	frame=gtk_frame_new(frame_title?frame_title:"");
	gtk_frame_set_shadow_type(GTK_FRAME(frame),GTK_SHADOW_OUT);
	//	    gtk_container_border_width(GTK_CONTAINER(frame),5);
	gtk_container_add(GTK_CONTAINER(frame),vbox);
	gtk_container_add(GTK_CONTAINER(window),frame);
	gtk_widget_show_all(window);
	gtk_window_set_default(GTK_WINDOW(window),ok_button);
	return 1;
};

tStringDialog::~tStringDialog() {
};
