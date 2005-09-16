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

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include "mainlog.h"
#include "var.h"
#include "face/colors.h"
#include "face/list.h"
#include "face/prefs.h"
#include "locstr.h"
#include "face/lmenu.h"
#include "face/log.h"
#include "ntlocale.h"
#include "main.h"

static gint list_menu_open_row_main_log(GtkWidget *widget, tMLog *Log) {
	Log->open_selected_row();
	return TRUE;
};

static gint list_menu_clear_main_log(GtkWidget *widget, tMLog *Log) {
	Log->done();
	return TRUE;
};

static gint list_menu_open_properties(GtkWidget *widget, tMLog *Log){
	d4x_prefs_init_page(PREFS_PAGE_MAINLOG);
	return TRUE;
};

int main_log_event_handler2(GtkWidget *widget,GdkEventButton *event,tMLog *log) {
	return log->popup(event);
};

tMLog::tMLog():tStringList(){
	start=time(NULL);
	list=NULL;
	string=NULL;
	current_line=0;
	fd=0;
};

void tMLog::reinit(int a) {
	MaxNum=a;
};

static void _ml_clist_addr_destroy_(d4x::URL *addr){
	if (addr) delete(addr);
};

void tMLog::add_to_list() {
	current_line+=1;
	tLogString *str=(tLogString *)Last;
	char *color;
	char str_type;
	switch (str->type & (LOG_DETAILED-1)) {
		case LOG_OK:{
				color="black";
				str_type='+';
				break;
			};
		case LOG_FROM_SERVER:
			{
				color="blue";
				str_type='-';
				break;
			};
		case LOG_WARNING:
			{
				str_type='?';
				color="darkgreen";
				break;
			};
		case LOG_ERROR:
			{
				str_type='!';
				color="red";
				break;
			};
		default:
			str_type=' ';
			color="black";
	};
	struct tm msgtime;
	localtime_r(&(str->time),&msgtime);
	char useful[MAX_LEN];
	strftime(useful,MAX_LEN,"%T",&msgtime);
	char tmpdate[MAX_LEN];
	strftime(tmpdate,MAX_LEN,"%d %b %Y",&msgtime);
	GtkTreeIter iter;
	if (list){
		char *date_utf=g_convert(tmpdate,-1,"UTF-8",LOCALE_CODEPAGE,NULL,NULL,NULL);
		GtkListStore *store=(GtkListStore *)gtk_tree_view_get_model(list);
		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store, &iter,
				   ML_COL_NUM, current_line,
				   ML_COL_TIME, useful,
				   ML_COL_DATE, date_utf?date_utf:tmpdate,
				   ML_COL_STRING, str->body,
				   ML_COL_LAST, color,
				   -1);
		if (date_utf) g_free(date_utf);
/*FIXME: GTK2
			gtk_clist_set_row_data_full(list,row,addr,
						    GtkDestroyNotify(_ml_clist_addr_destroy_));
*/
	};
	strftime(useful,MAX_LEN,"%T %d %b %Y ",&msgtime);
	if (CFG.WITHOUT_FACE){
		if (str->type==LOG_ERROR){
			if (CFG.COLORIFIED_OUTPUT) printf("%c[31;40;1m",27); //red
			printf("%c %s %s\n",str_type,useful,str->body);
		}else{
			if (CFG.COLORIFIED_OUTPUT) printf("%c[35;40m",27);   //magenta
			printf("%c ",str_type);
			if (CFG.COLORIFIED_OUTPUT) printf("%c[34;40;1m",27); //blue
			g_print("%s ",useful);
			if (CFG.COLORIFIED_OUTPUT) printf("%c[32;40;1m",27);
			g_print("%s\n",str->body);
		};
	};
	if (fd) {
		if (CFG.MAIN_LOG_FILE_LIMIT){
			struct stat finfo;
			fstat(fd,&finfo);
			if (finfo.st_size>(CFG.MAIN_LOG_FILE_LIMIT*1024)){
				ftruncate(fd,0);
				lseek(fd,0,SEEK_SET);
				add(_("Limitation for size of file of mainlog reached! File have been truncated."),LOG_ERROR);
			};
		};
		if (write(fd,&str_type,sizeof(str_type))<0 ||
		    write(fd,useful,strlen(useful))<0 ||
		    write(fd,str->body,strlen(str->body))<0 ||
		    write(fd,"\n",strlen("\n"))<0) {
			close(fd);
			fd=0;
			add(_("Can't write to file interrupting write to file"),LOG_ERROR);
		};
	};
};

