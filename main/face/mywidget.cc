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

#include "mywidget.h"
#include "edit.h"
#include "misc.h"
#include "../ntlocale.h"
#include "../filter.h"

static GtkWidgetClass *parent_class = (GtkWidgetClass *)NULL;
static GtkWidgetClass *color_parent_class = (GtkWidgetClass *)NULL;
static GtkWidgetClass *rule_parent_class = (GtkWidgetClass *)NULL;
static GtkWidgetClass *filter_parent_class = (GtkWidgetClass *)NULL;
static GtkWidgetClass *filtersel_parent_class = (GtkWidgetClass *)NULL;
static GtkWidgetClass *linkssel_parent_class = (GtkWidgetClass *)NULL;
static GtkWidgetClass *stringedit_parent_class = (GtkWidgetClass *)NULL;

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
		g_signal_connect(G_OBJECT(GTK_FILE_SELECTION(filesel->browser)->ok_button),
				   "clicked",G_CALLBACK(my_gtk_filesel_ok),filesel);
		g_signal_connect(G_OBJECT(GTK_FILE_SELECTION(filesel->browser)->cancel_button),
				   "clicked",G_CALLBACK(my_gtk_filesel_cancel),filesel);
		g_signal_connect(G_OBJECT(GTK_FILE_SELECTION(filesel->browser)),
				   "delete_event",G_CALLBACK(my_gtk_filesel_delete),filesel);
		gtk_widget_show(filesel->browser);
		d4x_eschandler_init(filesel->browser,filesel);
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
	g_signal_connect(G_OBJECT(button),"clicked",G_CALLBACK(my_gtk_filesel_init_browser),filesel);
	gtk_combo_set_case_sensitive(GTK_COMBO(filesel->combo),TRUE);
	gtk_box_set_spacing(GTK_BOX(filesel),5);
	gtk_box_pack_start(GTK_BOX(filesel),filesel->combo,TRUE,TRUE,0);
	gtk_box_pack_start(GTK_BOX(filesel),button,FALSE,FALSE,0);
};

guint my_gtk_filesel_get_type(){
	static guint my_filesel_type=0;
	if (!my_filesel_type){
		GTypeInfo my_filesel_info={
			sizeof(MyGtkFileselClass),
			NULL,NULL,
			(GClassInitFunc)my_gtk_filesel_class_init,
			NULL,NULL,
			sizeof(MyGtkFilesel),
			0,
			(GInstanceInitFunc)my_gtk_filesel_init
		};
		my_filesel_type = g_type_register_static (GTK_TYPE_HBOX,"MyGtkFilesel",&my_filesel_info,(GTypeFlags)0);
	};
	return my_filesel_type;
};

