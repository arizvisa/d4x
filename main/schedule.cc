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
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include "schedule.h"
#include "dbc.h"
#include "locstr.h"
#include "main.h"
#include "savedvar.h"
#include "var.h"
#include "face/list.h"
#include "face/fsched.h"
#include "savelog.h"

d4xScheduler *MainScheduler=(d4xScheduler *)NULL;

d4xSchedAction::~d4xSchedAction(){
};

int d4xSchedAction::load(int fd){
	tSavedVar table_of_fields[]={
		{"start_time",	SV_TYPE_TIME,	&start_time},
		{"period",	SV_TYPE_TIME,	&period},
		{"retries",	SV_TYPE_INT,	&retries},
		{"end.",	SV_TYPE_END,	NULL}
	};
	char buf[MAX_LEN];
	while(f_rstr(fd,buf,MAX_LEN)>0){
		unsigned int i;
		for (i=0;i<sizeof(table_of_fields)/sizeof(tSavedVar);i++){
			if (equal_uncase(buf,table_of_fields[i].name)){
				if (table_of_fields[i].type==SV_TYPE_END)
					return(0);
				else{
					if (sv_parse_file(fd,&(table_of_fields[i]),buf,MAX_LEN))
						return(-1);
				};
					
			};
		};
	};
	return(-1);
};

int d4xSchedAction::save(int fd){
	if (write_named_time(fd,"start_time",start_time))
		return(-1);
	if (write_named_time(fd,"period",period))
		return(-1);
	if (write_named_integer(fd,"retries",retries))
		return(-1);
	if (f_wstr_lf(fd,"end.")<=0)
		return(-1);
	return(0);
};

/*------------------------------------------------------------*/
int d4xSASpeed::type(){
	return(SACT_SET_SPEED);
};

int d4xSASpeed::load(int fd){
	tSavedVar table_of_fields[]={
		{"start_time",	SV_TYPE_TIME,	&start_time},
		{"period",	SV_TYPE_TIME,	&period},
		{"retries",	SV_TYPE_INT,	&retries},
		{"speed",	SV_TYPE_INT,	&speed},
		{"end.",	SV_TYPE_END,	NULL}
	};
	char buf[MAX_LEN];
	while(f_rstr(fd,buf,MAX_LEN)>0){
		unsigned int i;
		for (i=0;i<sizeof(table_of_fields)/sizeof(tSavedVar);i++){
			if (equal_uncase(buf,table_of_fields[i].name)){
				if (table_of_fields[i].type==SV_TYPE_END)
					return(0);
				else{
					if (sv_parse_file(fd,&(table_of_fields[i]),buf,MAX_LEN))
						return(-1);
				};
					
			};
		};
	};
	return(-1);
};

int d4xSASpeed::save(int fd){
	if (f_wstr_lf(fd,"d4x_sa_speed:")<=0)
		return(-1);
	if (write_named_integer(fd,"speed",speed))
		return(-1);	
	return(d4xSchedAction::save(fd));
};

void d4xSASpeed::run(tMain *papa){
	papa->set_speed(speed);
};
/*------------------------------------------------------------*/

int d4xSAPopup::type(){
	return(SACT_POPUP_WINDOW);
};

int d4xSAPopup::save(int fd){
	if (f_wstr_lf(fd,"d4x_sa_popup:")<=0)
		return(-1);
	return(d4xSchedAction::save(fd));
};

void d4xSAPopup::run(tMain *papa){
	main_window_popup();
};
/*-----------------------------------------------------------*/
int d4xSAExit::type(){
	return(SACT_EXIT);
};

int d4xSAExit::save(int fd){
	if (f_wstr_lf(fd,"d4x_sa_exit:")<=0)
		return(-1);
	return(d4xSchedAction::save(fd));
};

void d4xSAExit::run(tMain *papa){
	//hehe nothing to do here :-)
};
/*------------------------------------------------------------*/
int d4xSADelCompleted::type(){
	return(SACT_DEL_COMPLETED);
};

