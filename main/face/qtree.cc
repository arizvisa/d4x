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

#include "qtree.h"
#include "list.h"
#include "misc.h"
#include "colors.h"
#include "../ntlocale.h"
#include "../main.h"
#include "../var.h"
#include "mywidget.h"
#include "buttons.h"

enum {
	QROW_NAME,
	QROW_TOTAL,
	QROW_WAIT,
	QROW_RUN,
	QROW_QUEUE,
	QROW_LAST
};

extern tMain aa;

static gint _event_queue_(GtkWidget *widget,GdkEventButton *event,d4xQsTree *q){
	if (event->type==GDK_BUTTON_PRESS && event->button==3) {
		GtkTreeSelection *sel=gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
		GtkTreePath *path=NULL;
		if (gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(widget),
						  gint(event->x),
						  gint(event->y),
						  &path,
						  NULL,NULL,NULL)){
			gtk_tree_selection_select_path(sel,path);
			gtk_tree_path_free(path);
			q->popup_menu((GdkEvent*)event,1);
		}else{
			q->popup_menu((GdkEvent*)event,0);
			gtk_tree_selection_unselect_all(sel);
		};
		return TRUE;
	};
	return FALSE;
};

static void _menu_event_(d4xQsTree *qt,guint action,GtkWidget *widget){
	switch(action){
	case 0:
		qt->create_init();
		break;
	case 1:
		qt->create_init(1);
		break;
	case 2:
		qt->delete_queue();
		break;
	case 3:
		qt->prefs_init();
		break;
	};
};

static void _create_cancel_(GtkButton *button,d4xQsTree *qt){
	qt->create_cancel();
};

static void _create_ok_(GtkWidget *widget,d4xQsTree *qt){
	qt->create_ok();
};

static void _create_delete_(GtkWidget *window,GdkEvent *event,d4xQsTree *qt){
	qt->create_cancel();
};

static void _create_changed_(GtkWidget *entry,GtkWidget *ok_button){
	if (strlen(text_from_combo(entry)))
		gtk_widget_set_sensitive(ok_button,TRUE);
	else
		gtk_widget_set_sensitive(ok_button,FALSE);
};

void d4xQsTree::create_ok(){
	d4xDownloadQueue *papa=NULL;
	if (create_mode)
		papa=selected();
	char *name=text_from_combo(dialog_entry);
	create_new_queue(name,papa);
	create_cancel();
};

void d4xQsTree::create_cancel(){
	gtk_widget_destroy(dialog);
	dialog=NULL;
};

void d4xQsTree::create_init(int mode){
	if (dialog){
		gdk_window_show(dialog->window);
		return;
	};
	create_mode=mode;
	dialog = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_wmclass(GTK_WINDOW(dialog),
			       "D4X_CreateQueueDialog","D4X");
	gtk_window_set_title(GTK_WINDOW (dialog),
			     _("Create new queue"));
	gtk_window_set_position(GTK_WINDOW(dialog),
				GTK_WIN_POS_CENTER);
	gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
	GtkWidget *vbox=gtk_vbox_new(FALSE,0);
	GtkWidget *hbox=gtk_hbutton_box_new();
	gtk_box_set_spacing(GTK_BOX(vbox),5);
	dialog_entry=gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(vbox),dialog_entry,FALSE,FALSE,0);
	gtk_box_set_spacing(GTK_BOX(hbox),5);
	GtkWidget *ok_button=gtk_button_new_with_label(_("Ok"));
	GtkWidget *cancel_button=gtk_button_new_with_label(_("Cancel"));
	GTK_WIDGET_SET_FLAGS(ok_button,GTK_CAN_DEFAULT);
	GTK_WIDGET_SET_FLAGS(cancel_button,GTK_CAN_DEFAULT);
	gtk_widget_set_sensitive(ok_button,FALSE);
	gtk_box_pack_start(GTK_BOX(hbox),ok_button,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(hbox),cancel_button,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);
	g_signal_connect(G_OBJECT (dialog_entry),"changed",
			   (GtkSignalFunc)_create_changed_, ok_button);
	g_signal_connect(G_OBJECT(ok_button),"clicked",
			   G_CALLBACK(_create_ok_),this);
	g_signal_connect(G_OBJECT(cancel_button),"clicked",
			   G_CALLBACK(_create_cancel_),this);
	g_signal_connect(G_OBJECT(dialog),"delete_event",
			   G_CALLBACK(_create_delete_),this);
	g_signal_connect(G_OBJECT(dialog_entry), "activate",
			   G_CALLBACK (_create_ok_), this);
	d4x_eschandler_init(dialog,this);
	gtk_container_add(GTK_CONTAINER(dialog),vbox);
	gtk_widget_show_all(dialog);
	gtk_window_set_default(GTK_WINDOW(dialog),ok_button);
	gtk_widget_grab_focus(dialog_entry);
	gtk_window_set_modal (GTK_WINDOW(dialog),TRUE);
	gtk_window_set_transient_for (GTK_WINDOW (dialog),
				      GTK_WINDOW (MainWindow));
};