GtkWidget *my_gtk_filesel_new(tHistory *history){
	MyGtkFilesel *filesel=(MyGtkFilesel *)g_object_new(my_gtk_filesel_get_type(),NULL);
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

static void my_gtk_colorsel_expose(GtkWidget *widget,GdkEventExpose *event,MyGtkColorsel *colsel){
	gint w=widget->allocation.width,h=widget->allocation.height;
	guchar *buf=g_new(guchar,3*w*h);
	gint i=0;
	for (int y=0;y<h;y++){
		for (int x=0;x<w;x++){
			buf[i++]=colsel->color.red/0xff;
			buf[i++]=colsel->color.green/0xff;
			buf[i++]=colsel->color.blue/0xff;
		};
	};
	gdk_draw_rgb_image(widget->window,widget->style->black_gc,
			   0,0,w,h,
			   GDK_RGB_DITHER_NORMAL,
			   buf,3*w);
	g_free(buf);
};


static gint my_gtk_colorsel_ok(GtkButton *button,MyGtkColorsel *colsel){
	g_return_val_if_fail(colsel!=NULL,FALSE);

	if (colsel->browser){
		gtk_color_selection_get_current_color(GTK_COLOR_SELECTION(GTK_COLOR_SELECTION_DIALOG(colsel->browser)->colorsel),
						      &(colsel->color));
		gtk_widget_queue_draw (colsel->preview); 
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
		gtk_color_selection_set_current_color(GTK_COLOR_SELECTION(GTK_COLOR_SELECTION_DIALOG(colsel->browser)->colorsel),
						      &(colsel->color));
		g_signal_connect(G_OBJECT(GTK_COLOR_SELECTION_DIALOG(colsel->browser)->ok_button),
				   "clicked",G_CALLBACK(my_gtk_colorsel_ok),colsel);
		g_signal_connect(G_OBJECT(GTK_COLOR_SELECTION_DIALOG(colsel->browser)->cancel_button),
				   "clicked",G_CALLBACK(my_gtk_colorsel_cancel),colsel);
		g_signal_connect(G_OBJECT(GTK_COLOR_SELECTION_DIALOG(colsel->browser)),
				   "delete_event",G_CALLBACK(my_gtk_colorsel_delete),colsel);
		d4x_eschandler_init(GTK_WIDGET(GTK_COLOR_SELECTION_DIALOG(colsel->browser)),colsel);
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
	colsel->preview=gtk_drawing_area_new();
	g_signal_connect(G_OBJECT(colsel->preview),"expose_event",G_CALLBACK(my_gtk_colorsel_expose),colsel);
	GtkWidget *button=gtk_button_new_with_label(_("Select"));
	g_signal_connect(G_OBJECT(button),"clicked",G_CALLBACK(my_gtk_colorsel_init_browser),colsel);
	gtk_box_set_spacing(GTK_BOX(colsel),5);
	gtk_widget_set_size_request(colsel->preview,50,50);
	gtk_box_pack_start(GTK_BOX(colsel),colsel->preview,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(colsel),button,FALSE,FALSE,0);
};

guint my_gtk_colorsel_get_type(){
	static guint my_filesel_type=0;
	if (!my_filesel_type){
		GTypeInfo my_filesel_info={
			sizeof(MyGtkColorselClass),
			NULL,NULL,
			(GClassInitFunc) my_gtk_colorsel_class_init,
			NULL,NULL,
			sizeof(MyGtkColorsel),
			0,
			(GInstanceInitFunc)my_gtk_colorsel_init
		};
		my_filesel_type = g_type_register_static (GTK_TYPE_HBOX,"MyGtkColorsel",&my_filesel_info,(GTypeFlags)0);
	};
	return my_filesel_type;
};

gint my_gtk_colorsel_get_color(MyGtkColorsel *colsel){
	gint color=0;
	color=((gint(colsel->color.blue/0xff))&0xff)+
	      (((gint(colsel->color.green/0xff))&0xff)<<8)+
	      (((gint(colsel->color.red/0xff))&0xff)<<16);
	return color;
};

void my_gtk_colorsel_set_color(MyGtkColorsel *colsel, gint color){
	g_return_if_fail(colsel!=NULL);
	colsel->color.blue=(color&0xff)*0xff;
	colsel->color.green=((color>>8)&0xff)*0xff;
	colsel->color.red=((color>>16)&0xff)*0xff;
	if (GTK_WIDGET_VISIBLE(colsel))
		gtk_widget_queue_draw (colsel->preview); 
};

GtkWidget *my_gtk_colorsel_new(gint color,gchar *title){
	MyGtkColorsel *colsel=(MyGtkColorsel *)g_object_new(my_gtk_colorsel_get_type(),NULL);
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
		if (*(text_from_combo(edit->params))){
			edit->rule->params.set(text_from_combo(edit->params));
		}else
			edit->rule->params.set((char*)NULL);
		if (*(text_from_combo(edit->tag))){
			edit->rule->tag.set(text_from_combo(edit->tag));
		}else
			edit->rule->tag.set((char*)NULL);
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
	gtk_window_set_resizable(GTK_WINDOW(edit), FALSE);

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
	gtk_editable_set_editable(GTK_EDITABLE(GTK_COMBO(edit->proto)->entry),FALSE);
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
	label=gtk_label_new(_("?"));
	edit->params=gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox),edit->params,TRUE,TRUE,0);
	gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(edit->vbox),hbox,FALSE,FALSE,0);

	hbox=gtk_hbox_new(FALSE,0);
	label=gtk_label_new(_("tag"));
	edit->tag=gtk_combo_new();
	list=(GList*)NULL;
	char *tags[]={"",
		      "a",
		      "applet",
		      "area",
		      "bgsound",
		      "blockqute",
		      "body",
		      "col",
		      "del",
		      "div",
		      "embed",
		      "fig",
		      "frame",
		      "head",
		      "iframe",
		      "img",
		      "input",
		      "ins",
		      "layer",
		      "link",
		      "meta",
		      "object",
		      "overlay",
		      "q",
		      "table",
		      "tbody",
		      "td",
		      "tfoot",
		      "th",
		      "thead",
		      "script",
		      "sound",
		      "span"
	};
	for (unsigned int i=0;i<sizeof(tags)/sizeof(char*);i++)
		list = g_list_append (list, tags[i]);
	gtk_combo_set_popdown_strings (GTK_COMBO (edit->tag), list);
	g_list_free(list);
	gtk_editable_set_editable(GTK_EDITABLE(GTK_COMBO(edit->tag)->entry),FALSE);
	gtk_box_pack_start(GTK_BOX(hbox),edit->tag,TRUE,TRUE,0);
	gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(edit->vbox),hbox,FALSE,FALSE,0);
	

	hbox=gtk_hbox_new(FALSE,0);
	edit->include=gtk_radio_button_new_with_label((GSList *)NULL,
						      _("include"));
	edit->exclude=gtk_radio_button_new_with_label(gtk_radio_button_get_group(GTK_RADIO_BUTTON(edit->include)),
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
		GTypeInfo info={
			sizeof(d4xRuleEditClass),
			NULL,NULL,
			(GClassInitFunc) d4x_rule_edit_class_init,
			NULL,NULL,
			sizeof(d4xRuleEdit),
			0,
			(GInstanceInitFunc)d4x_rule_edit_init
		};
		d4x_rule_edit_type=g_type_register_static(GTK_TYPE_WINDOW,"d4xRuleEdit",&info,(GTypeFlags)0);
	};
	return d4x_rule_edit_type;
};

