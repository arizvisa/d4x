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

#include "dlist.h"
#include "ftpd.h"
#include "locstr.h"
#include "string.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "face/lod.h"
#include "face/edit.h"
#include "var.h"
#include "ntlocale.h"
#include "main.h"
#include "config.h"

extern tMain aa;

void tTriger::reset() {
	old=curent;
};

void tTriger::clear() {
	old=-1;
	curent=0;
};

void tTriger::set(int a ) {
	curent=a;
};

void tTriger::update() {
	old=curent-1;
};

int tTriger::change() {
	return old==curent?0:1;
};


/**********************************************/
tDownload::tDownload() {
	next=prev=NULL;
	who=NULL;
	info=NULL;
	LOG=NULL;
	editor=NULL;
	SaveName=SavePath=NULL;
	config.ftp_recurse_depth=config.http_recurse_depth=1;
	SpeedLimit=NULL;
	finfo.size=-1;
	finfo.type=T_NONE;
	DIR=NULL;
	finfo.perm=S_IWUSR | S_IRUSR;
	Start=Pause=0;
	Percent.clear();
	Attempt.clear();
	Status.clear();
	Size.clear();
	Speed.clear();
	Remain.clear();
	owner=DL_ALONE;
	thread_id=0;
	GTKCListRow=-1;
	NanoSpeed=0;
	DIR=NULL;
	action=ACTION_NONE;
	ScheduleTime=0;
};

void tDownload::clear() {
	if (who) {
		delete(who);
		who=NULL;
	};
};

void tDownload::delete_editor() {
	if (editor)
		delete editor;
};

void tDownload::print() {
	//do nothing
};

void tDownload::set_SavePath(char *what) {
	if (SavePath) delete(SavePath);
	SavePath=copy_string(what);
};

void tDownload::set_SaveName(char *what) {
	if (SaveName) delete SaveName;
	SaveName=copy_string(what);
};

int tDownload::create_file() {
	if (!who) return -1;
	return (who->create_file(SavePath,SaveName));
};

void tDownload::set_date_file() {
	if (!who) return;
	who->set_date_file(SavePath,SaveName);
};

void tDownload::update_trigers() {
	Percent.update();
	Speed.update();
	Status.update();
	Size.update();
	Attempt.update();
	Remain.update();
	finfo.oldtype=finfo.type-1;
};

void tDownload::make_file_visible(){
	if (who)
		who->make_file_visible(SavePath,SaveName);
};

void tDownload::set_default_cfg(){
	config.timeout=CFG.TIME_OUT;
	config.time_for_sleep=CFG.RETRY_TIME_OUT;
	config.number_of_attempts=CFG.MAX_RETRIES;
	config.passive=CFG.FTP_PASSIVE_MODE;
	config.permisions=CFG.FTP_PERMISIONS;
	config.get_date=CFG.GET_DATE;
	config.retry=CFG.RETRY_IF_NOREGET;
	config.ftp_recurse_depth=CFG.FTP_RECURSE_DEPTH;
	config.http_recurse_depth=CFG.HTTP_RECURSE_DEPTH;
	config.rollback=CFG.ROLLBACK;
	config.http_recursing=config.http_recurse_depth==1?0:1;
	config.set_user_agent(CFG.USER_AGENT);
};

char *tDownload::create_new_file_path(){
	if (info->mask==0) 
		return (compose_path(info->get_path(),info->get_file()));
	return(copy_string(info->get_path()));
};

char *tDownload::create_new_save_path(){
	if (info->mask==0) {
		if (SavePath) {
			if (SaveName && strlen(SaveName))
				return(compose_path(SavePath,SaveName));
			else 
				return(compose_path(SavePath,info->get_file()));
		} else {
			if (SaveName && strlen(SaveName))
				return(copy_string(SaveName));
			else
				return(copy_string(info->get_file()));
		};
	};
	return(copy_string(SavePath));
};

