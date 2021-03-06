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
#include "dlist.h"

void d4x::Alt::save(int fd){
	f_wstr_lf(fd,std::string(info).c_str());
	f_wstr_lf(fd,std::string(proxy).c_str());
};

void d4x::Alt::set_proxy_settings(tDownload *dwn){
	if (proxy.proto==D_PROTO_UNKNOWN || proxy.port==0) return;
	switch(proxy.proto){
	case D_PROTO_FTP:{
		if (info.proto==D_PROTO_FTP){
			dwn->config->proxy.type=0;
			dwn->config->proxy.ftp_port=proxy.port;
			dwn->config->proxy.ftp_host.set(proxy.host.c_str());
			dwn->config->proxy.ftp_user.set(proxy.user.c_str());
			dwn->config->proxy.ftp_pass.set(proxy.pass.c_str());
		};
		break;
	};
	case D_PROTO_HTTP:{
		dwn->config->proxy.type=1;
		if (info.proto==D_PROTO_HTTP){
			dwn->config->proxy.http_port=proxy.port;
			dwn->config->proxy.http_host.set(proxy.host.c_str());
			dwn->config->proxy.http_user.set(proxy.user.c_str());
			dwn->config->proxy.http_pass.set(proxy.pass.c_str());
		}else{
			dwn->config->proxy.ftp_port=proxy.port;
			dwn->config->proxy.ftp_host.set(proxy.host.c_str());
			dwn->config->proxy.ftp_user.set(proxy.user.c_str());
			dwn->config->proxy.ftp_pass.set(proxy.pass.c_str());
		};
		break;
	};
	};
};

d4xAltList::d4xAltList(){
	edit=NULL;
	add_edit=NULL;
	mod_edit=NULL;
	str2mod=NULL;
	ftp_searching=0;
};

d4xAltList::~d4xAltList(){
	clear();
	edit_destroy();
	if (str2mod) gtk_tree_iter_free(str2mod);
};

void d4xAltList::lock_by_download(){
	lock.lock();		
};

void d4xAltList::unlock_by_download(){
	lock.unlock();		
};

void d4xAltList::del(d4x::Alt *alt){
	LST.remove(alt);
};

void d4xAltList::add(d4x::Alt *alt){
	LST.push_back(alt);
	if (edit){
		d4x_links_sel_add(edit,std::string(alt->info).c_str(),alt);
	};
};

void d4xAltList::check(const std::string &filename){
	if (!LST.empty() && filename!=(*LST.begin())->info.file)
		clear();
};

static void _destroy_it_(d4x::Alt *a){
	delete a;
};

void d4xAltList::clear(){
	std::for_each(LST.begin(),LST.end(),_destroy_it_);
	LST.clear();
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
		d4x::Alt *alt=new d4x::Alt;
		alt->info=tmp->info;
		add(alt);
		tmp=fs->DIR->prev();
	};
	lock.unlock();
};

static void d4x_alt_find(GtkWidget *button,tDownload *papa){
	_aa_.ftp_search(papa,1);
	if (papa && papa->ALTS){
		papa->ALTS->ftp_searching=1;
		papa->ALTS->set_find_sens();
	};
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

void d4xAltList::set_find_sens(){
	if (edit==NULL) return;
	if (ftp_searching)
		gtk_widget_set_sensitive(edit->find,FALSE);
	else
		gtk_widget_set_sensitive(edit->find,TRUE);
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
	set_find_sens();
	print2edit();
};

static void _tmp_remove_(d4xLinksSel *sel,GtkTreeIter *iter,const gchar *s,gpointer p,gpointer data){
	d4xAltList *alt=(d4xAltList *)data;
	alt->del((d4x::Alt *)p);
	d4x_links_sel_del(sel,iter);
};

void d4xAltList::edit_remove(){
	if (!edit) return;
	lock.lock();
	d4x_links_sel_selected_foreach(edit,_tmp_remove_,this);
	lock.unlock();
};

void d4xAltList::print2edit(){
	for(std::list<d4x::Alt*>::iterator it=LST.begin();it!=LST.end();it++){
		d4x_links_sel_add(edit,std::string((*it)->info).c_str(),*it);
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
	add_edit=(d4xAltEdit *)d4x_alt_edit_new();
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
	d4x::Alt *alt=new d4x::Alt;
	alt->info=std::string(text_from_combo(GTK_WIDGET(add_edit->entry)));
	d4x_alt_edit_get(add_edit,alt->proxy);
	lock.lock();
	add(alt);
	lock.unlock();
};

int d4xAltList::save_to_config(int fd){
	f_wstr_lf(fd,"Alt:");
	std::for_each(LST.begin(),LST.end(),std::bind2nd(std::mem_fun(&d4x::Alt::save),fd));
	f_wstr_lf(fd,"EndAlt");
	return(0);
};

int d4xAltList::load_from_config(int fd){
	char buf[MAX_LEN];
	d4x::Alt *alt=NULL;
	while(f_rstr(fd,buf,MAX_LEN)>0){
		if (equal_uncase(buf,"EndAlt")){
			if (alt) delete(alt);
			return(0);
		};
		if (alt){
			alt->proxy=std::string(buf);
			add(alt);
			alt=NULL;
		}else{
			alt=new d4x::Alt;
			alt->info=std::string(buf);
		};
	};
	if (alt)
		delete(alt);
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
	d4x::Alt *alt=(d4x::Alt*)d4x_links_sel_get_data(edit,str2mod);
	if (alt){
		alt->info=std::string(text_from_combo(GTK_WIDGET(mod_edit->entry)));
		d4x_alt_edit_get(mod_edit,alt->proxy);
		d4x_links_sel_set(edit,str2mod,std::string(alt->info).c_str(),alt);
	};
};

void d4xAltList::init_edit_mod(GtkTreeIter *iter){
	d4x::Alt *alt=(d4x::Alt*)d4x_links_sel_get_data(edit,iter);
	if (!alt) return;
	std::string url(alt->info);
	if (str2mod) gtk_tree_iter_free(str2mod);
	str2mod=gtk_tree_iter_copy(iter);
	if (mod_edit){
		gdk_window_show(GTK_WIDGET(mod_edit)->window);
		text_to_combo(GTK_WIDGET(mod_edit->entry),url.c_str());
		d4x_alt_edit_set(mod_edit,alt->proxy);
		return;
	};
	mod_edit=(d4xAltEdit *)d4x_alt_edit_new();
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
	text_to_combo(GTK_WIDGET(mod_edit->entry),url.c_str());
	d4x_alt_edit_set(mod_edit,alt->proxy);
};