static gboolean target_drag_drop (GtkTreeView        *view,
				  GdkDragContext     *context,
				  gint                x,
				  gint                y,
				  guint               time,
				  d4xQsTree *qt){
	GtkTreePath *path=NULL;
	if (context->targets &&
	    gtk_tree_view_get_path_at_pos(view,x,y,&path,NULL,NULL,NULL)){
		GtkTreeModel *model=gtk_tree_view_get_model(view);
		GtkTreeIter iter;
		if (gtk_tree_model_get_iter(model,&iter,path))
			if (qt->drop_to_row) gtk_tree_iter_free(qt->drop_to_row);
			qt->drop_to_row=gtk_tree_iter_copy(&iter);
			gtk_drag_get_data (GTK_WIDGET(view), context,
					   GDK_POINTER_TO_ATOM (context->targets->data),
					   time);
		gtk_tree_path_free(path);
		return TRUE;
	};
	if (qt->drop_to_row) gtk_tree_iter_free(qt->drop_to_row);
	qt->drop_to_row=NULL;
	return FALSE;
};

static void _foreach_move_prepare_(GtkTreeModel *model,GtkTreePath *path,
				   GtkTreeIter *iter,gpointer p){
	tQueue *q=(tQueue*)p;
	tmpIterNode *i=new tmpIterNode(iter);
	q->insert(i);
};

tDownload * d4xQsTree::get_download(GtkTreeView *view,GtkTreeIter *iter){
	GtkListStore *list_store=GTK_LIST_STORE(gtk_tree_view_get_model(view));
	GValue val={0,};
	gtk_tree_model_get_value(GTK_TREE_MODEL(list_store),iter,
				 NOTHING_COL,&val);
	tDownload *what=(tDownload *)g_value_get_pointer(&val);
	g_value_unset(&val);
	return (what);
};

void d4xQsTree::drop_from(GtkTreeView *src_view){
	drag_motion(NULL);
	if (drop_to_row==NULL) return;

	tQueue q;
	GtkTreeSelection *sel=gtk_tree_view_get_selection(src_view);
	gtk_tree_selection_selected_foreach(sel,
					    _foreach_move_prepare_,
					    &q);
	tNode *t=q.last();
	if (t==NULL) return;
	tDownload *temp=get_download(src_view,((tmpIterNode*)t)->iter);
	d4xDownloadQueue *src=temp->myowner->PAPA;
	if (src==NULL) return;
	GValue val={0,};
	gtk_tree_model_get_value(GTK_TREE_MODEL(store),drop_to_row,
				 QROW_QUEUE,&val);
	d4xDownloadQueue *dst=(d4xDownloadQueue *)g_value_get_pointer(&val);
	g_value_unset(&val);
	
	if (dst && dst!=src){
		while(t){
			tDownload *temp=get_download(src_view,((tmpIterNode*)t)->iter);
			int type=temp->owner();
			src->del(temp);
			src->qv.remove(temp);
			dst->add(temp,type);
			dst->qv.add(temp);
			t=t->next;
		};
		aa.try_to_run_wait(dst);
		aa.try_to_run_wait(src);
		prepare_buttons();
	};
};