GtkWidget *d4x_rule_edit_new(d4xRule *rule){
	d4xRuleEdit *edit=(d4xRuleEdit *)g_object_new(d4x_rule_edit_get_type(),NULL);
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
		if (rule->params.get())
			text_to_combo(edit->params,rule->params.get());
		if (rule->tag.get())
			text_to_combo(edit->tag,rule->tag.get());
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
	g_signal_connect(G_OBJECT(edit->ok_button),"clicked",
			   G_CALLBACK(d4x_rule_edit_ok),edit);
	g_signal_connect(G_OBJECT(edit->cancel_button),"clicked",
			   G_CALLBACK(d4x_rule_edit_cancel),edit);
	g_signal_connect(G_OBJECT(edit),"delete_event",
			   G_CALLBACK(d4x_rule_edit_delete), edit);
	d4x_eschandler_init(GTK_WIDGET(edit),edit);

	return GTK_WIDGET(edit);
};

/************************************************************/

enum {
	FE_COL_INC,
	FE_COL_PROTO,
	FE_COL_HOST,
	FE_COL_PATH,
	FE_COL_FILE,
	FE_COL_PARAMS,
	FE_COL_TAG,
	FE_COL_RULE,
	FE_COL_LAST
};

static void d4x_filter_edit_destroy(GtkObject *widget){
	g_return_if_fail(widget!=NULL);

	if (GTK_OBJECT_CLASS (filter_parent_class)->destroy)
		(* GTK_OBJECT_CLASS (filter_parent_class)->destroy) (widget);
};


void d4x_filter_edit_add_rule(d4xFilterEdit *edit,d4xRule *rule){
	gtk_list_store_append(edit->store,&(rule->iter));
	gtk_list_store_set(edit->store,&(rule->iter),
			   FE_COL_INC,rule->include?(char*)"+":(char*)("-"),
			   FE_COL_PROTO,rule->proto?get_name_by_proto(rule->proto):(char*)(""),
			   FE_COL_HOST,rule->host.get()?rule->host.get():(char*)(""),
			   FE_COL_PATH,rule->path.get()?rule->path.get():(char*)(""),
			   FE_COL_FILE,rule->file.get()?rule->file.get():(char*)(""),
			   FE_COL_PARAMS,rule->params.get()?rule->params.get():(char*)(""),
			   FE_COL_TAG,rule->tag.get()?rule->tag.get():(char*)(""),
			   FE_COL_RULE,rule,
			   -1);
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
	g_signal_connect(G_OBJECT(ruleedit->ok_button),"clicked",
			   G_CALLBACK(d4x_filter_edit_add_ok),
			   ruleedit);
	g_signal_connect(G_OBJECT(ruleedit->cancel_button),"clicked",
			   G_CALLBACK(d4x_filter_edit_add_cancel),
			   ruleedit);
	g_signal_connect(G_OBJECT(ruleedit),"delete_event",
			   G_CALLBACK(d4x_filter_edit_add_delete),
			   ruleedit);
	d4x_eschandler_init(GTK_WIDGET(ruleedit),ruleedit);
};

static void d4x_filter_edit_edit_ok(GtkWidget *widget,d4xRuleEdit *edit){
	d4xFilterEdit *filtedit=(d4xFilterEdit *)(edit->filter_edit);
	d4x_rule_edit_apply(edit);
	d4xRule *rule=edit->rule;
	gtk_list_store_set(filtedit->store,&(rule->iter),
			   FE_COL_INC,rule->include?"+":"-",
			   FE_COL_PROTO,rule->proto?get_name_by_proto(rule->proto):"",
			   FE_COL_HOST,rule->host.get()?rule->host.get():"",
			   FE_COL_PATH,rule->path.get()?rule->path.get():"",
			   FE_COL_FILE,rule->file.get()?rule->file.get():"",
			   FE_COL_PARAMS,rule->params.get()?rule->params.get():"",
			   FE_COL_TAG,rule->tag.get()?rule->tag.get():"",
			   -1);
	gtk_widget_destroy(GTK_WIDGET(edit));
};

static void d4x_filter_edit_edit(GtkWidget *widget,d4xFilterEdit *edit){
	GtkTreeSelection *sel=gtk_tree_view_get_selection(edit->view);
	GtkTreeModel *model;
	GtkTreeIter iter;
	if (gtk_tree_selection_get_selected(sel,&model,&iter)){
		GValue val={0,};
		gtk_tree_model_get_value(model,&iter,FE_COL_RULE,&val);
		d4xRule *rule=(d4xRule *)g_value_get_pointer(&val);
		g_value_unset(&val);
		d4xRuleEdit *ruleedit=(d4xRuleEdit *)d4x_rule_edit_new(rule);
		ruleedit->filter_edit=(GtkWidget *)edit;
		g_signal_connect(G_OBJECT(ruleedit->ok_button),"clicked",
				   G_CALLBACK(d4x_filter_edit_edit_ok),
				   ruleedit);
		g_signal_connect(G_OBJECT(ruleedit->cancel_button),"clicked",
				   G_CALLBACK(d4x_rule_edit_cancel),
				   ruleedit);
		g_signal_connect(G_OBJECT(ruleedit),"delete_event",
				   G_CALLBACK(d4x_rule_edit_delete),
				   ruleedit);
		d4x_eschandler_init(GTK_WIDGET(ruleedit),ruleedit);
		gtk_widget_show_all(GTK_WIDGET(ruleedit));
		gtk_window_set_modal (GTK_WINDOW(ruleedit),TRUE);
		gtk_window_set_transient_for (GTK_WINDOW (ruleedit),
					      GTK_WINDOW (edit));
	};
};

