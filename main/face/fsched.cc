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
#include <package_config.h>
#include "fsched.h"
#include "../schedule.h"
#include "../var.h"
#include "../ntlocale.h"
#include "misc.h"
#include "mywidget.h"
#include "edit.h"

GtkWidget *d4x_scheduler_window=(GtkWidget *)NULL;
GtkWidget *d4x_scheduler_clist=(GtkWidget *)NULL;

/*
static void d4x_scheduler_select(GtkWidget *clist, gint row, gint column,
				 GdkEventButton *event) {
	if (event && event->type==GDK_2BUTTON_PRESS && event->button==1)
};
*/

static char *action_names[]={
	N_("limit speed"),
	N_("popup main window"),
	N_("exit"),
	N_("remove completed"),
	N_("remove failed"),
	N_("restart download"),
	N_("stop download"),
	N_("remove download"),
	N_("remove download if completed"),
	N_("add download"),
	N_("save list"),
	N_("execute command")
};

void d4x_scheduler_insert(d4xSchedAction *act,d4xSchedAction *prev){
	if (d4x_scheduler_window==NULL) return;
	char *text[2];
	char buf[MAX_LEN];
	char buf2[MAX_LEN];
	struct tm tm_time;
	*buf2=0;
	localtime_r(&(act->start_time),&tm_time);
	strftime(buf,MAX_LEN,"%T %d %b %Y",&tm_time);
	text[0]=buf;
	text[1]=_(action_names[act->type()]);
	text[2]=buf2;
	switch(act->type()){
	case SACT_EXECUTE:{
		d4xSAExecute *a=(d4xSAExecute *)act;
		g_snprintf(buf2,MAX_LEN,"%s",a->command.get());
		break;
	};
	case SACT_SAVE_LIST:{
		d4xSASaveList *a=(d4xSASaveList *)act;
		g_snprintf(buf2,MAX_LEN,"%s",a->path.get());
		break;
	};
	case SACT_SET_SPEED:{
		d4xSASpeed *a=(d4xSASpeed *)act;
		sprintf(buf2,"%s",_(SPEED_LIMITATIONS_NAMES[a->speed]));
		break;
	};
	case SACT_ADD_DOWNLOAD:{
		d4xSAAddDownload *a=(d4xSAAddDownload *)act;
		if (a->dwn && a->dwn->info){
			char *url=a->dwn->info->url();
			g_snprintf(buf2,MAX_LEN,"%s",url);
			delete[] url;
		};
		break;
	};
	case SACT_DEL_IF_COMPLETED:
	case SACT_DELETE_DOWNLOAD:
	case SACT_RUN_DOWNLOAD:
	case SACT_PAUSE_DOWNLOAD:{
		d4xSAUrl *a=(d4xSAUrl *)act;
		if (a->url){
			char *url=a->url->url();
			g_snprintf(buf2,MAX_LEN,"%s",url);
			delete[] url;
		};
		break;
	};
	};
	gint row=0;
	if (prev)
		row=gtk_clist_find_row_from_data(GTK_CLIST(d4x_scheduler_clist),prev)+1;
	row=gtk_clist_insert(GTK_CLIST(d4x_scheduler_clist),row,text);
	gtk_clist_set_row_data(GTK_CLIST(d4x_scheduler_clist),row,act);
};

void d4x_scheduler_remove(d4xSchedAction *act){
	if (d4x_scheduler_window==NULL) return;
	gint row=gtk_clist_find_row_from_data(GTK_CLIST(d4x_scheduler_clist),act);
	gtk_clist_remove(GTK_CLIST(d4x_scheduler_clist),row);
};

static gint compare_nodes2(gconstpointer a,gconstpointer b){
    gint aa=GPOINTER_TO_INT(a);
    gint bb=GPOINTER_TO_INT(b);
    if (aa>bb) return -1;
    if (aa==bb) return 0;
    return 1;
};

void d4x_scheduler_remove_selected(){
	if (d4x_scheduler_window==NULL) return;
	GList *select=g_list_copy(GTK_CLIST(d4x_scheduler_clist)->selection);
	select=g_list_sort(select,compare_nodes2);
	while (select) {
		gint row=GPOINTER_TO_INT(select->data);
		d4xSchedAction *act=(d4xSchedAction *)gtk_clist_get_row_data(GTK_CLIST(d4x_scheduler_clist),row);
		if (act){
			MainScheduler->del_action(act);
			delete(act);
		};
		select=select->next;
	};
	g_list_free(select);
};

gint d4x_scheduler_close() {
	if (!d4x_scheduler_window) return(FALSE);
	gtk_widget_destroy(d4x_scheduler_window);
	d4x_scheduler_window=(GtkWidget *)NULL;
	return(TRUE);
};

void d4x_scheduler_init_editor(){
	GtkWidget *tmp=my_gtk_aeditor_new();
//	MyGtkAEditor *editor=MY_GTK_AEDITOR(tmp);

	gtk_widget_show_all(tmp);
	gtk_window_set_modal (GTK_WINDOW(tmp),TRUE);
	gtk_window_set_transient_for (GTK_WINDOW (tmp), GTK_WINDOW (d4x_scheduler_window));
};

