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

#include "fsearch.h"
#include "dbc.h"
#include "signal.h"
#include "var.h"
#include "face/fsface.h"
#include "face/lod.h"
#include "main.h"

tFtpSearchCtrl::tFtpSearchCtrl(){
	for (int i=0;i<DL_FS_LAST;i++){
		queues[i]=new tDList(i);
		queues[i]->init(0);
	};
	clist=(GtkCList*)NULL;
	parent=(tMain*)NULL;
	log=(tMLog*)NULL;
};

void tFtpSearchCtrl::init(GtkCList *list, tMain *papa,tMLog *mylog){
	parent=papa;
	clist=list;
	log=mylog;
	/* FIXME: set up right mouse click handler for clist */
};

void tFtpSearchCtrl::add(tDownload *what){
	DBC_RETURN_IF_FAIL(what!=NULL);
	what->info->proto=D_PROTO_SEARCH;
	what->action=ACTION_NONE; //reping only flag
	what->Status.curent=0; //cumulative reping flag
	queues[DL_FS_WAIT]->insert(what);
	if (clist){
		fs_list_add(clist,what);
		fs_list_show();
	};
};

void tFtpSearchCtrl::reping(tDownload *what){
	what->action=ACTION_REPING;
	what->myowner->del(what);
	queues[DL_FS_WAIT]->insert(what);
	fs_list_set_icon(clist,what,PIX_WAIT);
};

void tFtpSearchCtrl::remove(tDownload *what){
	DBC_RETURN_IF_FAIL(what!=NULL);
	switch(what->owner()){
	case DL_FS_RUN:
		stop_thread(what);
		what->action=ACTION_DELETE;
		fs_list_set_icon(clist,what,PIX_STOP_WAIT);
		break;
	case DL_FS_STOP:
	case DL_FS_WAIT:
		what->myowner->del(what);
		remove_from_clist(what);
		delete(what);
		break;
	default:
		printf("WARNING: bug in ftp search!\n");
	};
};

void tFtpSearchCtrl::remove_from_clist(tDownload *what){
	if (clist){
		fs_list_remove(clist,what);
		if (queues[DL_FS_WAIT]->count()+
		    queues[DL_FS_STOP]->count()+
		    queues[DL_FS_RUN]->count()==0)
			fs_list_hide();
	};
};

void tFtpSearchCtrl::cycle(){
	/* stoping completed and failed */
	tDownload *tmp=queues[DL_FS_RUN]->last();
	while (tmp){
		tDownload *tmpnext=queues[DL_FS_RUN]->next();
		if (tmp->status==DOWNLOAD_REAL_STOP ||
		    tmp->status==DOWNLOAD_COMPLETE  ||
		    tmp->status==DOWNLOAD_FATAL) {
			queues[DL_FS_RUN]->del(tmp);
			parent->prepare_for_stoping(tmp);
			real_stop_thread(tmp);
			parent->post_stopping(tmp);
			switch(tmp->action){
			case ACTION_DELETE:{
				remove_from_clist(tmp);
				delete(tmp);
				break;
			};
			default:{
				if (clist){
					switch (tmp->status){
					case DOWNLOAD_COMPLETE:{
						fs_list_set_icon(clist,tmp,PIX_COMPLETE);
						tDownload *a=ALL_DOWNLOADS->find(tmp);
						if (a){
							if (a->ALTS==NULL) a->ALTS=new d4xAltList;
							a->ALTS->fill_from_ftpsearch(tmp);
							remove_from_clist(tmp);
							delete(tmp);
							tmp=NULL;
						};
						break;
					};
					case DOWNLOAD_REAL_STOP:
						fs_list_set_icon(clist,tmp,PIX_PAUSE);
						break;
					default:
						fs_list_set_icon(clist,tmp,PIX_STOP);
					};
				};
				if (tmp) queues[DL_FS_STOP]->insert(tmp);
				break;
			};
			};
		};		
		tmp=tmpnext;
	};
	/* runing new */
	tmp=queues[DL_FS_WAIT]->last();
	while (tmp!=NULL && queues[DL_FS_RUN]->count()<5){
		tDownload *tmpnext=queues[DL_FS_WAIT]->next();
		if (parent->run_new_thread(tmp))
			break;
		queues[DL_FS_WAIT]->del(tmp);
		queues[DL_FS_RUN]->insert(tmp);
		if (clist)
			fs_list_set_icon(clist,tmp,PIX_RUN);
		tmp=tmpnext;
	};
};

tFtpSearchCtrl::~tFtpSearchCtrl(){
	tDownload *tmp=queues[DL_FS_STOP]->last();
	while(tmp){
		remove(tmp);
		tmp=queues[DL_FS_STOP]->last();
	};
	tmp=queues[DL_FS_WAIT]->last();
	while(tmp){
		remove(tmp);
		tmp=queues[DL_FS_WAIT]->last();
	};
	tmp=queues[DL_FS_RUN]->last();
	while(tmp){
		remove(tmp);
		tmp=queues[DL_FS_RUN]->next();
	};
	while(queues[DL_FS_RUN]->count()){
		cycle();
	};
	for (int i=0;i<DL_FS_LAST;i++){
		delete(queues[i]);
	};
};
