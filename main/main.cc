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

#include <sys/types.h>
#include <sys/stat.h>

#if (defined(__unix__) || defined(unix)) && !defined(USG)
#include <sys/param.h>
#endif

#include <sys/timeb.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <signal.h>
#include "var.h"
#include "ftpd.h"
#include "httpd.h"
#include "hproxy.h"
#include "locstr.h"
#include "main.h"
#include "dlist.h"
#include "meter.h"
#include "log.h"
#include "mainlog.h"
#include "signal.h"
#include "savelog.h"
#include "face/list.h"
#include "face/buttons.h"
#include "face/log.h"
#include "face/edit.h"
#include "face/addd.h"
#include "face/fsface.h"
#include "config.h"
#include "ntlocale.h"
#include "memwl.h"
#include "schedule.h"
#include "html.h"
#include "filter.h"
#include "sndserv.h"
#include "xml.h"
#include "face/passface.h"

tMLog *MainLog=NULL;

d4xDownloadQueue *D4X_QUEUE=NULL;
tQueue D4X_QTREE;
tMeter *GlobalMeter=NULL;
tMeter *LocalMeter=NULL;
tMeter *GraphMeter=NULL;
tMeter *GraphLMeter=NULL;
tMsgQueue *LogsMsgQueue;

int calc_curent_run(char *host,int port) {
	return (D4X_QUEUE->current_run(host,port));
};

int d4x_only_one_queue(){
	if (D4X_QTREE.count()>1 || ((d4xDownloadQueue*)(D4X_QTREE.first()))->count()) return(0);
	return(1);
};

static void d4x_qtree_for_each_rec(tQueue *q,d4xQTreeFunc dothis,void *a){
	d4xDownloadQueue *dq=(d4xDownloadQueue*)(q->first());
	while (dq){
		d4xDownloadQueue *next=(d4xDownloadQueue *)(dq->prev);
		d4x_qtree_for_each_rec(&(dq->child),dothis,a);
		dothis(dq,a);
		dq=next;
	};
};

void d4x_qtree_for_each(d4xQTreeFunc dothis,void *a){
	d4x_qtree_for_each_rec(&D4X_QTREE,dothis,a);
};

//**********************************************/

typedef void (*SigactionHandler)(int);

void _sig_pipe_handler_(){
};

int tMain::init() {
	TO_WAIT_IF_HERE=DONTRY2RUN=0;
	GVARS.SOCKETS=new d4xSocketsHistory;
	ftpsearch=NULL;
	prev_speed_limit=0;
	list_to_delete=NULL;
	GlobalMeter=new tMeter;
	GlobalMeter->init(METER_LENGTH);
	LocalMeter=new tMeter;
	LocalMeter->init(METER_LENGTH);
	GraphMeter=new tMeter;
	GraphMeter->init(GRAPH_METER_LENGTH);
	GraphLMeter=new tMeter;
	GraphLMeter->init(GRAPH_METER_LENGTH);
	
	FILTERS_DB=new d4xFiltersTree;
	FILTERS_DB->load_from_ntrc();
	SpeedScheduler=new tSpeedQueue;
	SpeedScheduler->init(0);
	MainScheduler=new d4xScheduler;
	MainScheduler->load();

	ALL_DOWNLOADS=new tDB;

	LastReadedBytes=0;
	/* Create msgqueue for logs update */
	MsgQueue=new tMsgQueue;
	MsgQueue->init(0);
	LogsMsgQueue=MsgQueue;

	SOUND_SERVER=new d4xSndServer;
	SOUND_SERVER->init_sounds();

	/* setting up signal handlers */
	struct sigaction action,old_action;
	action.sa_handler=SigactionHandler(my_main_quit);
	action.sa_flags=0;//SA_NOCLDSTOP;
	sigaction(SIGINT,&action,&old_action);
	sigaction(SIGTERM,&action,&old_action);
	action.sa_flags=0;
	action.sa_handler=SigactionHandler(_sig_pipe_handler_);
	sigaction(SIGPIPE,&action,&old_action);
	sigset_t oldmask,newmask;
	sigemptyset(&newmask);
	sigaddset(&newmask,SIGINT);
	sigaddset(&newmask,SIGTERM);
	pthread_sigmask(SIG_UNBLOCK,&newmask,&oldmask);
	server=new tMsgServer;
	if (server->init()){
		perror(_("Can't init control socket!\n"));
		delete(server);
		return(-1);
	};
	FaceForPasswords=new tFacePass;
	FaceForPasswords->load();
	return(0);
};

void create_new_queue(char *name,d4xDownloadQueue *papa){
	d4xDownloadQueue *q=new d4xDownloadQueue;
	q->name.set(name);
	if (papa){
		q->inherit_settings(papa);
		papa->subq_add(q);
	}else
		D4X_QTREE.insert(q);
	if (CFG.WITHOUT_FACE==0){
		q->qv.init();
		q->init_pixmaps();
		D4X_QVT->add(q,papa);
		if (!D4X_QUEUE) D4X_QVT->switch_remote(q);
	}else{
		if (!D4X_QUEUE) D4X_QUEUE=q;
	};
};

void tMain::init_qtree(tQueue *list,d4xDownloadQueue *papa){
	d4xDownloadQueue *q=(d4xDownloadQueue *)(list->first());
	while(q){
		q->parent=papa;
		if (CFG.WITHOUT_FACE==0){
			D4X_QVT->add(q,papa);
			if (q->IamDefault)
				D4X_QVT->switch_remote(q);
		}else{
			if (q->IamDefault){
				D4X_QUEUE=q;
			};
		};
		init_qtree(&(q->child),q);
		try_to_run_run(q);
		try_to_run_wait(q);
		q=(d4xDownloadQueue *)(q->prev);
	};
};

void tMain::load_defaults() {
	MainLog->add(_("Loading default list of downloads"),LOG_OK|LOG_DETAILED);
	read_list();
	if (D4X_QTREE.count()){
		init_qtree(&D4X_QTREE);
		if (!D4X_QUEUE){
			d4xDownloadQueue *q=(d4xDownloadQueue *)(D4X_QTREE.first());
			if (CFG.WITHOUT_FACE){
				D4X_QUEUE=q;
			}else{
				D4X_QVT->switch_remote(q);
			};
		};
	}else{
		create_new_queue("Main");
	};
	if (!CFG.WITHOUT_FACE)
		D4X_QUEUE->qv.set_shift(CFG.CLIST_SHIFT);
};


void tMain::init_main_log() {
	MainLog=new tMLog;
	MainLog->init(CFG.MAX_MAIN_LOG_LENGTH);
	if (CFG.WITHOUT_FACE==0)
		MainLog->init_list(GTK_TREE_VIEW(MainLogList));
	MainLog->reinit_file();
	MainLog->add("----------------------------------------",LOG_FROM_SERVER);
};

void tMain::redraw_logs() {
	int limit = 12;
	GList *tmplist=NULL;
	while (limit>0) {
		limit--;
		tLogMsg *Msg=(tLogMsg*)MsgQueue->first();
		if (Msg) {
			if (Msg->type&MQT_MY){
				if (Msg->what){
					if (Msg->which->freezed_flag==0){
						tmplist=log_window_freeze(tmplist,Msg->which);
					};
					log_window_add_string(Msg->which,Msg->what);
				}else{
					del_first_from_log(Msg->which);
				};
			};
			if (Msg->type&MQT_COM){
				if (Msg->what){
					if (Msg->which==D4X_LOG_DISPLAY.log)
						d4x_main_log_add_string(Msg->what);
				}else{
					if (Msg->which==D4X_LOG_DISPLAY.log)
						d4x_main_log_del_string();
				};
			};
			Msg->which->unlock();
			Msg->which->ref_dec();
			MsgQueue->del(Msg);
			delete(Msg);
		}else
			break;
	};
	while(tmplist){
		tmplist=log_window_unfreeze(tmplist);
	};
};


void tMain::absolute_delete_download(tDownload *what) {
	DBC_RETURN_IF_FAIL(what!=NULL);
	DQV(what).remove(what);
	d4xDownloadQueue *q=what->myowner->PAPA;
	q->del(what);
	ALL_DOWNLOADS->del(what);
	FaceForPasswords->stop_matched(what);
	delete(what);
};