void d4x_scheduler_edit(){
	if (d4x_scheduler_window==NULL) return;
	GList *select=GTK_CLIST(d4x_scheduler_clist)->selection;
	if (select){		
		d4xSchedAction *act=(d4xSchedAction *)gtk_clist_get_row_data(GTK_CLIST(d4x_scheduler_clist),
									     GPOINTER_TO_INT(select->data));
		if (act && act->lock==0){
			act->lock=1;
			GtkWidget *tmp=my_gtk_aeditor_new(act);
//			MyGtkAEditor *editor=MY_GTK_AEDITOR(tmp);
			gtk_widget_show_all(tmp);
			gtk_window_set_modal (GTK_WINDOW(tmp),TRUE);
			gtk_window_set_transient_for (GTK_WINDOW (tmp), GTK_WINDOW (d4x_scheduler_window));
		};
	};
};

gint d4x_scheduler_select(GtkWidget *widget, gint row, gint column,
			  GdkEventButton *event, gpointer data,
			  gpointer nothing){
	if (event && event->type==GDK_2BUTTON_PRESS && event->button==1){
		d4x_scheduler_edit();
	};
	return(TRUE);
};

void d4x_scheduler_init(){
	if (d4x_scheduler_window) {
		gdk_window_show(d4x_scheduler_window->window);
		return;
	};
	d4x_scheduler_window = gtk_window_new(GTK_WINDOW_DIALOG);
	gtk_window_set_title(GTK_WINDOW (d4x_scheduler_window),
			     _("Scheduler"));
	gtk_window_set_wmclass(GTK_WINDOW(d4x_scheduler_window),
			       "D4X_Scheduler","D4X");
	gtk_window_set_position(GTK_WINDOW(d4x_scheduler_window),GTK_WIN_POS_CENTER);
	gtk_widget_set_usize(d4x_scheduler_window,-1,400);
	gtk_container_border_width(GTK_CONTAINER(d4x_scheduler_window),5);
	gchar *titles[]={_("Time"),_("Action"),_("Info")};
	d4x_scheduler_clist = gtk_clist_new_with_titles(3, titles);
	gtk_clist_set_shadow_type (GTK_CLIST(d4x_scheduler_clist), GTK_SHADOW_IN);
	gtk_clist_set_column_auto_resize(GTK_CLIST(d4x_scheduler_clist),0,TRUE);
	gtk_clist_set_column_auto_resize(GTK_CLIST(d4x_scheduler_clist),1,TRUE);
	gtk_clist_set_column_auto_resize(GTK_CLIST(d4x_scheduler_clist),2,TRUE);
//	gtk_clist_set_column_width (GTK_CLIST(d4x_scheduler_clist), 1 , 200);
	gtk_clist_set_selection_mode(GTK_CLIST(d4x_scheduler_clist),GTK_SELECTION_EXTENDED);
	gtk_signal_connect(GTK_OBJECT(d4x_scheduler_clist), "select_row",
	                   GTK_SIGNAL_FUNC(d4x_scheduler_select),NULL);

	GtkWidget *scroll_window=gtk_scrolled_window_new((GtkAdjustment *)NULL,(GtkAdjustment *)NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll_window),
	                                GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(scroll_window),d4x_scheduler_clist);
	GtkWidget *button=gtk_button_new_with_label(_("Ok"));
	GtkWidget *add_button=gtk_button_new_with_label(_("New"));
	GtkWidget *edit_button=gtk_button_new_with_label(_("Edit"));
	GtkWidget *del_button=gtk_button_new_with_label(_("Remove"));
	GTK_WIDGET_SET_FLAGS(button,GTK_CAN_DEFAULT);
	GTK_WIDGET_SET_FLAGS(edit_button,GTK_CAN_DEFAULT);
	GTK_WIDGET_SET_FLAGS(add_button,GTK_CAN_DEFAULT);
	GTK_WIDGET_SET_FLAGS(del_button,GTK_CAN_DEFAULT);
	GtkWidget *vbox=gtk_vbox_new(FALSE,0);
	GtkWidget *hbox=gtk_hbutton_box_new();
	gtk_box_set_spacing(GTK_BOX(vbox),5);
	gtk_box_set_spacing(GTK_BOX(hbox),3);
	gtk_box_pack_start(GTK_BOX(vbox),scroll_window,TRUE,TRUE,0);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);
	gtk_box_pack_end(GTK_BOX(hbox),add_button,FALSE,FALSE,0);
	gtk_box_pack_end(GTK_BOX(hbox),del_button,FALSE,FALSE,0);
	gtk_box_pack_end(GTK_BOX(hbox),edit_button,FALSE,FALSE,0);
	gtk_box_pack_end(GTK_BOX(hbox),button,FALSE,FALSE,0);
	gtk_container_add(GTK_CONTAINER(d4x_scheduler_window),vbox);
	gtk_window_set_default(GTK_WINDOW(d4x_scheduler_window),button);
	MainScheduler->redraw();
	gtk_widget_show_all(d4x_scheduler_window);

	gtk_signal_connect(GTK_OBJECT(d4x_scheduler_window),"delete_event",GTK_SIGNAL_FUNC(d4x_scheduler_close), NULL);
	gtk_signal_connect(GTK_OBJECT(add_button),"clicked",GTK_SIGNAL_FUNC(d4x_scheduler_init_editor),NULL);
	gtk_signal_connect(GTK_OBJECT(del_button),"clicked",GTK_SIGNAL_FUNC(d4x_scheduler_remove_selected),NULL);
	gtk_signal_connect(GTK_OBJECT(edit_button),"clicked",GTK_SIGNAL_FUNC(d4x_scheduler_edit),NULL);
	gtk_signal_connect(GTK_OBJECT(button),"clicked",GTK_SIGNAL_FUNC(d4x_scheduler_close),NULL);
	d4x_eschandler_init(d4x_scheduler_window,NULL);
