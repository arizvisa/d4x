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
#include "filtrgui.h"
#include "../ntlocale.h"
#include "../var.h"
#include "mywidget.h"
#include "misc.h"

GtkWidget *d4x_filters_window=(GtkWidget *)NULL;
GtkWidget *d4x_filters_clist=(GtkWidget *)NULL;
static int d4x_filters_changed;

/******************************************************/

static void d4x_filters_window_edit_ok(GtkWidget *widget,d4xFilterEdit *edit){
	char *name=text_from_combo(edit->name);
	if (name && *name){
		d4xFNode *node=edit->node;
		if (strcmp(name,node->filter->name.get())){
			FILTERS_DB->del(node);
			if (FILTERS_DB->find(name)){
				FILTERS_DB->add(node);
				return;
			};
			node->filter->name.set(name);
			FILTERS_DB->add(node);
		};
		node->filter->default_inc=GTK_TOGGLE_BUTTON(edit->include)->active;
		gint row=gtk_clist_find_row_from_data (GTK_CLIST(d4x_filters_clist),
						       node);
		gtk_clist_set_text (GTK_CLIST(d4x_filters_clist),row,0,name);
		gtk_widget_destroy(GTK_WIDGET(edit));
		d4x_filters_changed=1;
	};
};

static void d4x_filters_window_edit_delete(GtkWidget *widget,
					  GdkEvent *event,
					  d4xFilterEdit *edit){
	gtk_widget_destroy(GTK_WIDGET(edit));
};

void d4x_filters_window_edit(){
	GList *select=((GtkCList *)d4x_filters_clist)->selection_end;
	if (select) {
		gint row=GPOINTER_TO_INT(select->data);
		d4xFNode *node=(d4xFNode *)gtk_clist_get_row_data(
			GTK_CLIST(d4x_filters_clist),row);
		d4xFilterEdit *edit=(d4xFilterEdit *)d4x_filter_edit_new(node);
		gtk_signal_connect(GTK_OBJECT(edit->ok),"clicked",
				   GTK_SIGNAL_FUNC(d4x_filters_window_edit_ok),edit);
		gtk_signal_connect(GTK_OBJECT(edit),"delete_event",
				   GTK_SIGNAL_FUNC(d4x_filters_window_edit_delete), edit);
		d4x_eschandler_init(GTK_WIDGET(edit),edit);
		gtk_widget_show_all(GTK_WIDGET(edit));
		gtk_window_set_modal (GTK_WINDOW(edit),TRUE);
		gtk_window_set_transient_for (GTK_WINDOW (edit), GTK_WINDOW(d4x_filters_window));
	};
};

static void d4x_filters_window_add_delete(GtkWidget *widget,
					  GdkEvent *event,
					  d4xFilterEdit *edit){
	delete(edit->node);
	gtk_widget_destroy(GTK_WIDGET(edit));
};

static void d4x_filters_window_add_ok(GtkWidget *widget,d4xFilterEdit *edit){
	char *name=text_from_combo(edit->name);
	if (name && *name && FILTERS_DB->find(name)==NULL){
		d4xFNode *node=edit->node;
		node->filter->name.set(name);
		node->filter->default_inc=GTK_TOGGLE_BUTTON(edit->include)->active;
		FILTERS_DB->add(node);
		d4x_filters_window_add(node);
		gtk_widget_destroy(GTK_WIDGET(edit));
		d4x_filters_changed=1;
	};
};

void d4x_filters_window_add_new(){
	d4xFNode *node=new d4xFNode;
	node->filter=new d4xFilter;
	node->filter->ref();
	d4xFilterEdit *edit=(d4xFilterEdit *)d4x_filter_edit_new(node);
	gtk_signal_connect(GTK_OBJECT(edit->ok),"clicked",
			   GTK_SIGNAL_FUNC(d4x_filters_window_add_ok),edit);
	gtk_signal_connect(GTK_OBJECT(edit),"delete_event",
			   GTK_SIGNAL_FUNC(d4x_filters_window_add_delete), edit);
	d4x_eschandler_init(GTK_WIDGET(edit),edit);
	gtk_widget_show_all(GTK_WIDGET(edit));
	gtk_window_set_modal (GTK_WINDOW(edit),TRUE);
	gtk_window_set_transient_for (GTK_WINDOW (edit), GTK_WINDOW(d4x_filters_window));
};

void d4x_filters_clist_select_row(GtkWidget *clist, gint row, gint column,
				  GdkEventButton *event,gpointer data) {
	if (event && event->type==GDK_2BUTTON_PRESS && event->button==1)
		d4x_filters_window_edit();
};

void d4x_filters_window_delete(){
	if (d4x_filters_changed) FILTERS_DB->save_to_ntrc();
	gtk_widget_destroy(d4x_filters_window);
	d4x_filters_window=(GtkWidget*)NULL;
};

void d4x_filters_window_del(){
	GList *select=((GtkCList *)d4x_filters_clist)->selection_end;
	if (select) {
		gint row=GPOINTER_TO_INT(select->data);
		d4xFNode *node=(d4xFNode *)gtk_clist_get_row_data(
			GTK_CLIST(d4x_filters_clist),row);
		gtk_clist_remove(GTK_CLIST(d4x_filters_clist),row);
		FILTERS_DB->del(node);
		delete(node);
	};
};

