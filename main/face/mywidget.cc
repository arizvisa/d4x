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
#include "mywidget.h"
#include "edit.h"
#include "misc.h"
#include "../ntlocale.h"
#include "../filter.h"

static GtkWidgetClass *parent_class = (GtkWidgetClass *)NULL;
static GtkWidgetClass *color_parent_class = (GtkWidgetClass *)NULL;
static GtkWidgetClass *rule_parent_class = (GtkWidgetClass *)NULL;
static GtkWidgetClass *filter_parent_class = (GtkWidgetClass *)NULL;

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
	my_gtk_filesel_cancel((GtkButton *)NULL,filesel);
	return TRUE;
};

static gint my_gtk_filesel_init_browser(GtkButton *button, MyGtkFilesel *filesel){
	g_return_val_if_fail(filesel!=NULL,FALSE);
	if (filesel->browser){
		gdk_window_show(filesel->browser->window);
	}else{
		if (filesel->only_dirs){
			filesel->browser=gtk_file_selection_new((gchar*)_("Select directory"));
			gtk_widget_set_sensitive(GTK_FILE_SELECTION(filesel->browser)->file_list,FALSE);
		}else
			filesel->browser=gtk_file_selection_new((gchar*)_("Select file"));
		gtk_window_set_wmclass(GTK_WINDOW(filesel->browser),
				       "D4X_FileSel","D4X");
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
	filesel->modal=(GtkWindow *)NULL;
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
			(GtkClassInitFunc)my_gtk_filesel_class_init,
			(GtkObjectInitFunc)my_gtk_filesel_init,
			NULL,NULL
//			(GtkArgSetFunc) NULL,
//			(GtkArgGetFunc) NULL
		};
		my_filesel_type = gtk_type_unique (gtk_hbox_get_type (), &my_filesel_info);
	};
	return my_filesel_type;
};

GtkWidget *my_gtk_filesel_new(tHistory *history){
	MyGtkFilesel *filesel=(MyGtkFilesel *)gtk_type_new(my_gtk_filesel_get_type());
	GList *list=make_glist_from_mylist(history);
	if (list){
		gtk_combo_set_popdown_strings (GTK_COMBO (filesel->combo), list);
		g_list_free(list);
	};
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
		gtk_widget_draw (colsel->preview, (GdkRectangle *)NULL); 
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
	my_gtk_colorsel_cancel((GtkButton *)NULL,colsel);
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
		gtk_widget_destroy(GTK_COLOR_SELECTION_DIALOG(colsel->browser)->help_button);
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
			NULL,NULL
//			(GtkArgSetFunc) NULL,
//			(GtkArgGetFunc) NULL
		};
		my_filesel_type = gtk_type_unique (gtk_hbox_get_type (), &my_filesel_info);
	};
	return my_filesel_type;
};

gint my_gtk_colorsel_get_color(MyGtkColorsel *colsel){
	gint color=0;
/*
	guchar *a=(guchar *)&color;
	g_return_val_if_fail(colsel!=NULL,0);

	a[0]=guchar(colsel->color[2]*0xff);
	a[1]=guchar(colsel->color[1]*0xff);
	a[2]=guchar(colsel->color[0]*0xff);
*/
	color=((gint(colsel->color[2]*0xff))&0xff)+
	      (((gint(colsel->color[1]*0xff))&0xff)<<8)+
	      (((gint(colsel->color[0]*0xff))&0xff)<<16);
	return color;
};