void d4xQsTree::drag_motion(GtkTreeIter *iter){
	if (row_to_color){
		gtk_tree_store_set(store,row_to_color,
				   QROW_LAST,FALSE,
				   -1);
		gtk_tree_iter_free(row_to_color);
	};
	if (iter==NULL){
		row_to_color=NULL;
	}else{
		row_to_color=gtk_tree_iter_copy(iter);
		gtk_tree_store_set(store,row_to_color,
				   QROW_LAST,TRUE,
				   -1);
	};
};

static gboolean target_drag_motion (GtkTreeView *view,
				    GdkDragContext *context,
				    gint x,gint y,
				    guint time,
				    d4xQsTree *qt){

	GtkTreeIter iter;
	GtkTreePath *path=NULL;
	if (context->targets &&
	    gtk_tree_view_get_path_at_pos(view,x,y,&path,NULL,NULL,NULL)){
		GtkTreeModel *model=gtk_tree_view_get_model(view);
		if (gtk_tree_model_get_iter(model,&iter,path))
			qt->drag_motion(&iter);
		gtk_tree_path_free(path);
		return TRUE;
	};
	qt->drag_motion(NULL);
	return FALSE;
};

static void target_drag_data_received  (GtkWidget *widget,
					GdkDragContext     *context,
					gint                x,
					gint                y,
					GtkSelectionData   *data,
					guint               info,
					guint               time,
					d4xQsTree *qt){
	if ((data->length == sizeof(GtkWidget *)) && (data->format == 8)){
		GtkTreeView **src_view=NULL;
		src_view=(GtkTreeView **)(data->data);
		if (src_view && *src_view){
			qt->drop_from(*src_view);
		};
		gtk_drag_finish (context, TRUE, FALSE, time);
		return;
	}
	gtk_drag_finish (context, FALSE, FALSE, time);
};

static GtkTargetEntry ltarget_table[] = {
	{ "d4x/dpointer",     0, 0 }
};

static guint ln_targets = sizeof(ltarget_table) / sizeof(ltarget_table[0]);

static void target_drag_leave(GtkWidget *widget,GdkDragContext *context, guint time,d4xQsTree *qt){
	qt->drag_motion(NULL);
}

static gboolean d4x_qstree_select_func(GtkTreeSelection *sel, GtkTreeModel *model,GtkTreePath *path,
				       gboolean is_sel, gpointer data){
	if (!is_sel){
		d4xQsTree *qt=(d4xQsTree*)data;
		GtkTreeIter iter;
		gtk_tree_model_get_iter(model,&iter,path);
		qt->select_row(&iter);
	};
	return(TRUE);
};

