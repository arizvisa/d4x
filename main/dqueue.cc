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
#include "dqueue.h"
#include "dbc.h"
#include "face/list.h"
#include "face/lod.h"
#include "var.h"
#include "savedvar.h"

int RUN_OR_WAIT_DOWNLOADS=0;

int d4x_run_or_wait_downloads(){
	return(RUN_OR_WAIT_DOWNLOADS);
};

d4xDownloadQueue::d4xDownloadQueue(){
	MAX_ACTIVE=5;
	TIME_FORMAT=NICE_DEC_DIGITALS=0;
	AUTODEL_COMPLETED=AUTODEL_FAILED=0;
	queues[DL_ALONE]=NULL;
	for (int i=DL_ALONE+1;i<DL_TEMP;i++){
		queues[i]=new tDList(i);
		queues[i]->init(0);
		queues[i]->PAPA=this;
	};
	parent=NULL;
	save_path.set(CFG.GLOBAL_SAVE_PATH);
	IamDefault=0;
	qv.ListOfDownloads=NULL;
};

void d4xDownloadQueue::done(){
	for (int i=DL_ALONE+1;i<DL_TEMP;i++)
		queues[i]->done();
};

int d4xDownloadQueue::count(int q=DL_ALONE){
	if (q!=DL_ALONE){
		return(queues[q]->count());
	};
	int num=0;
	for (int i=DL_ALONE+1;i<DL_TEMP;i++)
		num+=queues[i]->count();
	return num;
};

void d4xDownloadQueue::reset_empty_func(){
	queues[DL_COMPLETE]->set_empty_func(NULL,NULL);
	queues[DL_STOP]->set_empty_func(NULL,NULL);
};

void d4xDownloadQueue::set_defaults(){
	queues[DL_COMPLETE]->set_empty_func(main_menu_completed_empty,main_menu_completed_nonempty);
	queues[DL_STOP]->set_empty_func(main_menu_failed_empty,main_menu_failed_nonempty);
};

void d4xDownloadQueue::init_pixmaps(){
	queues[DL_COMPLETE]->init_pixmap(PIX_COMPLETE);
	queues[DL_RUN]->init_pixmap(PIX_RUN_PART);
	queues[DL_WAIT]->init_pixmap(PIX_WAIT);
	queues[DL_PAUSE]->init_pixmap(PIX_PAUSE);
	queues[DL_STOP]->init_pixmap(PIX_STOP);
	queues[DL_STOPWAIT]->init_pixmap(PIX_STOP_WAIT);
};

int d4xDownloadQueue::current_run(char *host,int port){
	DBC_RETVAL_IF_FAIL(host!=NULL,0);

	tDownload *temp=queues[DL_RUN]->last();
	int count=0;
	while(temp) {
		if (strcasecmp(host,temp->info->host.get())==0 &&
		    port==temp->info->port){
			count+=1;	
		if (temp->split){
				tDownload *temp1=temp->split->next_part;
				while (temp1){
					count+=1;
					temp1=temp1->split->next_part;
				};
			};
		};
		temp=queues[DL_RUN]->next();
	};
	return count;
};

void d4xDownloadQueue::replace_list(tDList *list,int q){
	if (queues[q]) delete(queues[q]);
	queues[q]=list;
};

tDownload *d4xDownloadQueue::first(int q){
	return(queues[q]->first());
};

tDownload *d4xDownloadQueue::last(int q){
	return(queues[q]->last());
};

void d4xDownloadQueue::forward(tDownload *what){
	if (what->myowner)
		what->myowner->forward(what);
};

void d4xDownloadQueue::backward(tDownload *what){
	if (what->myowner)
		what->myowner->backward(what);
};

void d4xDownloadQueue::insert_before(tDownload *what,tDownload *where){
	if (where->myowner){
		where->myowner->insert_before(what,where);
	}else{
		printf("Strange error in d4xDownloadQueue::insert_before\n");
	};
};