void tMain::del_completed(d4xDownloadQueue *queue) {
	MainLog->add(_("Delete completed downloads"),LOG_OK|LOG_DETAILED);
	del_all_from_list(DL_COMPLETE,queue);
};

void tMain::rerun_failed(){
	tDownload *temp=D4X_QUEUE->first(DL_STOP);
	while (temp) {
		continue_download(temp);
		temp=D4X_QUEUE->first(DL_STOP);
	};
};

void tMain::del_fataled(d4xDownloadQueue *queue){
	MainLog->add(_("Delete failed downloads"),LOG_OK|LOG_DETAILED);
	del_all_from_list(DL_STOP,queue);
};

void tMain::del_all_from_list(int list,d4xDownloadQueue *queue){
	tDownload *temp=queue?queue->first(list):D4X_QUEUE->first(list);
	while (temp) {
		tDownload *next=(tDownload *)(temp->prev);
		if (!temp->protect){
			absolute_delete_download(temp);
		};
		temp=next;
	};
};

void tMain::del_all() {
	if (D4X_QUEUE->count())
		MainLog->add(_("Clear queue of downloads"),LOG_ERROR);
	tDownload *temp=D4X_QUEUE->first(DL_RUN);
	while (temp) {
		stop_download(temp);
		temp=D4X_QUEUE->first(DL_RUN);
	};
	temp=D4X_QUEUE->first(DL_STOPWAIT);
	while (temp) {
		if (!temp->protect)
			temp->action=ACTION_DELETE;
		temp=(tDownload *)(temp->prev);
	};
	del_all_from_list(DL_PAUSE);
	del_all_from_list(DL_WAIT);
	del_all_from_list(DL_STOP);
	del_all_from_list(DL_COMPLETE);
	del_all_from_list(DL_LIMIT);
};


int tMain::run_new_thread(tDownload *what) {
	DBC_RETVAL_IF_FAIL(what!=NULL,-1);
	pthread_attr_t attr_p;
	what->status=READY_TO_RUN;
	what->BLOCKED=0;
	if (!what->LOG) {
		what->LOG=new tLog;
		what->LOG->ref_inc();
		if (CFG.WITHOUT_FACE)
			what->LOG->init(2); // two strings in log if run without interface
		else
			what->LOG->init(CFG.MAX_LOG_LENGTH);
	};
	if (what->info->proto==D_PROTO_SEARCH){
		what->WL=new tMemoryWL;
		((tMemoryWL *)(what->WL))->set_log(what->LOG);
	}else{
		what->WL=new tDefaultWL;
		((tDefaultWL *)(what->WL))->set_log(what->LOG);
	};
	what->update_trigers();
	what->config->redirect_count=0;
	what->Size.old=what->Size.curent; // no need update size

	if (what->SpeedLimit==NULL) what->SpeedLimit=new tSpeed;
/* set speed limitation */
	if (what->split && what->split->NumOfParts){
		what->SpeedLimit->base = what->config->speed/what->split->NumOfParts;
	}else{
		what->SpeedLimit->base = what->config->speed;
	};
	if (CFG.SPEED_LIMIT<3 && CFG.SPEED_LIMIT>0){
		what->SpeedLimit->set((CFG.SPEED_LIMIT==1 ? CFG.SPEED_LIMIT_1:CFG.SPEED_LIMIT_2)/50+1);
	};
	SpeedScheduler->insert(what->SpeedLimit);
	if (what->editor) what->editor->disable_ok_button();

	MainLog->myprintf(LOG_OK|LOG_DETAILED,_("Run new thread for %z"),what);

	pthread_attr_init(&attr_p);
	pthread_attr_setdetachstate(&attr_p,PTHREAD_CREATE_JOINABLE);
	pthread_attr_setscope(&attr_p,PTHREAD_SCOPE_SYSTEM);
	if (pthread_create(&what->thread_id,&attr_p,download_last,(void *)what)){
		MainLog->add(_("Can't run new thread for downloading!"),LOG_ERROR);
		return(-1);
	};
	return(0);
};

void tMain::stop_split(tDownload *what){
	DBC_RETURN_IF_FAIL(what!=NULL);
	tDownload *tmp=what->split->next_part;
	int a=1;
	while(tmp){
		a++;
		if (tmp->split->run)
			stop_thread(tmp);
		else
			what->split->stopcount+=1;
		tmp=tmp->split?tmp->split->next_part:NULL;
	};
	what->split->stopcount+=what->split->NumOfParts-a;
};

tDownload *tMain::find_url(tAddr *adr){
	DBC_RETVAL_IF_FAIL(adr!=NULL,NULL);
	tDownload tmp;
	tmp.info=adr;
	tDownload *a=ALL_DOWNLOADS->find(&tmp);
	tmp.info=NULL;
	return(a);
};

void tMain::stop_download_url(tAddr *adr){
	tDownload *a=find_url(adr);
	if (a) stop_download(a);
};

void tMain::continue_download_url(tAddr *adr){
	tDownload *a=find_url(adr);
	if (a) continue_download(a);
};

void tMain::delete_download_url(tAddr *adr){
	tDownload *a=find_url(adr);
	if (a) delete_download(a);
};

void tMain::stop_download(tDownload *what) {
	DBC_RETURN_IF_FAIL(what!=NULL);
	int owner=what->owner();
	if (owner==DL_STOPWAIT && what->action!=ACTION_DELETE) {
		what->action=ACTION_STOP;
		return;
	};
	d4xDownloadQueue *papa=what->myowner->PAPA;
	if (papa->is_first(DL_SIZEQUERY,what)){
		stop_thread(what);
		return;
	};
	if (owner==DL_RUN) {
		papa->del(what);
		MainLog->myprintf(LOG_WARNING,_("Downloading of file %s from %s was terminated [by user]"),
				  what->info->file.get(),
				  what->info->host.get());
		stop_thread(what);
		if (what->split)
			stop_split(what);
		papa->add(what,DL_STOPWAIT);
		what->ActStatus.clear();
	} else {
		if (owner==DL_WAIT || owner==DL_LIMIT || owner==DL_SIZEQUERY) {
			FaceForPasswords->stop_matched(what);
			papa->del(what);
			papa->add(what,DL_PAUSE);
			what->ActStatus.clear();
			try_to_run_wait(papa);
		};
	};
};

int tMain::delete_download(tDownload *what,int flag) {
	if (!what) return 0;
	if (what->protect) return 0;
	if (what->owner()==DL_RUN)
		stop_download(what);
	if (what->owner()==DL_STOPWAIT){
		if (flag)
			what->action=ACTION_REAL_DELETE;
		else
			what->action=ACTION_DELETE;
		return 0;
	};
	if (CFG.WITHOUT_FACE==0 && D4X_LOG_DISPLAY.papa==what)
		D4X_LOG_DISPLAY.papa=NULL;
	MainLog->myprintf(LOG_WARNING,_("Delete file %s from queue of downloads"),what->info->file.get());
	if (flag)
		what->remove_tmp_files();
	absolute_delete_download(what);
	return 1;
};

void tMain::try_to_run_split(tDownload *what){
	DBC_RETURN_IF_FAIL(what!=NULL);
	if (what->split->runcount>=what->split->NumOfParts) return;
	if (what->status==DOWNLOAD_GO || what->status==DOWNLOAD_COMPLETE){
		tDownload *dwn=what->split->next_part;
		while (dwn && FaceForPasswords->limit_check(what)==0){
			if (dwn->split->run==0){
				if (run_new_thread(dwn)) return;
				dwn->split->run=1;
				what->split->runcount+=1;
				FaceForPasswords->limit_inc(what);
			};
			dwn=dwn->split->next_part;
		};
	};
};