static void d4x_filter_edit_del(GtkWidget *widget,d4xFilterEdit *edit){
	GtkTreeSelection *sel=gtk_tree_view_get_selection(edit->view);
	GtkTreeModel *model;
	GtkTreeIter iter;
	if (gtk_tree_selection_get_selected(sel,&model,&iter)){
		GValue val={0,};
		gtk_tree_model_get_value(model,&iter,FE_COL_RULE,&val);
		d4xRule *rule=(d4xRule *)g_value_get_pointer(&val);
		g_value_unset(&val);
		gtk_list_store_remove(edit->store,&(rule->iter));
		edit->node->filter->del(rule);
		delete(rule);
	};
};

static void d4x_filter_edit_down(GtkWidget *widget,d4xFilterEdit *edit){
	GtkTreeSelection *sel=gtk_tree_view_get_selection(edit->view);
	GtkTreeModel *model;
	GtkTreeIter iter;
	if (gtk_tree_selection_get_selected(sel,&model,&iter)){
		GValue val={0,};
		gtk_tree_model_get_value(model,&iter,FE_COL_RULE,&val);
		d4xRule *rule=(d4xRule *)g_value_get_pointer(&val);
		g_value_unset(&val);
		if (rule->prev){
			tNode *next=rule->prev;
			gtk_tree_model_swap_rows_l(GTK_TREE_MODEL(edit->store),
						   &(rule->iter),&(((d4xRule*)next)->iter));
			d4xFilter *filter=edit->node->filter;
			filter->del(next);
			filter->insert_before(next,rule);
			GtkTreeIter iter_tmp;
			memcpy(&iter_tmp,&(((d4xRule*)next)->iter),sizeof(rule->iter));
			memcpy(&(((d4xRule*)next)->iter),&(rule->iter),sizeof(rule->iter));
			memcpy(&(rule->iter),&iter_tmp,sizeof(rule->iter));
			gtk_tree_selection_select_iter(sel,&(rule->iter));
		};
	};
};

static void d4x_filter_edit_up(GtkWidget *widget,d4xFilterEdit *edit){
	GtkTreeSelection *sel=gtk_tree_view_get_selection(edit->view);
	GtkTreeModel *model;
	GtkTreeIter iter;
	if (gtk_tree_selection_get_selected(sel,&model,&iter)){
		GValue val={0,};
		gtk_tree_model_get_value(model,&iter,FE_COL_RULE,&val);
		d4xRule *rule=(d4xRule *)g_value_get_pointer(&val);
		g_value_unset(&val);
		if (rule->next){
			tNode *next=rule->next;
			gtk_tree_model_swap_rows_l(GTK_TREE_MODEL(edit->store),
						   &(rule->iter),&(((d4xRule*)next)->iter));
			d4xFilter *filter=edit->node->filter;
			filter->del(rule);
			filter->insert_before(rule,next);
			GtkTreeIter iter_tmp;
			memcpy(&iter_tmp,&(((d4xRule*)next)->iter),sizeof(rule->iter));
			memcpy(&(((d4xRule*)next)->iter),&(rule->iter),sizeof(rule->iter));
			memcpy(&(rule->iter),&iter_tmp,sizeof(rule->iter));
			gtk_tree_selection_select_iter(sel,&(rule->iter));
		};
	};
};

static gboolean d4x_filter_edit_select(GtkTreeView *view,GdkEventButton *event,d4xFilterEdit *edit) {
	if (event && event->type==GDK_2BUTTON_PRESS && event->button==1){
		d4x_filter_edit_edit(GTK_WIDGET(view),edit);
		return TRUE;
	};
	return FALSE;
};

static void d4x_filter_edit_init(d4xFilterEdit *edit){
	gtk_window_set_title(GTK_WINDOW (edit),_("Modify filter"));
	gtk_widget_set_size_request(GTK_WIDGET (edit),-1,400);

	edit->vbox=gtk_vbox_new(FALSE,0);
	gchar *titles[]={"",_("protocol"),_("host"),_("path"),_("file"),"?",_("tag")};
	edit->store=gtk_list_store_new(FE_COL_LAST,
				       G_TYPE_STRING,
				       G_TYPE_STRING,
				       G_TYPE_STRING,
				       G_TYPE_STRING,
				       G_TYPE_STRING,
				       G_TYPE_STRING,
				       G_TYPE_STRING,
				       G_TYPE_POINTER);
	edit->view=GTK_TREE_VIEW(gtk_tree_view_new_with_model(GTK_TREE_MODEL(edit->store)));
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *col;
	for (int i=0;i<FE_COL_RULE;i++){
			renderer = gtk_cell_renderer_text_new ();
			col=gtk_tree_view_column_new_with_attributes (titles[i],
								      renderer,
								      "text",i,
								      NULL);
			gtk_tree_view_append_column (edit->view, col);
	};
//	gtk_clist_set_selection_mode(GTK_CLIST(clist),GTK_SELECTION_EXTENDED);
	g_signal_connect(G_OBJECT(edit->view),
			   "event",
			   G_CALLBACK(d4x_filter_edit_select),
			   edit);

	GtkWidget *scroll_window=gtk_scrolled_window_new((GtkAdjustment *)NULL,(GtkAdjustment *)NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll_window),
	                                GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(scroll_window),GTK_WIDGET(edit->view));
	gtk_box_pack_start(GTK_BOX(edit->vbox),scroll_window,TRUE,TRUE,0);

	edit->include=gtk_radio_button_new_with_label((GSList *)NULL,
						      _("include by default"));
	edit->exclude=gtk_radio_button_new_with_label(gtk_radio_button_get_group(GTK_RADIO_BUTTON(edit->include)),
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

	g_signal_connect(G_OBJECT(edit->edit),"clicked",
			   G_CALLBACK(d4x_filter_edit_edit),edit);
	g_signal_connect(G_OBJECT(del),"clicked",
			   G_CALLBACK(d4x_filter_edit_del),edit);
	g_signal_connect(G_OBJECT(add),"clicked",
			   G_CALLBACK(d4x_filter_edit_add),edit);
	g_signal_connect(G_OBJECT(up),"clicked",
			   G_CALLBACK(d4x_filter_edit_up),edit);
	g_signal_connect(G_OBJECT(down),"clicked",
			   G_CALLBACK(d4x_filter_edit_down),edit);
};

