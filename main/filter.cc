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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "dbc.h"
#include "savedvar.h"
#include "var.h"
#include "filter.h"
#include "face/filtrgui.h"
#include "face/mywidget.h"
#include "face/edit.h"
#include "signal.h"

d4xFiltersTree *FILTERS_DB=NULL;

d4xRule::d4xRule(){
	proto=include=0;
};

void d4xRule::print(){
};

int d4xRule::match(tAddr *addr){
	if (proto && addr->proto!=proto)
		return(0);
	if (file.get() &&
	    addr->file.get() &&
	    !check_mask2_uncase(addr->file.get(),file.get()))
		return(0);
	if (host.get() &&
	    addr->host.get() &&
	    !check_mask2(addr->host.get(),host.get()))
		return(0);
	if (path.get() &&
	    addr->path.get() &&
	    !check_mask2(addr->path.get(),path.get()))
		return(0);
	if (tag.get() && addr->tag.get()  &&
	    equal_uncase(addr->tag.get(),tag.get())==0){
		return(0);
	};
	if (params.get() &&
	    (addr->params.get()==NULL ||
	     !check_mask2(addr->params.get(),params.get())))
		return(0);
	return(1);
};

void d4xRule::save(int fd){
	f_wstr_lf(fd,"d4xRule:");
	if (include)
		write_named_integer(fd,"inc:",include);
	if (proto)
		write_named_integer(fd,"proto:",proto);
	if (host.get())
		write_named_string(fd,"host:",host.get());
	if (path.get()){
		if (*(path.get())=='/')
			write_named_string(fd,"path:",path.get()+1);
		else
			write_named_string(fd,"path:",path.get());
	};
	if (tag.get())
		write_named_string(fd,"tag:",tag.get());
	if (file.get())
		write_named_string(fd,"file:",file.get());
	if (params.get())
		write_named_string(fd,"params:",params.get());
	f_wstr_lf(fd,"d4xRule_end");
};

int d4xRule::load(int fd){
	tSavedVar table_of_fields[]={
		{"inc:",	SV_TYPE_INT,	&include},
		{"proto:",	SV_TYPE_INT,	&proto},
		{"host:",	SV_TYPE_PSTR,	&(host)},
		{"path:",	SV_TYPE_PSTR,	&(path)},
		{"file:",   	SV_TYPE_PSTR,	&(file)},
		{"tag:",   	SV_TYPE_PSTR,	&(tag)},
		{"params:",   	SV_TYPE_PSTR,	&(params)},
		{"d4xRule_end",SV_TYPE_END,	NULL}
	};
	char buf[MAX_LEN];
	while(f_rstr(fd,buf,MAX_LEN)>0){
		unsigned int i;
		for (i=0;i<sizeof(table_of_fields)/sizeof(tSavedVar);i++){
			if (equal_uncase(buf,table_of_fields[i].name)){
				if (table_of_fields[i].type==SV_TYPE_END){
					return(0);
				}else{
					if (sv_parse_file(fd,&(table_of_fields[i]),buf,MAX_LEN))
						return(-1);
				};
			};
		};
	};
	return -1;	
};

d4xRule::~d4xRule(){
};
/*********************************************************/
d4xFilter::d4xFilter():tQueue(){
	default_inc=1;
	refcount=0;
	pthread_mutex_init(&my_mutex,NULL);
};

void d4xFilter::lock(){
	pthread_mutex_lock(&my_mutex);
};

void d4xFilter::unlock(){
	pthread_mutex_unlock(&my_mutex);
};

void d4xFilter::ref(){
	refcount+=1;
};

void d4xFilter::unref(){
	refcount-=1;
	if (refcount<=0) delete(this);
};

void d4xFilter::insert_before(tNode *node,tNode *where){
	lock();
	tQueue::insert_before(node,where);
	unlock();
};

void d4xFilter::insert(tNode *node){
	lock();
	tQueue::insert(node);
	unlock();
};

void d4xFilter::del(tNode *node){
	lock();
	tQueue::del(node);
	unlock();
};

int d4xFilter::match(tAddr *addr){
	DBC_RETVAL_IF_FAIL(addr!=NULL,0);
	download_set_block(1);
	lock();
	d4xRule *rule=(d4xRule *)First;
	while(rule){
		if (rule->match(addr)){
			unlock();
			download_set_block(0);
			return(rule->include);
		};
		rule=(d4xRule *)(rule->prev);
	};
	unlock();
	download_set_block(0);
	return(default_inc);
};

void d4xFilter::print(d4xFilterEdit *edit){
	d4xRule *rule=(d4xRule *)First;
	while(rule){
		d4x_filter_edit_add_rule(edit,rule);
		rule=(d4xRule *)(rule->prev);
	};
};

void d4xFilter::save(int fd){
	f_wstr_lf(fd,"d4xFilter:");
	if (!default_inc)
		write_named_integer(fd,"inc:",default_inc);
	if (name.get())
		write_named_string(fd,"name:",name.get());
	d4xRule *rule=(d4xRule *)First;
	while(rule){
		rule->save(fd);
		rule=(d4xRule *)(rule->prev);
	};
	f_wstr_lf(fd,"d4xFilter_end");
};



