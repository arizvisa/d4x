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

#include "alt.h"
#include "signal.h"
#include "dlist.h"
#include "face/misc.h"
#include "locstr.h"
#include "var.h"
#include "ntlocale.h"
#include "main.h"
extern tMain aa;

d4xAltList::d4xAltList(){
	FIRST=END=NULL;
	edit=NULL;
	add_edit=NULL;
	mod_edit=NULL;
	str2mod=NULL;
};

d4xAltList::~d4xAltList(){
	clear();
	edit_destroy();
	if (str2mod) gtk_tree_iter_free(str2mod);
};

void d4xAltList::lock_by_download(){
	download_set_block(1);
	lock.lock();		
};

void d4xAltList::unlock_by_download(){
	lock.unlock();		
	download_set_block(0);
};

void d4xAltList::del(d4xAlt *alt){
	if (alt->prev)
		alt->prev->next=alt->next;
	else
		FIRST=alt->next;
	if (alt->next)
		alt->next->prev=alt->prev;
	else
		END=alt->prev;
};

void d4xAltList::add(d4xAlt *alt){
	alt->prev=NULL;
	if ((alt->next=FIRST))
		FIRST->prev=alt;
	else
		END=alt;
	if (edit){
		char *url=alt->info.url();
		d4x_links_sel_add(edit,url,alt);
		delete[] url;
	};
	FIRST=alt;
};

void d4xAltList::check(char *filename){
	if (FIRST && !equal(filename,FIRST->info.file.get()))
		clear();
};

void d4xAltList::clear(){
	while (FIRST){
		d4xAlt *tmp=FIRST;
		FIRST=FIRST->next;
		delete(tmp);
	};
	if (edit){
		d4x_links_sel_clear(edit);
	};
};

void d4xAltList::fill_from_ftpsearch(tDownload *fs){
	if (fs->DIR==NULL) return;
	lock.lock();
	clear();
	tDownload *tmp=fs->DIR->first();
	while(tmp){
		d4xAlt *alt=new d4xAlt;
		alt->info.copy(tmp->info);
		add(alt);
		tmp=fs->DIR->prev();
	};
	lock.unlock();
};

static void d4x_alt_find(GtkWidget *button,tDownload *papa){
	aa.ftp_search(papa,1);
};


static void d4x_alt_remove(GtkWidget *button,d4xAltList *alt){
	alt->edit_remove();
};

static void d4x_alt_add(GtkWidget *button,d4xAltList *alt){
	alt->init_add();
};

static void d4x_alt_ok(GtkWidget *button,d4xAltList *alt){
	alt->edit_destroy();
};

static void d4x_alt_delete(GtkWidget *window,GdkEvent *event,d4xAltList *alt){
	alt->edit_destroy();
};

void d4xAltList::edit_destroy(){
	if (edit){
		gtk_widget_destroy(GTK_WIDGET(edit));
		edit=NULL;
	};
	add_edit_destroy();
	edit_mod_destroy();
};

void d4xAltList::add_edit_destroy(){
	if (add_edit){
		gtk_widget_destroy(GTK_WIDGET(add_edit));
		add_edit=NULL;
	};
};
void d4xAltList::edit_mod_destroy(){
	if (mod_edit){
		gtk_widget_destroy(GTK_WIDGET(mod_edit));
		mod_edit=NULL;
	};
};

static gboolean d4d_alt_dblclick(GtkTreeView *view, GdkEventButton *event, d4xAltList *alt) {
	if (event && event->type==GDK_2BUTTON_PRESS && event->button==1) {
		GtkTreeSelection *sel=gtk_tree_view_get_selection(view);
		GtkTreePath *path;
		if (gtk_tree_view_get_path_at_pos(view,int(event->x),int(event->y),&path,NULL,NULL,NULL)){
			GtkTreeIter iter;
			GtkTreeModel *model=gtk_tree_view_get_model(view);
			gtk_tree_model_get_iter(model,&iter,path);
			gtk_tree_selection_select_iter(sel,&iter);
			gtk_tree_path_free(path);
			alt->init_edit_mod(&iter);
		};
		return TRUE;
	};
	return FALSE;
};

void d4xAltList::init_edit(tDownload *papa){
	if (edit){
		gdk_window_show(GTK_WIDGET(edit)->window);
		return;
	};
	edit=(d4xLinksSel *)d4x_links_sel_new_with_add();
	g_signal_connect(G_OBJECT(edit->ok),"clicked",
			   G_CALLBACK(d4x_alt_ok),
			   this);	
	g_signal_connect(G_OBJECT(edit->cancel),"clicked",
			   G_CALLBACK(d4x_alt_add),
			   this);	
	g_signal_connect(G_OBJECT(edit->remove),"clicked",
			   G_CALLBACK(d4x_alt_remove),
			   this);
	g_signal_connect(G_OBJECT(edit->find),"clicked",
			   G_CALLBACK(d4x_alt_find),
			   papa);
	g_signal_connect(G_OBJECT(edit),"delete_event",
			   G_CALLBACK(d4x_alt_delete),
			   this);
	g_signal_connect(G_OBJECT(edit->view),"event",
			   G_CALLBACK(d4d_alt_dblclick),this);
	print2edit();
};