/*
	gtk_signal_connect(GTK_OBJECT(d4x_scheduler_clist),"select_row",GTK_SIGNAL_FUNC(d4x_scheduler_select),NULL);
	gtk_signal_connect(GTK_OBJECT(d4x_scheduler_clist),"event",GTK_SIGNAL_FUNC(face_limits_list_event_callback),this);
	gtk_signal_connect(GTK_OBJECT(del_button),"clicked",GTK_SIGNAL_FUNC(face_limits_del),this);
*/
};

/*********************************************************************/

static GtkWidgetClass *parent_class = (GtkWidgetClass *)NULL;

static void my_gtk_aeditor_edit_ok(GtkWidget *widget,MyGtkAEditor *editor){
	tDownload *what=editor->dwn;
	what->editor->apply_changes();
	what->delete_editor();
	char *url=what->info->url();
	text_to_combo(editor->url_entry,url);
	delete[] url;
};

static void my_gtk_aeditor_edit_download(GtkWidget *widget,MyGtkAEditor *editor){
	tDownload *what;
	int flag=0;
	if (editor->dwn==NULL){
		editor->dwn=new tDownload;
		editor->dwn->config=new tCfg;
		editor->dwn->set_default_cfg();
		editor->dwn->config->save_path.set(CFG.GLOBAL_SAVE_PATH);
		char *url_entry_cont=text_from_combo(editor->url_entry);
		editor->dwn->info=new tAddr(url_entry_cont);
		if (url_entry_cont==NULL || *url_entry_cont==0)
			flag=1;
	};
	what=editor->dwn;
	init_edit_window_without_ok(what);
	what->editor->disable_time();
	if (flag)
		what->editor->clear_url();
	gtk_window_set_modal (GTK_WINDOW(what->editor->window),TRUE);
	gtk_window_set_transient_for (GTK_WINDOW (what->editor->window), GTK_WINDOW (editor));
	gtk_signal_connect(GTK_OBJECT(what->editor->ok_button),"clicked",
			   GTK_SIGNAL_FUNC(my_gtk_aeditor_edit_ok),
			   editor);
};

static void my_gtk_aeditor_browse_ok(GtkWidget *widget,MyGtkAEditor *editor){
	text_to_combo(editor->path_entry,
		      gtk_file_selection_get_filename(GTK_FILE_SELECTION(editor->browser)));
	gtk_widget_destroy(editor->browser);
};

static void my_gtk_aeditor_browse_cancel(GtkWidget *widget,MyGtkAEditor *editor){
	gtk_widget_destroy(editor->browser);	
};

static gint my_gtk_aeditor_browse_delete(GtkWidget *window,GdkEvent *event, MyGtkAEditor *editor) {
	gtk_widget_destroy(editor->browser);	
	return TRUE;
};

static void my_gtk_aeditor_browse(GtkWidget *widget,MyGtkAEditor *editor){
	GtkWidget *browser=gtk_file_selection_new(_("Select file"));
	char *tmp=text_from_combo(editor->path_entry);
	if (tmp && *tmp)
		gtk_file_selection_set_filename(GTK_FILE_SELECTION(browser),tmp);

	gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(browser)->ok_button),
			   "clicked",GTK_SIGNAL_FUNC(my_gtk_aeditor_browse_ok),editor);
	gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(browser)->cancel_button),
			   "clicked",GTK_SIGNAL_FUNC(my_gtk_aeditor_browse_cancel),editor);
	gtk_signal_connect(GTK_OBJECT(&(GTK_FILE_SELECTION(browser)->window)),
			   "delete_event",GTK_SIGNAL_FUNC(my_gtk_aeditor_browse_delete),editor);
	d4x_eschandler_init(GTK_WIDGET(&(GTK_FILE_SELECTION(browser)->window)),editor);
	editor->browser=browser;
	gtk_widget_show(browser);
	gtk_window_set_modal (GTK_WINDOW(browser),TRUE);
	gtk_window_set_transient_for (GTK_WINDOW (browser), GTK_WINDOW(editor));
};

static int _is_sa_url_(int type){
	if (type==SACT_DELETE_DOWNLOAD || type==SACT_PAUSE_DOWNLOAD ||
	    type==SACT_RUN_DOWNLOAD || type==SACT_DEL_IF_COMPLETED)
		return(1);
	return(0);
};