void my_gtk_colorsel_set_color(MyGtkColorsel *colsel, gint color){
	g_return_if_fail(colsel!=NULL);
	colsel->color[2]=double(color&0xff)/double(0xff);
	colsel->color[1]=double((color>>8)&0xff)/double(0xff);
	colsel->color[0]=double((color>>16)&0xff)/double(0xff);
	my_gtk_colorsel_update(colsel);
	if (GTK_WIDGET_VISIBLE(colsel))
		gtk_widget_draw (colsel->preview, (GdkRectangle *)NULL); 
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
/***********************************************************************/

static void d4x_rule_edit_destroy(GtkObject *widget){
	g_return_if_fail(widget!=NULL);

	if (GTK_OBJECT_CLASS (rule_parent_class)->destroy)
		(* GTK_OBJECT_CLASS (rule_parent_class)->destroy) (widget);
};

static void d4x_rule_edit_class_init(d4xRuleEditClass *klass){
	GtkObjectClass *object_class=(GtkObjectClass *)klass;
	
	object_class->destroy=d4x_rule_edit_destroy;
	rule_parent_class=(GtkWidgetClass *)gtk_type_class(gtk_window_get_type());
};

void d4x_rule_edit_apply(d4xRuleEdit *edit){
	if (edit && edit->rule){
		if (*(text_from_combo(edit->host))){
			edit->rule->host.set(text_from_combo(edit->host));
		}else
			edit->rule->host.set((char*)NULL);
		if (*(text_from_combo(edit->path))){
			edit->rule->path.set(text_from_combo(edit->path));
		}else
			edit->rule->path.set((char*)NULL);
		if (*(text_from_combo(edit->file))){
			edit->rule->file.set(text_from_combo(edit->file));
		}else
			edit->rule->file.set((char*)NULL);
		edit->rule->proto=get_proto_by_name(text_from_combo(edit->proto));
		edit->rule->include=GTK_TOGGLE_BUTTON(edit->include)->active;
	};
};

static void d4x_rule_edit_ok(GtkWidget *widget,d4xRuleEdit *edit) {
	g_return_if_fail(edit!=NULL);
	d4x_rule_edit_apply(edit);
	gtk_widget_destroy(GTK_WIDGET(edit));
};

static void d4x_rule_edit_cancel(GtkWidget *widget,d4xRuleEdit *edit) {
	gtk_widget_destroy(GTK_WIDGET(edit));
};

static void d4x_rule_edit_init(d4xRuleEdit *edit){
	gtk_window_set_title(GTK_WINDOW (edit),_("Edit properties of Rule"));
	gtk_window_set_policy (GTK_WINDOW(edit), FALSE,FALSE,FALSE);

	GtkWidget *label=gtk_label_new(_("protocol"));
	GtkWidget *hbox=gtk_hbox_new(FALSE,0);
	edit->vbox=gtk_vbox_new(FALSE,0);
	
	edit->proto=gtk_combo_new();
	GList *list=(GList*)NULL;
	list = g_list_append (list, get_name_by_proto(D_PROTO_FTP));
	list = g_list_append (list, get_name_by_proto(D_PROTO_HTTP));
	list = g_list_append (list, (void*)"");
	gtk_combo_set_popdown_strings (GTK_COMBO (edit->proto), list);
	g_list_free(list);
	gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(edit->proto)->entry),FALSE);
	text_to_combo(edit->proto,"");
	gtk_box_pack_start(GTK_BOX(hbox),edit->proto,TRUE,TRUE,0);
	gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(edit->vbox),hbox,FALSE,FALSE,0);

	hbox=gtk_hbox_new(FALSE,0);
	label=gtk_label_new(_("host"));
	edit->host=gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox),edit->host,TRUE,TRUE,0);
	gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(edit->vbox),hbox,FALSE,FALSE,0);

	hbox=gtk_hbox_new(FALSE,0);
	label=gtk_label_new(_("path"));
	edit->path=gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox),edit->path,TRUE,TRUE,0);
	gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(edit->vbox),hbox,FALSE,FALSE,0);

	hbox=gtk_hbox_new(FALSE,0);
	label=gtk_label_new(_("file"));
	edit->file=gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox),edit->file,TRUE,TRUE,0);
	gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(edit->vbox),hbox,FALSE,FALSE,0);

	hbox=gtk_hbox_new(FALSE,0);
	edit->include=gtk_radio_button_new_with_label((GSList *)NULL,
						      _("include"));
	edit->exclude=gtk_radio_button_new_with_label(gtk_radio_button_group(GTK_RADIO_BUTTON(edit->include)),
						      _("exclude"));
	gtk_box_pack_start(GTK_BOX(hbox),edit->include,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(hbox),edit->exclude,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(edit->vbox),hbox,FALSE,FALSE,0);
	
	edit->ok_button=gtk_button_new_with_label(_("Ok"));
	edit->cancel_button=gtk_button_new_with_label(_("Cancel"));
	GTK_WIDGET_SET_FLAGS(edit->ok_button,GTK_CAN_DEFAULT);
	GTK_WIDGET_SET_FLAGS(edit->cancel_button,GTK_CAN_DEFAULT);
	hbox=gtk_hbutton_box_new();
	gtk_box_set_spacing(GTK_BOX(hbox),5);
	gtk_box_pack_start(GTK_BOX(hbox),edit->ok_button,TRUE,TRUE,0);
	gtk_box_pack_start(GTK_BOX(hbox),edit->cancel_button,TRUE,TRUE,0);
	gtk_box_pack_start(GTK_BOX(edit->vbox),hbox,TRUE,FALSE,0);

	gtk_container_add(GTK_CONTAINER(edit),edit->vbox);
	gtk_window_set_default(GTK_WINDOW(edit),edit->cancel_button);
};