static void d4x_filter_edit_class_init(d4xFilterEditClass *klass){
	GtkObjectClass *object_class=(GtkObjectClass *)klass;
	
	object_class->destroy=d4x_filter_edit_destroy;
	filter_parent_class=(GtkWidgetClass *)gtk_type_class(gtk_window_get_type());
};

guint d4x_filter_edit_get_type(){
	static guint d4x_filter_edit_type=0;
	if (!d4x_filter_edit_type){
		GTypeInfo info={
			sizeof(d4xFilterEditClass),
			NULL,NULL,
			(GClassInitFunc) d4x_filter_edit_class_init,
			NULL,NULL,
			sizeof(d4xFilterEdit),
			0,
			(GInstanceInitFunc)d4x_filter_edit_init
		};
		d4x_filter_edit_type=g_type_register_static(GTK_TYPE_WINDOW,"d4xFilterEdit",&info,(GTypeFlags)0);
	};
	return d4x_filter_edit_type;
};

GtkWidget *d4x_filter_edit_new(d4xFNode *node){
	d4xFilterEdit *edit=(d4xFilterEdit *)g_object_new(d4x_filter_edit_get_type(),NULL);
	edit->node=node;
	node->filter->print(edit);
	if (node->filter->name.get())
		text_to_combo(edit->name,node->filter->name.get());
	GTK_TOGGLE_BUTTON(edit->include)->active=node->filter->default_inc;
	GTK_TOGGLE_BUTTON(edit->exclude)->active=!node->filter->default_inc;
	return GTK_WIDGET(edit);
};

/**********************************************************/
static void d4x_filter_sel_destroy(GtkObject *widget){
	g_return_if_fail(widget!=NULL);

	if (GTK_OBJECT_CLASS (filtersel_parent_class)->destroy)
		(* GTK_OBJECT_CLASS (filtersel_parent_class)->destroy) (widget);
};

static void d4x_filter_sel_init(d4xFilterSel *sel){
	gtk_window_set_wmclass(GTK_WINDOW(sel),
			       "D4X_FilterSel","D4X");
	gtk_widget_set_size_request(GTK_WIDGET(sel),-1,300);
	gtk_window_set_title(GTK_WINDOW(sel),_("Select filter"));

	GtkListStore *list_store = gtk_list_store_new(2,
						      G_TYPE_STRING,
						      G_TYPE_POINTER);
	sel->view = GTK_TREE_VIEW(gtk_tree_view_new_with_model(GTK_TREE_MODEL(list_store)));
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *col;
	renderer = gtk_cell_renderer_text_new();
	col=gtk_tree_view_column_new_with_attributes(_("filter name"),
						     renderer,
						     "text",0,
						     NULL);
	gtk_tree_view_set_headers_visible(sel->view,TRUE);
	gtk_tree_view_column_set_visible(col,TRUE);
	gtk_tree_view_append_column(sel->view,col);

	GtkWidget *scroll_window=gtk_scrolled_window_new((GtkAdjustment*)NULL,(GtkAdjustment*)NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll_window),
	                                GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(scroll_window),GTK_WIDGET(sel->view));
	sel->ok=gtk_button_new_with_label(_("Ok"));
	sel->cancel=gtk_button_new_with_label(_("Cancel"));
	GTK_WIDGET_SET_FLAGS(sel->ok,GTK_CAN_DEFAULT);
	GTK_WIDGET_SET_FLAGS(sel->cancel,GTK_CAN_DEFAULT);
	GtkWidget *vbox=gtk_vbox_new(FALSE,0);
	GtkWidget *hbox=gtk_hbutton_box_new();
	gtk_box_set_spacing(GTK_BOX(vbox),5);
	gtk_box_set_spacing(GTK_BOX(hbox),5);
	gtk_box_pack_start(GTK_BOX(vbox),scroll_window,TRUE,TRUE,0);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);
	gtk_box_pack_end(GTK_BOX(hbox),sel->ok,FALSE,FALSE,0);
	gtk_box_pack_end(GTK_BOX(hbox),sel->cancel,FALSE,FALSE,0);
	gtk_container_add(GTK_CONTAINER(sel),vbox);
	gtk_window_set_default(GTK_WINDOW(sel),sel->ok);
	FILTERS_DB->print(sel);
};