int d4xSADelCompleted::save(int fd){
	if (f_wstr_lf(fd,"d4x_sa_del_completed:")<=0)
		return(-1);
	return(d4xSchedAction::save(fd));
};

void d4xSADelCompleted::run(tMain *papa){
        papa->del_completed();
};
/*------------------------------------------------------------*/
int d4xSADelFailed::type(){
	return(SACT_DEL_FAILED);
};

int d4xSADelFailed::save(int fd){
	if (f_wstr_lf(fd,"d4x_sa_del_failed:")<=0)
		return(-1);
	return(d4xSchedAction::save(fd));
};

void d4xSADelFailed::run(tMain *papa){
	papa->del_fataled();
};
/*------------------------------------------------------------*/
d4xSAAddDownload::d4xSAAddDownload(){
	dwn=NULL;
};

int d4xSAAddDownload::type(){
	return(SACT_ADD_DOWNLOAD);
};

int d4xSAAddDownload::load(int fd){
	tSavedVar table_of_fields[]={
		{"start_time",	SV_TYPE_TIME,	&start_time},
		{"period",	SV_TYPE_TIME,	&period},
		{"retries",	SV_TYPE_INT,	&retries},
		{"Download:",	SV_TYPE_DOWNLOAD, &dwn},
		{"end.",	SV_TYPE_END,	NULL}
	};
	char buf[MAX_LEN];
	while(f_rstr(fd,buf,MAX_LEN)>0){
		unsigned int i;
		for (i=0;i<sizeof(table_of_fields)/sizeof(tSavedVar);i++){
			if (equal_uncase(buf,table_of_fields[i].name)){
				if (table_of_fields[i].type==SV_TYPE_END)
					return(0);
				else{
					if (sv_parse_file(fd,&(table_of_fields[i]),buf,MAX_LEN))
						return(-1);
				};
					
			};
		};
	};
	return(-1);
};

int d4xSAAddDownload::save(int fd){
	if (f_wstr_lf(fd,"d4x_sa_add_download:")<=0)
		return(-1);
	if (dwn) dwn->save_to_config(fd);
	return(d4xSchedAction::save(fd));
};

void d4xSAAddDownload::run(tMain *papa){
	if (dwn){
		tDownload *dwn_copy=new tDownload;
		dwn_copy->copy(dwn);
		if (papa->add_downloading(dwn_copy)) delete(dwn_copy);
	};
};

d4xSAAddDownload::~d4xSAAddDownload(){
	if (dwn) delete(dwn);
};
/*------------------------------------------------------------*/
int d4xSAUrl::load(int fd){
	tSavedVar table_of_fields[]={
		{"start_time",	SV_TYPE_TIME,	&start_time},
		{"period",	SV_TYPE_TIME,	&period},
		{"retries",	SV_TYPE_INT,	&retries},
		{"URL:",	SV_TYPE_URL,	&url},
		{"end.",	SV_TYPE_END,	NULL}
	};
	char buf[MAX_LEN];
	while(f_rstr(fd,buf,MAX_LEN)>0){
		unsigned int i;
		for (i=0;i<sizeof(table_of_fields)/sizeof(tSavedVar);i++){
			if (equal_uncase(buf,table_of_fields[i].name)){
				if (table_of_fields[i].type==SV_TYPE_END)
					return(0);
				else{
					if (sv_parse_file(fd,&(table_of_fields[i]),buf,MAX_LEN))
						return(-1);
				};
			};
		};
	};
	return(-1);
};

int d4xSAUrl::save(int fd){
	if (url) url->save_to_config(fd);
	return(d4xSchedAction::save(fd));	
};

//  working with downloads

d4xSADelDownload::d4xSADelDownload():d4xSAUrl(){
};

int d4xSADelDownload::type(){
	return(SACT_DELETE_DOWNLOAD);
};