int d4xFilter::load(int fd){
	tSavedVar table_of_fields[]={
		{"d4xRule:",	SV_TYPE_RULE,	this},
		{"inc:",	SV_TYPE_INT,	&default_inc},
		{"name:",	SV_TYPE_PSTR,	&(name)},
		{"d4xFilter_end",SV_TYPE_END,	NULL}
	};
	char buf[MAX_LEN];
	while(f_rstr(fd,buf,MAX_LEN)>0){
		unsigned int i;
		for (i=0;i<sizeof(table_of_fields)/sizeof(tSavedVar);i++){
			if (equal_uncase(buf,table_of_fields[i].name)){
				if (table_of_fields[i].type==SV_TYPE_END){
					return(0);
				}else{
					if (sv_parse_file(fd,&(table_of_fields[i]),buf,MAX_LEN))
						return(-1);
				};
			};
		};
	};
	return -1;	
};

d4xFilter::~d4xFilter(){
	pthread_mutex_destroy(&my_mutex);
};

/*********************************************************/

d4xFNode::d4xFNode(){
	filter=NULL;
};

d4xFNode::~d4xFNode(){
	if (filter)
		filter->unref();
};

int d4xFNode::cmp(tAbstractSortNode *what){
	d4xFNode *node=(d4xFNode *)what;
	char *a=node->filter->name.get();
	char *b=filter->name.get();
	if (a==NULL || b==NULL) return(0);
	return(strcmp(a,b));
};

d4xFiltersTree::d4xFiltersTree():tAbstractSortTree(){
	pthread_mutex_init(&my_mutex,NULL);
};

void d4xFiltersTree::lock(){
	pthread_mutex_lock(&my_mutex);
};

void d4xFiltersTree::unlock(){
	pthread_mutex_unlock(&my_mutex);
};

void d4xFiltersTree::add(tAbstractSortNode *what){
	lock();
	tAbstractSortTree::add(what);
	unlock();
};

void d4xFiltersTree::del(tAbstractSortNode *what){
	lock();
	tAbstractSortTree::del(what);
	unlock();
};

d4xFilter *d4xFiltersTree::find(char *name){
	lock();
	d4xFNode *tmp=new d4xFNode;
	tmp->filter=new d4xFilter;
	tmp->filter->name.set(name);
	d4xFNode *result=(d4xFNode *)(tAbstractSortTree::find(tmp));
	delete(tmp);
	if (result){
		result->filter->ref();
		unlock();
		return(result->filter);
	};
	unlock();
	return(NULL);
};

tAbstractSortNode *d4xFiltersTree::max(){
	lock();
	tAbstractSortNode *tmp=tAbstractSortTree::max();
	unlock();
	return(tmp);
};

d4xFiltersTree::~d4xFiltersTree(){
	while (Top){
		d4xFNode *node=(d4xFNode *)Top;
		del(Top);
		delete(node);
	};
	pthread_mutex_destroy(&my_mutex);
};

void d4xFiltersTree::print_recurse(d4xFilterSel *sel,d4xFNode *node){
	if (node){
		print_recurse(sel,(d4xFNode *)(node->less));
		d4x_filter_sel_add(sel,node);
		print_recurse(sel,(d4xFNode *)(node->more));
	};
};

void d4xFiltersTree::print_recurse(d4xFNode *node){
	if (node){
		print_recurse((d4xFNode *)(node->less));
		d4x_filters_window_add(node);
		print_recurse((d4xFNode *)(node->more));
	};
};

void d4xFiltersTree::print(){
	d4xFNode *tmp=(d4xFNode*)Top;
	print_recurse(tmp);
};

void d4xFiltersTree::print(d4xFilterSel *sel){
	d4xFNode *tmp=(d4xFNode*)Top;
	print_recurse(sel,tmp);
};

void d4xFiltersTree::save_recurse(int fd,d4xFNode *what){
	if (what){
		what->filter->save(fd);
		save_recurse(fd,(d4xFNode*)(what->less));
		save_recurse(fd,(d4xFNode*)(what->more));
	};
};

void d4xFiltersTree::save(int fd=-1){
	d4xFNode *tmp=(d4xFNode*)Top;
	save_recurse(fd,tmp);
};

int d4xFiltersTree::load(int fd){

	tSavedVar table_of_fields[]={
		{"d4xFilter:",	SV_TYPE_FILTER,	this},
		{"d4xFilters_end:",SV_TYPE_END,	NULL}
	};
	char buf[MAX_LEN];
	while(f_rstr(fd,buf,MAX_LEN)>0){
		unsigned int i;
		for (i=0;i<sizeof(table_of_fields)/sizeof(tSavedVar);i++){
			if (equal_uncase(buf,table_of_fields[i].name)){
				if (table_of_fields[i].type==SV_TYPE_END){
					return(0);
				}else{
					if (sv_parse_file(fd,&(table_of_fields[i]),buf,MAX_LEN))
						return(-1);
				};
			};
		};
	};
	return -1;	
};

void d4xFiltersTree::load_from_ntrc(){
	if (!HOME_VARIABLE)
		return;
	char *path=sum_strings(HOME_VARIABLE,"/",CFG_DIR,"/","Filters",NULL);
	int fd=open(path,O_RDONLY,S_IRUSR | S_IWUSR);
	delete[] path;
	if (fd>=0){
		load(fd);
		close(fd);
	};
};

void d4xFiltersTree::save_to_ntrc(){
	if (!HOME_VARIABLE)
		return;
	char *path=sum_strings(HOME_VARIABLE,"/",CFG_DIR,"/","Filters",NULL);
	int fd=open(path,O_TRUNC | O_CREAT |O_RDWR,S_IRUSR | S_IWUSR);
	delete[] path;
	if (fd>=0){
		save(fd);
		close(fd);
	};
};