int tMain::try_to_run_download(tDownload *what){
	DBC_RETVAL_IF_FAIL(what!=NULL,-1);
	time_t NOW;
	time(&NOW);
	d4xDownloadQueue *papa=what->myowner->PAPA;
	if (papa->count(DL_RUN)<50) {
		what->SpeedCalc.reset();
		what->Start=what->Pause=time(NULL);
		if (what->config==NULL){
			what->config=new tCfg;
			what->set_default_cfg();
		};
		// to avoid old info in columns
		if (CFG.WITHOUT_FACE==0)
			DQV(what).change_data(what->list_iter,PAUSE_COL,"");
		if (what->split){
			what->finfo.size=-1;
			what->split->FirstByte=0;
			what->split->LastByte=-1;
			what->split->reset();
			what->config->rollback=0;
//			what->config->ftp_recurse_depth = 1;
			what->split->grandparent=what;
//			what->config->http_recurse_depth = 1;
		};
		if (run_new_thread(what)) return -1;
		if (what->split){
			what->split->runcount=1;
			what->split->run=1;
		};
		return 1;
	};
	return 0;
};

void tMain::insert_into_wait_list(tDownload *what,
				  d4xDownloadQueue *dq){
	if (CFG.WITHOUT_FACE){
		dq->add(what);
	}else{
		tDownload *temp=dq->last(DL_WAIT);
		if (!temp || dq->qv.get_row_num(temp) < dq->qv.get_row_num(what))
			dq->add(what);
		else {
			temp=dq->first(DL_WAIT);
			while (temp && dq->qv.get_row_num(temp) < dq->qv.get_row_num(what))
				temp=(tDownload*)(temp->prev);
			dq->insert_before(what,temp);
		};
		D4X_QVT->update(dq);
	};
};

void tMain::continue_download(tDownload *what) {
	if (CFG.OFFLINE_MODE) return;
	if (!what) return;
	switch (what->owner()) {
	case DL_SIZEQUERY:
		break;
	case DL_STOPWAIT:
		if (what->action!=ACTION_DELETE &&
		    what->action!=ACTION_REAL_DELETE)
			what->action=ACTION_CONTINUE;
		break;
	case DL_RUN:
		stop_download(what);
		if (what->owner()==DL_STOPWAIT){
			what->action=ACTION_CONTINUE;
			break;
		};
	default:
		MainLog->myprintf(LOG_OK,_("Continue downloading of file %s from %s..."),
				  what->info->file.get(),
				  what->info->host.get());
		d4xDownloadQueue *papa=what->myowner->PAPA;
		if (CFG.ALLOW_FORCE_RUN && try_to_run_download(what)) {
			papa->del(what);
			papa->add(what,DL_RUN);
		} else {
			papa->del(what);
			insert_into_wait_list(what,papa);
			try_to_run_wait(papa);
		};
	};
	what->Attempt.clear();
	what->finfo.size=-1;
};

void tMain::add_dir(tDownload *parent,int http) {
	if (parent==NULL || parent->DIR==NULL) return;
	tDownload *temp=parent->DIR->last();
	d4xDownloadQueue *q=D4X_QUEUE;
	D4X_QUEUE=parent->myowner->PAPA;
	while(temp) {
		parent->DIR->del(temp);
		int totop=parent->config->ftp_dirontop && temp->finfo.type==T_DIR;
		tDownload *ex=add_downloading(temp,totop);
		if (ex) {
			if (ex->config && ex->config->http_recurse_depth<temp->config->http_recurse_depth &&
			    ex->myowner==parent->myowner){
				ex->config->http_recurse_depth=temp->config->http_recurse_depth;
				continue_download(ex);
			};
			delete temp;
		};
		temp=parent->DIR->last();
	};
	D4X_QUEUE=q;
};

void tMain::speed_calculation(tDownload *what){
	DBC_RETURN_IF_FAIL(what!=NULL);
	time_t NOWTMP;
	time(&NOWTMP);
	switch(what->finfo.type) {
	case T_FILE:{
		time_t newdiff=NOWTMP-what->Start;
		time_t difdif=what->Difference-newdiff;
		/* detecting clock skew */
		if (difdif<-1800)
			what->Start+=difdif;
		else if (difdif>1800)
			what->Start-=difdif;
		what->Difference=NOWTMP-what->Start;
		int REAL_SIZE=what->finfo.size;
		if (REAL_SIZE==0 && what->who!=NULL)
			what->finfo.size=REAL_SIZE=what->who->another_way_get_size();
		if (what->who) what->Size.set(what->get_loaded());
		what->Remain.set(REAL_SIZE-what->Size.curent);
		if (what->Difference!=0 && what->who) {
			what->Speed.set(what->SpeedCalc.speed());
		};
	};
	};
};

void tMain::print_info(tDownload *what) {
	DBC_RETURN_IF_FAIL(what!=NULL);
	if (CFG.WITHOUT_FACE){
		speed_calculation(what);
		return;
	};
	d4xDownloadQueue *PAPA=what->myowner->PAPA;
	char data[MAX_LEN];
	int downloading_started=0;
	if (what->who) {
		what->ActStatus.set(what->who->get_status());
		if (what->ActStatus.curent==D_DOWNLOAD ||
		    what->status==DOWNLOAD_COMPLETE ||
		    what->status==DOWNLOAD_FATAL){
			downloading_started=1;
			if (!what->who->reget())
				what->ActStatus.set(D_DOWNLOAD_BAD);
		};
		if (what->ActStatus.change()) {
			what->ActStatus.reset();
//			DQV(what).set_run_icon(what);
		};
	};
	switch(what->finfo.type) {
	case T_FILE:{
		if (what->finfo.type!=what->finfo.oldtype)
			DQV(what).change_data(what->list_iter,FILE_TYPE_COL,_("file"));
		fsize_t REAL_SIZE=what->filesize();
		make_number_nice(data,REAL_SIZE,PAPA->NICE_DEC_DIGITALS);
		DQV(what).change_data(what->list_iter,FULL_SIZE_COL,data);
		what->Size.set(what->get_loaded());
		what->Remain.set(REAL_SIZE-what->Size.curent);
		if (what->Remain.change() && what->Remain.curent>=0){
			make_number_nice(data,what->Remain.curent,PAPA->NICE_DEC_DIGITALS);
			DQV(what).change_data(what->list_iter,REMAIN_SIZE_COL,data);
		};
		time_t NOWTMP;
		time(&NOWTMP);
		if (what->Start>0) {
			time_t newdiff=NOWTMP-what->Start;
			time_t difdif=what->Difference-newdiff;
			/* detecting clock skew */
			if (difdif<-1800)
				what->Start+=difdif;
			else if (difdif>1800)
				what->Start-=difdif;
			convert_time(newdiff,data,PAPA->TIME_FORMAT);
			what->Difference=NOWTMP-what->Start;
			DQV(what).change_data(what->list_iter,TIME_COL,data);
		};
		if (what->Size.change()) {
			make_number_nice(data,what->Size.curent,PAPA->NICE_DEC_DIGITALS);
			DQV(what).change_data(what->list_iter,DOWNLOADED_SIZE_COL,data);
			time_t Pause=NOWTMP;
			if (Pause - what->Pause > 4)
				DQV(what).change_data(what->list_iter,PAUSE_COL,"");
			what->Pause = Pause;
		} else {
			if (what->status==DOWNLOAD_GO) {
				int Pause=NOWTMP-what->Pause;
				if (Pause>=30) {
					convert_time(Pause,data,PAPA->TIME_FORMAT);
					DQV(what).change_data(what->list_iter,PAUSE_COL,data);
				};
			};
		};
		what->Percent=100;
		if (REAL_SIZE!=0)
			what->Percent=(float(what->Size.curent)*float(100))/float(REAL_SIZE);
/* setting new title of log*/
		if (CFG.USE_MAINWIN_TITLE){
			char title[MAX_LEN];
			char *rfile=unparse_percents(what->info->file.get());
			sprintf(title,"%2.0f%% %lli/%lli %s",what->Percent,what->Size.curent,REAL_SIZE,rfile);
			delete[] rfile;
			log_window_set_title(what,title);
			log_window_set_split_info(what);
		};
		DQV(what).set_run_icon(what);

		if (what->Size.change()) {
			DQV(what).set_percent(what->list_iter,what->Percent);
		};
		what->Size.reset();
	/*	Speed calculation
	 */
		if (what->Difference!=0 && what->who) {
			what->Speed.set(what->SpeedCalc.speed());
			if (what->Speed.change()){
				sprintf(data,"%lli",what->Speed.curent);
				DQV(what).change_data(what->list_iter,SPEED_COL,data);
				what->Speed.reset();
			};
			if (what->finfo.size>0 && what->Speed.curent>0){
				int tmp=(REAL_SIZE-what->Size.curent)/what->Speed.curent;
				if (tmp>=0 && tmp<24*3660) {
					convert_time(int(tmp),data,PAPA->TIME_FORMAT);
					DQV(what).change_data(what->list_iter,ELAPSED_TIME_COL,data);
				} else
					DQV(what).change_data(what->list_iter,ELAPSED_TIME_COL,"...");
			} else
				DQV(what).change_data(what->list_iter,ELAPSED_TIME_COL,"...");
		};
		break;
	};
	case T_DIR:{
		if (what->finfo.type!=what->finfo.oldtype)
			DQV(what).change_data(what->list_iter,FILE_TYPE_COL,_("dir"));
		break;
	};
	case T_LINK:{
		if (what->finfo.type!=what->finfo.oldtype)
			DQV(what).change_data(what->list_iter,FILE_TYPE_COL,_("link"));
		break;
	};
	case T_DEVICE:{
		if (what->finfo.type!=what->finfo.oldtype)
			DQV(what).change_data(what->list_iter,FILE_TYPE_COL,_("device"));
		break;
	};
	default:
		if (what->finfo.type!=what->finfo.oldtype)
			DQV(what).change_data(what->list_iter,FILE_TYPE_COL,"???");
	};
	if (what->finfo.type==T_DIR || what->finfo.type==T_NONE){
		if (what->who) what->Size.set(what->who->get_readed());
		if (what->Size.change()) {
			char data[MAX_LEN];
			make_number_nice(data,what->Size.curent,PAPA->NICE_DEC_DIGITALS);
			DQV(what).change_data(what->list_iter,DOWNLOADED_SIZE_COL,data);
			what->Size.reset();
		};				
	};
	if (what->Attempt.change()) {
		what->Attempt.reset();
		if (what->config->number_of_attempts > 0)
			sprintf(data,"%lli/%i",what->Attempt.curent, what->config->number_of_attempts);
		else
			sprintf(data,"%lli",what->Attempt.curent);
		DQV(what).change_data(what->list_iter,TREAT_COL,data);
	};
	what->finfo.oldtype=what->finfo.type;
};