void d4xQsTree::init(){
	store=gtk_tree_store_new(QROW_LAST+1,
				 G_TYPE_STRING,  //name
				 G_TYPE_INT,     //Total
				 G_TYPE_INT,     //Wait
				 G_TYPE_STRING,  //Run 
				 G_TYPE_POINTER,
				 G_TYPE_BOOLEAN);//queue
	view=GTK_TREE_VIEW(gtk_tree_view_new_with_model(GTK_TREE_MODEL(store)));
	GtkTreeSelection *sel=gtk_tree_view_get_selection(view);
	gtk_tree_selection_set_select_function(sel,d4x_qstree_select_func,this,NULL);
	gtk_tree_view_set_headers_visible(view,FALSE);
	GtkCellRenderer *renderer = gtk_cell_renderer_text_new ();
	g_object_set (G_OBJECT (renderer),"background-gdk", &BLUE,NULL);
	g_object_set (G_OBJECT (renderer),"foreground-gdk", &WHITE,NULL);
	GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes ("Name",
									      renderer,
									      "text",QROW_NAME,
									      "background_set", QROW_LAST,
									      "foreground_set", QROW_LAST,
									      NULL);
	gtk_tree_view_append_column(view, column);
	
	renderer = gtk_cell_renderer_text_new ();
	g_object_set (G_OBJECT (renderer),"background-gdk", &BLUE,NULL);
	g_object_set (G_OBJECT (renderer),"foreground-gdk", &WHITE,NULL);
	column = gtk_tree_view_column_new_with_attributes ("Total",
							   renderer,
							   "text",QROW_TOTAL,
							   "background_set", QROW_LAST,
							   "foreground_set", QROW_LAST,
							   NULL);
	gtk_tree_view_append_column(view, column);
	
	renderer = gtk_cell_renderer_text_new ();
	g_object_set (G_OBJECT (renderer),"background-gdk", &BLUE,NULL);
	g_object_set (G_OBJECT (renderer),"foreground-gdk", &WHITE,NULL);
	column = gtk_tree_view_column_new_with_attributes ("Wait",
							   renderer,
							   "text",QROW_WAIT,
							   "background_set", QROW_LAST,
							   "foreground_set", QROW_LAST,
							   NULL);
	gtk_tree_view_append_column(view, column);

	renderer = gtk_cell_renderer_text_new ();
	g_object_set (G_OBJECT (renderer),"background-gdk", &BLUE,NULL);
	g_object_set (G_OBJECT (renderer),"foreground-gdk", &WHITE,NULL);
	column = gtk_tree_view_column_new_with_attributes ("Run",
							   renderer,
							   "text",QROW_RUN,
							   "background_set", QROW_LAST,
							   "foreground_set", QROW_LAST,
							   NULL);
	gtk_tree_view_append_column(view, column);

	drop_to_row=row_to_color=NULL;
	gtk_drag_dest_set (GTK_WIDGET(view),
			   (GtkDestDefaults)(GTK_DEST_DEFAULT_MOTION |
					     GTK_DEST_DEFAULT_HIGHLIGHT |
					     GTK_DEST_DEFAULT_DROP),
			   ltarget_table, ln_targets,
			   (GdkDragAction)(GDK_ACTION_COPY|GDK_ACTION_MOVE));
	g_signal_connect(G_OBJECT (view), "drag_leave",
			   G_CALLBACK (target_drag_leave), this);
	g_signal_connect(G_OBJECT (view), "drag_drop",
			   G_CALLBACK (target_drag_drop), this);
	g_signal_connect(G_OBJECT (view), "drag_data_received",
			   G_CALLBACK (target_drag_data_received), this);
	g_signal_connect(G_OBJECT (view), "drag_motion",
			   G_CALLBACK (target_drag_motion), this);
	g_signal_connect(G_OBJECT(view), "event",
	                   G_CALLBACK(_event_queue_),this);
	menu1=menu2=dialog=prefs=NULL;
};

void d4xQsTree::add(d4xDownloadQueue *what,d4xDownloadQueue *papa){
	what->inserted=1;
	if (papa){
		gtk_tree_store_append(store,&(what->tree_iter),&(papa->tree_iter));
		GtkTreePath *path=gtk_tree_model_get_path(GTK_TREE_MODEL(store),&(papa->tree_iter));
		gtk_tree_view_expand_row(view,path,TRUE);
		gtk_tree_path_free(path);
	}else
		gtk_tree_store_append(store,&(what->tree_iter),NULL);
	char data[100];
	sprintf(data,"%i/%i",what->count(DL_RUN),what->MAX_ACTIVE);
	gtk_tree_store_set(store,&(what->tree_iter),
			   QROW_NAME,what->name.get(),
			   QROW_TOTAL,what->count(),
			   QROW_WAIT,what->count(DL_WAIT),
			   QROW_RUN,data,
			   QROW_QUEUE,what,
			   QROW_LAST,FALSE,
			   -1);
};

void d4xQsTree::del(d4xDownloadQueue *what){
	gtk_tree_store_remove(store,&(what->tree_iter));
	what->inserted=0;
};