guint d4x_rule_edit_get_type(){
	static guint d4x_rule_edit_type=0;
	if (!d4x_rule_edit_type){
		GtkTypeInfo info={
			"d4xRuleEdit",
			sizeof(d4xRuleEdit),
			sizeof(d4xRuleEditClass),
			(GtkClassInitFunc) d4x_rule_edit_class_init,
			(GtkObjectInitFunc) d4x_rule_edit_init,
			NULL,NULL
//			(GtkArgSetFunc) NULL,
//			(GtkArgGetFunc) NULL
		};
		d4x_rule_edit_type = gtk_type_unique (gtk_window_get_type (),
							&info);
	};
	return d4x_rule_edit_type;
};

GtkWidget *d4x_rule_edit_new(d4xRule *rule){
	d4xRuleEdit *edit=(d4xRuleEdit *)gtk_type_new(d4x_rule_edit_get_type());
	edit->rule=rule;
	if (rule){
		if (rule->host.get())
			text_to_combo(edit->host,rule->host.get());
		if (rule->proto)
			text_to_combo(edit->proto,get_name_by_proto(rule->proto));
		if (rule->path.get())
			text_to_combo(edit->path,rule->path.get());
		if (rule->file.get())
			text_to_combo(edit->file,rule->file.get());
		GTK_TOGGLE_BUTTON(edit->include)->active=rule->include;
		GTK_TOGGLE_BUTTON(edit->exclude)->active=!rule->include;
	};
	return GTK_WIDGET(edit);
};

void d4x_rule_edit_delete(GtkWidget *widget, GdkEvent *event,GtkWidget *edit){
	gtk_widget_destroy(edit);
};

GtkWidget *d4x_rule_edit_new_full(d4xRule *rule){
	d4xRuleEdit *edit=(d4xRuleEdit *)d4x_rule_edit_new(rule);
	gtk_signal_connect(GTK_OBJECT(edit->ok_button),"clicked",
			   GTK_SIGNAL_FUNC(d4x_rule_edit_ok),edit);
	gtk_signal_connect(GTK_OBJECT(edit->cancel_button),"clicked",
			   GTK_SIGNAL_FUNC(d4x_rule_edit_cancel),edit);
	gtk_signal_connect(GTK_OBJECT(edit),"delete_event",
			   GTK_SIGNAL_FUNC(d4x_rule_edit_delete), edit);

	return GTK_WIDGET(edit);
};

/************************************************************/
static void d4x_filter_edit_destroy(GtkObject *widget){
	g_return_if_fail(widget!=NULL);

	if (GTK_OBJECT_CLASS (filter_parent_class)->destroy)
		(* GTK_OBJECT_CLASS (filter_parent_class)->destroy) (widget);
};


void d4x_filter_edit_add_rule(d4xFilterEdit *edit,d4xRule *rule){
	char *data[5];
	data[0]=rule->include?(char*)"+":(char*)("-");
	data[1]=rule->proto?get_name_by_proto(rule->proto):(char*)("");
	data[2]=rule->host.get()?rule->host.get():(char*)("");
	data[3]=rule->path.get()?rule->path.get():(char*)("");
	data[4]=rule->file.get()?rule->file.get():(char*)("");
	gint row=gtk_clist_append(GTK_CLIST(edit->clist),data);
	gtk_clist_set_row_data(GTK_CLIST(edit->clist),row,rule);
};

static void d4x_filter_edit_add_ok(GtkWidget *widget,d4xRuleEdit *edit){
	((d4xFilterEdit *)(edit->filter_edit))->node->filter->insert(edit->rule);
	d4x_rule_edit_apply(edit);
	d4x_filter_edit_add_rule((d4xFilterEdit *)(edit->filter_edit),
				 edit->rule);
	gtk_widget_destroy(GTK_WIDGET(edit));
};

static void d4x_filter_edit_add_cancel(GtkWidget *widget,d4xRuleEdit *edit){
	delete(edit->rule);
	gtk_widget_destroy(GTK_WIDGET(edit));
};