void tMain::redirect(tDownload *what,d4xDownloadQueue *dq) {
	DBC_RETURN_IF_FAIL(what!=NULL);
	what->config->redirect_count+=1;
	if (what->config->redirect_count>10){
		what->delete_who();
		MainLog->myprintf(LOG_ERROR,_("Too many redirections!"),what);
		dq->add(what,DL_COMPLETE);
		what->finfo.type=T_NONE;
		return;
	};
	tAddr *addr=what->redirect_url();
	if (addr) {
		/*
		if (what->config->leave_server==0 && equal_uncase(addr->host.get(),what->info->host.get())==0){
			MainLog->myprintf(LOG_ERROR,_("Redirection from [%z] to the different host forbidden by download's preferences!"),what);
			D4X_QUEUE->add(what,DL_COMPLETE);
			delete(addr);
			return;
		};
		*/
		char *oldurl=what->info->url();
		if (addr->cmp(what->info) &&
		    equal(what->config->referer.get(),oldurl)){
			MainLog->myprintf(LOG_ERROR,_("Redirection from [%z] to the same location!"),what);
			dq->add(what,DL_COMPLETE);
			delete(addr);
			delete[] oldurl;
			return;
		};
		if (ALL_DOWNLOADS->find(addr)) {
			dq->add(what,DL_COMPLETE);
			delete(addr);
			delete[] oldurl;
			return;
		};
		ALL_DOWNLOADS->del(what);
		delete(what->info);
		what->info=addr;
		what->config->referer.set(oldurl);
		delete[] oldurl;
		ALL_DOWNLOADS->insert(what);
		tDownload *temp=dq->first(DL_WAIT);
		if (temp)
			dq->insert_before(what,temp);
		else
			dq->add(what,DL_WAIT);
//		normalize_path(what->get_SavePath());
		what->finfo.type=what->status=0;
		what->finfo.size=-1;
		if (CFG.WITHOUT_FACE==0){
			char *URL=what->info->url_parsed();
			dq->qv.change_data(what->list_iter,URL_COL,URL);
			delete [] URL;
			dq->qv.set_filename(what);
			for (int i=FILE_TYPE_COL;i<PERCENT_COL;i++)
				if (i!=PERCENT_COL)
					DQV(what).change_data(what->list_iter,i,"");
		};
	}else{
		MainLog->myprintf(LOG_ERROR,_("Redirection from [%z] to nowhere!"),what);
		dq->add(what,DL_COMPLETE);
		what->finfo.type=T_NONE;
	};
	what->delete_who();
};

void tMain::post_stopping(tDownload *what){
	/* dispose tSegmentator only for main thread */
	if (what->segments){
		delete(what->segments);
		what->segments=NULL;
	};
	if (what->split && what->split->cond){
		delete(what->split->cond);
		what->split->cond=NULL;
	};
	if (what->config->isdefault && what->editor==NULL){
		delete(what->config);
		what->config=NULL;
	};
	if (what->editor) what->editor->enable_ok_button();
	FaceForPasswords->stop_matched(what);
};

void tMain::prepare_for_stoping_pre(tDownload *what){
	MainLog->myprintf(LOG_OK|LOG_DETAILED,_("Prepare thread %i of [%z] for stoping"),what->split?what->split->thread_num:1,what);
	if (what->SpeedLimit) SpeedScheduler->del(what->SpeedLimit);
	delete (what->SpeedLimit);
	what->SpeedLimit=NULL;
	if (what->WL){
		delete(what->WL);
		what->WL=NULL;
	};
};

void tMain::prepare_for_stoping(tDownload *what) {
	DBC_RETURN_IF_FAIL(what!=NULL);
	prepare_for_stoping_pre(what);
	if (what->split){
		what->split->grandparent->split->stopcount+=1;
		FaceForPasswords->limit_dec(what->split->grandparent);
//		what->split->run=0;
	}else
		FaceForPasswords->limit_dec(what);
};

void tMain::case_download_completed(tDownload *what){
	DBC_RETURN_IF_FAIL(what!=NULL);
	d4xDownloadQueue *papa=what->myowner->PAPA;
	papa->del(what);
	if (what->finfo.type==T_REDIRECT) {
		MainLog->myprintf(LOG_OK,_("Redirect from %z"),what);
		redirect(what,papa);
		real_stop_thread(what);
		post_stopping(what);
	}else{
		papa->add(what,DL_COMPLETE);
		if (what->file_type()==T_DIR) {
			MainLog->myprintf(LOG_OK,_("Downloading of directory %z was completed"),what);
			if (what->config->ftp_recurse_depth!=1) add_dir(what);
		} else {
			int bytes = what->finfo.size==0 ? what->who->get_readed():what->finfo.size;
			MainLog->myprintf(LOG_OK,_("Downloading of file %z (%i bytes) was completed at speed %i bytes/sec"),what,bytes,what->Speed.curent);
			if (what->config->http_recurse_depth!=1 && what->DIR)
				add_dir(what,1);
		};
		real_stop_thread(what);
		post_stopping(what);
		if (papa->AUTODEL_COMPLETED && what->protect==0) {
			MainLog->myprintf(LOG_WARNING|LOG_DETAILED,_("%z was deleted from queue of downloads as completed download"),what);
			absolute_delete_download(what);
		};
	};
};

void tMain::case_download_failed(tDownload *what){
	DBC_RETURN_IF_FAIL(what!=NULL);
	d4xDownloadQueue *papa=what->myowner->PAPA;
	papa->del(what);
	papa->add(what,DL_STOP);
	MainLog->myprintf(LOG_ERROR,_("Downloading of file %z was terminated by fatal error"),what);
	if (papa->AUTODEL_FAILED && what->protect==0) {
		MainLog->myprintf(LOG_WARNING|LOG_DETAILED,_("%z was deleted from queue of downloads as failed download"),what);
		absolute_delete_download(what);
	};
};

