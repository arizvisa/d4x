/*	WebDownloader for X-Window
 *	Copyright (C) 1999-2000 Koshelev Maxim
 *	This Program is free but not GPL!!! You can't modify it
 *	without agreement with author. You can't distribute modified
 *	program but you can distribute unmodified program.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */
#include "mainlog.h"
#include "var.h"
#include "face/colors.h"
#include "face/list.h"
#include "locstr.h"
#include "face/lmenu.h"
#include "ntlocale.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>

static gint list_menu_open_row_main_log(GtkWidget *widget, tMLog *Log) {
	Log->open_selected_row();
	return TRUE;
};

static gint list_menu_clear_main_log(GtkWidget *widget, tMLog *Log) {
	Log->done();
	return TRUE;
};

static void main_log_event_handler( GtkWidget *clist, gint row, gint column,
                                    GdkEventButton *event,tMLog *parent) {
	if (event) {
		if (event->type==GDK_2BUTTON_PRESS && event->button==1) {
			parent->open_row(row);
		};
	};
};

int main_log_event_handler2(GtkWidget *widget,GdkEventButton *event,tMLog *log) {
	return log->popup(event);
};

tMLog::tMLog() {
	start=time(NULL);
	list=NULL;
	fd=0;
	MaxNum=0;
	Num=0;
	string=NULL;
	current_line=0;
};

void tMLog::reinit(int a) {
	MaxNum=a;
};

void tMLog::add_to_list() {
	current_line+=1;
	tLogString *str=(tLogString *)Last;
	struct tm msgtime;
	localtime_r(&(str->time),&msgtime);
	char useful[MAX_LEN];
	strftime(useful,MAX_LEN,"%T",&msgtime);
	char line_number[MAX_LEN];
	sprintf(line_number,"[%i]",current_line);
	char *data[ML_COL_LAST];
	data[ML_COL_NUM] = line_number;
	data[ML_COL_TIME] = useful;
	data[ML_COL_STRING] = str->body;
	int row=0;
	if (list)
		row=gtk_clist_append(list,data);
	GdkColor color;
	char str_type;
	switch (str->type & (LOG_DETAILED-1)) {
		case LOG_OK:{
				color=BLACK;
				str_type='+';
				break;
			};
		case LOG_FROM_SERVER:
			{
				color=BLUE;
				str_type='-';
				break;
			};
		case LOG_WARNING:
			{
				str_type='?';
				color=GREEN;
				break;
			};
		case LOG_ERROR:
			{
				str_type='!';
				color=RED;
				break;
			};
		default:
			str_type=' ';
			color=BLACK;
	};
	if (list){
		GdkColormap *colormap = gtk_widget_get_colormap (MainWindow);
		gdk_color_alloc (colormap, &color);
		gtk_clist_set_foreground(list,row,&color);
	};
	strftime(useful,MAX_LEN,"%T %d %b %Y ",&msgtime);
	if (CFG.WITHOUT_FACE)
		printf("%c %s %s\n",str_type,useful,str->body);
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
	if ((type & LOG_DETAILED)  && !CFG.MAIN_LOG_DETAILED) return;
	tLogString *temp=new tLogString(str,len,type);
	temp->time=time(NULL);
	insert(temp);
	add_to_list();
	Size+=len;
};

void tMLog::add(char *str,int type) {
	if ((type & LOG_DETAILED)  && !CFG.MAIN_LOG_DETAILED) return;	
	int len=strlen(str);
	tLogString *ins=new tLogString(str,len,type);
	insert(ins);
	add_to_list();
	Size+=len;
};

void tMLog::init_list(GtkCList *clist) {
	list=clist;
	gtk_signal_connect(GTK_OBJECT(list),"select_row",GTK_SIGNAL_FUNC(main_log_event_handler),this);
	gtk_signal_connect(GTK_OBJECT(list),"event",GTK_SIGNAL_FUNC(main_log_event_handler2),this);
	/* Initing popup menu
	 */
	popup_menu=gtk_menu_new();
	open_row_item=gtk_menu_item_new_with_label(_("Open a row"));
	gtk_menu_append(GTK_MENU(popup_menu),open_row_item);
	gtk_signal_connect(GTK_OBJECT(open_row_item),"activate",GTK_SIGNAL_FUNC(list_menu_open_row_main_log),this);
	clear_item=gtk_menu_item_new_with_label(_("Clear log"));
	gtk_menu_append(GTK_MENU(popup_menu),clear_item);
	gtk_signal_connect(GTK_OBJECT(clear_item),"activate",GTK_SIGNAL_FUNC(list_menu_clear_main_log),this);
	gtk_widget_show_all(popup_menu);
};

int tMLog::popup(GdkEventButton *event) {
	if (!list) return FALSE;
	if (event && event->type==GDK_BUTTON_PRESS && event->button==3) {
		int row;
		gtk_clist_unselect_all(list);
		if (gtk_clist_get_selection_info(list,int(event->x),int(event->y),&row,NULL)) {
			gtk_clist_select_row(list,row,-1);
		};
		GList *select=((GtkCList *)list)->selection;
		if (select)
			gtk_widget_set_sensitive(open_row_item,TRUE);
		else
			gtk_widget_set_sensitive(open_row_item,FALSE);
		if (First)
			gtk_widget_set_sensitive(clear_item,TRUE);
		else
			gtk_widget_set_sensitive(clear_item,FALSE);
		gtk_menu_popup(GTK_MENU(popup_menu),NULL,NULL,NULL,NULL,event->button,event->time);
		return TRUE;
	};
	return FALSE;
};

void tMLog::open_row(int row) {
	if (!list) return;
	char data[MAX_LEN];
	sprintf(data,_("row number %i of main log"),row+1);
	char *text;
	gtk_clist_get_text(list,row,ML_COL_STRING,&text);
	if (!string)
		string=new tStringDialog;
	string->init(text,data);
};

void tMLog::open_selected_row() {
	if (!list) return;
	GList *select=((GtkCList *)list)->selection;
	if (select) {
		open_row(GPOINTER_TO_INT(select->data));
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
	int len=strlen(str);
	tLogString *ins=new tLogString(str,len,LOG_FROM_SERVER);
	insert(ins);
	add_to_list();
	Size+=len;
};

void tMLog::myprintf(int type,char *fmt,...){
	char str[MAX_LEN+1];
	char *cur=str;
	va_list ap;
	va_start(ap,fmt);
	*cur=0;
	while (*fmt && cur-str<MAX_LEN){
		if (*fmt=='%'){
			fmt+=1;
			switch(*fmt){
			case 's':{
				char *s=va_arg(ap,char *);
				g_snprintf(cur,MAX_LEN-(cur-str),"%s",s);
				break;
			};
			case 'z':{
				tDownload *temp=va_arg(ap,tDownload *);
				char *s=temp->info->url();
				g_snprintf(cur,MAX_LEN-(cur-str),"%s",s);
				delete(s);
				break;
			};
			case 'i':{
				g_snprintf(cur,MAX_LEN-(cur-str),"%i",va_arg(ap,int));
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
	if (!CFG.WITHOUT_FACE)
		gtk_clist_remove(list,0);
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
	if (list) gtk_clist_freeze(GTK_CLIST(list));
	tStringList::done();
};

tMLog::~tMLog() {
	done();
	if (string) delete(string);
};