void d4xQsTree::update(d4xDownloadQueue *what){
	if (store==NULL) return;
	char data[100];
	sprintf(data,"%i/%i",what->count(DL_RUN),what->MAX_ACTIVE);
	gtk_tree_store_set(store,&(what->tree_iter),
			   QROW_NAME,what->name.get(),
			   QROW_TOTAL,what->count(),
			   QROW_WAIT,what->count(DL_WAIT),
			   QROW_RUN,data,
			   -1);
};

void d4xQsTree::switch_remote(d4xDownloadQueue *what){
	if (store && what->inserted){
		GtkTreeSelection *sel=gtk_tree_view_get_selection(view);
		gtk_tree_selection_select_iter(sel,&(what->tree_iter));
	};
};

void d4xQsTree::switch_to(d4xDownloadQueue *what){
	if (D4X_QUEUE==what) return;
	if (D4X_QUEUE){
		D4X_QUEUE->reset_empty_func();
//		gtk_widget_hide(D4X_QUEUE->qv.ListOfDownloads);
		gtk_container_remove(GTK_CONTAINER(ContainerForCList),D4X_QUEUE->qv.ListOfDownloads);
	};
	gtk_container_add(GTK_CONTAINER(ContainerForCList),what->qv.ListOfDownloads);
	gtk_widget_show(what->qv.ListOfDownloads);
	what->set_defaults();
	D4X_QUEUE=what;
	prepare_buttons();
};

void d4xQsTree::select_row(GtkTreeIter *iter){
	GValue val={0,};
	gtk_tree_model_get_value(GTK_TREE_MODEL(store),iter,
				 QROW_QUEUE,&val);
	d4xDownloadQueue *q=(d4xDownloadQueue *)g_value_peek_pointer(&val);
	g_value_unset(&val);
	if (q) switch_to(q);
};

void d4xQsTree::init_menus() {
	GtkItemFactoryEntry menu_items1[] = {
		{_("/Create new queue"),	(gchar *)NULL,	(GtkItemFactoryCallback)_menu_event_,	0, (gchar *)NULL},
		{_("/Create new subqueue"),	(gchar *)NULL,	(GtkItemFactoryCallback)_menu_event_,	1, (gchar *)NULL},
		{_("/Delete queue"),		(gchar *)NULL,	(GtkItemFactoryCallback)_menu_event_,	2,(gchar *)NULL},
		{_("/Properties"),		(gchar *)NULL,	(GtkItemFactoryCallback)_menu_event_,	3, (gchar *)NULL}
	};
	GtkItemFactoryEntry menu_items2[] = {
		{_("/Create new queue"),	(gchar *)NULL,	(GtkItemFactoryCallback)_menu_event_,	0, (gchar *)NULL}
	};
	int nmenu_items1 = sizeof(menu_items1) / sizeof(menu_items1[0]);
	int nmenu_items2 = sizeof(menu_items2) / sizeof(menu_items2[0]);

	GtkAccelGroup *accel_group = gtk_accel_group_new();
	GtkItemFactory *item_factory = gtk_item_factory_new(GTK_TYPE_MENU, "<main>",accel_group);
	gtk_item_factory_create_items(item_factory, nmenu_items1, menu_items1, this);
	menu1 = gtk_item_factory_get_widget(item_factory, "<main>");

	accel_group = gtk_accel_group_new();
	item_factory = gtk_item_factory_new(GTK_TYPE_MENU, "<main>",accel_group);
	gtk_item_factory_create_items(item_factory, nmenu_items2, menu_items2, this);
	menu2 = gtk_item_factory_get_widget(item_factory, "<main>");
};

void d4xQsTree::popup_menu(GdkEvent *event,int selected){
	if (menu1==NULL) init_menus();	
	GdkEventButton *bevent=(GdkEventButton *)event;
	if (selected)
		gtk_menu_popup(GTK_MENU(menu1),
			       (GtkWidget *)NULL,
			       (GtkWidget *)NULL,
			       (GtkMenuPositionFunc)NULL,
			       (gpointer)NULL,
			       bevent->button,bevent->time);
	else
		gtk_menu_popup(GTK_MENU(menu2),
			       (GtkWidget *)NULL,
			       (GtkWidget *)NULL,
			       (GtkMenuPositionFunc)NULL,
			       (gpointer)NULL,
			       bevent->button,bevent->time);
};