void tMain::main_circle_first(tDownload *dwn){
/* look for stopped threads */
	tDownload *grandpapa=dwn;
	prepare_for_stoping(dwn);
	real_stop_thread(dwn);
	if (dwn->split){
		grandpapa=dwn->split->grandparent;
		if (grandpapa->split->stopcount!=grandpapa->split->NumOfParts &&
		    grandpapa->split->prepared)
			return;
	};
	int status=grandpapa->status;
	post_stopping(grandpapa);
	d4xDownloadQueue *papa=grandpapa->myowner->PAPA;
	if (status==DOWNLOAD_REAL_STOP ||
	    status==DOWNLOAD_COMPLETE  ||
	    status==DOWNLOAD_FATAL) {
		papa->del(grandpapa);
		papa->add(grandpapa,DL_PAUSE);
		switch(grandpapa->action){
		case ACTION_REAL_DELETE:
			delete_download(grandpapa,1);
			break;
		case ACTION_DELETE:
			delete_download(grandpapa,0);
			break;
		case ACTION_CONTINUE:
			continue_download(grandpapa);
			dwn->action=ACTION_NONE;
			break;
		case ACTION_STOP:
			dwn->action=ACTION_NONE;
			break;
		case ACTION_FAILED:
			dwn->action=ACTION_NONE;
			papa->del(grandpapa);
			papa->add(grandpapa,DL_STOP);
			break;
		case ACTION_SIZEQUERY:
			move_to_sizequery(dwn);
			break;
		};
	};
};

int tMain::try_to_switch(tDownload *dwn){
	if (dwn->ALTS==NULL || dwn->ALTS->FIRST==NULL) return(0);
	d4xAlt *alt=dwn->ALTS->FIRST;
	while(alt){
		if (equal(alt->info.host.get(),dwn->info->host.get())){
			alt=alt->next;
			break;
		};
		alt=alt->next;
	};
	if (!alt){
		alt=dwn->ALTS->FIRST;
		/* first switching, add this url to its own alternates
		 */
		d4xAlt *newalt=new d4xAlt;
		newalt->info.copy(dwn->info);
		dwn->ALTS->lock.lock();
		dwn->ALTS->add(newalt);
		dwn->ALTS->lock.unlock();
	};
	if (ALL_DOWNLOADS->find(&(alt->info))){
		return(0);
	};
	ALL_DOWNLOADS->del(dwn);
	dwn->info->copy(&(alt->info));
	ALL_DOWNLOADS->insert(dwn);
	FaceForPasswords->set_do_not_run(1);
	prepare_for_stoping(dwn);
	real_stop_thread(dwn);
	if (run_new_thread(dwn))
		return(0);
	FaceForPasswords->limit_inc(dwn);
	FaceForPasswords->set_do_not_run(0);
	if (dwn->split) dwn->split->stopcount-=1;
	return(1);
};

int tMain::try_to_switch_split(tDownload *dwn,tDownload *gp){
	if (gp==dwn) return(try_to_switch(dwn));
	if (gp->ALTS==NULL || gp->ALTS->FIRST==NULL) return(0);
	int n=dwn->split->alt+1;
	d4xAlt *alt=gp->ALTS->FIRST;
	while (alt && n>1){
		n--;
		alt=alt->next;
	};
	if (alt){
		dwn->split->alt+=1;
		dwn->info->copy(&(alt->info));
	}else{
		dwn->info->copy(gp->info);
	};
	dwn->split->run=0;
	FaceForPasswords->set_do_not_run(1);
	prepare_for_stoping(dwn);
	gp->split->stopcount-=1; //to avoid wrong stopcount
	real_stop_thread(dwn);
	try_to_run_split(gp);
	FaceForPasswords->set_do_not_run(0);
	return(1);
};

void tMain::check_split(tDownload *dwn){
	tDownload *grandparent=dwn->split->grandparent;
	if (dwn->status==DOWNLOAD_FATAL){
		if (try_to_switch_split(dwn,grandparent))
			return;
		stop_download(grandparent);
		grandparent->action=ACTION_FAILED;
		main_circle_first(dwn);
		return;
	};
	if (grandparent->split->prepared){
		// we need to check overlaping event here for splited downloads
		// just find best part to download, and go! :-)
		if (dwn->WL->is_overlaped() || dwn->status==DOWNLOAD_COMPLETE){
			if (dwn->find_best_split() && dwn && dwn->who->reget()){
				real_stop_thread(dwn);
				prepare_for_stoping_pre(dwn);
				dwn->split->cond->inc();
				run_new_thread(dwn);
				return;
			};
		};
		prepare_for_stoping(dwn);
		try_to_run_split(grandparent);
		if (dwn!=grandparent) real_stop_thread(dwn);
		if (grandparent->split->NumOfParts==grandparent->split->stopcount)
			main_circle_second(grandparent);
	}else{
		try_to_run_split(grandparent);
		if (grandparent->status==DOWNLOAD_COMPLETE ||
		    grandparent->status==DOWNLOAD_FATAL)
			prepare_for_stoping(grandparent);
		main_circle_second(grandparent);
	};
};

void tMain::main_circle_second(tDownload *dwn){
/* look for completeted or faild threads */
	int failed=0,completed=0;
	d4xDownloadQueue *papa=dwn->myowner->PAPA;
	dwn->status_cp=dwn->status;
	switch(dwn->status) {
	case DOWNLOAD_COMPLETE:{
		if (dwn->segments)
			dwn->segments->complete();
		print_info(dwn); //to avoid wrong percentage after completing
		case_download_completed(dwn);
		completed=1;
		break;
	};
	case DOWNLOAD_FATAL:{
		if (dwn->split==NULL && try_to_switch(dwn)) return;
		case_download_failed(dwn);
		real_stop_thread(dwn);
		post_stopping(dwn);
		failed=1;
		break;
	};
	};
	int paparun=papa->count(DL_RUN);
	if (paparun==0){
		papa->speed.reset();
		D4X_QVT->update_speed(papa);
	};
	if (completed){
		if (paparun==0 &&
		    papa->count(DL_WAIT)==0)
			SOUND_SERVER->add_event(SND_QUEUE_FINISH);
		else
			SOUND_SERVER->add_event(SND_COMPLETE);
		try_to_run_wait(papa);
	};
	if (failed){
		try_to_run_wait(papa);
		SOUND_SERVER->add_event(SND_FAIL);
	};
};

void tMain::main_circle_nano1(){
	int i=10;
	D4X_UPDATE.lock();
	while(D4X_UPDATE.first && i>0){
		tDownload *dwn=D4X_UPDATE.first;
		if (dwn->owner()==DL_RUN){
			if (dwn->split)
				try_to_run_split(dwn);
			if (dwn->myowner->PAPA==D4X_QUEUE){
				print_info(dwn);
			};
			if (dwn->myowner && dwn->myowner->PAPA)
				D4X_QVT->update_speed(dwn->myowner->PAPA);
		};
		D4X_UPDATE.del();
		i--;
	};
	D4X_UPDATE.unlock();
};

void tMain::main_circle_nano2(){
	if (CFG.OFFLINE_MODE) return;
	D4X_UPDATE.lock_s();
	D4X_UPDATE.lock();
	int i=0;
	while (D4X_UPDATE.first_s){
		tDownload *dwn=D4X_UPDATE.first_s;
		tDownload *gp=dwn;
		D4X_UPDATE.del_s();
		D4X_UPDATE.del(dwn);
		if (dwn->split)
			gp=dwn->split->grandparent;
//		printf("%p %p\n",dwn,gp);
		switch(gp->owner()){
		case DL_RUN:
			if (dwn->split){
				check_split(dwn);
				break;
			}else{
				if (dwn->WL->is_overlaped() && dwn->status==DOWNLOAD_COMPLETE){
					real_stop_thread(dwn);
					prepare_for_stoping_pre(dwn);
					run_new_thread(dwn);
					break;
				};
				prepare_for_stoping(gp);
			};
			main_circle_second(gp);
			break;
		case DL_STOPWAIT:
			main_circle_first(dwn);
			break;
		case DL_SIZEQUERY:{
			real_stop_thread(dwn);
			prepare_for_stoping(dwn);
			d4xDownloadQueue *papa=dwn->myowner->PAPA;
			print_info(dwn);
			papa->del(dwn);
			if (dwn->action==DL_WAIT || dwn->action==DL_SIZEQUERY){
				dwn->sizequery=0;
				insert_into_wait_list(dwn,papa);
			}else{
				papa->add(dwn,dwn->action);
			};
			sizequery_run_first(papa);
			if (dwn->editor) dwn->editor->enable_ok_button();
			break;
		};
		default:
			break;
		};
		if (i++>5) break;
	};
	D4X_UPDATE.unlock();
	D4X_UPDATE.unlock_s();
};