void tMLog::print() {
	if (!list) return;
	tLogString *prom=(tLogString *)First;
	while (prom) {
		prom=(tLogString *)prom->prev;
	};
};

void tMLog::add(char *str,int len,int type) {
	DBC_RETURN_IF_FAIL(str!=NULL);
	if ((type & LOG_DETAILED)  && !CFG.MAIN_LOG_DETAILED) return;
	tLogString *temp=new tLogString(str,len,type);
	temp->time=time(NULL);
	insert(temp);
	add_to_list();
	Size+=len;
};

void tMLog::add(char *str,int type) {
	DBC_RETURN_IF_FAIL(str!=NULL);
	if ((type & LOG_DETAILED)  && !CFG.MAIN_LOG_DETAILED) return;	
	int len=strlen(str);
	tLogString *ins=new tLogString(str,len,type);
	insert(ins);
	add_to_list();
	Size+=len;
};

void tMLog::init_list(GtkTreeView *clist) {
	list=clist;
	if (list==NULL) return;
	g_signal_connect(G_OBJECT(list),"event",G_CALLBACK(main_log_event_handler2),this);
	/* Initing popup menu
	 */
	popup_menu=gtk_menu_new();
	open_row_item=gtk_menu_item_new_with_label(_("Open a row"));
	gtk_menu_shell_append(GTK_MENU_SHELL(popup_menu),open_row_item);
	g_signal_connect(G_OBJECT(open_row_item),"activate",G_CALLBACK(list_menu_open_row_main_log),this);
	clear_item=gtk_menu_item_new_with_label(_("Clear log"));
	gtk_menu_shell_append(GTK_MENU_SHELL(popup_menu),clear_item);
	g_signal_connect(G_OBJECT(clear_item),"activate",G_CALLBACK(list_menu_clear_main_log),this);
	GtkWidget *item=gtk_menu_item_new_with_label(_("Properties"));
	gtk_menu_shell_append(GTK_MENU_SHELL(popup_menu),item);
	g_signal_connect(G_OBJECT(item),"activate",G_CALLBACK(list_menu_open_properties),this);
	gtk_widget_show_all(popup_menu);
};

int tMLog::popup(GdkEventButton *event) {
	if (!list) return FALSE;
	GtkTreeSelection *sel=gtk_tree_view_get_selection(list);
	GtkTreePath *path=NULL;
	if (event && event->type==GDK_BUTTON_PRESS && event->button==3) {
		gtk_tree_selection_unselect_all(sel);
		int selected=0;
		if (gtk_tree_view_get_path_at_pos(list,gint(event->x),gint(event->y),&path,NULL,NULL,NULL)){
			selected=1;
			gtk_tree_selection_select_path(sel,path);
			gtk_tree_path_free(path);
		};
		if (selected)
			gtk_widget_set_sensitive(open_row_item,TRUE);
		else
			gtk_widget_set_sensitive(open_row_item,FALSE);
		if (Num>0)
			gtk_widget_set_sensitive(clear_item,TRUE);
		else
			gtk_widget_set_sensitive(clear_item,FALSE);
		gtk_menu_popup(GTK_MENU(popup_menu),NULL,NULL,NULL,NULL,event->button,event->time);
		return TRUE;
	};
	if (event && event->type==GDK_2BUTTON_PRESS && event->button==1){
		if (gtk_tree_view_get_path_at_pos(list,gint(event->x),gint(event->y),&path,NULL,NULL,NULL)){
			gtk_tree_selection_select_path(sel,path);
			GtkTreeIter iter;
			GtkTreeModel *model=gtk_tree_view_get_model(list);
			gtk_tree_model_get_iter(model,&iter,path);
			gtk_tree_path_free(path);
			open_row(&iter);
		};
		return TRUE;
	};
	return FALSE;
};

void tMLog::real_open_row(GtkTreeIter *iter){
	char data[MAX_LEN];
	char *text;
	int num=0;
// FIXME GTK2
	GValue val={0,};
	GtkTreeModel *model=gtk_tree_view_get_model(list);
	gtk_tree_model_get_value(model,iter,ML_COL_NUM,&val);
	num=g_value_get_int(&val);
	g_value_unset(&val);
	sprintf(data,_("row number %i of main log"),num);
	gtk_tree_model_get_value(model,iter,ML_COL_STRING,&val);
	text=(char*)g_value_get_string(&val);
	if (!string)
		string=new tStringDialog;
	string->init(text,data);
	g_value_unset(&val);
};