void d4x_filters_window_add(d4xFNode *node){
	gchar *data[1];
	data[0]=node->filter->name.get();
	gint row=gtk_clist_append(GTK_CLIST(d4x_filters_clist),data);
	gtk_clist_set_row_data(GTK_CLIST(d4x_filters_clist),row,node);
};

void d4x_filters_window_destroy(){
	if (d4x_filters_window)
		gtk_widget_destroy(d4x_filters_window);
	d4x_filters_window=(GtkWidget*)NULL;
};

void d4x_filters_window_init(){
	if (d4x_filters_window) {
		gdk_window_show(d4x_filters_window->window);
		return;
	};
	d4x_filters_changed=0;
	d4x_filters_window = gtk_window_new(GTK_WINDOW_DIALOG);
	gtk_window_set_title(GTK_WINDOW (d4x_filters_window),_("Filters"));
	gtk_window_set_wmclass(GTK_WINDOW(d4x_filters_window),
			       "D4X_Filters","D4X");
	gtk_window_set_position(GTK_WINDOW(d4x_filters_window),GTK_WIN_POS_CENTER);
	gtk_widget_set_usize(d4x_filters_window,-1,450);
	gtk_container_border_width(GTK_CONTAINER(d4x_filters_window),5);
	gchar *titles[]={_("filter name")};
	d4x_filters_clist = gtk_clist_new_with_titles(1, titles);
	gtk_signal_connect(GTK_OBJECT(d4x_filters_clist),
			   "select_row",
			   GTK_SIGNAL_FUNC(d4x_filters_clist_select_row),
			   NULL);
	gtk_clist_set_shadow_type(GTK_CLIST(d4x_filters_clist), GTK_SHADOW_IN);
	gtk_clist_set_column_auto_resize(GTK_CLIST(d4x_filters_clist),0,TRUE);
//	gtk_clist_set_selection_mode(GTK_CLIST(clist),GTK_SELECTION_EXTENDED);
	GtkWidget *scroll_window=gtk_scrolled_window_new((GtkAdjustment *)NULL,(GtkAdjustment *)NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll_window),
	                                GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(scroll_window),d4x_filters_clist);
	GtkWidget *button=gtk_button_new_with_label(_("Ok"));
	GtkWidget *add_button=gtk_button_new_with_label(_("Add new"));
	GtkWidget *edit_button=gtk_button_new_with_label(_("Edit"));
	GtkWidget *del_button=gtk_button_new_with_label(_("Remove"));
	GTK_WIDGET_SET_FLAGS(button,GTK_CAN_DEFAULT);
	GTK_WIDGET_SET_FLAGS(edit_button,GTK_CAN_DEFAULT);
	GTK_WIDGET_SET_FLAGS(add_button,GTK_CAN_DEFAULT);
	GTK_WIDGET_SET_FLAGS(del_button,GTK_CAN_DEFAULT);
	GtkWidget *vbox=gtk_vbox_new(FALSE,0);
	GtkWidget *hbox=gtk_hbutton_box_new();
	gtk_box_set_spacing(GTK_BOX(vbox),5);
	gtk_box_set_spacing(GTK_BOX(hbox),5);
	gtk_box_pack_start(GTK_BOX(vbox),scroll_window,TRUE,TRUE,0);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);
	gtk_box_pack_end(GTK_BOX(hbox),add_button,FALSE,FALSE,0);
	gtk_box_pack_end(GTK_BOX(hbox),del_button,FALSE,FALSE,0);
	gtk_box_pack_end(GTK_BOX(hbox),edit_button,FALSE,FALSE,0);
	gtk_box_pack_end(GTK_BOX(hbox),button,FALSE,FALSE,0);
	gtk_container_add(GTK_CONTAINER(d4x_filters_window),vbox);
	gtk_window_set_default(GTK_WINDOW(d4x_filters_window),button);
	FILTERS_DB->print();
	gtk_widget_show_all(d4x_filters_window);
//	gtk_signal_connect(GTK_OBJECT(clist),"event",GTK_SIGNAL_FUNC(face_pass_list_event_callback),this);
	gtk_signal_connect(GTK_OBJECT(button),"clicked",GTK_SIGNAL_FUNC(d4x_filters_window_delete),NULL);
	gtk_signal_connect(GTK_OBJECT(del_button),"clicked",GTK_SIGNAL_FUNC(d4x_filters_window_del),NULL);
	gtk_signal_connect(GTK_OBJECT(add_button),"clicked",GTK_SIGNAL_FUNC(d4x_filters_window_add_new),NULL);
	gtk_signal_connect(GTK_OBJECT(edit_button),"clicked",GTK_SIGNAL_FUNC(d4x_filters_window_edit),NULL);
	gtk_signal_connect(GTK_OBJECT(d4x_filters_window),"delete_event",GTK_SIGNAL_FUNC(d4x_filters_window_delete),NULL);
	d4x_eschandler_init(d4x_filters_window,NULL);
};