static void aeditor_select_mode_int(MyGtkAEditor *editor,int i){
	if (editor->last_action!=i){
		switch(editor->last_action){
		case SACT_RUN_DOWNLOAD:
		case SACT_PAUSE_DOWNLOAD:
		case SACT_DEL_IF_COMPLETED:
		case SACT_DELETE_DOWNLOAD:{
			if (_is_sa_url_(i)){
				editor->last_action=i;
				return;
			};
		};
		default:
			/* these actions have no properties */
			if (editor->frame_child){
				gtk_widget_destroy(editor->frame_child);
			};
			break;
		};
		switch(i){
		case SACT_SET_SPEED:{
			GtkWidget *hbox=gtk_hbox_new(FALSE,0);
			gtk_box_set_spacing(GTK_BOX(hbox),5);
			editor->frame_child=hbox;
			
			editor->sb_low=gtk_radio_button_new_with_label((GSList *)NULL,
								       _(SPEED_LIMITATIONS_NAMES[1]));
			gtk_box_pack_start(GTK_BOX(hbox),editor->sb_low,
					   FALSE,FALSE,0);
			GSList *group1=gtk_radio_button_group(GTK_RADIO_BUTTON(editor->sb_low));
			editor->sb_middle=gtk_radio_button_new_with_label(group1,
									  _(SPEED_LIMITATIONS_NAMES[2]));
			gtk_box_pack_start(GTK_BOX(hbox),editor->sb_middle,
					   FALSE,FALSE,0);
			editor->sb_high=gtk_radio_button_new_with_label(gtk_radio_button_group(
				GTK_RADIO_BUTTON(editor->sb_middle)),_(SPEED_LIMITATIONS_NAMES[3]));
			gtk_box_pack_start(GTK_BOX(hbox),
					   editor->sb_high,FALSE,FALSE,0);
			gtk_container_add(GTK_CONTAINER(editor->frame),hbox);
			if (editor->action){
				d4xSASpeed *act=(d4xSASpeed *)(editor->action);
				if (act->speed==1)
					gtk_signal_emit_by_name(GTK_OBJECT(editor->sb_low), "clicked");
				else
					gtk_signal_emit_by_name(act->speed==2?GTK_OBJECT(editor->sb_middle):GTK_OBJECT(editor->sb_high),
								"clicked");
			};
			break;
		};
		case SACT_RUN_DOWNLOAD:
		case SACT_PAUSE_DOWNLOAD:
		case SACT_DEL_IF_COMPLETED:
		case SACT_DELETE_DOWNLOAD:{
			editor->url_entry=my_gtk_combo_new(ALL_HISTORIES[URL_HISTORY]);//gtk_entry_new();
			GtkWidget *hbox=gtk_hbox_new(FALSE,0);
			gtk_box_set_spacing(GTK_BOX(hbox),5);
			editor->frame_child=hbox;
			gtk_box_pack_start(GTK_BOX(hbox),
					   gtk_label_new("URL:"),
					   FALSE,FALSE,0);
			gtk_box_pack_start(GTK_BOX(hbox),editor->url_entry,
					   TRUE,TRUE,0);
			gtk_container_add(GTK_CONTAINER(editor->frame),hbox);
			if (editor->action && _is_sa_url_(editor->action->type())){
				d4xSAUrl *act=(d4xSAUrl *)(editor->action);
				if (act->url){
					char *url=act->url->url();
					text_to_combo(editor->url_entry,url);
					delete[] url;
				};
			};
			break;
		};
		case SACT_ADD_DOWNLOAD:{
			editor->url_entry=my_gtk_combo_new(ALL_HISTORIES[URL_HISTORY]);//gtk_entry_new();
			GtkWidget *hbox=gtk_hbox_new(FALSE,0);
			gtk_box_set_spacing(GTK_BOX(hbox),5);
			editor->frame_child=hbox;
			gtk_box_pack_start(GTK_BOX(hbox),
					   gtk_label_new("URL:"),
					   FALSE,FALSE,0);
			gtk_box_pack_start(GTK_BOX(hbox),editor->url_entry,
					   TRUE,TRUE,0);
			if (editor->dwn==NULL && editor->action && editor->action->type()==SACT_ADD_DOWNLOAD){
				d4xSAAddDownload *a=(d4xSAAddDownload *)editor->action;
				editor->dwn=new tDownload;
				editor->dwn->copy(a->dwn);
			};
			if (editor->dwn){
				if (editor->dwn && editor->dwn->info){
					char *url=editor->dwn->info->url();
					text_to_combo(editor->url_entry,url);
					delete[] url;
				}else
				text_to_combo(editor->url_entry,"");
			}else
				text_to_combo(editor->url_entry,"");
			GtkWidget *button=gtk_button_new_with_label(_("Edit"));
			gtk_signal_connect(GTK_OBJECT(button),"clicked",
					   GTK_SIGNAL_FUNC(my_gtk_aeditor_edit_download),
					   editor);
			gtk_box_pack_start(GTK_BOX(hbox),button,
					   FALSE,FALSE,0);
			gtk_container_add(GTK_CONTAINER(editor->frame),hbox);
			break;
		};
		case SACT_EXECUTE:{
			editor->path_entry=gtk_entry_new();
			GtkWidget *hbox=gtk_hbox_new(FALSE,0);
			gtk_box_set_spacing(GTK_BOX(hbox),5);
			editor->frame_child=hbox;
			gtk_box_pack_start(GTK_BOX(hbox),
					   gtk_label_new(_("Command:")),
					   FALSE,FALSE,0);
			gtk_box_pack_start(GTK_BOX(hbox),editor->path_entry,
					   TRUE,TRUE,0);
			GtkWidget *button=gtk_button_new_with_label(_("Browse"));
			gtk_signal_connect(GTK_OBJECT(button),"clicked",
					   GTK_SIGNAL_FUNC(my_gtk_aeditor_browse),editor);
			gtk_box_pack_start(GTK_BOX(hbox),button,
					   FALSE,FALSE,0);
			gtk_container_add(GTK_CONTAINER(editor->frame),hbox);
			if (editor->action && editor->action->type()==SACT_EXECUTE){
				d4xSAExecute *a=(d4xSAExecute *)editor->action;
				text_to_combo(editor->path_entry,a->command.get());
			}else
				text_to_combo(editor->path_entry,"");
			break;
		};
		case SACT_SAVE_LIST:{
			editor->path_entry=my_gtk_combo_new(ALL_HISTORIES[LOAD_SAVE_HISTORY]);//gtk_entry_new();
			GtkWidget *hbox=gtk_hbox_new(FALSE,0);
			gtk_box_set_spacing(GTK_BOX(hbox),5);
			editor->frame_child=hbox;
			gtk_box_pack_start(GTK_BOX(hbox),
					   gtk_label_new(_("Path:")),
					   FALSE,FALSE,0);
			gtk_box_pack_start(GTK_BOX(hbox),editor->path_entry,
					   TRUE,TRUE,0);
			GtkWidget *button=gtk_button_new_with_label(_("Browse"));
			gtk_signal_connect(GTK_OBJECT(button),"clicked",
					   GTK_SIGNAL_FUNC(my_gtk_aeditor_browse),editor);
			gtk_box_pack_start(GTK_BOX(hbox),button,
					   FALSE,FALSE,0);
			gtk_container_add(GTK_CONTAINER(editor->frame),hbox);
//			gtk_container_add(GTK_CONTAINER(editor->frame),editor->path_entry);
			if (editor->action && editor->action->type()==SACT_SAVE_LIST){
				d4xSASaveList *a=(d4xSASaveList *)editor->action;
				text_to_combo(editor->path_entry,a->path.get());
			}else
				text_to_combo(editor->path_entry,"");
			break;
		};
		default:
			editor->frame_child=gtk_label_new(_("This action has no properties"));
			gtk_container_add(GTK_CONTAINER(editor->frame),
					  editor->frame_child);
			break;
		};
		editor->last_action=i;
		gtk_widget_show_all(editor->frame_child);
	};
};