void tDownload::convert_list_to_dir() {
	if (who==NULL || info==NULL) {
		return;
	};
	tFtpDownload *tmp=(tFtpDownload *)(who);
	tStringList *dir=tmp->dir();
	if (DIR) {
		DIR->done();
	} else {
		DIR=new tDList(DL_TEMP);
		DIR->init(0);
	};
	if (dir==NULL || dir->first()==NULL) {
		return;
	};
	char *path=create_new_file_path();
	char *savepath=create_new_save_path();
	tString *temp=dir->last();
	tFileInfo *prom=new tFileInfo;
	while (temp) {
		cut_string_list(temp->body,prom,1);
		if (prom->get_name() && !equal(prom->get_name(),".")
		    && !equal(prom->get_name(),"..")
		    && (prom->type!=T_DIR || config.ftp_recurse_depth!=2)
		    && (prom->type==T_DIR || info->mask==0 || check_mask(prom->get_name(),info->get_file()))) {
			tAddr *addrnew=new tAddr;
			tDownload *onenew=new tDownload;
			if (prom->type==T_DIR && info->mask) {
				addrnew->compose_path(path,prom->get_name());
				addrnew->set_file(info->get_file());
				onenew->SavePath=compose_path(savepath,prom->get_name());
//				mkdir(onenew->SavePath,prom->perm);
				addrnew->mask=info->mask;
			} else {
				addrnew->set_path(path);
				addrnew->set_file(prom->get_name());
				onenew->SavePath=copy_string(savepath);
			};
			addrnew->copy_host(info);

			onenew->info=addrnew;

			onenew->config.copy(&config);
			onenew->config.ftp_recurse_depth = config.ftp_recurse_depth ? config.ftp_recurse_depth-1 : 0;
			onenew->config.http_recurse_depth = config.http_recurse_depth;
			if (CFG.RECURSIVE_OPTIMIZE) {
				onenew->finfo.type=prom->type;
				onenew->finfo.size=prom->size;
				onenew->finfo.date=prom->date;
				if (config.permisions) onenew->finfo.perm=prom->perm;
				if (onenew->finfo.type==T_LINK) {
					onenew->finfo.set_body(prom->get_body());
				};
			};
			DIR->insert(onenew);
		};
		dir->del(temp);
		delete(temp);
		temp=dir->last();
		prom->set_name(NULL);
		prom->set_body(NULL);
	};
	delete (prom);
	delete (path);
	delete (savepath);
};


void make_dir_hier(char *path) {
	char *temp=path;
	while (temp) {
		temp=index(temp+1,'/');
		if (temp) *temp=0;
		mkdir(path,S_IRWXU);
		if (temp) *temp='/';
	};
};

void tDownload::convert_list_to_dir2() {
	if (!who) {
		return;
	};
	tStringList *dir=who->dir();
	if (!dir) {
		return;
	};
	if (DIR) {
		DIR->done();
		DIR->init(0);
	} else {
		DIR=new tDList(DL_TEMP);
		DIR->init(0);
	};
	tString *temp=dir->last();
	while (temp) {
		if (!global_url(temp->body) && !begin_string_uncase(temp->body,"javascript:")) {
			tAddr *addrnew=new tAddr;
			tDownload *onenew=new tDownload;
			char *tmp=rindex(temp->body,'/');
			onenew->SavePath=copy_string(SavePath);
			if (tmp) {
				addrnew->set_file(tmp+1);
				*tmp=0;
				if (temp->body[0]=='/')
					addrnew->set_path(temp->body+1);
				else{
					addrnew->compose_path(info->get_path(),temp->body);
				};
				*tmp='/';
			} else {
				addrnew->set_path(info->get_path());
				addrnew->set_file(temp->body);
			};
/*
			if (addrnew->path[0]!='/'){
				tmp=compose_path("/",addrnew->path);
				delete(addrnew->path);
				addrnew->path=tmp;
			};
 */
			addrnew->file_del_sq();
			normalize_path(onenew->SavePath);
			addrnew->copy_host(info);

			onenew->config.http_recursing=1;
			onenew->info=addrnew;

			onenew->config.copy(&config);
			onenew->config.http_recurse_depth = config.http_recurse_depth ? config.http_recurse_depth-1 : 0;
			onenew->config.ftp_recurse_depth = config.ftp_recurse_depth;

			DIR->insert(onenew);
		}else{
			if (begin_string_uncase(temp->body,"http://")){
					tAddr *addrnew=new tAddr(temp->body);
					if (equal(addrnew->get_host(),info->get_host()) && addrnew->port==info->port){
						tDownload *onenew=new tDownload;
						onenew->SavePath=copy_string(SavePath);
						onenew->config.http_recursing=1;
						onenew->info=addrnew;

						onenew->config.copy(&config);
						onenew->config.http_recurse_depth = config.http_recurse_depth ? config.http_recurse_depth-1 : 0;
						onenew->config.ftp_recurse_depth = config.ftp_recurse_depth;
						DIR->insert(onenew);
					}else
						delete addrnew;
			};
		};
		dir->del(temp);
		delete temp;
		temp=dir->last();
	};
};