void tMLog::open_row(GtkTreeIter *iter) {
	if (!list) return;
/*
	tAddr *addr=(tAddr *)gtk_clist_get_row_data(list,row);
	tDownload *dwn;
	if (addr  && (dwn=_aa_.find_url(addr))){
		log_window_init(dwn);
		D4X_QVT->move_to(dwn);
	}else{
*/
		real_open_row(iter);
//	};
};

void tMLog::open_selected_row() {
	if (!list) return;
	GtkTreeSelection *sel=gtk_tree_view_get_selection(list);
	GtkTreeIter iter;
	if (gtk_tree_selection_get_selected(sel,NULL,&iter)){
		real_open_row(&iter);
	};
};

void tMLog::reinit_file() {
	if (fd) {
		close(fd);
		fd=0;
	};
	if (CFG.SAVE_MAIN_LOG && CFG.SAVE_LOG_PATH) {
		if (CFG.APPEND_REWRITE_LOG) {
			fd=open(CFG.SAVE_LOG_PATH,O_WRONLY|O_CREAT,S_IRUSR | S_IWUSR );
			lseek(fd,0,SEEK_END);
		} else
			fd=open(CFG.SAVE_LOG_PATH,O_TRUNC | O_CREAT |O_RDWR,S_IRUSR | S_IWUSR);
		if (fd<0) {
			add(_("Can't open file for saving log"),LOG_ERROR);
			fd=0;
		};
	};
};

void tMLog::add(char *str) {
	DBC_RETURN_IF_FAIL(str!=NULL);
	int len=strlen(str);
	tLogString *ins=new tLogString(str,len,LOG_FROM_SERVER);
	insert(ins);
	add_to_list();
	Size+=len;
};

void tMLog::myprintf(int type,char *fmt,...){
	DBC_RETURN_IF_FAIL(fmt!=NULL);

	char str[MAX_LEN+1];
	char *cur=str;
	va_list ap;
	va_start(ap,fmt);
	last_error.clear();
	*cur=0;
	while (*fmt && cur-str<MAX_LEN){
		if (*fmt=='%'){
			fmt+=1;
			switch(*fmt){
			case 's':{
				char *s=va_arg(ap,char *);
				if (s)
					g_snprintf(cur,MAX_LEN-(cur-str),"%s",s);
				else
					g_snprintf(cur,MAX_LEN-(cur-str),"%s","NULL");
				break;
			};
			case 'z':{
				tDownload *temp=va_arg(ap,tDownload *);
				if (temp){
					g_snprintf(cur,MAX_LEN-(cur-str),"%s",std::string(temp->info).c_str());
					last_error=temp->info;
				}else{
					g_snprintf(cur,MAX_LEN-(cur-str),"%s","NULL");
				};
				break;
			};
			case 'i':{
				g_snprintf(cur,MAX_LEN-(cur-str),"%i",va_arg(ap,int));
				break;
			};
			case 'l':{
				switch(*(fmt+1)){
				case 'u':{
					fmt+=1;
					g_snprintf(cur,MAX_LEN-(cur-str),"%lu",va_arg(ap,unsigned long));
					break;
				};
				case 'i':{
					fmt+=1;
					g_snprintf(cur,MAX_LEN-(cur-str),"%li",va_arg(ap,long));
					break;
				};
				};
				break;
			};
			default:{
				*cur=*fmt;
				cur+=1;
				*cur=0;			       
			};
			};
			if (*fmt==0) break;
			while(*cur) cur+=1;
		}else{
			*cur=*fmt;
			cur+=1;
			*cur=0;
		};
		fmt+=1;
	};
	va_end(ap);
	add(str,type);
};

void tMLog::dispose() {
	if (list){
		GtkTreeIter iter;
		GtkListStore *store=(GtkListStore *)gtk_tree_view_get_model(list);
		gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store),&iter);
		gtk_list_store_remove(store,&iter);
	};
	tStringList::dispose();
};

tLogString *tMLog::last() {
	return (tLogString *)tStringList::last();
};

tLogString *tMLog::next() {
	return (tLogString *)tStringList::next();
};

tLogString *tMLog::first() {
	return (tLogString *)tStringList::first();
};

void tMLog::done(){
	tStringList::done();
};

tMLog::~tMLog() {
	done();
	if (string) delete(string);
};