static void aeditor_select_mode(GtkWidget *widget,MyGtkAEditor *editor){
	GSList *group=gtk_radio_menu_item_group((GtkRadioMenuItem *)((GtkOptionMenu *)editor->omenu)->menu_item);
	int i=0;
	while(group && !((GtkCheckMenuItem *)(group->data))->active){
		group = group->next;
		i++;
	};
	i=SACT_LAST-i-1;
	aeditor_select_mode_int(editor,i);
};

static GtkWidget *my_option_menu (char *labels[],
				  gint num_items,
				  gint active,
				  gpointer data){
	GtkWidget *omenu;
	GtkWidget *menu;
	GtkWidget *menu_item;
	GSList *group;
	gint i;
	
	omenu = gtk_option_menu_new();
	
	menu = gtk_menu_new ();
	group = (GSList *)NULL;
	
	for (i = 0; i < num_items; i++){
		menu_item = gtk_radio_menu_item_new_with_label (group, _(labels[i]));
		gtk_signal_connect (GTK_OBJECT (menu_item),
				    "activate",
				    (GtkSignalFunc)aeditor_select_mode,
				    data);
		group = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (menu_item));
		gtk_menu_append (GTK_MENU (menu), menu_item);
		if (i == active)
			gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (menu_item), TRUE);
		gtk_widget_show (menu_item);
	};
	
	gtk_option_menu_set_menu (GTK_OPTION_MENU (omenu), menu);
	gtk_option_menu_set_history (GTK_OPTION_MENU (omenu), active);

	return omenu;
};

static void my_gtk_aeditor_destroy(GtkObject *widget){
	g_return_if_fail(widget!=NULL);
	MyGtkAEditor *editor=MY_GTK_AEDITOR(widget);
	if (editor->dwn) delete(editor->dwn);
	if (editor->action) editor->action->lock=0;
	if (GTK_OBJECT_CLASS (parent_class)->destroy)
		(* GTK_OBJECT_CLASS (parent_class)->destroy) (widget);
};

static void my_gtk_aeditor_class_init(MyGtkAEditorClass *klass){
	GtkObjectClass *object_class=(GtkObjectClass *)klass;
	
	object_class->destroy=my_gtk_aeditor_destroy;
	parent_class=(GtkWidgetClass *)gtk_type_class(gtk_window_get_type());
};

