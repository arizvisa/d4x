#include "qtree.h"
#include "list.h"
#include "misc.h"
#include "colors.h"
#include "../ntlocale.h"
#include "../main.h"
#include "../var.h"
#include "mywidget.h"
#include "buttons.h"

extern tMain aa;

static void _select_queue_(GtkCTree *tree,
			   gint row,
			   gint column,
			   GdkEvent *event,
			   d4xQsTree *q){
	q->select_row(row);
};

static gint _event_queue_(GtkWidget *widget,GdkEvent *event,d4xQsTree *q){
	if (event->type == GDK_BUTTON_PRESS) {
		GdkEventButton *bevent=(GdkEventButton *)event;
		if (bevent->button==3) {
			gint row;
			if (gtk_clist_get_selection_info(GTK_CLIST(widget),
							 int(bevent->x),
							 int(bevent->y),
							 &row,(gint *)NULL)) {
				gtk_clist_select_row(GTK_CLIST(widget),row,-1);
			}else{
				if (GTK_CLIST(widget)->selection)
					gtk_clist_unselect_all(GTK_CLIST(widget));
			};
			q->popup_menu(event);
		};
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

void d4xQsTree::create_init(int mode=0){
	if (dialog){
		gdk_window_show(dialog->window);
		return;
	};
	create_mode=mode;
	dialog = gtk_window_new(GTK_WINDOW_DIALOG);
	gtk_window_set_wmclass(GTK_WINDOW(dialog),
			       "D4X_CreateQueueDialog","D4X");
	gtk_window_set_title(GTK_WINDOW (dialog),
			     _("Create new queue"));
	gtk_window_set_position(GTK_WINDOW(dialog),
				GTK_WIN_POS_CENTER);
	gtk_container_border_width(GTK_CONTAINER(dialog),5);
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
	gtk_box_pack_start(GTK_BOX(hbox),ok_button,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(hbox),cancel_button,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);
	gtk_signal_connect(GTK_OBJECT(ok_button),"clicked",
			   GTK_SIGNAL_FUNC(_create_ok_),this);
	gtk_signal_connect(GTK_OBJECT(cancel_button),"clicked",
			   GTK_SIGNAL_FUNC(_create_cancel_),this);
	gtk_signal_connect(GTK_OBJECT(dialog),"delete_event",
			   GTK_SIGNAL_FUNC(_create_delete_),this);
	gtk_signal_connect(GTK_OBJECT(dialog_entry), "activate",
			   GTK_SIGNAL_FUNC (_create_ok_), this);
	d4x_eschandler_init(dialog,this);
	gtk_container_add(GTK_CONTAINER(dialog),vbox);
	gtk_widget_show_all(dialog);
	gtk_window_set_default(GTK_WINDOW(dialog),ok_button);
	gtk_widget_grab_focus(dialog_entry);
	gtk_window_set_modal (GTK_WINDOW(dialog),TRUE);
	gtk_window_set_transient_for (GTK_WINDOW (dialog),
				      GTK_WINDOW (MainWindow));
};

static gboolean target_drag_drop (GtkWidget          *widget,
				  GdkDragContext     *context,
				  gint                x,
				  gint                y,
				  guint               time,
				  d4xQsTree *qt){
	int row;
	if (context->targets &&
	    gtk_clist_get_selection_info(GTK_CLIST(widget),
					 x,y,
					 &row,(gint *)NULL)){
		qt->drop_to_row=row;
		gtk_drag_get_data (widget, context,
				   GPOINTER_TO_INT (context->targets->data),
				   time);
		return TRUE;
	};
	qt->drop_to_row=-1;
	return FALSE;
};


void d4xQsTree::drop_from(GtkWidget *clist){
	drag_motion(-1);
	if (drop_to_row<0) return;
	GList *select=(GTK_CLIST(clist))->selection_end;
	if (select==NULL) return;
	tDownload *temp=(tDownload *)gtk_clist_get_row_data(
		GTK_CLIST(clist),GPOINTER_TO_INT(select->data));
	if (temp==NULL || temp->myowner==NULL) return;
	d4xDownloadQueue *src=temp->myowner->PAPA;
	if (src==NULL) return;
	d4xDownloadQueue *dst=(d4xDownloadQueue *)gtk_clist_get_row_data(GTK_CLIST(tree),drop_to_row);
	if (dst && dst!=src){
//		printf("Dropped to %s from %s\n",dst->name.get(),src->name.get());
		select=(GTK_CLIST(clist))->selection_end;
		while (select){
			temp=(tDownload *)gtk_clist_get_row_data(
				GTK_CLIST(clist),GPOINTER_TO_INT(select->data));
			int type=temp->owner();
			src->del(temp);
			src->qv.remove(temp);
			dst->add(temp,type);
			dst->qv.add(temp);
			select=(GTK_CLIST(clist))->selection_end;
		};
		aa.try_to_run_wait(dst);
		aa.try_to_run_wait(src);
		prepare_buttons();
	};
};

void d4xQsTree::drag_motion(int row){
	if (row==row_to_color) return;
	if (row_to_color>=0){
		gtk_clist_set_background(GTK_CLIST(tree),row_to_color,NULL);
		gtk_clist_set_foreground(GTK_CLIST(tree),row_to_color,NULL);
	};
	row_to_color=row;
	if (row>=0){
		GtkStyle *style=gtk_widget_get_style(GTK_WIDGET(tree));
		gtk_clist_set_background(GTK_CLIST(tree),row,&(style->bg[GTK_STATE_SELECTED]));
		gtk_clist_set_foreground(GTK_CLIST(tree),row,&(style->fg[GTK_STATE_SELECTED]));
	};
};

static gboolean target_drag_motion (GtkWidget *widget,
				    GdkDragContext *context,
				    gint x,gint y,
				    guint time,
				    d4xQsTree *qt){
	
	int row;
	if (context->targets &&
	    gtk_clist_get_selection_info(GTK_CLIST(widget),
					 x,y,
					 &row,(gint *)NULL)){
		qt->drag_motion(row);
		return TRUE;
	};
	qt->drag_motion(-1);
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
		GtkWidget **clist=NULL;
		clist=(GtkWidget **)(data->data);
		if (clist && *clist){
			qt->drop_from(*clist);
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
	qt->drag_motion(-1);
}

void d4xQsTree::init(){
	char *title[]={
		N_("Name"),
		N_("Total"),
		N_("Wait"),
		N_("Run")
	};
	row_to_color=-1;
	tree = GTK_CTREE(gtk_ctree_new_with_titles (4, 0, title));
	gtk_drag_dest_set (GTK_WIDGET(tree),
			   (GtkDestDefaults)(GTK_DEST_DEFAULT_MOTION |
					     GTK_DEST_DEFAULT_HIGHLIGHT |
					     GTK_DEST_DEFAULT_DROP),
			   ltarget_table, ln_targets,
			   (GdkDragAction)(GDK_ACTION_COPY|GDK_ACTION_MOVE));
	gtk_signal_connect (GTK_OBJECT (tree), "drag_leave",
			    GTK_SIGNAL_FUNC (target_drag_leave), this);
	gtk_signal_connect (GTK_OBJECT (tree), "drag_drop",
			    GTK_SIGNAL_FUNC (target_drag_drop), this);
	gtk_signal_connect (GTK_OBJECT (tree), "drag_data_received",
			    GTK_SIGNAL_FUNC (target_drag_data_received), this);
	gtk_signal_connect (GTK_OBJECT (tree), "drag_motion",
			    GTK_SIGNAL_FUNC (target_drag_motion), this);
	gtk_clist_column_titles_hide(GTK_CLIST(tree));
	gtk_clist_set_selection_mode(GTK_CLIST(tree),GTK_SELECTION_SINGLE);
	for (int i=0;i<4;i++)
		gtk_clist_set_column_auto_resize(GTK_CLIST(tree),i,TRUE);
	gtk_signal_connect(GTK_OBJECT(tree), "select_row",
	                   GTK_SIGNAL_FUNC(_select_queue_),this);
	gtk_signal_connect(GTK_OBJECT(tree), "event",
	                   GTK_SIGNAL_FUNC(_event_queue_),this);
	menu1=menu2=dialog=prefs=NULL;
};

void d4xQsTree::add(d4xDownloadQueue *what,d4xDownloadQueue *papa=NULL){
	char *text[4];
	text[0]=what->name.get();
	char data1[10],data2[10],data3[10];
	sprintf(data1,"%i",what->count());
	sprintf(data2,"%i",what->count(DL_WAIT));
	sprintf(data3,"%i/%i",what->count(DL_RUN),what->MAX_ACTIVE);
	text[1]=data1;
	text[2]=data2;
	text[3]=data3;
	if (papa){
		GtkCTreeNode *a=gtk_ctree_find_by_row_data(tree,NULL,papa);
		if (a){
			GtkCTreeNode *node=gtk_ctree_insert_node (tree, a, NULL,text,5,NULL,NULL,NULL,NULL,FALSE,TRUE);
			gtk_ctree_node_set_row_data(tree,node,what);
			return;
		};
	};
	GtkCTreeNode *node=gtk_ctree_insert_node (tree, NULL, NULL,text,5,NULL,NULL,NULL,NULL,FALSE,TRUE);
	gtk_ctree_node_set_row_data(tree,node,what);
};

void d4xQsTree::del(d4xDownloadQueue *what){
	GtkCTreeNode *a=gtk_ctree_find_by_row_data(tree,NULL,what);
	gtk_ctree_remove_node(tree,a);
};

void d4xQsTree::update(d4xDownloadQueue *what){
	GtkCTreeNode *a=gtk_ctree_find_by_row_data(tree,NULL,what);
	if (a){
		what->name.get();
		gtk_ctree_node_set_text(tree,a,0,what->name.get());
		char data1[10];
		sprintf(data1,"%i",what->count());
		gtk_ctree_node_set_text(tree,a,1,data1);
		sprintf(data1,"%i",what->count(DL_WAIT));
		gtk_ctree_node_set_text(tree,a,2,data1);
		sprintf(data1,"%i/%i",what->count(DL_RUN),what->MAX_ACTIVE);
		gtk_ctree_node_set_text(tree,a,3,data1);
	};
};

void d4xQsTree::switch_to(d4xDownloadQueue *what){
	if (D4X_QUEUE==what) return;
	if (D4X_QUEUE){
		D4X_QUEUE->reset_empty_func();
		gtk_widget_hide(D4X_QUEUE->qv.ListOfDownloads);
		gtk_container_remove(GTK_CONTAINER(ContainerForCList),D4X_QUEUE->qv.ListOfDownloads);
	};
	gtk_container_add(GTK_CONTAINER(ContainerForCList),what->qv.ListOfDownloads);
	gtk_widget_show(what->qv.ListOfDownloads);
	what->set_defaults();
	D4X_QUEUE=what;
	prepare_buttons();
	if (GTK_CLIST(tree)->selection==NULL){
		GtkCTreeNode *node=gtk_ctree_find_by_row_data(tree,
							      NULL,what);
		if (node)
			gtk_ctree_select(tree,node);
 	};
};

void d4xQsTree::select_row(int row){
	d4xDownloadQueue *q=(d4xDownloadQueue *)gtk_clist_get_row_data(GTK_CLIST(tree),row);
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

void d4xQsTree::popup_menu(GdkEvent *event){
	if (menu1==NULL) init_menus();	
	GdkEventButton *bevent=(GdkEventButton *)event;
	if (GTK_CLIST(tree)->selection)
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
	if (GTK_CLIST(tree)->selection==NULL) return(NULL);
	GtkCTreeNode *node=(GtkCTreeNode *)(GTK_CLIST(tree)->selection->data);
	return((d4xDownloadQueue *)gtk_ctree_node_get_row_data(tree,node));
};

void d4xQsTree::delete_queue(){
	if (GTK_CLIST(tree)->selection==NULL) return;
	GtkCTreeNode *node=(GtkCTreeNode *)(GTK_CLIST(tree)->selection->data);
	d4xDownloadQueue *q=(d4xDownloadQueue *)gtk_ctree_node_get_row_data(tree,node);
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
	delete(q);
	gtk_ctree_remove_node(tree,node);
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

	prefs = gtk_window_new(GTK_WINDOW_DIALOG);
	gtk_window_set_wmclass(GTK_WINDOW(prefs),
			       "D4X_QueuePrefs","D4X");
	gtk_window_set_title(GTK_WINDOW (prefs),
			     _("Queue properties"));
	gtk_window_set_position(GTK_WINDOW(prefs),
				GTK_WIN_POS_CENTER);
	gtk_container_border_width(GTK_CONTAINER(prefs),5);

	GtkWidget *prefs_limits_tbox=gtk_hbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(prefs_limits_tbox),5);
	max_threads=my_gtk_entry_new_with_max_length(3,q->MAX_ACTIVE);
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
	gtk_container_border_width(GTK_CONTAINER(columns_frame1),5);
	gtk_container_border_width(GTK_CONTAINER(columns_frame2),5);
	GtkWidget *columns_vbox1=gtk_vbox_new(FALSE,0);
	GtkWidget *columns_vbox2=gtk_vbox_new(FALSE,0);

	columns_nums1=gtk_radio_button_new_with_label((GSList *)NULL,"123456");
	gtk_box_pack_start(GTK_BOX(columns_vbox1),columns_nums1,FALSE,FALSE,0);
	GSList *columns_group1=gtk_radio_button_group(GTK_RADIO_BUTTON(columns_nums1));
	columns_nums2=gtk_radio_button_new_with_label(columns_group1,"123 456");
	gtk_box_pack_start(GTK_BOX(columns_vbox1),columns_nums2,FALSE,FALSE,0);
	columns_nums3=gtk_radio_button_new_with_label(
		gtk_radio_button_group(GTK_RADIO_BUTTON(columns_nums2)),"123K");
	gtk_box_pack_start(GTK_BOX(columns_vbox1),columns_nums3,FALSE,FALSE,0);
	columns_nums4=gtk_radio_button_new_with_label(
		gtk_radio_button_group(GTK_RADIO_BUTTON(columns_nums3)),"123'456");
	gtk_box_pack_start(GTK_BOX(columns_vbox1),columns_nums4,FALSE,FALSE,0);

	switch(q->NICE_DEC_DIGITALS) {
	case 1:	{
		gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(columns_nums2),TRUE);
		break;
	};
	case 2:{
		gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(columns_nums3),TRUE);
		break;
	};
	case 3:	{
		gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(columns_nums4),TRUE);
		break;
	};
	default:
		gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(columns_nums1),TRUE);
	};
	columns_time1=gtk_radio_button_new_with_label((GSList *)NULL,"12:34:56");
	gtk_box_pack_start(GTK_BOX(columns_vbox2),columns_time1,FALSE,FALSE,0);
	GSList *columns_group2=gtk_radio_button_group(GTK_RADIO_BUTTON(columns_time1));
	columns_time2=gtk_radio_button_new_with_label(columns_group2,"12:34");
	gtk_box_pack_start(GTK_BOX(columns_vbox2),columns_time2,FALSE,FALSE,0);
	if (q->TIME_FORMAT)
		gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(columns_time2),TRUE);
	else
		gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(columns_time1),TRUE);
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

	GtkWidget *columns_vbox=gtk_vbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(columns_hbox),5);
	gtk_box_pack_start(GTK_BOX(columns_vbox),columns_vbox11,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(columns_vbox),columns_vbox21,FALSE,FALSE,0);
	columns_order.init(&(q->qv));
	gtk_box_pack_start(GTK_BOX(columns_hbox),columns_vbox,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(columns_hbox),columns_order.body(),FALSE,FALSE,0);

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

	gtk_signal_connect(GTK_OBJECT(ok_button),"clicked",
			   GTK_SIGNAL_FUNC(_prefs_ok_),this);
	gtk_signal_connect(GTK_OBJECT(cancel_button),"clicked",
			   GTK_SIGNAL_FUNC(_prefs_cancel_),this);
	gtk_signal_connect(GTK_OBJECT(prefs),"delete_event",
			   GTK_SIGNAL_FUNC(_prefs_delete_),this);
	d4x_eschandler_init(prefs,this);
	
	gtk_container_add(GTK_CONTAINER(prefs),vbox);
	gtk_widget_show_all(prefs);
	gtk_window_set_default(GTK_WINDOW(prefs),ok_button);
	gtk_window_set_modal (GTK_WINDOW(prefs),TRUE);
	gtk_window_set_transient_for (GTK_WINDOW (prefs),
				      GTK_WINDOW (MainWindow));
};