int d4xSADelDownload::save(int fd){
	if (f_wstr_lf(fd,"d4x_sa_del_download:")<=0)
		return(-1);
	return(d4xSAUrl::save(fd));
};

void d4xSADelDownload::run(tMain *papa){
	if (url) papa->delete_download_url(url);
};
/*------------------------------------------------------------*/
d4xSADelIfCompleted::d4xSADelIfCompleted():d4xSAUrl(){
};

int d4xSADelIfCompleted::type(){
	return(SACT_DEL_IF_COMPLETED);
};

int d4xSADelIfCompleted::save(int fd){
	if (f_wstr_lf(fd,"d4x_sa_del_if_completed:")<=0)
		return(-1);
	return(d4xSAUrl::save(fd));
};

void d4xSADelIfCompleted::run(tMain *papa){
	if (url){
		tDownload *tmp=papa->find_url(url);
		if (tmp && tmp->owner()==DL_COMPLETE)
			papa->delete_download(tmp);
	};
};
/*------------------------------------------------------------*/
d4xSARunDownload::d4xSARunDownload():d4xSAUrl(){
};

int d4xSARunDownload::type(){
	return(SACT_RUN_DOWNLOAD);
};

int d4xSARunDownload::save(int fd){
	if (f_wstr_lf(fd,"d4x_sa_run_download:")<=0)
		return(-1);
	return(d4xSAUrl::save(fd));
};

void d4xSARunDownload::run(tMain *papa){
	if (url) papa->continue_download_url(url);
};
/*------------------------------------------------------------*/
d4xSAStopDownload::d4xSAStopDownload():d4xSAUrl(){
};

int d4xSAStopDownload::type(){
	return(SACT_PAUSE_DOWNLOAD);
};

int d4xSAStopDownload::save(int fd){
	if (f_wstr_lf(fd,"d4x_sa_stop_download:")<=0)
		return(-1);
	return(d4xSAUrl::save(fd));
};

void d4xSAStopDownload::run(tMain *papa){
	if (url) papa->stop_download_url(url);
};
/*------------------------------------------------------------*/

int d4xSASaveList::type(){
	return(SACT_SAVE_LIST);
};

int d4xSASaveList::load(int fd){
	tSavedVar table_of_fields[]={
		{"start_time",	SV_TYPE_TIME,	&start_time},
		{"period",	SV_TYPE_TIME,	&period},
		{"retries",	SV_TYPE_INT,	&retries},
		{"path",	SV_TYPE_PSTR,	&path},
		{"end.",	SV_TYPE_END,	NULL}
	};
	char buf[MAX_LEN];
	while(f_rstr(fd,buf,MAX_LEN)>0){
		unsigned int i;
		for (i=0;i<sizeof(table_of_fields)/sizeof(tSavedVar);i++){
			if (equal_uncase(buf,table_of_fields[i].name)){
				if (table_of_fields[i].type==SV_TYPE_END)
					return(0);
				else{
					if (sv_parse_file(fd,&(table_of_fields[i]),buf,MAX_LEN))
						return(-1);
				};
					
			};
		};
	};
	return(-1);
};

int d4xSASaveList::save(int fd){
	if (f_wstr_lf(fd,"d4x_sa_save_list:")<=0)
		return(-1);
	if (path.get())
		write_named_string(fd,"path",path.get());
	return(d4xSchedAction::save(fd));
};

void d4xSASaveList::run(tMain *papa){
	if (path.get())
		save_list_to_file(path.get());
};
/*------------------------------------------------------------*/
int d4xSAExecute::type(){
	return(SACT_EXECUTE);
};