static void d4x_filter_edit_add_delete(GtkWidget *widget,
				  GdkEvent *event,
				  d4xRuleEdit *edit){
	delete(edit->rule);
	gtk_widget_destroy(GTK_WIDGET(edit));
};

void d4x_filter_edit_add(GtkWidget *widget,d4xFilterEdit *edit){
	d4xRule *rule=new d4xRule;
	d4xRuleEdit *ruleedit=(d4xRuleEdit *)d4x_rule_edit_new(rule);
	gtk_widget_show_all(GTK_WIDGET(ruleedit));
	gtk_window_set_modal (GTK_WINDOW(ruleedit),TRUE);
	gtk_window_set_transient_for (GTK_WINDOW (ruleedit),
				      GTK_WINDOW (edit));
	ruleedit->filter_edit=(GtkWidget *)edit;
	gtk_signal_connect(GTK_OBJECT(ruleedit->ok_button),"clicked",
			   GTK_SIGNAL_FUNC(d4x_filter_edit_add_ok),
			   ruleedit);
	gtk_signal_connect(GTK_OBJECT(ruleedit->cancel_button),"clicked",
			   GTK_SIGNAL_FUNC(d4x_filter_edit_add_cancel),
			   ruleedit);
	gtk_signal_connect(GTK_OBJECT(ruleedit),"delete_event",
			   GTK_SIGNAL_FUNC(d4x_filter_edit_add_delete),
			   ruleedit);
};

static void d4x_filter_edit_edit_ok(GtkWidget *widget,d4xRuleEdit *edit){
	d4xFilterEdit *filtedit=(d4xFilterEdit *)(edit->filter_edit);
	d4x_rule_edit_apply(edit);
	gint row=gtk_clist_find_row_from_data (GTK_CLIST(filtedit->clist),
					       edit->rule);
	gtk_clist_set_text (GTK_CLIST(filtedit->clist),row,0,
			    edit->rule->include?"+":"-");
	gtk_clist_set_text (GTK_CLIST(filtedit->clist),row,1,
			    edit->rule->proto?get_name_by_proto(edit->rule->proto):"");
	gtk_clist_set_text (GTK_CLIST(filtedit->clist),row,2,
			    edit->rule->host.get()?edit->rule->host.get():"");
	gtk_clist_set_text (GTK_CLIST(filtedit->clist),row,3,
			    edit->rule->path.get()?edit->rule->path.get():"");
	gtk_clist_set_text (GTK_CLIST(filtedit->clist),row,4,
			    edit->rule->file.get()?edit->rule->file.get():"");
	gtk_widget_destroy(GTK_WIDGET(edit));
};

static void d4x_filter_edit_edit(GtkWidget *widget,d4xFilterEdit *edit){
	GList *select=((GtkCList *)edit->clist)->selection_end;
	if (select) {
		gint row=GPOINTER_TO_INT(select->data);
		d4xRule *rule=(d4xRule *)gtk_clist_get_row_data(
			GTK_CLIST(edit->clist),row);
		d4xRuleEdit *ruleedit=(d4xRuleEdit *)d4x_rule_edit_new(rule);
		ruleedit->filter_edit=(GtkWidget *)edit;
		gtk_signal_connect(GTK_OBJECT(ruleedit->ok_button),"clicked",
				   GTK_SIGNAL_FUNC(d4x_filter_edit_edit_ok),
				   ruleedit);
		gtk_signal_connect(GTK_OBJECT(ruleedit->cancel_button),"clicked",
				   GTK_SIGNAL_FUNC(d4x_rule_edit_cancel),
				   ruleedit);
		gtk_signal_connect(GTK_OBJECT(ruleedit),"delete_event",
				   GTK_SIGNAL_FUNC(d4x_rule_edit_delete),
				   ruleedit);
		gtk_widget_show_all(GTK_WIDGET(ruleedit));
		gtk_window_set_modal (GTK_WINDOW(ruleedit),TRUE);
		gtk_window_set_transient_for (GTK_WINDOW (ruleedit),
					      GTK_WINDOW (edit));
	};
};

static void d4x_filter_edit_del(GtkWidget *widget,d4xFilterEdit *edit){
	GList *select=((GtkCList *)edit->clist)->selection_end;
	if (select) {
		gint row=GPOINTER_TO_INT(select->data);
		d4xRule *rule=(d4xRule *)gtk_clist_get_row_data(
			GTK_CLIST(edit->clist),row);
		gtk_clist_remove(GTK_CLIST(edit->clist),row);
		edit->node->filter->del(rule);
		delete(rule);
	};
};