static void _tmp_remove_(d4xLinksSel *sel,GtkTreeIter *iter,const gchar *s,gpointer p,gpointer data){
	d4xAltList *alt=(d4xAltList *)data;
	alt->del((d4xAlt *)p);
	d4x_links_sel_del(sel,iter);
};

void d4xAltList::edit_remove(){
	if (!edit) return;
	lock.lock();
	d4x_links_sel_selected_foreach(edit,_tmp_remove_,this);
	lock.unlock();
};

void d4xAltList::print2edit(){
	d4xAlt *alt=FIRST;
	while(alt){
		char *url=alt->info.url();
		d4x_links_sel_add(edit,url,alt);
		delete[] url;
		alt=alt->next;
	};
};


static void d4x_alt_add_ok(GtkWidget *button, d4xAltList *alt){
	alt->add_edit_ok();
	alt->add_edit_destroy();
};
static void d4x_alt_add_cancel(GtkWidget *button, d4xAltList *alt){
	alt->add_edit_destroy();
};

static void d4x_alt_add_delete(GtkWidget *window,GdkEvent *event,d4xAltList *alt){
	alt->add_edit_destroy();
};

void d4xAltList::init_add(){
	if (add_edit){
		gdk_window_show(GTK_WIDGET(add_edit)->window);
		return;
	};
	add_edit=(d4xStringEdit *)d4x_string_edit_new();
	gtk_window_set_title(GTK_WINDOW (add_edit),_("Add new alternate"));
	g_signal_connect(G_OBJECT(add_edit->ok),"clicked",
			   G_CALLBACK(d4x_alt_add_ok),
			   this);	
	g_signal_connect(G_OBJECT(add_edit->cancel),"clicked",
			   G_CALLBACK(d4x_alt_add_cancel),
			   this);	
	g_signal_connect(G_OBJECT(add_edit),"delete_event",
			   G_CALLBACK(d4x_alt_add_delete),
			   this);
};

void d4xAltList::add_edit_ok(){
	d4xAlt *alt=new d4xAlt;
	alt->info.from_string(text_from_combo(GTK_WIDGET(add_edit->entry)));
	lock.lock();
	add(alt);
	lock.unlock();
};

int d4xAltList::save_to_config(int fd){
	if (!FIRST) return(0);
	f_wstr_lf(fd,"Alt:");
	d4xAlt *alt=END;
	while(alt){
		char *url=alt->info.url();
		char *parsed=unparse_percents(url);
		delete[] url;
		f_wstr_lf(fd,parsed);
		delete[] parsed;
		alt=alt->prev;
	};
	f_wstr_lf(fd,"EndAlt");
	return(0);
};

int d4xAltList::load_from_config(int fd){
	char buf[MAX_LEN];
	while(f_rstr(fd,buf,MAX_LEN)>0){
		if (equal_uncase(buf,"EndAlt"))
			return(0);
		d4xAlt *alt=new d4xAlt;
		char *url=parse_percents(buf);
		alt->info.from_string(url);
		delete[] url;
		add(alt);
	};
	return -1;
};


static void d4x_alt_mod_delete(GtkWidget *window,GdkEvent *event,d4xAltList *alt){
	alt->edit_mod_destroy();
};
static void d4x_alt_mod_cancel(GtkWidget *button,d4xAltList *alt){
	alt->edit_mod_destroy();
};
static void d4x_alt_mod_ok(GtkWidget *button,d4xAltList *alt){
	alt->edit_mod_ok();
	alt->edit_mod_destroy();
};

void d4xAltList::edit_mod_ok(){
	if (!mod_edit) return;
	d4xAlt *alt=(d4xAlt*)d4x_links_sel_get_data(edit,str2mod);
	if (alt){
		alt->info.from_string(text_from_combo(GTK_WIDGET(mod_edit->entry)));
		char *url=alt->info.url();
		d4x_links_sel_set(edit,str2mod,url,alt);
		delete[] url;
	};
};

void d4xAltList::init_edit_mod(GtkTreeIter *iter){
	d4xAlt *alt=(d4xAlt*)d4x_links_sel_get_data(edit,iter);
	if (!alt) return;
	char *url=alt->info.url();
	if (str2mod) gtk_tree_iter_free(str2mod);
	str2mod=gtk_tree_iter_copy(iter);
	if (mod_edit){
		gdk_window_show(GTK_WIDGET(mod_edit)->window);
		text_to_combo(GTK_WIDGET(mod_edit->entry),url);
		delete[] url;
		return;
	};
	mod_edit=(d4xStringEdit *)d4x_string_edit_new();
	gtk_window_set_title(GTK_WINDOW (mod_edit),_("Modify alternate"));
	g_signal_connect(G_OBJECT(mod_edit->ok),"clicked",
			 G_CALLBACK(d4x_alt_mod_ok),
			 this);	
	g_signal_connect(G_OBJECT(mod_edit->cancel),"clicked",
			 G_CALLBACK(d4x_alt_mod_cancel),
			 this);	
	g_signal_connect(G_OBJECT(mod_edit),"delete_event",
			 G_CALLBACK(d4x_alt_mod_delete),
			 this);
	text_to_combo(GTK_WIDGET(mod_edit->entry),url);
	delete[] url;
};