void d4xQsTree::prefs_cancel(){
	columns_order.reset();
	gtk_widget_destroy(prefs);
	prefs=NULL;
};

void d4xQsTree::prefs_ok(){
	d4xDownloadQueue *q=selected();
	columns_order.apply_changes_tmp();
	q->NICE_DEC_DIGITALS=(GTK_TOGGLE_BUTTON(columns_nums2)->active?1:0)+
		(GTK_TOGGLE_BUTTON(columns_nums3)->active?2:0)+
		(GTK_TOGGLE_BUTTON(columns_nums4)->active?3:0);
	q->TIME_FORMAT=GTK_TOGGLE_BUTTON(columns_time2)->active;
	q->name.set(text_from_combo(name));
	char *path=normalize_path_full(text_from_combo(MY_GTK_FILESEL(path_entry)->combo));
	q->save_path.set(path);
	delete[] path;
	q->update();
	sscanf(gtk_entry_get_text(GTK_ENTRY(max_threads)),"%u",&(q->MAX_ACTIVE));
	if (q->MAX_ACTIVE<0) q->MAX_ACTIVE=0;
	if (q->MAX_ACTIVE>50) q->MAX_ACTIVE=50;
	q->AUTODEL_FAILED=GTK_TOGGLE_BUTTON(del_fataled)->active;
	q->AUTODEL_COMPLETED=GTK_TOGGLE_BUTTON(del_completed)->active;
	if (q==D4X_QUEUE && columns_order.apply_changes()){
		gtk_container_add(GTK_CONTAINER(ContainerForCList),q->qv.ListOfDownloads);
		gtk_widget_show(q->qv.ListOfDownloads);
	};
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