int tMain::set_auto_run(int a){
	int old=DONTRY2RUN;
	DONTRY2RUN=a;
	return(old);
};

void tMain::try_to_run_run(d4xDownloadQueue *papa){
	tDownload *temp=papa->first(DL_RUN);
	while(temp) {
		tDownload *temp_next=(tDownload *)(temp->prev);
		int rvalue=try_to_run_download(temp);
		if (rvalue<0){
			papa->del(temp);
			papa->add(temp,DL_WAIT);
		}else{
			FaceForPasswords->match_and_check(temp,1);
			FaceForPasswords->limit_to_run(temp);
		};
		temp=temp_next;
	};
};

void tMain::try_to_run_wait(d4xDownloadQueue *papa){
	if (DONTRY2RUN) return;
	tDownload *temp=papa->first(DL_WAIT);
	while(temp && papa->count(DL_RUN)<papa->MAX_ACTIVE) {
		tDownload *temp_next=(tDownload *)(temp->prev);
		if (FaceForPasswords->match_and_check(temp)==0){
			FaceForPasswords->limit_to_run(temp);
			int rvalue=try_to_run_download(temp);
			if (rvalue<0){
				break;
			};
			if (rvalue) {
				papa->del(temp);
				papa->add(temp,DL_RUN);
			};
		};
		temp=temp_next;
	};
};

void tMain::main_circle() {
	if (ftpsearch) ftpsearch->cycle();
	speed();
	MainScheduler->run(this);
	if (CFG.WITHOUT_FACE==0){
		prepare_buttons();
		D4X_UPDATE.update(D4X_QUEUE->get_queue(DL_RUN));
	};
	GVARS.SOCKETS->kill_old();
};

void tMain::set_speed(int speed){
	CFG.SPEED_LIMIT=speed;
	if (CFG.SPEED_LIMIT>3) CFG.SPEED_LIMIT=3;
	if (CFG.SPEED_LIMIT<1) CFG.SPEED_LIMIT=1;
	if (CFG.WITHOUT_FACE==0) set_speed_buttons();
};

void tMain::check_for_remote_commands(){
	tString *addnew=server->get_string();
	int i=0;
	while (addnew){
		switch (addnew->temp){
		case PACKET_RERUN_FAILED:{
			MainLog->myprintf(LOG_FROM_SERVER|LOG_DETAILED,"%s %s",_("Restarting failed downloads"),_("[control socket]"));
			rerun_failed();
			break;
		};
		case PACKET_ADD_OPEN:{
			if (CFG.WITHOUT_FACE==0){
				init_add_dnd_window(addnew->body,NULL);
				break;
			};
		};
		case PACKET_STOP:{
//			MainLog->myprintf(LOG_FROM_SERVER,_("Stop the download via control socket [%s]"),addnew->body);
			tAddr *addr=new tAddr(addnew->body);
			stop_download_url(addr);
			delete(addr);
			break;
		};
		case PACKET_DEL:{
			MainLog->myprintf(LOG_FROM_SERVER,_("Remove the download via control socket [%s]"),addnew->body);
			tAddr *addr=new tAddr(addnew->body);
			delete_download_url(addr);
			delete(addr);
			break;
		};
		case PACKET_ADD:{
			TO_WAIT_IF_HERE=1;
			MainLog->myprintf(LOG_FROM_SERVER,_("Adding downloading via control socket [%s]"),addnew->body);
			add_downloading(addnew->body,CFG.LOCAL_SAVE_PATH);
			TO_WAIT_IF_HERE=0;
			break;
		};
		case PACKET_SET_SPEED_LIMIT:{
			sscanf(addnew->body,"%i",&CFG.SPEED_LIMIT);
			set_speed(CFG.SPEED_LIMIT);
			MainLog->myprintf(LOG_FROM_SERVER|LOG_DETAILED,_("Set speed limitation to %s %s"),
					  _(SPEED_LIMITATIONS_NAMES[CFG.SPEED_LIMIT]),
					  _("[control socket]"));
			break;
		};
		case PACKET_SET_SAVE_PATH:{
			/* to avoid misunderstandings we allow only absolute
			   pathes here 
			 */
			if (addnew->body && addnew->body[0]=='/'){
				delete[] CFG.LOCAL_SAVE_PATH;
				CFG.LOCAL_SAVE_PATH=copy_string(addnew->body);
			};
			break;
		};
		case PACKET_SET_MAX_THREADS:{
			sscanf(addnew->body,"%i",&(D4X_QUEUE->MAX_ACTIVE));
			if (D4X_QUEUE->MAX_ACTIVE>50) D4X_QUEUE->MAX_ACTIVE=50;
			if (D4X_QUEUE->MAX_ACTIVE<0) D4X_QUEUE->MAX_ACTIVE=0;
			MainLog->myprintf(LOG_FROM_SERVER|LOG_DETAILED,"%s %i %s",_("Setup maximum active downloads to"),D4X_QUEUE->MAX_ACTIVE,_("[control socket]"));
			if (CFG.WITHOUT_FACE==0) D4X_QVT->update(D4X_QUEUE);
			break;
		};
		case PACKET_DEL_COMPLETED:{
			MainLog->myprintf(LOG_FROM_SERVER|LOG_DETAILED,"%s %s",_("Delete completed downloads"),_("[control socket]"));
			del_all_from_list(DL_COMPLETE);
			break;
		};
		case PACKET_MSG:
			MainLog->myprintf(LOG_FROM_SERVER,"%s %s",addnew->body,_("[control socket]"));
			break;
		case PACKET_ICONIFY:
			if (CFG.WITHOUT_FACE==0) main_window_iconify();
			break;
		case PACKET_POPUP:
			if (CFG.WITHOUT_FACE==0) main_window_popup();
			break;
		case PACKET_EXIT_TIME:{
			int tmp;
			if (addnew->body && sscanf(addnew->body,"%d",&tmp)){
				if (tmp==0){
					CFG.EXIT_COMPLETE=0;
					MainLog->myprintf(LOG_FROM_SERVER,_("Exiting if nothing to do is switched off"),_("[control socket]"));
				};
				if (tmp>0){
					CFG.EXIT_COMPLETE=1;
					CFG.EXIT_COMPLETE_TIME=tmp;
					MainLog->myprintf(LOG_FROM_SERVER,_("Downloader will exit if nothing to do after %i minutes %s"),tmp,_("[control socket]"));
				};
			};
			break;
		};
		case PACKET_SWITCH_QUEUE:{
			int num=0;
			if (addnew->body && sscanf(addnew->body,"%d",&num)==1 && num>0){
				d4xDownloadQueue *q=d4x_get_queue_num(num);
				if (q){
					if (CFG.WITHOUT_FACE==0){
						D4X_QVT->switch_remote(q);
					}else{
						MainLog->myprintf(LOG_FROM_SERVER,_("Default queue is '%s' now."),q->name.get());
						D4X_QUEUE=q;
					};
				};
			};
			break;
		};
		};
		delete(addnew);
		i+=1;
		if (i>10) break;
		addnew=server->get_string();
	};
};
//**********************************************/
void tMain::ftp_search(tDownload *what,int type){
	DBC_RETURN_IF_FAIL(what!=NULL);
	if (what->info->file.get()){
		tDownload *tmp=new tDownload;
		tmp->info=new tAddr;
		tmp->config=new tCfg;
		tmp->fsearch=type;
		tmp->set_default_cfg();
		tmp->info->copy(what->info);
		tmp->finfo.size=what->finfo.size;
		tmp->info->proto=D_PROTO_SEARCH;
		if (tmp->split){
			delete(tmp->split);
			tmp->split=NULL;
		};
		ftpsearch->add(tmp);
	};
};