static void d4x_filter_sel_class_init(d4xFilterSelClass *klass){
	GtkObjectClass *object_class=(GtkObjectClass *)klass;
	
	object_class->destroy=d4x_filter_sel_destroy;
	filtersel_parent_class=(GtkWidgetClass *)gtk_type_class(gtk_window_get_type());
};

guint d4x_filter_sel_get_type(){
	static guint d4x_filter_sel_type=0;
	if (!d4x_filter_sel_type){
		GTypeInfo info={
			sizeof(d4xFilterSelClass),
			NULL,NULL,
			(GClassInitFunc) d4x_filter_sel_class_init,
			NULL,NULL,
			sizeof(d4xFilterSel),
			0,
			(GInstanceInitFunc)d4x_filter_sel_init
		};
		d4x_filter_sel_type=g_type_register_static (GTK_TYPE_WINDOW,"d4xFilterSel",&info,(GTypeFlags)0);
	};
	return d4x_filter_sel_type;
};

GtkWidget *d4x_filter_sel_new(){
	d4xFilterSel *sel=(d4xFilterSel *)g_object_new(d4x_filter_sel_get_type(),NULL);
	gtk_widget_show_all(GTK_WIDGET(sel));
	return GTK_WIDGET(sel);
};

void d4x_filter_sel_add(d4xFilterSel *sel,d4xFNode *node){
	GtkListStore *list_store=GTK_LIST_STORE(gtk_tree_view_get_model(sel->view));
	GtkTreeIter iter;
	gtk_list_store_append(list_store, &iter);
	gtk_list_store_set(list_store, &iter,
			   0,node->filter->name.get(),
			   1,node,
			   -1);
};

void d4x_filter_sel_to_combo(d4xFilterSel *sel,GtkWidget *combo){
	GtkTreeSelection *s=gtk_tree_view_get_selection(sel->view);
	GtkTreeIter iter;
	GtkTreeModel *model;
	if (gtk_tree_selection_get_selected(s,&model,&iter)){
		GValue val={0,};
		gtk_tree_model_get_value(model,&iter,0,&val);
		const char *name=g_value_get_string(&val);
		text_to_combo(combo,(char*)name);
		g_value_unset(&val);
	}else
		text_to_combo(combo,"");
};

/********************************************************/

static void d4x_links_sel_destroy(GtkObject *widget){
	g_return_if_fail(widget!=NULL);

	if (GTK_OBJECT_CLASS (linkssel_parent_class)->destroy)
		(* GTK_OBJECT_CLASS (linkssel_parent_class)->destroy) (widget);
};

static void _foreach_remove_prepare_(GtkTreeModel *model,GtkTreePath *path,
				     GtkTreeIter *iter,gpointer p){
	tQueue *q=(tQueue*)p;
	tmpIterNode *i=new tmpIterNode(iter);
	q->insert(i);
};

void d4x_links_sel_del(d4xLinksSel *sel,GtkTreeIter *iter){
	GtkListStore *store=GTK_LIST_STORE(gtk_tree_view_get_model(sel->view));
	gtk_list_store_remove(store,iter);
};


static void d4x_links_sel_remove(GtkWidget *button,d4xLinksSel *sel){
	GtkTreeSelection *s=gtk_tree_view_get_selection(sel->view);
	tQueue q;
	gtk_tree_selection_selected_foreach(s,
					    _foreach_remove_prepare_,
					    &q);
	tNode *t=q.last();
	while(t){
		d4x_links_sel_del(sel,((tmpIterNode*)t)->iter);
		t=t->next;
	};
};

void d4x_links_sel_set(d4xLinksSel *sel,GtkTreeIter *iter,char *url,gpointer p){
	GtkListStore *list_store=GTK_LIST_STORE(gtk_tree_view_get_model(sel->view));
	gtk_list_store_set(list_store, iter,
			   0,url,
			   1,p,
			   -1);
};

gpointer d4x_links_sel_get_data(d4xLinksSel *sel,GtkTreeIter *iter){
	GtkTreeModel *model=gtk_tree_view_get_model(sel->view);
	GValue val={0,};
	gtk_tree_model_get_value(model,iter,1,&val);
	gpointer p=g_value_get_pointer(&val);
	g_value_unset(&val);
	return(p);
};

void d4x_links_sel_selected_foreach(d4xLinksSel *sel,d4xLinksSelForeachFunc func,gpointer data){
	GtkTreeSelection *s=gtk_tree_view_get_selection(sel->view);
	GtkTreeModel *model=gtk_tree_view_get_model(sel->view);
	tQueue q;
	gtk_tree_selection_selected_foreach(s,
					    _foreach_remove_prepare_,
					    &q);
	tNode *t=q.last();
	while(t){
		GValue val={0,};
		gtk_tree_model_get_value(model,((tmpIterNode*)t)->iter,1,&val);
		gpointer p=g_value_get_pointer(&val);
		g_value_unset(&val);
		gtk_tree_model_get_value(model,((tmpIterNode*)t)->iter,0,&val);
		const gchar *s=g_value_get_string(&val);
		func(sel,((tmpIterNode*)t)->iter,s,p,data);
		g_value_unset(&val);
		t=t->next;
	};
};

static void d4x_links_sel_cancel(GtkWidget *button,d4xLinksSel *sel){
	gtk_widget_destroy(GTK_WIDGET(sel));
}