void tDownload::save_to_config(int fd){
	write(fd,"Download:\n",strlen("Download:\n"));
	if (info) info->save_to_config(fd);
	if (SaveName){
		write_named_string(fd,"SaveName:",SaveName);
	};
	if (SavePath){
		write_named_string(fd,"SavePath:",SavePath);
	};
	config.save_to_config(fd);
	if (ScheduleTime)
		write_named_time(fd,"Time:",ScheduleTime);
	write_named_integer(fd,"State:",owner);
	write(fd,"EndDownload:\n",strlen("EndDownload:\n"));
};

int tDownload::load_from_config(int fd){
	char *table_of_fields[]={
		"Url:",
		"SaveName:",
		"SavePath:",
		"State:",
		"Time:",
		"Cfg:",
		"EndDownload:"
	};
	char buf[MAX_LEN];
	while(read_string(fd,buf,MAX_LEN)>0){
		unsigned int i;
		for (i=0;i<sizeof(table_of_fields)/sizeof(char *);i++){
			if (equal_uncase(buf,table_of_fields[i])) break;
		};
		switch(i){
		case 0:{
			if (read_string(fd,buf,MAX_LEN)<0) return -1;
			if (info) delete(info);
			info=new tAddr(buf);
			break;
		};
		case 1:{
			if (read_string(fd,buf,MAX_LEN)<0) return -1;
			set_SaveName(buf);
			break;
		};
		case 2:{
			if (read_string(fd,buf,MAX_LEN)<0) return -1;
			set_SavePath(buf);
			break;
		};
		case 3:{
			if (read_string(fd,buf,MAX_LEN)<0) return -1;
			sscanf(buf,"%d",&owner);
			break;
		};		
		case 4:{
			if (read_string(fd,buf,MAX_LEN)<0) return -1;
			sscanf(buf,"%ld",&ScheduleTime);
			break;
		};
		case 5:{
			if (config.load_from_config(fd)<0) return -1;
			break;
		};
		case 6: return info==NULL?-1:0;
		};
	};
	return -1;
};


tDownload::~tDownload() {
	if (who) delete who;
	if (info) delete info;
	if (SavePath) delete SavePath;
	if (SaveName) delete SaveName;
	if (editor) delete editor;
	if (LOG) delete LOG;
	if (DIR) delete DIR;
};


//**********************************************/

tDList::tDList() {
	Pixmap=PIX_UNKNOWN;
};

tDList::tDList(int key) {
	Pixmap=PIX_UNKNOWN;
	OwnerKey=key;
};

void tDList::init_pixmap(int a){
	Pixmap=a;
};

int tDList::owner(tDownload *which) {
	if (which && which->owner==OwnerKey) return 1;
	return 0;
};

void tDList::insert(tDownload *what) {
	what->owner=OwnerKey;
	tQueue::insert(what);
	if (Pixmap!=PIX_UNKNOWN) list_of_downloads_set_pixmap(what->GTKCListRow,Pixmap);
};

void tDList::insert_before(tDownload *what,tDownload *where) {
	what->owner=OwnerKey;
	tQueue::insert_before(what,where);
	if (Pixmap!=PIX_UNKNOWN) list_of_downloads_set_pixmap(what->GTKCListRow,Pixmap);
};

void tDList::del(tDownload *what) {
	if (owner(what)) tQueue::del(what);
	else puts(_("***WARN***!!!"
		            "Attempt to delete download from list which does'nt contain it!"));
	what->owner=DL_ALONE;
};

void tDList::forward(tDownload *what) {
	if (what->next) {
		tDownload *temp=(tDownload *)(what->next);
		if ((temp->prev=what->prev))
			what->prev->next=temp;
		else
			Last=what->next;
		what->prev=temp;
		if ((what->next=temp->next))
			temp->next->prev=what;
		else
			First=what;
		what->prev->next=what;
	};
};

void tDList::backward(tDownload *what) {
	if (what->prev) {
		tDownload *temp=(tDownload *)(what->prev);
		if ((temp->next=what->next))
			what->next->prev=temp;
		else
			First=what->prev;
		what->next=temp;
		if ((what->prev=temp->prev))
			temp->prev->next=what;
		else
			Last=what;
		what->next->prev=what;
	};
};

void tDList::dispose(){
	ALL_DOWNLOADS->del((tDownload *)First);
	tQueue::dispose();
};

tDownload *tDList::last() {
	return (tDownload *)(Curent=Last);
};

tDownload *tDList::first() {
	return (tDownload *)(Curent=First);
};

tDownload *tDList::next() {
	Curent=Curent->next;
	return (tDownload *)Curent;
};

tDownload *tDList::prev() {
	Curent=Curent->prev;
	return (tDownload *)Curent;
};

tDList::~tDList() {
	done();
};
