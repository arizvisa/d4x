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
GtkTreeView *d4x_filters_view=(GtkTreeView *)NULL;
GtkListStore *d4x_filters_store=(GtkListStore *)NULL;
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
		gtk_list_store_set(d4x_filters_store,&(node->iter),
				   0,name,-1);
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
	GtkTreeSelection *sel=gtk_tree_view_get_selection(d4x_filters_view);
	GtkTreeModel *model;
	GtkTreeIter iter;
	if (gtk_tree_selection_get_selected(sel,&model,&iter)){
		GValue val={0,};
		gtk_tree_model_get_value(model,&iter,1,&val);
		d4xFNode *node=(d4xFNode*)g_value_get_pointer(&val);
		g_value_unset(&val);
		d4xFilterEdit *edit=(d4xFilterEdit *)d4x_filter_edit_new(node);
		g_signal_connect(G_OBJECT(edit->ok),"clicked",
				 G_CALLBACK(d4x_filters_window_edit_ok),edit);
		g_signal_connect(G_OBJECT(edit),"delete_event",
				 G_CALLBACK(d4x_filters_window_edit_delete), edit);
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
	g_signal_connect(G_OBJECT(edit->ok),"clicked",
			 G_CALLBACK(d4x_filters_window_add_ok),edit);
	g_signal_connect(G_OBJECT(edit),"delete_event",
			 G_CALLBACK(d4x_filters_window_add_delete), edit);
	d4x_eschandler_init(GTK_WIDGET(edit),edit);
	gtk_widget_show_all(GTK_WIDGET(edit));
	gtk_window_set_modal (GTK_WINDOW(edit),TRUE);
	gtk_window_set_transient_for (GTK_WINDOW (edit), GTK_WINDOW(d4x_filters_window));
};

static gboolean d4x_filters_view_event(GtkTreeView *view,GdkEventButton *event,gpointer data) {
	if (event && event->type==GDK_2BUTTON_PRESS && event->button==1){
		d4x_filters_window_edit();
		return TRUE;
	};
	return FALSE;
};

void d4x_filters_window_delete(){
	if (d4x_filters_changed) FILTERS_DB->save_to_ntrc();
	gtk_widget_destroy(d4x_filters_window);
	d4x_filters_window=(GtkWidget*)NULL;
};

void d4x_filters_window_del(){
	GtkTreeSelection *sel=gtk_tree_view_get_selection(d4x_filters_view);
	GtkTreeModel *model;
	GtkTreeIter iter;
	if (gtk_tree_selection_get_selected(sel,&model,&iter)){
		GValue val={0,};
		gtk_tree_model_get_value(model,&iter,1,&val);
		d4xFNode *node=(d4xFNode*)g_value_get_pointer(&val);
		g_value_unset(&val);
		gtk_list_store_remove(GTK_LIST_STORE(model),&(node->iter));
		FILTERS_DB->del(node);
		delete(node);
	};
};

void d4x_filters_window_add(d4xFNode *node){
	gtk_list_store_append(d4x_filters_store,&(node->iter));
	gtk_list_store_set(d4x_filters_store,&(node->iter),
			   0,node->filter->name.get(),
			   1,node,
			   -1);
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
	d4x_filters_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW (d4x_filters_window),_("Filters"));
	gtk_window_set_wmclass(GTK_WINDOW(d4x_filters_window),
			       "D4X_Filters","D4X");
	gtk_window_set_position(GTK_WINDOW(d4x_filters_window),GTK_WIN_POS_CENTER);
	gtk_widget_set_size_request(d4x_filters_window,-1,450);
	gtk_container_set_border_width(GTK_CONTAINER(d4x_filters_window),5);

	GtkListStore *list_store=gtk_list_store_new(2,
						    G_TYPE_STRING,
						    G_TYPE_POINTER);
	d4x_filters_store=list_store;
	d4x_filters_view = GTK_TREE_VIEW(gtk_tree_view_new_with_model(GTK_TREE_MODEL(list_store)));
	GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
	GtkTreeViewColumn *col=gtk_tree_view_column_new_with_attributes(_("filter name"),
									renderer,
									"text",0,
									NULL);
	gtk_tree_view_append_column(d4x_filters_view,col);
	gtk_tree_view_column_set_visible(col,TRUE);
	
	g_signal_connect(G_OBJECT(d4x_filters_view),
			 "event",
			 G_CALLBACK(d4x_filters_view_event),
			 NULL);
//	gtk_clist_set_selection_mode(GTK_CLIST(clist),GTK_SELECTION_EXTENDED);
	GtkWidget *scroll_window=gtk_scrolled_window_new((GtkAdjustment *)NULL,(GtkAdjustment *)NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll_window),
	                                GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(scroll_window),GTK_WIDGET(d4x_filters_view));
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
//	g_signal_connect(G_OBJECT(clist),"event",G_CALLBACK(face_pass_list_event_callback),this);
	g_signal_connect(G_OBJECT(button),"clicked",G_CALLBACK(d4x_filters_window_delete),NULL);
	g_signal_connect(G_OBJECT(del_button),"clicked",G_CALLBACK(d4x_filters_window_del),NULL);
	g_signal_connect(G_OBJECT(add_button),"clicked",G_CALLBACK(d4x_filters_window_add_new),NULL);
	g_signal_connect(G_OBJECT(edit_button),"clicked",G_CALLBACK(d4x_filters_window_edit),NULL);
	g_signal_connect(G_OBJECT(d4x_filters_window),"delete_event",G_CALLBACK(d4x_filters_window_delete),NULL);
	d4x_eschandler_init(d4x_filters_window,NULL);
};