static void my_gtk_aeditor_ok(GtkWidget *widget,MyGtkAEditor *editor){
	if (editor->action){
		MainScheduler->del_action(editor->action);
		delete(editor->action);
	};
	time_t start_time;
	time_t period=0;
	int retries=0;
	/* preparing time */
	time_t NOW=time((time_t*)NULL);
	struct tm date;
	date.tm_isdst=-1;
	localtime_r(&NOW,&date);
	sscanf(text_from_combo(editor->hour),"%i",&date.tm_hour);
	sscanf(text_from_combo(editor->min),"%i",&date.tm_min);
	sscanf(text_from_combo(editor->sec),"%i",&date.tm_sec);
	gtk_calendar_get_date(GTK_CALENDAR(editor->calendar),
			      (guint *)&date.tm_year,
			      (guint *)&date.tm_mon,
			      (guint *)&date.tm_mday);
	date.tm_year-=1900;
	start_time=mktime(&date);

	if (GTK_TOGGLE_BUTTON(editor->retry)->active){
		int days,hours,mins;
		sscanf(text_from_combo(editor->retry_times),"%i",&retries);
		sscanf(text_from_combo(editor->period_days),"%i",&days);
		sscanf(text_from_combo(editor->period_hours),"%i",&hours);
		sscanf(text_from_combo(editor->period_mins),"%i",&mins);
		period=days*24*3600+hours*3600+mins*60;
		if (period==0) period=60;
	}else{
		retries=0;
	};
	/*********************/
	d4xSchedAction *action=(d4xSchedAction *)NULL;
	switch(editor->last_action){
	case SACT_SET_SPEED:{
		d4xSASpeed *act=new d4xSASpeed;
		action=act;
		if (GTK_TOGGLE_BUTTON(editor->sb_low)->active)
			act->speed=1;
		else{
			act->speed=GTK_TOGGLE_BUTTON(editor->sb_middle)->active?2:3;
		};
		break;
	};
	case SACT_POPUP_WINDOW:{
		d4xSAPopup *act=new d4xSAPopup;
		action=act;
		break;
	};
	case SACT_EXIT:{
		d4xSAExit *act=new d4xSAExit;
		action=act;
		break;
	};
	case SACT_DEL_COMPLETED:{
		d4xSADelCompleted *act=new d4xSADelCompleted;
		action=act;
		break;
	};
	case SACT_DEL_FAILED:{
		d4xSADelFailed *act=new d4xSADelFailed;
		action=act;
		break;
	};
	case SACT_RUN_DOWNLOAD:{
		d4xSARunDownload *act=new d4xSARunDownload;
		act->url=new tAddr(text_from_combo(editor->url_entry));
		action=act;
		break;
	};
	case SACT_PAUSE_DOWNLOAD:{
		d4xSAStopDownload *act=new d4xSAStopDownload;
		act->url=new tAddr(text_from_combo(editor->url_entry));
		action=act;
		break;
	};
	case SACT_DELETE_DOWNLOAD:{
		d4xSADelDownload *act=new d4xSADelDownload;
		act->url=new tAddr(text_from_combo(editor->url_entry));
		action=act;
		break;
	};
	case SACT_SAVE_LIST:{
		d4xSASaveList *act=new d4xSASaveList;
		act->path.set(text_from_combo(editor->path_entry));
		action=act;
		break;
	};
	case SACT_EXECUTE:{
		d4xSAExecute *act=new d4xSAExecute;
		act->command.set(text_from_combo(editor->path_entry));
		action=act;
		break;
	};
	case SACT_ADD_DOWNLOAD:{
		d4xSAAddDownload *act=new d4xSAAddDownload;
		if (act->dwn)
			delete(act->dwn);
		if (editor->dwn==NULL){
			editor->dwn=new tDownload;
			editor->dwn->config=new tCfg;
			editor->dwn->set_default_cfg();
			editor->dwn->config->save_path.set(CFG.GLOBAL_SAVE_PATH);
		};
		if (editor->dwn->info)
			delete(editor->dwn->info);
		editor->dwn->info=new tAddr(text_from_combo(editor->url_entry));
		act->dwn=editor->dwn;
		editor->dwn=(tDownload*)NULL;
		action=act;
		break;
	};
	case SACT_DEL_IF_COMPLETED:{
		d4xSADelIfCompleted *act=new d4xSADelIfCompleted;
		act->url=new tAddr(text_from_combo(editor->url_entry));
		action=act;
		break;
	};
	};
	if (action){
		action->start_time=start_time;
		action->period=period;
		action->retries=retries;
		MainScheduler->add_action(action);
	};
	gtk_widget_destroy(GTK_WIDGET(editor));
};

static void my_gtk_aeditor_cancel(GtkWidget *widget,GtkWidget *editor){
	gtk_widget_destroy(editor);
};

static void my_gtk_aeditor_close(GtkWidget *widget,GdkEvent *event,GtkWidget *editor){
	gtk_widget_destroy(editor);
};