int d4xSAExecute::load(int fd){
	tSavedVar table_of_fields[]={
		{"start_time",	SV_TYPE_TIME,	&start_time},
		{"period",	SV_TYPE_TIME,	&period},
		{"retries",	SV_TYPE_INT,	&retries},
		{"command",	SV_TYPE_PSTR,	&command},
		{"end.",	SV_TYPE_END,	NULL}
	};
	char buf[MAX_LEN];
	while(f_rstr(fd,buf,MAX_LEN)>0){
		unsigned int i;
		for (i=0;i<sizeof(table_of_fields)/sizeof(tSavedVar);i++){
			if (equal_uncase(buf,table_of_fields[i].name)){
				if (table_of_fields[i].type==SV_TYPE_END)
					return(0);
				else{
					if (sv_parse_file(fd,&(table_of_fields[i]),buf,MAX_LEN))
						return(-1);
				};
					
			};
		};
	};
	return(-1);
};

int d4xSAExecute::save(int fd){
	if (f_wstr_lf(fd,"d4x_sa_execute:")<=0)
		return(-1);
	if (command.get())
		write_named_string(fd,"command",command.get());
	return(d4xSchedAction::save(fd));
};

static void *_sa_exec_run_(void *what){
	d4xSAExecute *act=(d4xSAExecute *)what;
	system(act->command.get());
	pthread_exit(NULL);
	return(NULL);
};

void d4xSAExecute::run(tMain *papa){
	if (command.get()){
		pthread_attr_t attr_p;
		pthread_t thread_id;
		pthread_attr_init(&attr_p);
		pthread_attr_setdetachstate(&attr_p,PTHREAD_CREATE_DETACHED);
		pthread_attr_setscope(&attr_p,PTHREAD_SCOPE_SYSTEM);
		pthread_create(&thread_id,&attr_p,
			       _sa_exec_run_,(void *)this);
	};
};
/**************************************************************/

d4xScheduler::d4xScheduler(){
	FIRST=NULL;
	changed=0;
};

d4xScheduler::~d4xScheduler(){
	while (FIRST){
		d4xSchedAction *tmp=FIRST;
		del_action(tmp);
		delete(tmp);
	};
};

void d4xScheduler::add_scheduled(tDownload *what){
	d4xSAAddDownload *act=new d4xSAAddDownload;
	act->dwn=what;
	act->start_time=what->ScheduleTime;
	act->period=0;
	act->retries=0;
	add_action(act);
};

void d4xScheduler::add_action(d4xSchedAction *act){
	DBC_RETURN_IF_FAIL(act!=NULL);
	changed=1;
	act->lock=0;
	if (FIRST){
		d4xSchedAction *tmp=FIRST;
		while (tmp->next){
			if (tmp->start_time>act->start_time)
				break;
			tmp=tmp->next;
		};
		if (tmp->start_time>act->start_time){
			act->next=tmp;
			if ((act->prev=tmp->prev))
				act->prev->next=act;
			else
				FIRST=act;
			tmp->prev=act;
		}else{
			act->prev=tmp;
			tmp->next=act;
			act->next=NULL;
		};
		d4x_scheduler_insert(act,act->prev);
	}else{
		FIRST=act;
		act->next=act->prev=NULL;
		d4x_scheduler_insert(act,NULL);
	};
};

void d4xScheduler::del_action(d4xSchedAction *act){
	DBC_RETURN_IF_FAIL(act!=NULL);
	changed=1;
	if (act->prev)
		act->prev->next=act->next;
	else
		FIRST=act->next;
	if (act->next)
		act->next->prev=act->prev;
	d4x_scheduler_remove(act);
};

void d4xScheduler::redraw(){
	d4xSchedAction *tmp=FIRST;
	while (tmp){
		d4x_scheduler_insert(tmp,tmp->prev);
		tmp=tmp->next;
	};
};

int d4xScheduler::save(int fd){
	d4xSchedAction *tmp=FIRST;
	while (tmp){
		if (tmp->save(fd))
			return(-1);
		tmp=tmp->next;
	};
	return(0);
};