static void d4x_links_sel_delete(GtkWindow *window,
				 GdkEvent *event,
				 d4xLinksSel *sel){
	gtk_widget_destroy(GTK_WIDGET(sel));
};

static void d4x_links_sel_init(d4xLinksSel *sel){
	gtk_window_set_wmclass(GTK_WINDOW(sel),
			       "D4X_LinksSel","D4X");
	d4x_eschandler_init(GTK_WIDGET(sel),sel);
	gtk_widget_set_size_request(GTK_WIDGET(sel),-1,300);
	gtk_window_set_title(GTK_WINDOW(sel),_("List of links"));

	GtkListStore *list_store = gtk_list_store_new(2,
						      G_TYPE_STRING,
						      G_TYPE_POINTER);
	sel->view = GTK_TREE_VIEW(gtk_tree_view_new_with_model(GTK_TREE_MODEL(list_store)));
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *col;
	renderer = gtk_cell_renderer_text_new();
	col=gtk_tree_view_column_new_with_attributes(_("URL"),
						     renderer,
						     "text",0,
						     NULL);
	gtk_tree_view_set_headers_visible(sel->view,TRUE);
	gtk_tree_view_column_set_visible(col,TRUE);
	gtk_tree_view_append_column(sel->view,col);
	col=gtk_tree_view_get_column(sel->view,0);
	
	GtkWidget *scroll_window=gtk_scrolled_window_new((GtkAdjustment*)NULL,(GtkAdjustment*)NULL);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW (scroll_window),
					    GTK_SHADOW_IN);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll_window),
	                                GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(scroll_window),GTK_WIDGET(sel->view));
	sel->ok=gtk_button_new_with_label(_("Ok"));
	sel->remove=gtk_button_new_with_label(_("Remove"));
	sel->cancel=gtk_button_new_with_label(_("Cancel"));
	GTK_WIDGET_SET_FLAGS(sel->ok,GTK_CAN_DEFAULT);
	GTK_WIDGET_SET_FLAGS(sel->remove,GTK_CAN_DEFAULT);
	GTK_WIDGET_SET_FLAGS(sel->cancel,GTK_CAN_DEFAULT);
	GtkWidget *vbox=gtk_vbox_new(FALSE,0);
	sel->hbbox=gtk_hbutton_box_new();
	gtk_box_set_spacing(GTK_BOX(vbox),5);
	gtk_box_set_spacing(GTK_BOX(sel->hbbox),5);
	gtk_box_pack_start(GTK_BOX(vbox),scroll_window,TRUE,TRUE,0);
	gtk_box_pack_start(GTK_BOX(vbox),sel->hbbox,FALSE,FALSE,0);
	gtk_box_pack_end(GTK_BOX(sel->hbbox),sel->ok,FALSE,FALSE,0);
	gtk_box_pack_end(GTK_BOX(sel->hbbox),sel->remove,FALSE,FALSE,0);
	gtk_container_add(GTK_CONTAINER(sel),vbox);
	gtk_window_set_default(GTK_WINDOW(sel),sel->ok);
};

static void d4x_links_sel_class_init(d4xLinksSelClass *klass){
	GtkObjectClass *object_class=(GtkObjectClass *)klass;
	
	object_class->destroy=d4x_links_sel_destroy;
	linkssel_parent_class=(GtkWidgetClass *)gtk_type_class(gtk_window_get_type());
};

guint d4x_links_sel_get_type(){
	static guint d4x_links_sel_type=0;
	if (!d4x_links_sel_type){
		GTypeInfo info={
			sizeof(d4xLinksSelClass),
			NULL,NULL,
			(GClassInitFunc) d4x_links_sel_class_init,
			NULL,NULL,
			sizeof(d4xLinksSel),
			0,
			(GInstanceInitFunc)d4x_links_sel_init
		};
		d4x_links_sel_type = g_type_register_static (GTK_TYPE_WINDOW,"d4xLinksSel",&info,(GTypeFlags)0);
	};
	return d4x_links_sel_type;
};

GtkWidget *d4x_links_sel_new_with_add(){
	d4xLinksSel *sel=(d4xLinksSel *)g_object_new(d4x_links_sel_get_type(),NULL);
	sel->find=gtk_button_new_with_label(_("Find"));
	GTK_WIDGET_SET_FLAGS(sel->find,GTK_CAN_DEFAULT);
	gtk_box_pack_end(GTK_BOX(sel->hbbox),sel->find,FALSE,FALSE,0);
	gtk_box_pack_end(GTK_BOX(sel->hbbox),sel->cancel,FALSE,FALSE,0);
	GList *c=gtk_container_get_children(GTK_CONTAINER(sel->cancel));
	GtkLabel *label=(GtkLabel *)(c->data);
	gtk_label_set_text(label,_("Add"));
	g_list_free(c);
	gtk_widget_show_all(GTK_WIDGET(sel));
	return GTK_WIDGET(sel);
};