d4xDownloadQueue *d4xQsTree::selected(){
	GtkTreeSelection *sel=gtk_tree_view_get_selection(view);
	GtkTreeIter iter;
	if (gtk_tree_selection_get_selected(sel,NULL,&iter)){
		GValue val={0,};
		gtk_tree_model_get_value(GTK_TREE_MODEL(store),&iter,
					 QROW_QUEUE,&val);
		d4xDownloadQueue *q=(d4xDownloadQueue *)g_value_peek_pointer(&val);
		g_value_unset(&val);
		return(q);
	};
	return(NULL);
};

void d4xQsTree::delete_queue(){
	d4xDownloadQueue *q=selected();
	if (q==NULL) return;
	if (q->count() || q->child.count()) return; //remove all downloads
	if (q->prev==NULL && q->next==NULL && q->parent==NULL) return; //last queue
	if (D4X_QUEUE==q){
		if (q->parent) switch_to(q->parent);
		else{
			d4xDownloadQueue *sw=(d4xDownloadQueue *)(q->next?q->next:q->prev);
			switch_to(sw);
		};
	};
	if (q->parent)
		q->parent->subq_del(q);
	else
		D4X_QTREE.del(q);
	del(q);
	delete(q);
};

static void _prefs_cancel_(GtkButton *button,d4xQsTree *qt){
	qt->prefs_cancel();
};

static void _prefs_ok_(GtkWidget *widget,d4xQsTree *qt){
	qt->prefs_ok();
};

static void _prefs_delete_(GtkWidget *window,GdkEvent *event,d4xQsTree *qt){
	qt->prefs_cancel();
};


void d4xQsTree::prefs_init(){
	d4xDownloadQueue *q=selected();
	if (q==NULL) return;
	if (prefs){
		gdk_window_show(prefs->window);
		return;
	};
	GtkWidget *vbox=gtk_vbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(vbox),5);

	prefs = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_wmclass(GTK_WINDOW(prefs),
			       "D4X_QueuePrefs","D4X");
	gtk_window_set_title(GTK_WINDOW (prefs),
			     _("Queue properties"));
	gtk_window_set_position(GTK_WINDOW(prefs),
				GTK_WIN_POS_CENTER);
	gtk_container_set_border_width(GTK_CONTAINER(prefs),5);

	GtkWidget *prefs_limits_tbox=gtk_hbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(prefs_limits_tbox),5);
	GtkAdjustment *adj = (GtkAdjustment *) gtk_adjustment_new (q->MAX_ACTIVE, 0, 50.0, 1.0, 3.0, 0.0);