void d4xDownloadQueue::add(tDownload *what,int where=DL_WAIT){
	if (where==DL_WAIT || where==DL_RUN)
		RUN_OR_WAIT_DOWNLOADS+=1;
	queues[where]->insert(what);
	update();
};

void d4xDownloadQueue::del(tDownload *what){
	if (what->myowner){
		if (what->myowner->get_key()==DL_WAIT ||
		    what->myowner->get_key()==DL_RUN)
			RUN_OR_WAIT_DOWNLOADS-=1;
		what->myowner->del(what);
	};
	update();
};

void d4xDownloadQueue::subq_add(d4xDownloadQueue *what){
	what->parent=this;
	child.insert(what);
};

void d4xDownloadQueue::subq_del(d4xDownloadQueue *what){
	child.del(what);
	what->parent=NULL;
};

void d4xDownloadQueue::update(){
	if (!CFG.WITHOUT_FACE)
		D4X_QVT->update(this);
};

tDList *d4xDownloadQueue::get_queue(int q){
	if (q>=0 && q<DL_TEMP)
		return(queues[q]);
	return(NULL);
};

d4xDownloadQueue::~d4xDownloadQueue(){
	for (int i=DL_ALONE+1;i<DL_TEMP;i++)
		delete(queues[i]);	
};

void d4xDownloadQueue::save_to_config_list(int fd){
	if (CFG.WITHOUT_FACE){
		d4xWFNode *node=(d4xWFNode *)(qv.ListOfDownloadsWF.first());
		while (node) {
			if (node->dwn)
				node->dwn->save_to_config(fd);
			node=(d4xWFNode *)(node->prev);
		};
	}else{
		int i=0;
		tDownload *temp=qv.get_download(i);
		while (temp) {
			i++;
			temp->save_to_config(fd);
			temp=qv.get_download(i);
		};
	};
};

void d4xDownloadQueue::save_to_config(int fd){
	f_wstr_lf(fd,"Queue:");
	write_named_integer(fd,"Max_active:",MAX_ACTIVE);
	write_named_string(fd,"Name:",name.get());
	write_named_string(fd,"SavePath:",save_path.get());
	if (D4X_QUEUE==this)
		f_wstr_lf(fd,"IamDefault");
	if (AUTODEL_COMPLETED)
		write_named_integer(fd,"Delcompleted:",AUTODEL_COMPLETED);
	if (AUTODEL_FAILED)
		write_named_integer(fd,"Delfailed:",AUTODEL_FAILED);
	if (TIME_FORMAT)
		write_named_integer(fd,"Timeformat:",TIME_FORMAT);
	if (NICE_DEC_DIGITALS)
		write_named_integer(fd,"Decformat:",NICE_DEC_DIGITALS);
	qv.save_to_config(fd);
	save_to_config_list(fd);
	d4xDownloadQueue *q=(d4xDownloadQueue *)child.first();
	while(q){
		q->save_to_config(fd);
		q=(d4xDownloadQueue *)(q->prev);
	};
	f_wstr_lf(fd,"QueueEnd");
};

int d4xDownloadQueue::load_from_config_list(int fd){
	tDList *q=queues[DL_RUN];
	queues[DL_RUN]=queues[DL_WAIT];
	int a=load_from_config(fd);
	queues[DL_RUN]=q;
	return(a);
};