GtkWidget *d4x_links_sel_new(){
	d4xLinksSel *sel=(d4xLinksSel *)g_object_new(d4x_links_sel_get_type(),NULL);
	gtk_box_pack_end(GTK_BOX(sel->hbbox),sel->cancel,FALSE,FALSE,0);
	g_signal_connect(G_OBJECT(sel),"delete_event",
			   G_CALLBACK(d4x_links_sel_delete), sel);
	g_signal_connect(G_OBJECT(sel->remove),"clicked",
			   G_CALLBACK(d4x_links_sel_remove),
			   sel);
	g_signal_connect(G_OBJECT(sel->cancel),"clicked",
			   G_CALLBACK(d4x_links_sel_cancel),
			   sel);
	gtk_widget_show_all(GTK_WIDGET(sel));
	return GTK_WIDGET(sel);
};

void d4x_links_sel_add(d4xLinksSel *sel,char *url,gpointer p){
	GtkListStore *list_store=GTK_LIST_STORE(gtk_tree_view_get_model(sel->view));
	GtkTreeIter iter;
	gtk_list_store_append(list_store, &iter);
	gtk_list_store_set(list_store, &iter,
			   0,url,
			   1,p,
			   -1);
};

struct d4xSelTmp{
	gpointer data;
	d4xLinksSelForeachFunc func;
	d4xLinksSel *sel;
};

static gboolean _tmp_sel_foreach_(GtkTreeModel *model,
				  GtkTreePath *path,
				  GtkTreeIter *iter,
				  gpointer data){
	d4xSelTmp *tmp=(d4xSelTmp *)data;
	GValue val={0,};
	gtk_tree_model_get_value(model,iter,1,&val);
	gpointer p=g_value_get_pointer(&val);
	g_value_unset(&val);
	gtk_tree_model_get_value(model,iter,0,&val);
	const gchar *s=g_value_get_string(&val);
	tmp->func(tmp->sel,iter,s,p,tmp->data);
	g_value_unset(&val);
	return FALSE;
};

void d4x_links_sel_foreach(d4xLinksSel *sel,d4xLinksSelForeachFunc func,gpointer data){
	GtkTreeModel *store=gtk_tree_view_get_model(sel->view);
	d4xSelTmp a={data,func,sel};
	gtk_tree_model_foreach(store,_tmp_sel_foreach_,&a);
};

void d4x_links_sel_clear(d4xLinksSel *sel){
	GtkListStore *store=GTK_LIST_STORE(gtk_tree_view_get_model(sel->view));
	gtk_list_store_clear(store);
};

/********************************************************/
static void d4x_string_edit_destroy(GtkObject *widget){
	g_return_if_fail(widget!=NULL);

	if (GTK_OBJECT_CLASS (stringedit_parent_class)->destroy)
		(* GTK_OBJECT_CLASS (stringedit_parent_class)->destroy) (widget);
};
static void d4x_string_edit_class_init(d4xStringEditClass *klass){
	GtkObjectClass *object_class=(GtkObjectClass *)klass;
	
	object_class->destroy=d4x_string_edit_destroy;
	stringedit_parent_class=(GtkWidgetClass *)gtk_type_class(gtk_window_get_type());
};

static void d4x_string_edit_init(d4xStringEdit *sel){
	gtk_window_set_wmclass(GTK_WINDOW(sel),
			       "D4X_EditString","D4X");
	d4x_eschandler_init(GTK_WIDGET(sel),sel);
	gtk_widget_set_size_request(GTK_WIDGET(sel),400,-1);

	GtkWidget *vbox=gtk_vbox_new(FALSE,0);
	GtkWidget *hbbox=gtk_hbutton_box_new();
	sel->ok=gtk_button_new_with_label(_("Ok"));
	sel->cancel=gtk_button_new_with_label(_("Cancel"));
	GTK_WIDGET_SET_FLAGS(sel->ok,GTK_CAN_DEFAULT);
	GTK_WIDGET_SET_FLAGS(sel->cancel,GTK_CAN_DEFAULT);
	gtk_box_set_spacing(GTK_BOX(vbox),5);
	gtk_box_set_spacing(GTK_BOX(hbbox),5);
	sel->entry=(GtkEntry*)gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(vbox),GTK_WIDGET(sel->entry),TRUE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),hbbox,TRUE,FALSE,0);
	gtk_box_pack_end(GTK_BOX(hbbox),sel->ok,FALSE,FALSE,0);
	gtk_box_pack_end(GTK_BOX(hbbox),sel->cancel,FALSE,FALSE,0);
	gtk_container_add(GTK_CONTAINER(sel),vbox);
	gtk_window_set_default(GTK_WINDOW(sel),sel->cancel);
};

guint d4x_string_edit_get_type(){
	static guint d4x_string_edit_type=0;
	if (!d4x_string_edit_type){
		GTypeInfo info={
			sizeof(d4xStringEditClass),
			NULL,NULL,
			(GClassInitFunc) d4x_string_edit_class_init,
			NULL,NULL,
			sizeof(d4xStringEdit),
			0,
			(GInstanceInitFunc)d4x_string_edit_init
		};
		d4x_string_edit_type = g_type_register_static (GTK_TYPE_WINDOW,"d4xStringEdit",&info,(GTypeFlags)0);
	};
	return d4x_string_edit_type;
};

GtkWidget *d4x_string_edit_new(){
	d4xStringEdit *edit=(d4xStringEdit *)g_object_new(d4x_string_edit_get_type(),NULL);
	gtk_widget_show_all(GTK_WIDGET(edit));
	return GTK_WIDGET(edit);
};

/********************************************************/