void d4xScheduler::run(tMain *papa){
	time_t now=time(NULL);
	int a=0; //counter
	d4xSchedAction *tmp=FIRST;
	int exit_flag=0;
	int se=0;
	while (tmp){
		if (now>tmp->start_time){
			d4xSchedAction *next=tmp->next;
			se=1;
			if (tmp->lock==0){ //if no editors opened
				tmp->run(papa);
				del_action(tmp);
				while (tmp->start_time<now && tmp->retries!=0){
					tmp->start_time+=tmp->period;
					if (tmp->retries>0)
						tmp->retries-=1;
				};
				if (tmp->type()==SACT_EXIT)
					exit_flag=1;
				if (tmp->retries==0)
					delete(tmp);
				else
					add_action(tmp);
			};
			tmp=next;
			if (a++>10) break;
		}else
			break;
		if (exit_flag)
			break;
	};
	if (se && changed)
		save();
	if (exit_flag){
		my_main_quit();
//		aa.run_after_quit();
		exit(0);
	};
};

int d4xScheduler::load(int fd){
	char *table_of_fields[]={
		"d4x_sa_speed:",
		"d4x_sa_popup:",
		"d4x_sa_exit:",
		"d4x_sa_del_completed:",
		"d4x_sa_del_failed:",
		"d4x_sa_run_download:",
		"d4x_sa_stop_download:",
		"d4x_sa_del_download:",
		"d4x_sa_del_if_completed:",
		"d4x_sa_add_download:",
		"d4x_sa_save_list:",
		"d4x_sa_execute:"
	};
	char buf[MAX_LEN];
	d4xSchedAction *action=(d4xSchedAction *)NULL;
	while(f_rstr(fd,buf,MAX_LEN)>0){
		unsigned int i;
		for (i=0;i<sizeof(table_of_fields)/sizeof(char *);i++){
			if (equal_uncase(buf,table_of_fields[i])){
				switch(i){
				case SACT_SET_SPEED:
					action=new d4xSASpeed;
					break;
				case SACT_POPUP_WINDOW:
					action=new d4xSAPopup;
					break;
				case SACT_EXIT:
					action=new d4xSAExit;
					break;
				case SACT_DEL_COMPLETED:
					action=new d4xSADelCompleted;
					break;
				case SACT_DEL_FAILED:
					action=new d4xSADelFailed;
					break;
				case SACT_RUN_DOWNLOAD:
					action=new d4xSARunDownload;
					break;
				case SACT_PAUSE_DOWNLOAD:
					action=new d4xSAStopDownload;
					break;
				case SACT_DELETE_DOWNLOAD:
					action=new d4xSADelDownload;
					break;
				case SACT_DEL_IF_COMPLETED:
					action=new d4xSADelIfCompleted;
					break;
				case SACT_ADD_DOWNLOAD:
					action=new d4xSAAddDownload;
					break;
				case SACT_SAVE_LIST:
					action=new d4xSASaveList;
					break;
				case SACT_EXECUTE:
					action=new d4xSAExecute;
					break;
				};
			};
		};
		if (action){
			if (action->load(fd)==0)
				add_action(action);
			else
				delete(action);
			action=NULL;
		};
	};
	return(0);
};

void d4xScheduler::save(){
	if (!HOME_VARIABLE)
		return;
	char *path=sum_strings(HOME_VARIABLE,"/",CFG_DIR,"/","Scheduler",NULL);
	int fd=open(path,O_TRUNC | O_CREAT |O_RDWR,S_IRUSR | S_IWUSR);
	if (fd>=0){
		save(fd);
		close(fd);
	};
	delete[] path;
};

void d4xScheduler::load(){
	if (!HOME_VARIABLE)
		return;
	char *path=sum_strings(HOME_VARIABLE,"/",CFG_DIR,"/","Scheduler",NULL);
	int fd=open(path,O_RDONLY,S_IRUSR | S_IWUSR);
	if (fd>=0){
		load(fd);
		close(fd);
	};
	delete[] path;
};