//	max_threads=my_gtk_entry_new_with_max_length(3,q->MAX_ACTIVE);
	max_threads = gtk_spin_button_new (adj, 0, 0);
	gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (max_threads), TRUE);
	gtk_box_pack_start(GTK_BOX(prefs_limits_tbox),max_threads,FALSE,FALSE,0);
	GtkWidget *prefs_limits_tlabel=gtk_label_new(_("Maximum active downloads"));
	gtk_box_pack_start(GTK_BOX(prefs_limits_tbox),prefs_limits_tlabel,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),prefs_limits_tbox,FALSE,FALSE,0);

	del_completed=gtk_check_button_new_with_label(_("Automatically delete completed downloads"));
	del_fataled=gtk_check_button_new_with_label(_("Automatically delete failed downloads"));
	GTK_TOGGLE_BUTTON(del_completed)->active=q->AUTODEL_COMPLETED;
	GTK_TOGGLE_BUTTON(del_fataled)->active=q->AUTODEL_FAILED;
	gtk_box_pack_start(GTK_BOX(vbox),del_completed,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),del_fataled,FALSE,FALSE,0);

	path_entry=my_gtk_filesel_new(ALL_HISTORIES[PATH_HISTORY]);
	MY_GTK_FILESEL(path_entry)->modal=GTK_WINDOW(prefs);
	MY_GTK_FILESEL(path_entry)->only_dirs=TRUE;
	text_to_combo(MY_GTK_FILESEL(path_entry)->combo,q->save_path.get());
	GtkWidget *path_label=gtk_label_new(_("Default folder to save downloaded files"));
	GtkWidget *path_vbox=gtk_vbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(path_vbox),path_label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(path_vbox),path_entry,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),path_vbox,FALSE,FALSE,0);
	
	GtkWidget *columns_hbox=gtk_hbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(columns_hbox),5);
	GtkWidget *columns_frame1=gtk_frame_new(_("Size format"));
	GtkWidget *columns_frame2=gtk_frame_new(_("Time format"));
	gtk_container_set_border_width(GTK_CONTAINER(columns_frame1),5);
	gtk_container_set_border_width(GTK_CONTAINER(columns_frame2),5);
	GtkWidget *columns_vbox1=gtk_vbox_new(FALSE,0);
	GtkWidget *columns_vbox2=gtk_vbox_new(FALSE,0);

	columns_nums1=gtk_radio_button_new_with_label((GSList *)NULL,"123456");
	gtk_box_pack_start(GTK_BOX(columns_vbox1),columns_nums1,FALSE,FALSE,0);
	GSList *columns_group1=gtk_radio_button_get_group(GTK_RADIO_BUTTON(columns_nums1));
	columns_nums2=gtk_radio_button_new_with_label(columns_group1,"123 456");
	gtk_box_pack_start(GTK_BOX(columns_vbox1),columns_nums2,FALSE,FALSE,0);
	columns_nums3=gtk_radio_button_new_with_label(
		gtk_radio_button_get_group(GTK_RADIO_BUTTON(columns_nums2)),"123K");
	gtk_box_pack_start(GTK_BOX(columns_vbox1),columns_nums3,FALSE,FALSE,0);
	columns_nums4=gtk_radio_button_new_with_label(
		gtk_radio_button_get_group(GTK_RADIO_BUTTON(columns_nums3)),"123'456");
	gtk_box_pack_start(GTK_BOX(columns_vbox1),columns_nums4,FALSE,FALSE,0);

	switch(q->NICE_DEC_DIGITALS) {
	case 1:	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(columns_nums2),TRUE);
		break;
	};
	case 2:{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(columns_nums3),TRUE);
		break;
	};
	case 3:	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(columns_nums4),TRUE);
		break;
	};
	default:
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(columns_nums1),TRUE);
	};
	columns_time1=gtk_radio_button_new_with_label((GSList *)NULL,"12:34:56");
	gtk_box_pack_start(GTK_BOX(columns_vbox2),columns_time1,FALSE,FALSE,0);
	GSList *columns_group2=gtk_radio_button_get_group(GTK_RADIO_BUTTON(columns_time1));
	columns_time2=gtk_radio_button_new_with_label(columns_group2,"12:34");
	gtk_box_pack_start(GTK_BOX(columns_vbox2),columns_time2,FALSE,FALSE,0);
	if (q->TIME_FORMAT)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(columns_time2),TRUE);
	else
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(columns_time1),TRUE);
	gtk_container_add(GTK_CONTAINER(columns_frame1),columns_vbox1);
	GtkWidget *columns_vbox11=gtk_vbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(columns_vbox11),columns_frame1,FALSE,FALSE,0);
	GtkWidget *columns_vbox12=gtk_vbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(columns_vbox11),columns_vbox12,FALSE,FALSE,0);

	gtk_container_add(GTK_CONTAINER(columns_frame2),columns_vbox2);
	GtkWidget *columns_vbox21=gtk_vbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(columns_vbox21),columns_frame2,FALSE,FALSE,0);
	GtkWidget *columns_vbox22=gtk_vbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(columns_vbox21),columns_vbox22,FALSE,FALSE,0);

	gtk_box_set_spacing(GTK_BOX(columns_hbox),5);
	gtk_box_pack_start(GTK_BOX(columns_hbox),columns_vbox11,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(columns_hbox),columns_vbox21,FALSE,FALSE,0);

	gtk_box_pack_start(GTK_BOX(vbox),columns_hbox,FALSE,FALSE,0);

	GtkWidget *prefs_tbox=gtk_hbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(prefs_tbox),5);
	name=gtk_entry_new();
	text_to_combo(name,q->name.get());
	GtkWidget *prefs_tlabel=gtk_label_new(_("Name of the queue"));
	gtk_box_pack_start(GTK_BOX(prefs_tbox),prefs_tlabel,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(prefs_tbox),name,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),prefs_tbox,FALSE,FALSE,0);

	GtkWidget *hbbox=gtk_hbutton_box_new();
	gtk_box_set_spacing(GTK_BOX(hbbox),5);
	GtkWidget *ok_button=gtk_button_new_with_label(_("Ok"));
	GtkWidget *cancel_button=gtk_button_new_with_label(_("Cancel"));
	GTK_WIDGET_SET_FLAGS(ok_button,GTK_CAN_DEFAULT);
	GTK_WIDGET_SET_FLAGS(cancel_button,GTK_CAN_DEFAULT);
	gtk_box_pack_end(GTK_BOX(hbbox),ok_button,FALSE,FALSE,0);
	gtk_box_pack_end(GTK_BOX(hbbox),cancel_button,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),hbbox,FALSE,FALSE,0);

	g_signal_connect(G_OBJECT(ok_button),"clicked",
			 G_CALLBACK(_prefs_ok_),this);
	g_signal_connect(G_OBJECT(cancel_button),"clicked",
			 G_CALLBACK(_prefs_cancel_),this);
	g_signal_connect(G_OBJECT(prefs),"delete_event",
			 G_CALLBACK(_prefs_delete_),this);
	d4x_eschandler_init(prefs,this);
	
	gtk_container_add(GTK_CONTAINER(prefs),vbox);
	gtk_widget_show_all(prefs);
	gtk_window_set_default(GTK_WINDOW(prefs),ok_button);
	gtk_window_set_modal (GTK_WINDOW(prefs),TRUE);
	gtk_window_set_transient_for(GTK_WINDOW (prefs),
				     GTK_WINDOW (MainWindow));
};