void tMain::ftp_search_name(char *name){
	if (name){
		tDownload *tmp=new tDownload;
		tmp->info=new tAddr;
		tmp->config=new tCfg;
		tmp->fsearch=0;
		tmp->set_default_cfg();
		tmp->info->host.set("");
		tmp->info->path.set("");
		tmp->info->file.set(name);
		tmp->finfo.size=-1;
		tmp->info->proto=D_PROTO_SEARCH;
		if (tmp->split){
			delete(tmp->split);
			tmp->split=NULL;
		};
		ftpsearch->add(tmp);
	};
};

void tMain::ftp_search_reping(tDownload *what){
	if (ftpsearch)
		ftpsearch->reping(what);
};

void tMain::ftp_search_remove(tDownload *what){
	if (ftpsearch)
		ftpsearch->remove(what);
};

void tMain::schedule_download(tDownload *what){
	if (what->owner()==DL_RUN || what->owner()==DL_STOPWAIT)
		return;
	DQV(what).remove(what);
	ALL_DOWNLOADS->del(what);
	what->myowner->PAPA->del(what);
	if (what->LOG){
		delete(what->LOG);
		what->LOG=NULL;
	};
	if (what->split){
		delete(what->split);
		what->split=NULL;
	};
	MainScheduler->add_scheduled(what);
};

void tMain::sizequery_run_first(d4xDownloadQueue *q){
	tDownload *torun=q->first(DL_SIZEQUERY);
	if (torun){
		if (torun->config==NULL){
			torun->config=new tCfg;
			torun->set_default_cfg();
		};
		if (torun->split)
			torun->split->grandparent=torun;
		run_new_thread(torun);
	};
};

void tMain::move_to_sizequery(tDownload *what){
	if (CFG.OFFLINE_MODE) return;
	if (what==NULL) return;
	if (what->owner()==DL_STOPWAIT){
		what->action=ACTION_SIZEQUERY;
		return;
	};
	int owner=what->owner();
	if (owner!=DL_SIZEQUERY && owner!=DL_RUN){
		d4xDownloadQueue *papa=what->myowner->PAPA;
		what->action=owner;
		papa->del(what);
		papa->add(what,DL_SIZEQUERY);
		what->sizequery=1;
		if (papa->count(DL_SIZEQUERY)==1)
			sizequery_run_first(papa);
	};
};

tDownload *tMain::add_downloading(tDownload *what,int to_top) {
	tDownload *tdwn=NULL;
	if ((tdwn=ALL_DOWNLOADS->find(what)) || !what->info->is_valid()) {
		if (TO_WAIT_IF_HERE && tdwn && tdwn->owner()!=DL_RUN){
			continue_download(tdwn);
		};
		return(tdwn);
	};
	if (what->ScheduleTime){
		MainScheduler->add_scheduled(what);
		return(NULL);
	};
	if (what->config==NULL)
		FaceForPasswords->set_cfg(what);
	ALL_DOWNLOADS->insert(what);
	tDownload *f=D4X_QUEUE->first(DL_WAIT);
	if (to_top && f)
		D4X_QUEUE->insert_before(what,f);
	else		
		D4X_QUEUE->add(what);
	if (to_top)
		D4X_QUEUE->qv.add_first(what);
	else
		D4X_QUEUE->qv.add(what);
	try_to_run_wait(D4X_QUEUE);
	return(NULL);
};

tDownload *tMain::add_downloading_to(tDownload *what,int to_top) {
	DBC_RETVAL_IF_FAIL(what!=NULL,NULL);	
	int owner=what->status;
	if (what->ScheduleTime){
		MainScheduler->add_scheduled(what);
		return NULL;
	};
	if (owner>DL_ALONE && owner<DL_TEMP){
		DONTRY2RUN=1;
		if (add_downloading(what,to_top)){
			tDownload *dwn=ALL_DOWNLOADS->find(what);
//			if (dwn && CFG.WITHOUT_FACE==0 && CFG.NEED_DIALOG_FOR_DND==0){
//				D4X_QVT->move_to(dwn);
//			};
			delete (what);
			DONTRY2RUN=0;;
			return dwn;
		};
		switch(owner){
		case DL_RUN:{
			if (try_to_run_download(what)){
				D4X_QUEUE->del(what);
				D4X_QUEUE->add(what,DL_RUN);
			};
			break;
		};
		case DL_STOPWAIT:
		case DL_STOP:{
			D4X_QUEUE->del(what);
			D4X_QUEUE->add(what,DL_STOP);
			break;
		};
		case DL_COMPLETE:{
			D4X_QUEUE->del(what);
			D4X_QUEUE->add(what,DL_COMPLETE);
			break;
		};
		case DL_PAUSE:{
			D4X_QUEUE->del(what);
			D4X_QUEUE->add(what,DL_PAUSE);
			break;
		};
		};
		DONTRY2RUN=0;
		try_to_run_wait(D4X_QUEUE);
	}else{
		delete(what);
	};
	return(NULL);
};

int tMain::add_downloading(char *adr,char *where,char *name,char *desc) {
	if (adr==NULL) return -1;
	tAddr *addr=new tAddr(adr);
//	if (!addr->is_valid()) return -1;
	tDownload *whatadd=new tDownload;
	whatadd->info=addr;
	if (where!=NULL && strlen(where)>0) {
		whatadd->config=new tCfg;
		whatadd->set_default_cfg();
		whatadd->config->save_path.set(where);
		whatadd->config->isdefault=0;
	};
	if (strlen(addr->file.get())==0) {
		whatadd->finfo.type=T_DIR;
		whatadd->finfo.size=0;
	};
//	normalize_path(whatadd->get_SavePath());

	if (name && strlen(name) && whatadd->config){
		whatadd->Name2Save.set(name);
	};

	whatadd->Description.set(desc);

	addr=NULL;
	if (add_downloading(whatadd)) {
		delete(whatadd);
		return -1;
	};
	return 0;
};

unsigned int tMain::get_precise_time(){
#if (defined(BSD) && (BSD >= 199306))
	struct timeval tp;
	gettimeofday(&tp, NULL);
	return(tp.tv_sec*1000+tp.tv_usec/1000);
#else
	struct timeb tp;
	ftime(&tp);
	return(tp.time*1000+tp.millitm);
#endif
};

void tMain::speed() {
	unsigned int curent_time=get_precise_time();
	unsigned int TimeLeft=curent_time-LastTime;
	int readed_bytes=GVARS.READED_BYTES;
	int bytes=readed_bytes-LastReadedBytes;
	if (TimeLeft!=0){
		int Speed=((bytes*1000)/TimeLeft);
		LastReadedBytes=readed_bytes;
		GlobalMeter->add(Speed);
		GraphMeter->add(Speed);
		LastTime=curent_time;
	};
	int SPEED_LIMIT=0;
	switch (CFG.SPEED_LIMIT) {
		case 1:	{
				SPEED_LIMIT=(TimeLeft*CFG.SPEED_LIMIT_1)/1000;
				break;
			};
		case 2:	{
				SPEED_LIMIT=(TimeLeft*CFG.SPEED_LIMIT_2)/1000;
				break;
			};
		case 3:
		default:{
				SPEED_LIMIT=0;
		};
	};
	if (SPEED_LIMIT){
//		SPEED_LIMIT+=(prev_speed_limit-bytes)/2;
		if (SPEED_LIMIT>0)
			SpeedScheduler->schedule(SPEED_LIMIT,1);
	}else
		SpeedScheduler->schedule(TimeLeft);
	prev_speed_limit=SPEED_LIMIT;
};