static void d4x_filter_edit_down(GtkWidget *widget,d4xFilterEdit *edit){
	GList *select=((GtkCList *)edit->clist)->selection_end;
	if (select) {
		gint row=GPOINTER_TO_INT(select->data);
		d4xRule *rule=(d4xRule *)gtk_clist_get_row_data(
			GTK_CLIST(edit->clist),row);
		if (rule->next){
			gtk_clist_swap_rows(GTK_CLIST(edit->clist),row,row-1);
			tNode *next=rule->next;
			d4xFilter *filter=edit->node->filter;
			filter->del(rule);
			filter->insert_before(rule,next);
		};
	};
};

static void d4x_filter_edit_up(GtkWidget *widget,d4xFilterEdit *edit){
	GList *select=((GtkCList *)edit->clist)->selection_end;
	if (select) {
		gint row=GPOINTER_TO_INT(select->data);
		d4xRule *rule=(d4xRule *)gtk_clist_get_row_data(
			GTK_CLIST(edit->clist),row);
		if (rule->prev){
			gtk_clist_swap_rows(GTK_CLIST(edit->clist),row,row+1);
			d4xFilter *filter=edit->node->filter;
			tNode *prev=rule->prev;
			filter->del(rule);
			if (prev->prev)
				filter->insert_before(rule,prev->prev);
			else
				filter->insert(rule);
		};
	};
};

static void d4x_filter_edit_select(GtkWidget *clist, gint row, gint column,
				   GdkEventButton *event,d4xFilterEdit *edit) {
	if (event && event->type==GDK_2BUTTON_PRESS && event->button==1)
		d4x_filter_edit_edit(clist,edit);
};

static void d4x_filter_edit_init(d4xFilterEdit *edit){
	gtk_window_set_title(GTK_WINDOW (edit),_("Modify filter"));
	gtk_widget_set_usize(GTK_WIDGET (edit),-1,400);

	edit->vbox=gtk_vbox_new(FALSE,0);
	gchar *titles[]={"",_("protocol"),_("host"),_("path"),_("file")};
	edit->clist=gtk_clist_new_with_titles(5,titles);;

	gtk_clist_set_shadow_type(GTK_CLIST(edit->clist), GTK_SHADOW_IN);
	gtk_clist_set_column_auto_resize(GTK_CLIST(edit->clist),0,TRUE);
	gtk_clist_set_column_auto_resize(GTK_CLIST(edit->clist),1,TRUE);
	gtk_clist_set_column_auto_resize(GTK_CLIST(edit->clist),2,TRUE);
	gtk_clist_set_column_auto_resize(GTK_CLIST(edit->clist),3,TRUE);
//	gtk_clist_set_selection_mode(GTK_CLIST(clist),GTK_SELECTION_EXTENDED);
	gtk_signal_connect(GTK_OBJECT(edit->clist),
			   "select_row",
			   GTK_SIGNAL_FUNC(d4x_filter_edit_select),
			   edit);

	GtkWidget *scroll_window=gtk_scrolled_window_new((GtkAdjustment *)NULL,(GtkAdjustment *)NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll_window),
	                                GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(scroll_window),edit->clist);
	gtk_box_pack_start(GTK_BOX(edit->vbox),scroll_window,TRUE,TRUE,0);

	edit->include=gtk_radio_button_new_with_label((GSList *)NULL,
						      _("include by default"));
	edit->exclude=gtk_radio_button_new_with_label(gtk_radio_button_group(GTK_RADIO_BUTTON(edit->include)),
						      _("exclude by default"));