void d4xQsTree::prefs_cancel(){
	gtk_widget_destroy(prefs);
	prefs=NULL;
};

void d4xQsTree::prefs_ok(){
	d4xDownloadQueue *q=selected();
	q->NICE_DEC_DIGITALS=(GTK_TOGGLE_BUTTON(columns_nums2)->active?1:0)+
		(GTK_TOGGLE_BUTTON(columns_nums3)->active?2:0)+
		(GTK_TOGGLE_BUTTON(columns_nums4)->active?3:0);
	q->TIME_FORMAT=GTK_TOGGLE_BUTTON(columns_time2)->active;
	q->name.set(text_from_combo(name));
	char *path=normalize_path_full(text_from_combo(MY_GTK_FILESEL(path_entry)->combo));
	q->save_path.set(path);
	delete[] path;
	q->update();
//	sscanf(gtk_entry_get_text(GTK_ENTRY(max_threads)),"%u",&(q->MAX_ACTIVE));
	q->MAX_ACTIVE=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON (max_threads));
	if (q->MAX_ACTIVE<0) q->MAX_ACTIVE=0;
	if (q->MAX_ACTIVE>50) q->MAX_ACTIVE=50;
	q->AUTODEL_FAILED=GTK_TOGGLE_BUTTON(del_fataled)->active;
	q->AUTODEL_COMPLETED=GTK_TOGGLE_BUTTON(del_completed)->active;
	if (q->AUTODEL_COMPLETED) aa.del_completed(q);
	if (q->AUTODEL_FAILED) aa.del_fataled(q);
	update(q);
	aa.try_to_run_wait(q);
	prefs_cancel();
};

void d4xQsTree::move_to(tDownload *where){
	if (where->myowner==NULL || where->myowner->PAPA==NULL) return;
	switch_to(where->myowner->PAPA);
	D4X_QUEUE->qv.move_to(where);
	D4X_QUEUE->qv.select(where);
};