void tMain::run(int argv,char **argc) {
	SOUND_SERVER->run_thread();
	if (CFG.WITHOUT_FACE==0){
		if (CFG.USE_THEME && CFG.THEME_FILE){
			char *tmp=sum_strings(CFG.THEMES_DIR,"/",CFG.THEME_FILE,".xml",NULL);
			D4X_THEME_DATA=d4x_xml_parse_file(tmp);
			delete[] tmp;
		};
		ftpsearch=new tFtpSearchCtrl;
		init_face(argv,argc);
		ftpsearch->init(FSearchView,this,MainLog);
		fs_list_set_size();
	};
	init_main_log();
	MainLog->add(VERSION_NAME,LOG_WARNING);
			  
	COOKIES=new tCookiesTree;
	COOKIES->load_cookies();
	load_defaults();
	if (CFG.WITHOUT_FACE==0){
		prepare_buttons();
		init_timeouts();
	};
	parse_command_line_postload(argv,argc);
	server->run_thread();
	LastTime=get_precise_time();
	var_check_all_limits();
	MainLog->add(_("Loading FTP-Search engines"),LOG_WARNING);
	D4X_SEARCH_ENGINES.load();
	MainLog->add(_("Normally started"),LOG_WARNING);
	check_for_remote_commands();
	GlobalMeter->set_mode(CFG.GRAPH_MODE);
	GraphMeter->set_mode(CFG.GRAPH_MODE);
	MainScheduler->clear_old();
	if (CFG.WITHOUT_FACE==0){
		SOUND_SERVER->add_event(SND_STARTUP);
		gtk_main();
	}else{
		run_without_face();
	};
};

void tMain::run_without_face(){
	int TIME_FOR_SAVING=CFG.SAVE_LIST_INTERVAL * 60;
	int COMPLETE_INTERVAL=CFG.EXIT_COMPLETE_TIME * 60;
	struct timespec ival={0,200000000};
	int i=0;
	while(1){
		check_for_remote_commands();
		main_circle_nano2();
		if (i++>=5){
			main_circle();
			i=0;
		};
		nanosleep(&ival,NULL);
//		sleep(1);
		TIME_FOR_SAVING-=1;
		if (!TIME_FOR_SAVING) {
			if (CFG.SAVE_LIST) {
				save_list();
			};
			TIME_FOR_SAVING=CFG.SAVE_LIST_INTERVAL * 60;
		};
		if (CFG.EXIT_COMPLETE && d4x_run_or_wait_downloads()==0){
			COMPLETE_INTERVAL-=1;
			if (COMPLETE_INTERVAL<0){
				break;
			};
		}else{
			COMPLETE_INTERVAL=CFG.EXIT_COMPLETE_TIME * 60;
		};
	};
	save_list();
	done();
	save_config();
	for (int i=0;i<LAST_HISTORY;i++)
		delete(ALL_HISTORIES[i]);
};

void tMain::run_after_quit(){
	if (CFG.EXEC_WHEN_QUIT && strlen(CFG.EXEC_WHEN_QUIT))
		system(CFG.EXEC_WHEN_QUIT);
};

void tMain::add_download_message(tDownload *what) {
	if (!what) return;
	char *rfile=unparse_percents(what->info->file.get());
	MainLog->myprintf(LOG_OK,_("Added downloading of file %s from %s [by user]"),rfile,what->info->host.get());
	delete[] rfile;
};

static int not_all_stopped(tQueue *q){
	d4xDownloadQueue *dq=(d4xDownloadQueue *)q->first();
	while(dq){
		if (dq->count(DL_STOPWAIT))
			return(1);
		if (not_all_stopped(&(dq->child)))
			return(1);
		dq=(d4xDownloadQueue *)(dq->prev);
	};
	return(0);
};

void tMain::stop_all(tQueue *q){
	d4xDownloadQueue *dq=(d4xDownloadQueue *)q->first();
	while(dq){
		stop_all(&(dq->child));
		tDownload *d=dq->first(DL_RUN);
		while (d){
			stop_download(d);
			d=dq->first(DL_RUN);
		}
		dq=(d4xDownloadQueue *)(dq->prev);
	};
};

void tMain::stop_all_offline(tQueue *q){
	d4xDownloadQueue *dq=(d4xDownloadQueue *)q->first();
	while(dq){
		stop_all_offline(&(dq->child));
		tDownload *d=dq->first(DL_RUN);
		while (d){
			stop_download(d);
			d->action=ACTION_CONTINUE;
			d=dq->first(DL_RUN);
		}
		d=dq->first(DL_SIZEQUERY);
		if (d)
			stop_download(d);
		dq=(d4xDownloadQueue *)(dq->prev);
	};
};

void tMain::switch_offline_mode(){
	if (CFG.OFFLINE_MODE==0){
		stop_all_offline(&D4X_QTREE);
		ftpsearch->stop_all_offline();
		CFG.OFFLINE_MODE=1;
		MainLog->add(_("Downloader is in offline mode now"),LOG_WARNING|LOG_DETAILED);
	}else{
		CFG.OFFLINE_MODE=0;
		MainLog->add(_("Offline mode is turned off"),LOG_WARNING|LOG_DETAILED);
	};
};

void tMain::done() {
	/* There are  we MUST stop all threads!!!
	 */
	server->stop_thread();

	/* removing ftpsearch before removing all queues
	   to avoid segfault at host-limit checks */
	if (ftpsearch) delete(ftpsearch);
	FaceForPasswords->save();
	if (FaceForPasswords)
		delete (FaceForPasswords);
	stop_all(&D4X_QTREE);
	while(not_all_stopped(&D4X_QTREE)){
		main_circle_nano2();
		sleep(1);
	};
	D4X_QUEUE->done();
	MainScheduler->save();
	delete(MainScheduler);
	delete(GlobalMeter);
	delete(GraphMeter);
	delete(GraphLMeter);
	delete(LocalMeter);
	delete(ALL_DOWNLOADS); // delete or not delete that is the question :-)
	COOKIES->save_cookies();
	delete(COOKIES);
	MainLog->init_list(NULL);
	MainLog->myprintf(LOG_OK,_("%lu bytes loaded during the session"),GVARS.READED_BYTES);
	MainLog->add(_("Downloader exited normaly"),LOG_OK);
	delete(MainLog);
	delete(SpeedScheduler);

	close(LOCK_FILE_D);
	delete (server);
	delete (MsgQueue);
	FILTERS_DB->save_to_ntrc();
	delete(FILTERS_DB);
	/*
	for (int i=URL_HISTORY;i<LAST_HISTORY;i++)
		if (ALL_HISTORIES[i]) delete(ALL_HISTORIES[i]);
	*/
	SOUND_SERVER->stop_thread();
	delete(SOUND_SERVER);
	delete(GVARS.SOCKETS);
	if (D4X_THEME_DATA) delete(D4X_THEME_DATA);
	if (LOCK_FILE) remove(LOCK_FILE);
	if (D4X_QVT) delete(D4X_QVT);
};

void tMain::reinit_main_log() {
	MainLog->reinit(CFG.MAX_MAIN_LOG_LENGTH);
	MainLog->reinit_file();
};
//*****************************************************************/
//-----------------------------------------------------------------
void *download_last(void *nothing) {
	tDownload *what=(tDownload *)nothing;
	my_pthread_key_init();
	my_pthread_key_set(what);
	init_signal_handler();
	if (what) {
		tAddr *addr=what->info;
		what->LOG->MsgQueue=LogsMsgQueue;
		if (what->config->log_save_path.get()){
			char *real_path=parse_save_path(what->config->log_save_path.get(),what->info->file.get());
			make_dir_hier_without_last(real_path);
			if (what->LOG->init_save(real_path)){
				what->WL->log_printf(LOG_ERROR,
						     _("Can't open '%s' to save log"),
						     real_path);
			};
			delete[] real_path;
		}else{
			what->LOG->init_save(NULL);
		};
		if ((what->config->hproxy_host.get() && addr->proto==D_PROTO_HTTP) ||
		     (what->config->fproxy_host.get() && addr->proto==D_PROTO_FTP && what->config->proxy_type)) {
			what->who=new tProxyDownload(what->WL);
			what->download_http();
			pthread_exit(NULL);
			return NULL;
		};
		switch(addr->proto){
		case D_PROTO_SEARCH:
			if (what->who==NULL){
				if (what->config->hproxy_host.get())
					what->who=new tProxyDownload(what->WL);
				else
					what->who=new tHttpDownload(what->WL);
			};
			what->ftp_search();
			break;
		case D_PROTO_HTTP:
			what->download_http();
			break;
		case D_PROTO_FTP:
			what->download_ftp();
			break;
		default:
			what->WL->log(LOG_ERROR,_("Such protocol is not supported!"));
			what->download_failed();
			break;
		};
	};
	pthread_exit(NULL);
	return NULL;
};
//------------------------------------------------------------------------