int d4xDownloadQueue::load_from_config(int fd){
	tSavedVar table_of_fields[]={
		{"Name:",	SV_TYPE_PSTR,	&name},
		{"SavePath:",	SV_TYPE_PSTR,	&save_path},
		{"Max_active:",	SV_TYPE_INT,	&MAX_ACTIVE},
		{"Delcompleted:",	SV_TYPE_INT,	&AUTODEL_COMPLETED},
		{"Delfailed:",	SV_TYPE_INT,	&AUTODEL_FAILED},
		{"Timeformat:",	SV_TYPE_INT,	&TIME_FORMAT},
		{"Decformat:",	SV_TYPE_INT,	&NICE_DEC_DIGITALS},
		{"Download:",	SV_TYPE_QDOWNLOAD, this},
		{"Queue:",	SV_TYPE_QUEUE, &child},
		{"QV:",		SV_TYPE_QV,	&qv},
		{"IamDefault",	SV_TYPE_TMP,	NULL},
		{"QueueEnd",	SV_TYPE_END,	NULL}
	};
	char buf[MAX_LEN];
	while(f_rstr(fd,buf,MAX_LEN)>0){
		unsigned int i;
		for (i=0;i<sizeof(table_of_fields)/sizeof(tSavedVar);i++){
			if (equal_uncase(buf,table_of_fields[i].name)){
				switch(table_of_fields[i].type){
				case SV_TYPE_END:{
					if (qv.ListOfDownloads==NULL && CFG.WITHOUT_FACE==0){
						qv.init();
						init_pixmaps();
					};
					return(0);
				};
				case SV_TYPE_TMP:{
					IamDefault=1;
					break;
				};
				default:{
					if (sv_parse_file(fd,&(table_of_fields[i]),buf,MAX_LEN))
						return(-1);
				};
				};
			};
		};
	};
	return -1;
};

void d4xDownloadQueue::inherit_settings(d4xDownloadQueue *papa){
	MAX_ACTIVE=papa->MAX_ACTIVE;
	TIME_FORMAT=papa->TIME_FORMAT;
	NICE_DEC_DIGITALS=papa->NICE_DEC_DIGITALS;
	AUTODEL_COMPLETED=papa->AUTODEL_COMPLETED;
	AUTODEL_FAILED=papa->AUTODEL_FAILED;
	save_path.set(papa->save_path.get());
	qv.inherit_settings(&(papa->qv));
};

/**********************************************************/

d4xDUpdate::d4xDUpdate():first(NULL),last(NULL){
	pthread_mutex_init(&mylock,NULL);
	pthread_mutex_init(&mylock_s,NULL);
};

void d4xDUpdate::add_without_lock(tDownload *dwn){
	if (dwn->split && dwn->split->grandparent)
		dwn=dwn->split->grandparent;
	if (dwn->next2update==NULL){
		if (last)
			last->next2update=dwn;
		else
			first=dwn;
		last=dwn;
	};
};

void d4xDUpdate::add(tDownload *dwn){
	lock();
	add_without_lock(dwn);
	unlock();
};

void d4xDUpdate::add(tDownload *dwn,int status){
	lock_s();
	if (dwn->next2stop==NULL){
		if (last_s)
			last_s->next2stop=dwn;
		else
			first_s=dwn;
		last_s=dwn;
	};
	dwn->status=status;
	unlock_s();
};

void d4xDUpdate::del(){
	if (first){
		tDownload *a=first;
		if ((first=first->next2update)==NULL)
			last=NULL;
		a->next2update=NULL;
	};
};

void d4xDUpdate::del_s(){
	if (first_s){
		tDownload *a=first_s;
		if ((first_s=first_s->next2stop)==NULL)
			last_s=NULL;
		a->next2stop=NULL;
	};
};

void d4xDUpdate::lock(){
	pthread_mutex_lock(&mylock);
};

void d4xDUpdate::unlock(){
	pthread_mutex_unlock(&mylock);
};

void d4xDUpdate::lock_s(){
	pthread_mutex_lock(&mylock_s);
};

void d4xDUpdate::unlock_s(){
	pthread_mutex_unlock(&mylock_s);
};

void d4xDUpdate::update(tDList *dl){
	lock();
	tDownload *dwn=dl->first();
	while(dwn){
		add_without_lock(dwn);
		dwn=(tDownload *)(dwn->prev);
	};
	unlock();
};

d4xDUpdate::~d4xDUpdate(){
	pthread_mutex_destroy(&mylock);
	pthread_mutex_destroy(&mylock_s);
};