static void my_gtk_aeditor_retry_cb(GtkWidget *widget,MyGtkAEditor *editor){
	gtk_widget_set_sensitive(editor->retry_times,GTK_TOGGLE_BUTTON(widget)->active);
	gtk_widget_set_sensitive(editor->period_days,GTK_TOGGLE_BUTTON(widget)->active);
	gtk_widget_set_sensitive(editor->period_hours,GTK_TOGGLE_BUTTON(widget)->active);
	gtk_widget_set_sensitive(editor->period_mins,GTK_TOGGLE_BUTTON(widget)->active);
//	gtk_widget_set_sensitive(editor->,GTK_TOGGLE_BUTTON(widget)->active);
};

static void my_gtk_aeditor_init(MyGtkAEditor *editor){
	GtkWidget *vbox=gtk_vbox_new(FALSE,0);
	editor->last_action=-1; //no last actions
	editor->dwn=(tDownload *)NULL;
	editor->action=(d4xSchedAction *)NULL;
	editor->frame_child=(GtkWidget *)NULL;
	editor->hbox=gtk_hbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(editor->hbox),5);
	gtk_box_set_spacing(GTK_BOX(vbox),5);
	editor->omenu = my_option_menu (action_names,
					sizeof(action_names)/sizeof(char*),
					0, editor);

	GtkWidget *hbox=gtk_hbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(hbox),1);

	GtkWidget *label=gtk_label_new(_("Start time(hour:min:sec)"));
	editor->hour=my_gtk_entry_new_with_max_length(2,0);
	editor->min=my_gtk_entry_new_with_max_length(2,0);
	editor->sec=my_gtk_entry_new_with_max_length(2,0);
	gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(hbox),editor->hour,FALSE,FALSE,0);
	label=gtk_label_new(":");
	gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(hbox),editor->min,FALSE,FALSE,0);
	label=gtk_label_new(":");
	gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(hbox),editor->sec,FALSE,FALSE,0);


	GtkWidget *tmpvbox=gtk_vbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(tmpvbox),5);

	editor->calendar=gtk_calendar_new();
	gtk_calendar_display_options(GTK_CALENDAR(editor->calendar),
				     GtkCalendarDisplayOptions(
				     GTK_CALENDAR_WEEK_START_MONDAY |
				     GTK_CALENDAR_SHOW_DAY_NAMES|
				     GTK_CALENDAR_SHOW_HEADING));
	gtk_box_pack_start(GTK_BOX(tmpvbox),hbox,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(tmpvbox),editor->calendar,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(editor->hbox),tmpvbox,FALSE,FALSE,0);

	tmpvbox=gtk_vbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(tmpvbox),5);
	editor->retry=gtk_check_button_new_with_label(_("Retry this action"));
	gtk_signal_connect(GTK_OBJECT(editor->retry),"clicked",
			   GTK_SIGNAL_FUNC(my_gtk_aeditor_retry_cb),editor);
	gtk_box_pack_start (GTK_BOX (tmpvbox),editor->retry,FALSE, FALSE, 0);

	hbox=gtk_hbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(hbox),5);
	editor->retry_times=my_gtk_entry_new_with_max_length(2,1);
	gtk_box_pack_start (GTK_BOX (hbox),editor->retry_times,FALSE, FALSE, 0);
	label=gtk_label_new(_("times to repeat (-1 unlimited)"));
	gtk_box_pack_start (GTK_BOX (hbox),label,FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (tmpvbox),hbox,FALSE, FALSE, 0);

	label=gtk_label_new(_("period (day/hour:min)"));
	hbox=gtk_hbox_new(FALSE,0);
	gtk_box_set_spacing(GTK_BOX(hbox),1);
	editor->period_hours=my_gtk_entry_new_with_max_length(2,0);
	editor->period_mins=my_gtk_entry_new_with_max_length(2,0);
	editor->period_days=my_gtk_entry_new_with_max_length(2,0);
	gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(hbox),editor->period_days,FALSE,FALSE,0);
	label=gtk_label_new("/");
	gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(hbox),editor->period_hours,FALSE,FALSE,0);
	label=gtk_label_new(":");
	gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(hbox),editor->period_mins,FALSE,FALSE,0);
	gtk_box_pack_start (GTK_BOX (tmpvbox),hbox,FALSE, FALSE, 0);


	GtkWidget *frame=gtk_frame_new(_("Action"));
	gtk_container_border_width(GTK_CONTAINER(frame),5);
	gtk_container_add(GTK_CONTAINER(frame),editor->omenu);
	gtk_box_pack_end(GTK_BOX (tmpvbox),frame,FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(editor->hbox),tmpvbox,FALSE,TRUE,0);

	gtk_box_pack_start(GTK_BOX(vbox),editor->hbox,FALSE,TRUE,0);
	
	editor->frame=gtk_frame_new(_("Properties"));
	gtk_container_border_width(GTK_CONTAINER(editor->frame),5);
	gtk_box_pack_start(GTK_BOX(vbox),editor->frame,FALSE,TRUE,0);
	
	hbox=gtk_hbutton_box_new();
	gtk_box_set_spacing(GTK_BOX(hbox),5);
	GtkWidget *ok_button=gtk_button_new_with_label(_("Ok"));
	GtkWidget *cancel_button=gtk_button_new_with_label(_("Cancel"));
	gtk_box_pack_end(GTK_BOX(hbox),ok_button,FALSE,FALSE,0);
	gtk_box_pack_end(GTK_BOX(hbox),cancel_button,FALSE,FALSE,0);
	gtk_signal_connect(GTK_OBJECT(ok_button),"clicked",
			   GTK_SIGNAL_FUNC(my_gtk_aeditor_ok),editor);
	gtk_signal_connect(GTK_OBJECT(cancel_button),"clicked",
			   GTK_SIGNAL_FUNC(my_gtk_aeditor_cancel),editor);
	gtk_signal_connect(GTK_OBJECT(editor),"delete_event",
			   GTK_SIGNAL_FUNC(my_gtk_aeditor_close), editor);
	d4x_eschandler_init(GTK_WIDGET(editor),editor);
	gtk_box_pack_start (GTK_BOX (vbox),hbox,
			    FALSE, TRUE, 0);

	gtk_window_set_title(GTK_WINDOW(editor),_("Edit scheduled action"));
	gtk_window_set_position(GTK_WINDOW(editor),GTK_WIN_POS_CENTER);
	gtk_container_border_width(GTK_CONTAINER(editor),5);
	gtk_container_add(GTK_CONTAINER(editor),vbox);

};