//	edit->include=gtk_check_button_new_with_label(_("include by default"));
	gtk_box_pack_start(GTK_BOX(edit->vbox),edit->include,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(edit->vbox),edit->exclude,FALSE,FALSE,0);

	GtkWidget *hbox=gtk_hbox_new(FALSE,0);
	GtkWidget *label=gtk_label_new(_("Name of filter"));
	edit->name=gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox),label,TRUE,TRUE,0);
	gtk_box_pack_start(GTK_BOX(hbox),edit->name,TRUE,TRUE,0);
	gtk_box_pack_start(GTK_BOX(edit->vbox),hbox,FALSE,FALSE,0);

	GtkWidget *up=gtk_button_new_with_label(_("Up"));
	GtkWidget *add=gtk_button_new_with_label(_("Add rule"));
	GtkWidget *down=gtk_button_new_with_label(_("Down"));
	hbox=gtk_hbutton_box_new();
	gtk_box_set_spacing(GTK_BOX(hbox),5);
	gtk_box_pack_start(GTK_BOX(hbox),up,TRUE,TRUE,0);
	gtk_box_pack_start(GTK_BOX(hbox),down,TRUE,TRUE,0);
	gtk_box_pack_start(GTK_BOX(hbox),add,TRUE,TRUE,0);
	gtk_box_pack_start(GTK_BOX(edit->vbox),hbox,FALSE,FALSE,0);

	edit->ok=gtk_button_new_with_label(_("Ok"));
	GtkWidget *del=gtk_button_new_with_label(_("Remove"));
	edit->edit=gtk_button_new_with_label(_("Edit"));
	GTK_WIDGET_SET_FLAGS(edit->ok,GTK_CAN_DEFAULT);
	GTK_WIDGET_SET_FLAGS(del,GTK_CAN_DEFAULT);
	GTK_WIDGET_SET_FLAGS(edit->edit,GTK_CAN_DEFAULT);
	hbox=gtk_hbutton_box_new();
	gtk_box_set_spacing(GTK_BOX(hbox),5);
	gtk_box_pack_start(GTK_BOX(hbox),edit->ok,TRUE,TRUE,0);
	gtk_box_pack_start(GTK_BOX(hbox),del,TRUE,TRUE,0);
	gtk_box_pack_start(GTK_BOX(hbox),edit->edit,TRUE,TRUE,0);
	gtk_box_pack_start(GTK_BOX(edit->vbox),hbox,FALSE,FALSE,0);
	
	gtk_container_add(GTK_CONTAINER(edit),edit->vbox);
	gtk_window_set_default(GTK_WINDOW(edit),edit->ok);

	gtk_signal_connect(GTK_OBJECT(edit->edit),"clicked",
			   GTK_SIGNAL_FUNC(d4x_filter_edit_edit),edit);
	gtk_signal_connect(GTK_OBJECT(del),"clicked",
			   GTK_SIGNAL_FUNC(d4x_filter_edit_del),edit);
	gtk_signal_connect(GTK_OBJECT(add),"clicked",
			   GTK_SIGNAL_FUNC(d4x_filter_edit_add),edit);
	gtk_signal_connect(GTK_OBJECT(up),"clicked",
			   GTK_SIGNAL_FUNC(d4x_filter_edit_down),edit);
	gtk_signal_connect(GTK_OBJECT(down),"clicked",
			   GTK_SIGNAL_FUNC(d4x_filter_edit_up),edit);
};

static void d4x_filter_edit_class_init(d4xFilterEditClass *klass){
	GtkObjectClass *object_class=(GtkObjectClass *)klass;
	
	object_class->destroy=d4x_filter_edit_destroy;
	filter_parent_class=(GtkWidgetClass *)gtk_type_class(gtk_window_get_type());
};

guint d4x_filter_edit_get_type(){
	static guint d4x_filter_edit_type=0;
	if (!d4x_filter_edit_type){
		GtkTypeInfo info={
			"d4xFilterEdit",
			sizeof(d4xFilterEdit),
			sizeof(d4xFilterEditClass),
			(GtkClassInitFunc) d4x_filter_edit_class_init,
			(GtkObjectInitFunc) d4x_filter_edit_init,
			NULL,NULL
//			(GtkArgSetFunc) NULL,
//			(GtkArgGetFunc) NULL
		};
		d4x_filter_edit_type = gtk_type_unique (gtk_window_get_type (),
							&info);
	};
	return d4x_filter_edit_type;
};

GtkWidget *d4x_filter_edit_new(d4xFNode *node){
	d4xFilterEdit *edit=(d4xFilterEdit *)gtk_type_new(d4x_filter_edit_get_type());
	edit->node=node;
	node->filter->print(edit);
	if (node->filter->name.get())
		text_to_combo(edit->name,node->filter->name.get());
	GTK_TOGGLE_BUTTON(edit->include)->active=node->filter->default_inc;
	GTK_TOGGLE_BUTTON(edit->exclude)->active=!node->filter->default_inc;
	return GTK_WIDGET(edit);
};