guint my_gtk_aeditor_get_type(){
	static guint my_aeditor_type=0;
	if (!my_aeditor_type){
		GtkTypeInfo my_aeditor_info={
			"MyGtkAEditor",
			sizeof(MyGtkAEditor),
			sizeof(MyGtkAEditorClass),
			(GtkClassInitFunc)my_gtk_aeditor_class_init,
			(GtkObjectInitFunc)my_gtk_aeditor_init,
			NULL,NULL
//			(GtkArgSetFunc) NULL,
//			(GtkArgGetFunc) NULL
		};
		my_aeditor_type = gtk_type_unique (gtk_window_get_type (), &my_aeditor_info);
	};
	return my_aeditor_type;
};


GtkWidget *my_gtk_aeditor_new(d4xSchedAction *action=(d4xSchedAction *)NULL){
	MyGtkAEditor *editor=(MyGtkAEditor *)gtk_type_new(my_gtk_aeditor_get_type());
	char data[MAX_LEN];
	if (action){
		struct tm date;
		localtime_r(&(action->start_time),&date);
		sprintf(data,"%i",date.tm_hour);
		text_to_combo(editor->hour,data);
		sprintf(data,"%i",date.tm_min);
		text_to_combo(editor->min,data);
		sprintf(data,"%i",date.tm_sec);
		text_to_combo(editor->sec,data);
		gtk_calendar_select_month(GTK_CALENDAR(editor->calendar),date.tm_mon,date.tm_year+1900);
		gtk_calendar_select_day(GTK_CALENDAR(editor->calendar),date.tm_mday);

		if (action->retries){
			sprintf(data,"%i",action->retries);
			text_to_combo(editor->retry_times,data);
			int mins,days,hours;
			GTK_TOGGLE_BUTTON(editor->retry)->active=TRUE;
			mins=(action->period%3600)/60;
			days=action->period/(3600*24);
			hours=(action->period%(3600*24))/3600;
			sprintf(data,"%i",mins);
			text_to_combo(editor->period_mins,data);
			sprintf(data,"%i",hours);
			text_to_combo(editor->period_hours,data);
			sprintf(data,"%i",days);
			text_to_combo(editor->period_days,data);
		};
		
		editor->action=action;
		gtk_option_menu_set_history (GTK_OPTION_MENU (editor->omenu),action->type());
		aeditor_select_mode_int(editor,action->type());
	}else{
		time_t now=time((time_t*)NULL);
		struct tm date;
		localtime_r(&now,&date);
		sprintf(data,"%i",date.tm_hour);
		text_to_combo(editor->hour,data);
		sprintf(data,"%i",date.tm_min);
		text_to_combo(editor->min,data);
		sprintf(data,"%i",date.tm_sec);
		text_to_combo(editor->sec,data);
		gtk_calendar_select_month(GTK_CALENDAR(editor->calendar),date.tm_mon,date.tm_year+1900);
		gtk_calendar_select_day(GTK_CALENDAR(editor->calendar),date.tm_mday);
		aeditor_select_mode((GtkWidget *)NULL,editor);
		if (editor->action && editor->action->type()==SACT_SET_SPEED){
			d4xSASpeed *act=(d4xSASpeed *)(editor->action);
			if (act->speed==1)
				gtk_signal_emit_by_name(GTK_OBJECT(editor->sb_low), "clicked");
			else
				gtk_signal_emit_by_name(act->speed==2?GTK_OBJECT(editor->sb_middle):GTK_OBJECT(editor->sb_high),
							"clicked");
		};
	};
	if (!GTK_TOGGLE_BUTTON(editor->retry)->active){
		gtk_widget_set_sensitive(editor->retry_times,FALSE);	
		gtk_widget_set_sensitive(editor->period_days,FALSE);
		gtk_widget_set_sensitive(editor->period_hours,FALSE);
		gtk_widget_set_sensitive(editor->period_mins,FALSE);
	};
	return GTK_WIDGET(editor);
};
