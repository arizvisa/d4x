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
#include "mdlist.h"
#include "meter.h"
#include "log.h"
#include "mainlog.h"
#include "signal.h"
#include "savelog.h"
#include "sortstr.h"
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

tMLog *MainLog=NULL;

tDList *DOWNLOAD_QUEUES[DL_TEMP];
tMeter *GlobalMeter=NULL;
tMeter *LocalMeter=NULL;
tMsgQueue *LogsMsgQueue;

int amount_of_downloads_in_queues(){
	int num=0;
	for (int i=DL_ALONE+1;i<DL_TEMP;i++)
		num+=DOWNLOAD_QUEUES[i]->count();
	return num;
};

int calc_curent_run(char *host,int port) {
	DBC_RETVAL_IF_FAIL(host!=NULL,0);

	tDownload *temp=DOWNLOAD_QUEUES[DL_RUN]->last();
	int count=0;
	while(temp) {
		if (strcmp(host,temp->info->host.get())==0 && port==temp->info->port){
			count+=1;
			if (temp->split){
				tDownload *temp1=temp->split->next_part;
				while (temp1){
					count+=1;
					temp1=temp1->split->next_part;
				};
			};
		};
		temp=DOWNLOAD_QUEUES[DL_RUN]->next();
	};
	return count;
};

//**********************************************/

typedef void (*SigactionHandler)(int);

void _sig_pipe_handler_(){
};

int tMain::init() {
	my_pthreads_mutex_init(&GVARS.READED_BYTES_MUTEX);
	GVARS.SOCKETS=new d4xSocketsHistory;
	ftpsearch=NULL;
	prev_speed_limit=0;
	for (int i=DL_ALONE+1;i<DL_TEMP;i++){
		DOWNLOAD_QUEUES[i]=new tDList(i);
		DOWNLOAD_QUEUES[i]->init(0);
	};
	list_to_delete=NULL;
	GlobalMeter=new tMeter;
	GlobalMeter->init(METER_LENGTH);
	LocalMeter=new tMeter;
	LocalMeter->init(METER_LENGTH);
	
	LimitsForHosts=new tHostsLimits;
	LimitsForHosts->set_default_limit(CFG.DEFAULT_HOST_LIMIT);
	FILTERS_DB=new d4xFiltersTree;
	FILTERS_DB->load_from_ntrc();
	PasswordsForHosts=load_passwords();
	read_limits();
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
	server=new tMsgServer;
	if (server->init()){
		perror(_("Can't init control socket!\n"));
		delete(server);
		return(-1);
	};
	return(0);
};

void tMain::load_defaults() {
	MainLog->add(_("Loading default list of downloads"),LOG_OK|LOG_DETAILED);
	if (!CFG.WITHOUT_FACE){
		DOWNLOAD_QUEUES[DL_COMPLETE]->set_empty_func(main_menu_completed_empty,main_menu_completed_nonempty);
		DOWNLOAD_QUEUES[DL_STOP]->set_empty_func(main_menu_failed_empty,main_menu_failed_nonempty);
	};
	tStringList *list=new tStringList;
	list->init(0);
	read_list(list);
	append_list(list);
	delete(list);
	read_list();
	if (!CFG.WITHOUT_FACE)
		list_of_downloads_set_shift(CFG.CLIST_SHIFT);
};

void tMain::append_list(tStringList *what) {
	DBC_RETURN_IF_FAIL(what!=NULL);

	MainLog->add(_("Append list to current queue of downloads"),LOG_OK|LOG_DETAILED);
	tString *temp=what->last();
	while(temp) {
		tString *temp2=what->next();
		if (temp2) {
			tString *temp3=what->next();
			if (temp3) {
				if (add_downloading(temp3->body,temp2->body,temp->body)==0) {
					tDownload *download=DOWNLOAD_QUEUES[DL_WAIT]->last();
					download->ScheduleTime=temp3->temp;
					download->config.set_flags(temp2->temp);
					switch (temp->temp) {
						case 0: break; //wait list
						case 1:	{ //completed
								DOWNLOAD_QUEUES[DL_WAIT]->del(download);
								DOWNLOAD_QUEUES[DL_COMPLETE]->insert(download);
								break;
							};
						case 2:
						default:{ //stopped
								DOWNLOAD_QUEUES[DL_WAIT]->del(download);
								DOWNLOAD_QUEUES[DL_PAUSE]->insert(download);
								break;
							};
						case 3:	{ //failed
								DOWNLOAD_QUEUES[DL_WAIT]->del(download);
								DOWNLOAD_QUEUES[DL_STOP]->insert(download);
								break;
							};
						case 4:	{ //runing
								if (try_to_run_download(download)) {
									DOWNLOAD_QUEUES[DL_WAIT]->del(download);
									DOWNLOAD_QUEUES[DL_RUN]->insert(download);
								};
						};
					};
				};
			};
		};
		temp=what->next();
	};
};

void tMain::init_main_log() {
	MainLog=new tMLog;
	MainLog->init(CFG.MAX_MAIN_LOG_LENGTH);
	if (CFG.WITHOUT_FACE==0)
		MainLog->init_list(GTK_CLIST(MainLogList));
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
			if (Msg->what){
				if (Msg->which->freezed_flag==0){
					tmplist=log_window_freeze(tmplist,Msg->which);
				};
				log_window_add_string(Msg->which,Msg->what);
			}else
				del_first_from_log(Msg->which);
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


void tMain::absolute_delete_download(tDList *where,tDownload *what) {
	DBC_RETURN_IF_FAIL(what!=NULL);
	if (CFG.WITHOUT_FACE==0)
		list_of_downloads_remove(what);
	if (where) where->del(what);
	ALL_DOWNLOADS->del(what);
	delete(what);
};


void tMain::del_completed() {
	MainLog->add(_("Delete completed downloads"),LOG_OK|LOG_DETAILED);
	del_all_from_list(DOWNLOAD_QUEUES[DL_COMPLETE]);
};

void tMain::rerun_failed(){
	tDownload *temp=DOWNLOAD_QUEUES[DL_STOP]->first();
	while (temp) {
		continue_download(temp);
		temp=DOWNLOAD_QUEUES[DL_STOP]->first();
	};
};

void tMain::del_fataled() {
	MainLog->add(_("Delete failed downloads"),LOG_OK|LOG_DETAILED);
	del_all_from_list(DOWNLOAD_QUEUES[DL_STOP]);
};

void tMain::del_all_from_list(tDList *list){
	DBC_RETURN_IF_FAIL(list!=NULL);
	tDownload *temp=list->first();
	/* delete from begin for speed, becouse
	 * after deleting from GtkClist list of downloads
	 * will be recalculated
	 */
	while (temp) {
		tDownload *next=(tDownload *)(temp->prev);
		if (!temp->protect)
			absolute_delete_download(list,temp);
		temp=next;
	};
};

void tMain::del_all() {
	if (amount_of_downloads_in_queues())
		MainLog->add(_("Clear queue of downloads"),LOG_ERROR);
	tDownload *temp=DOWNLOAD_QUEUES[DL_RUN]->first();
	while (temp) {
		stop_download(temp);
		temp=DOWNLOAD_QUEUES[DL_RUN]->first();
	};
	temp=DOWNLOAD_QUEUES[DL_STOPWAIT]->first();
	while (temp) {
		if (!temp->protect)
			temp->action=ACTION_DELETE;
		temp=DOWNLOAD_QUEUES[DL_STOPWAIT]->prev();
	};
	del_all_from_list(DOWNLOAD_QUEUES[DL_PAUSE]);
	del_all_from_list(DOWNLOAD_QUEUES[DL_WAIT]);
	del_all_from_list(DOWNLOAD_QUEUES[DL_STOP]);
	del_all_from_list(DOWNLOAD_QUEUES[DL_COMPLETE]);
};


int tMain::run_new_thread(tDownload *what) {
	DBC_RETVAL_IF_FAIL(what!=NULL,-1);
	pthread_attr_t attr_p;
	what->status=READY_TO_RUN;
	what->BLOCKED=0;
	if (!what->LOG) {
		what->LOG=new tLog;
		what->LOG->ref_inc();
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
	what->config.redirect_count=0;
	what->Size.old=what->Size.curent; // no need update size
	what->NanoSpeed=0;

	what->SpeedLimit=new tSpeed;
	what->StartSize=0;
/* set speed limitation */
	if (what->split && what->split->NumOfParts){
		what->split->stopped=0;
		what->SpeedLimit->base = what->config.speed/what->split->NumOfParts;
	}else
		what->SpeedLimit->base = what->config.speed;
	if (CFG.SPEED_LIMIT<3 && CFG.SPEED_LIMIT>0)
		what->SpeedLimit->set(((CFG.SPEED_LIMIT==1 ? CFG.SPEED_LIMIT_1:CFG.SPEED_LIMIT_2) * GLOBAL_SLEEP_DELAY)/ (DOWNLOAD_QUEUES[DL_RUN]->count()+1));
	SpeedScheduler->insert(what->SpeedLimit);
	if (what->editor) what->editor->disable_ok_button();

	MainLog->myprintf(LOG_OK|LOG_DETAILED,_("Run new thread for %z"),what);

	pthread_attr_init(&attr_p);
	pthread_attr_setdetachstate(&attr_p,PTHREAD_CREATE_JOINABLE);
	pthread_attr_setscope(&attr_p,PTHREAD_SCOPE_SYSTEM);
	if (pthread_create(&what->thread_id,&attr_p,download_last,(void *)what)){
		MainLog->add(_("Can't run new thread for downloading!"),LOG_ERROR);
		if (DOWNLOAD_QUEUES[DL_RUN]->count()){
			MainLog->myprintf(LOG_WARNING,_("Maximum active downloads is set to %i"),CFG.MAX_THREADS);
			CFG.MAX_THREADS = DOWNLOAD_QUEUES[DL_RUN]->count();
		};
		return(-1);
	};
	return(0);
};

void tMain::stop_split(tDownload *what){
	DBC_RETURN_IF_FAIL(what!=NULL);
	tDownload *tmp=what->split->next_part;
	what->split->status=1;
	while(tmp){
		stop_thread(tmp);
		tmp=tmp->split?tmp->split->next_part:NULL;
	};
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
	if (DOWNLOAD_QUEUES[DL_STOPWAIT]->owner(what) && what->action!=ACTION_DELETE) {
		what->action=ACTION_STOP;
		return;
	};
	if (DOWNLOAD_QUEUES[DL_RUN]->owner(what)) {
		DOWNLOAD_QUEUES[DL_RUN]->del(what);
		MainLog->myprintf(LOG_WARNING,_("Downloading of file %s from %s was terminated [by user]"),
				  what->info->file.get(),
				  what->info->host.get());
		stop_thread(what);
		if (what->split)
			stop_split(what);
		DOWNLOAD_QUEUES[DL_STOPWAIT]->insert(what);
		what->Status.clear();
	} else {
		if (DOWNLOAD_QUEUES[DL_WAIT]->owner(what)) {
			DOWNLOAD_QUEUES[DL_WAIT]->del(what);
			DOWNLOAD_QUEUES[DL_PAUSE]->insert(what);
			what->Status.clear();
		};
	};
};

int tMain::delete_download(tDownload *what,int flag=0) {
	if (!what) return 0;
	if (what->protect) return 0;
	switch (what->owner) {
		case DL_ALONE:{
				puts("WARN!!! Found alone download");
				return 0;
			};
		case DL_STOP:{
				DOWNLOAD_QUEUES[DL_STOP]->del(what);
				break;
			};
		case DL_COMPLETE:{
				DOWNLOAD_QUEUES[DL_COMPLETE]->del(what);
				break;
			};
		case DL_RUN:{
			stop_download(what);
			if (what->owner==DL_PAUSE) {
				DOWNLOAD_QUEUES[DL_PAUSE]->del(what);
				break;
			};
		};
		case DL_STOPWAIT:{
			if (flag)
				what->action=ACTION_REAL_DELETE;
			else
				what->action=ACTION_DELETE;
			return 0;
		};
		case DL_PAUSE:{
				DOWNLOAD_QUEUES[DL_PAUSE]->del(what);
				break;
		};
		case DL_WAIT:
				DOWNLOAD_QUEUES[DL_WAIT]->del(what);
	};
	MainLog->myprintf(LOG_WARNING,_("Delete file %s from queue of downloads"),what->info->file.get());
	if (flag)
		what->remove_tmp_files();
	absolute_delete_download(NULL,what);
	return 1;
};

void tMain::try_to_run_split(tDownload *what){
	DBC_RETURN_IF_FAIL(what!=NULL);
	if (what->status==DOWNLOAD_GO || what->status==DOWNLOAD_COMPLETE){
		tSortString *tmp=LimitsForHosts->find(what->info->host.get(),what->info->port);
		tDownload *dwn=what->split->next_part;
		while (dwn){
			if (tmp && tmp->curent>=tmp->upper)
				return;
			if (dwn->split->status==0){
				if (run_new_thread(dwn)){
					return;
				};
				if (tmp) tmp->increment();
			};
			dwn->split->status=1;
			dwn=dwn->split->next_part;
		};
	};
};

int tMain::try_to_run_download(tDownload *what){
	DBC_RETVAL_IF_FAIL(what!=NULL,-1);
	tSortString *tmp=LimitsForHosts->find(what->info->host.get(),what->info->port);
	time_t NOW;
	time(&NOW);
	if (DOWNLOAD_QUEUES[DL_RUN]->count()<50 &&
	    (tmp==NULL || tmp->curent<tmp->upper)) {
		// to avoid old info in columns
		if (CFG.WITHOUT_FACE==0)
			list_of_downloads_change_data(list_of_downloads_row(what),PAUSE_COL,"");
		if (what->split){
			what->finfo.size=-1;
			what->split->FirstByte=0;
			what->split->LastByte=-1;
			what->split->status=0;
			what->config.rollback=0;
			what->config.ftp_recurse_depth = what->config.http_recurse_depth = 1;
		};
		if (run_new_thread(what)) return -1;
		if (tmp) tmp->increment();
		return 1;
	};
	return 0;
};

void tMain::insert_into_wait_list(tDownload *what){
	tDownload *temp=DOWNLOAD_QUEUES[DL_WAIT]->last();
	if (!temp || list_of_downloads_row(temp) < list_of_downloads_row(what))
		DOWNLOAD_QUEUES[DL_WAIT]->insert(what);
	else {
		temp=DOWNLOAD_QUEUES[DL_WAIT]->first();
		while (temp && list_of_downloads_row(temp) < list_of_downloads_row(what))
			temp=DOWNLOAD_QUEUES[DL_WAIT]->prev();
		DOWNLOAD_QUEUES[DL_WAIT]->insert_before(what,temp);
	};
};

void tMain::continue_download(tDownload *what) {
	if (!what) return;
	switch (what->owner) {
	case DL_STOPWAIT:
		if (what->action!=ACTION_DELETE && what->action!=ACTION_REAL_DELETE)
			what->action=ACTION_CONTINUE;
		break;
	case DL_RUN:
		stop_download(what);
		if (DOWNLOAD_QUEUES[DL_STOPWAIT]->owner(what)){
			what->action=ACTION_CONTINUE;
		};
		break;
	default:
		MainLog->myprintf(LOG_OK,_("Continue downloading of file %s from %s..."),
				  what->info->file.get(),
				  what->info->host.get());
		DOWNLOAD_QUEUES[what->owner]->del(what);
		if (CFG.ALLOW_FORCE_RUN && try_to_run_download(what)) {
			DOWNLOAD_QUEUES[DL_RUN]->insert(what);
		} else {
			insert_into_wait_list(what);
		};
	};
	what->Attempt.clear();
	what->finfo.size=-1;
};

int tMain::complete() {
	return((DOWNLOAD_QUEUES[DL_WAIT]->count()+DOWNLOAD_QUEUES[DL_RUN]->count()+DOWNLOAD_QUEUES[DL_STOPWAIT]->count())>0?0:1);
};

void tMain::add_dir(tDownload *parent) {
	if (parent==NULL || parent->DIR==NULL) return;
	tDownload *temp=parent->DIR->last();
	while(temp) {
		parent->DIR->del(temp);
		if (add_downloading(temp)) {
			delete temp;
		};
		temp=parent->DIR->last();
	};
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
		if (what->who) what->Size.set(what->who->get_readed());
		what->Remain.set(REAL_SIZE-what->Size.curent);
		if (what->Difference!=0 && what->who) {
			float tmp=float(what->Size.curent-what->start_size());
			if (tmp>0) {
				what->Speed.set(int(tmp/what->Difference));
			};
		};
	};
	};
};

int tMain::get_split_loaded(tDownload *what){
	DBC_RETVAL_IF_FAIL(what!=NULL,0);
	int tmp=0;
	while (what){
		if (what->who){
			int readed=what->who->get_readed();
			tmp+=readed > what->split->FirstByte ? readed-what->split->FirstByte:0;
		};
		what=what->split->next_part;
	};
	return tmp;
};

void tMain::print_info(tDownload *what) {
	DBC_RETURN_IF_FAIL(what!=NULL);
	char data[MAX_LEN];
	int downloading_started=0;
	if (what->who) {
		what->Status.set(what->who->get_status());
		if (what->Status.curent==D_DOWNLOAD){
			downloading_started=1;
			if (!what->who->reget())
				what->Status.set(D_DOWNLOAD_BAD);
		};
		if (what->Status.change()) {
			what->Status.reset();
			list_of_downloads_set_run_icon(what);
		};
	};
	gint row=list_of_downloads_row(what);
	switch(what->finfo.type) {
	case T_FILE:{
		if (what->finfo.type!=what->finfo.oldtype) list_of_downloads_change_data(row,FILE_TYPE_COL,_("file"));
		fsize_t REAL_SIZE=what->finfo.size;
		if (REAL_SIZE==0 && what->who!=NULL)
			what->finfo.size=REAL_SIZE=what->who->another_way_get_size();
		if (REAL_SIZE<0) REAL_SIZE=0;
		make_number_nice(data,REAL_SIZE);
		if (downloading_started){
			list_of_downloads_change_data(row,FULL_SIZE_COL,data);
			if (what->who){
					what->Size.set(what->get_loaded());
			};
			what->Remain.set(REAL_SIZE-what->Size.curent);
		};
		
		if (what->Remain.change() && what->Remain.curent>=0){
			make_number_nice(data,what->Remain.curent);
			list_of_downloads_change_data(row,REMAIN_SIZE_COL,data);
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
			convert_time(newdiff,data);
			what->Difference=NOWTMP-what->Start;
			list_of_downloads_change_data(row,TIME_COL,data);
		};
		if (what->Size.change() || CFG.NICE_DEC_DIGITALS.change()) {
			make_number_nice(data,what->Size.curent);
			list_of_downloads_change_data(row,DOWNLOADED_SIZE_COL,data);
			time_t Pause=NOWTMP;
			if (Pause - what->Pause > 4)
				list_of_downloads_change_data(row,PAUSE_COL,"");
			what->Pause = Pause;
			// calculating speed for graph
			unsigned int TimeLeft=get_precise_time()-LastTime;
			if (what->Size.old<=0 && what->who) what->Size.old=what->start_size();
			if (TimeLeft!=0 && what->Size.curent>what->Size.old)
				what->NanoSpeed=((what->Size.curent-what->Size.old)*1000)/TimeLeft;
			else
				what->NanoSpeed=0;
		} else {
			what->NanoSpeed=0;
			if (what->status_cp==DOWNLOAD_GO) {
				int Pause=NOWTMP-what->Pause;
				if (Pause>=30) {
					convert_time(Pause,data);
					list_of_downloads_change_data(row,PAUSE_COL,data);
				};
			};
		};
		what->Percent=100;
		if (REAL_SIZE!=0)
			what->Percent=(float(what->Size.curent)*float(100))/float(REAL_SIZE);
/* setting new title of log*/
		if (CFG.USE_MAINWIN_TITLE){
			char title[MAX_LEN];
			sprintf(title,"%2.0f%% %li/%li %s",what->Percent,what->Size.curent,REAL_SIZE,what->info->file.get());
			log_window_set_title(what,title);
		};

		if (what->Size.change()) {
			list_of_downloads_set_percent(row,
						      PERCENT_COL,
						      what->Percent);
		};
		what->Size.reset();
	/*	Speed calculation
	 */
		if (what->Difference!=0 && what->who && what->start_size()>=0) {
			float tmp=float(what->Size.curent-what->start_size());
			if (tmp>0) {
				what->Speed.set(int(tmp/what->Difference));
				if (what->Speed.change()) {
					sprintf(data,"%li",what->Speed.curent);
					list_of_downloads_change_data(row,SPEED_COL,data);
					what->Speed.reset();
				};
			};
			/*	Remaining time calculation
			 */
			if (tmp>0) {
				tmp=(float(REAL_SIZE-what->Size.curent)*float(what->Difference))/tmp;
				if (tmp>=0 && tmp<24*3660) {
					convert_time(int(tmp),data);
					list_of_downloads_change_data(row,ELAPSED_TIME_COL,data);
				} else
					list_of_downloads_change_data(row,ELAPSED_TIME_COL,"...");
			} else
				list_of_downloads_change_data(row,ELAPSED_TIME_COL,"...");
		};
		break;
	};
	case T_DIR:{
		if (what->finfo.type!=what->finfo.oldtype) list_of_downloads_change_data(row,FILE_TYPE_COL,_("dir"));
		break;
	};
	case T_LINK:{
		if (what->finfo.type!=what->finfo.oldtype) list_of_downloads_change_data(row,FILE_TYPE_COL,_("link"));
		break;
	};
	case T_DEVICE:{
		if (what->finfo.type!=what->finfo.oldtype) list_of_downloads_change_data(row,FILE_TYPE_COL,_("device"));
		break;
	};
	default:
		if (what->finfo.type!=what->finfo.oldtype) list_of_downloads_change_data(row,FILE_TYPE_COL,"???");
	};
	if (what->finfo.type==T_DIR || what->finfo.type==T_NONE){
		if (what->who) what->Size.set(what->who->get_readed());
		if (what->Size.change() || CFG.NICE_DEC_DIGITALS.change()) {
			char data[MAX_LEN];
			make_number_nice(data,what->Size.curent);
			list_of_downloads_change_data(row,DOWNLOADED_SIZE_COL,data);
			what->Size.reset();
		};				
	};
	if (what->who) {
		what->Attempt.set(what->who->treat());
		if (what->Attempt.change()) {
			what->Attempt.reset();
			if (what->config.number_of_attempts > 0)
				sprintf(data,"%li/%i",what->Attempt.curent, what->config.number_of_attempts);
			else
				sprintf(data,"%li",what->Attempt.curent);
			list_of_downloads_change_data(row,TREAT_COL,data);
		};
	};
	what->finfo.oldtype=what->finfo.type;
};


void tMain::redirect(tDownload *what) {
	DBC_RETURN_IF_FAIL(what!=NULL);
	char *newurl=NULL;
	if (what->who){
		newurl = what->who->get_new_url();
		what->delete_who();
	};
	what->config.redirect_count+=1;
	if (what->config.redirect_count>10){
		MainLog->myprintf(LOG_ERROR,_("Too many redirections!"),what);
		DOWNLOAD_QUEUES[DL_COMPLETE]->insert(what);
		what->finfo.type=T_NONE;
		return;
	};
	if (newurl) {
		tAddr *addr=fix_url_global(newurl,what->info,-1,0);
		delete[] newurl;		
		if (addr==NULL){
			DOWNLOAD_QUEUES[DL_COMPLETE]->insert(what);
			return;
		};
		char *oldurl=what->info->url();
		if (addr->cmp(what->info) &&
		    equal(what->config.referer.get(),oldurl)){
			MainLog->myprintf(LOG_ERROR,_("Redirection from [%z] to the same location!"),what);
			DOWNLOAD_QUEUES[DL_COMPLETE]->insert(what);
			delete(addr);
			delete[] oldurl;
			return;
		};
		if (ALL_DOWNLOADS->find(addr)) {
			DOWNLOAD_QUEUES[DL_COMPLETE]->insert(what);
			delete(addr);
			delete[] oldurl;
			return;
		};
		ALL_DOWNLOADS->del(what);
		delete(what->info);
		what->info=addr;
		what->config.referer.set(oldurl);
		delete[] oldurl;
		ALL_DOWNLOADS->insert(what);
		tDownload *temp=DOWNLOAD_QUEUES[DL_WAIT]->first();
		if (temp)
			DOWNLOAD_QUEUES[DL_WAIT]->insert_before(what,temp);
		else
			DOWNLOAD_QUEUES[DL_WAIT]->insert(what);
//		normalize_path(what->get_SavePath());
		what->finfo.type=what->status=0;
		what->finfo.size=-1;
		if (CFG.WITHOUT_FACE==0){
			char *URL=what->info->url();
			gint row=list_of_downloads_row(what);
			list_of_downloads_change_data(row,URL_COL,URL);
			delete [] URL;
			list_of_downloads_change_data(row,FILE_COL,what->info->file.get());
			for (int i=FILE_TYPE_COL;i<URL_COL;i++)
				list_of_downloads_change_data(row,i,"");
		};
	} else {
		MainLog->myprintf(LOG_ERROR,_("Redirection from [%z] to nowhere!"),what);
		DOWNLOAD_QUEUES[DL_COMPLETE]->insert(what);
		what->finfo.type=T_NONE;
	};
};

void tMain::prepare_for_stoping(tDownload *what,tDList *list) {
	DBC_RETURN_IF_FAIL(what!=NULL);
	MainLog->myprintf(LOG_OK|LOG_DETAILED,_("Prepare [%z] for stoping"),what);
	if (what->thread_id &&
	    (what->split==NULL || what->split->stopped==0))
		LimitsForHosts->decrement(what);
	if (what->editor) what->editor->enable_ok_button();
	if (what->SpeedLimit) SpeedScheduler->del(what->SpeedLimit);
	delete (what->SpeedLimit);
	what->SpeedLimit=NULL;
	if (list) list->del(what);
	if (what->WL){
		delete(what->WL);
		what->WL=NULL;
	};
	/* dispose tSegmentator only for main thread */
	if (list && what->segments){
		delete(what->segments);
		what->segments=NULL;
	};
	if (what->split){
		if (what->split->cond){
			delete(what->split->cond);
			what->split->cond=NULL;
		};
		tDownload *next_part=what->split->next_part;
		if (next_part){
			if (next_part->split)
				next_part->split->cond=NULL;
			prepare_for_stoping(next_part,NULL);
			real_stop_thread(next_part);
		};
//		delete(what->split->next_part);
//		what->split->next_part=NULL;
	};
};

void tMain::case_download_completed(tDownload *what){
	DBC_RETURN_IF_FAIL(what!=NULL);
	prepare_for_stoping(what,DOWNLOAD_QUEUES[DL_RUN]);
	if (what->finfo.type==T_REDIRECT) {
		MainLog->myprintf(LOG_OK,_("Redirect from %z"),what);
		redirect(what);
	}else{
		if (what->file_type()==T_DIR) {
			MainLog->myprintf(LOG_OK,_("Downloading of directory %z was completed"),what);
			if (what->config.ftp_recurse_depth!=1) add_dir(what);
		} else {
			int bytes = what->finfo.size==0 ? what->who->get_readed():what->finfo.size;
			MainLog->myprintf(LOG_OK,_("Downloading of file %z (%i bytes) was completed at speed %i bytes/sec"),what,bytes,what->Speed.curent);
			if (what->config.http_recurse_depth!=1 && what->DIR)
				add_dir(what);
		};
		real_stop_thread(what);
		if (CFG.DELETE_COMPLETED && what->protect==0) {
			MainLog->myprintf(LOG_WARNING|LOG_DETAILED,_("%z was deleted from queue of downloads as completed download"),what);
			absolute_delete_download(NULL,what);
		} else {
			DOWNLOAD_QUEUES[DL_COMPLETE]->insert(what);
		};
	};
};

void tMain::case_download_failed(tDownload *what){
	DBC_RETURN_IF_FAIL(what!=NULL);
	prepare_for_stoping(what,DOWNLOAD_QUEUES[DL_RUN]);
	MainLog->myprintf(LOG_ERROR,_("Downloading of file %z was terminated by fatal error"),what);
	real_stop_thread(what);
	if (CFG.DELETE_FATAL && what->protect==0) {
		MainLog->myprintf(LOG_WARNING|LOG_DETAILED,_("%z was deleted from queue of downloads as failed download"),what);
		absolute_delete_download(NULL,what);
	} else {
		DOWNLOAD_QUEUES[DL_STOP]->insert(what);
	};
};

int tMain::get_status_split(tDownload *what){
	DBC_RETVAL_IF_FAIL(what!=NULL,DOWNLOAD_GO);
	tDownload *tmp=what;
	int status[3]={0,0,0};
	if (tmp && tmp->status==DOWNLOAD_COMPLETE &&
	    tmp->finfo.type==T_REDIRECT && tmp->split->next_part==NULL)
		return DOWNLOAD_COMPLETE;
	int count=tmp->split->NumOfParts;
	while (tmp){
		switch(tmp->status){
		case DOWNLOAD_COMPLETE:{
			status[0]+=1;
			if (tmp->split->stopped==0){
				LimitsForHosts->decrement(tmp);
				tmp->split->stopped=1;
			};
			break;
		};
		case DOWNLOAD_REAL_STOP:{
			status[1]+=1;
			break;
		};
		case DOWNLOAD_FATAL:{
			if (what->owner==DL_RUN){
//				tmp->status=DOWNLOAD_REAL_STOP;
				MainLog->myprintf(LOG_ERROR,_("Splited download [%z] was stopped because one of threads failed"),what);
				stop_download(what);
				what->action=ACTION_FAILED;
				return DOWNLOAD_GO;
			};
			return DOWNLOAD_FATAL;
		};
		default:
			status[2]+=1;
		};
		count-=1;
		if (count<=0) break;
		tmp=tmp->split->next_part;
	};
	if (status[2]) return DOWNLOAD_GO;
	if (status[1]==0)
		return DOWNLOAD_COMPLETE;
	return DOWNLOAD_REAL_STOP;
};

void tMain::main_circle_first(){
/* look for stopped threads */
	tDownload *temp=DOWNLOAD_QUEUES[DL_STOPWAIT]->last();
	while(temp) {
		tDownload *temp1=DOWNLOAD_QUEUES[DL_STOPWAIT]->next();
		int status=temp->status;
		if (temp->split) status=get_status_split(temp);
		if (status==DOWNLOAD_REAL_STOP ||
		    status==DOWNLOAD_COMPLETE  ||
		    status==DOWNLOAD_FATAL) {
			prepare_for_stoping(temp,DOWNLOAD_QUEUES[DL_STOPWAIT]);
			real_stop_thread(temp);
			DOWNLOAD_QUEUES[DL_PAUSE]->insert(temp);
			switch(temp->action){
			case ACTION_REAL_DELETE:
				delete_download(temp,1);
				break;
			case ACTION_DELETE:
				delete_download(temp,0);
				break;
			case ACTION_CONTINUE:
				continue_download(temp);
				temp->action=ACTION_NONE;
				break;
			case ACTION_STOP:
				temp->action=ACTION_NONE;
				break;
			case ACTION_FAILED:
				DOWNLOAD_QUEUES[DL_PAUSE]->del(temp);
				DOWNLOAD_QUEUES[DL_STOP]->insert(temp);
				break;
			};
		};
		temp=temp1;
	};
};

void tMain::main_circle_second(){
/* look for completeted or faild threads */
	tDownload *temp=DOWNLOAD_QUEUES[DL_RUN]->last();
	int failed=0,completed=0;
	while(temp) {
		tDownload *temp1=DOWNLOAD_QUEUES[DL_RUN]->next();
		int status=temp->status;
		if (temp->split){
			status=get_status_split(temp);
			if (temp->status!=DOWNLOAD_FATAL &&
			    temp->split->status==0){
				try_to_run_split(temp);
			};
		};
		temp->status_cp=status;
		if (CFG.WITHOUT_FACE==0) print_info(temp);
		else speed_calculation(temp);
		switch(status) {
		case DOWNLOAD_COMPLETE:{
			if (temp->segments)
				temp->segments->complete();
			case_download_completed(temp);
			completed=1;
			break;
		};
		case DOWNLOAD_FATAL:{
			case_download_failed(temp);
			failed=1;
			break;
		};
		};
		temp=temp1;
	};
	if (completed){
		if (DOWNLOAD_QUEUES[DL_RUN]->count()==0 && DOWNLOAD_QUEUES[DL_WAIT]->count()==0)
			SOUND_SERVER->add_event(SND_QUEUE_FINISH);
		else
			SOUND_SERVER->add_event(SND_COMPLETE);
	};
	if (failed)
		SOUND_SERVER->add_event(SND_FAIL);
};

void tMain::main_circle() {
	main_circle_first();
	main_circle_second();
	if (ftpsearch) ftpsearch->cycle();
/* look for run new */
	tDownload *temp=DOWNLOAD_QUEUES[DL_WAIT]->first();
	while(temp && DOWNLOAD_QUEUES[DL_RUN]->count()<CFG.MAX_THREADS) {
		tDownload *temp_next=DOWNLOAD_QUEUES[DL_WAIT]->prev();
		int rvalue=try_to_run_download(temp);
		if (rvalue<0){
			break;
		};
		if (rvalue) {
			DOWNLOAD_QUEUES[DL_WAIT]->del(temp);
			DOWNLOAD_QUEUES[DL_RUN]->insert(temp);
		};
		temp=temp_next;
	};
/* various stuff */
	speed();
	MainScheduler->run(this);
	if (CFG.WITHOUT_FACE==0)
		prepare_buttons();
	GVARS.SOCKETS->kill_old();
	CFG.NICE_DEC_DIGITALS.reset();
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
		case PACKET_DEL:{
			MainLog->myprintf(LOG_FROM_SERVER,_("Remove the download via control socket [%s]"),addnew->body);
			tAddr *addr=new tAddr(addnew->body);
			delete_download_url(addr);
			delete(addr);
			break;
		};
		case PACKET_ADD:{
			MainLog->myprintf(LOG_FROM_SERVER,_("Adding downloading via control socket [%s]"),addnew->body);
			add_downloading(addnew->body,CFG.LOCAL_SAVE_PATH);
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
			sscanf(addnew->body,"%i",&CFG.MAX_THREADS);
			var_check_all_limits();
			MainLog->myprintf(LOG_FROM_SERVER|LOG_DETAILED,"%s %i %s",_("Setup maximum active downloads to"),CFG.MAX_THREADS,_("[control socket]"));
			break;
		};
		case PACKET_DEL_COMPLETED:{
			MainLog->myprintf(LOG_FROM_SERVER|LOG_DETAILED,"%s %s",_("Delete completed downloads"),_("[control socket]"));
			del_all_from_list(DOWNLOAD_QUEUES[DL_COMPLETE]);
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
		};
		delete(addnew);
		i+=1;
		if (i>10) break;
		addnew=server->get_string();
	};
};
//**********************************************/
void tMain::ftp_search(tDownload *what){
	DBC_RETURN_IF_FAIL(what!=NULL);
	if (what->info->file.get()){
		tDownload *tmp=new tDownload;
		tmp->info=new tAddr;
		tmp->set_default_cfg();
		tmp->info->copy(what->info);
		tmp->finfo.size=what->finfo.size;
		tmp->info->proto=D_PROTO_SEARCH;
		if (CFG.USE_PROXY_FOR_HTTP) {
			tmp->config.proxy_host.set(CFG.HTTP_PROXY_HOST);
			tmp->config.proxy_port=CFG.HTTP_PROXY_PORT;
			if (CFG.NEED_PASS_HTTP_PROXY) {
				tmp->config.proxy_user.set(CFG.HTTP_PROXY_USER);
				tmp->config.proxy_pass.set(CFG.HTTP_PROXY_PASS);
			};
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
	if (DOWNLOAD_QUEUES[DL_RUN]->owner(what)
	    || DOWNLOAD_QUEUES[DL_STOPWAIT]->owner(what)) return;
	DOWNLOAD_QUEUES[what->owner]->del(what);
	list_of_downloads_remove(what);
	ALL_DOWNLOADS->del(what);
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

int tMain::add_downloading(tDownload *what,int to_top=0) {
	if (ALL_DOWNLOADS->find(what) || !what->info->is_valid()) 
		return 1;
	if (what->ScheduleTime){
		MainScheduler->add_scheduled(what);
		return(0);
	};
	if (what->info->username.get()==NULL){
		tUserPass *tmp=PasswordsForHosts->find(what->info->proto,
						       what->info->host.get());
		if (tmp){
			what->info->username.set(tmp->user.get());
			what->info->pass.set(tmp->pass.get());
		};
	};
	ALL_DOWNLOADS->insert(what);
	if (CFG.WITHOUT_FACE==0){
		if (to_top)
			list_of_downloads_add(what,0);
		else
			list_of_downloads_add(what);
	};
	tDownload *f=DOWNLOAD_QUEUES[DL_WAIT]->first();
	if (to_top && f)
		DOWNLOAD_QUEUES[DL_WAIT]->insert_before(what,f);
	else		
		DOWNLOAD_QUEUES[DL_WAIT]->insert(what);
	return 0;
};

void tMain::add_downloading_to(tDownload *what,int to_top=0) {
	DBC_RETURN_IF_FAIL(what!=NULL);	
	int owner=what->owner;
	if (what->ScheduleTime){
		MainScheduler->add_scheduled(what);
		return;
	};
	if (owner>DL_ALONE && owner<DL_TEMP){
		if (add_downloading(what,to_top)){
			delete (what);
			return;
		};
		switch(owner){
		case DL_RUN:{
			if (try_to_run_download(what)){
				DOWNLOAD_QUEUES[DL_WAIT]->del(what);
				DOWNLOAD_QUEUES[DL_RUN]->insert(what);
			};
			break;
		};
		case DL_STOPWAIT:
		case DL_STOP:{
			DOWNLOAD_QUEUES[DL_WAIT]->del(what);
			DOWNLOAD_QUEUES[DL_STOP]->insert(what);
			break;
		};
		case DL_COMPLETE:{
			DOWNLOAD_QUEUES[DL_WAIT]->del(what);
			DOWNLOAD_QUEUES[DL_COMPLETE]->insert(what);
			break;
		};
		case DL_PAUSE:{
			DOWNLOAD_QUEUES[DL_WAIT]->del(what);
			DOWNLOAD_QUEUES[DL_PAUSE]->insert(what);
			break;
		};
		};
	}else{
		delete(what);
	};
};

int tMain::add_downloading(char *adr,char *where=NULL,char *name=NULL,char *desc=NULL) {
	if (adr==NULL) return -1;
	tAddr *addr=new tAddr(adr);
//	if (!addr->is_valid()) return -1;
	tDownload *whatadd=new tDownload;
	whatadd->info=addr;
	if (where!=NULL && strlen(where)>0) {
		whatadd->config.save_path.set(where);
	} else
		whatadd->config.save_path.set(CFG.GLOBAL_SAVE_PATH);
	if (strlen(addr->file.get())==0) {
		whatadd->finfo.type=T_DIR;
		whatadd->finfo.size=0;
	};
	if (add_downloading(whatadd)) {
		delete(whatadd);
		return -1;
	};
//	normalize_path(whatadd->get_SavePath());

	if (name && strlen(name))
		whatadd->config.save_name.set(name);
	whatadd->set_default_cfg();

	whatadd->config.proxy_type=CFG.FTP_PROXY_TYPE;
	whatadd->Description.set(desc);
	switch(whatadd->info->proto){
	case D_PROTO_FTP:{
		if (CFG.USE_PROXY_FOR_FTP) {
			whatadd->config.proxy_host.set(CFG.FTP_PROXY_HOST);
			whatadd->config.proxy_port=CFG.FTP_PROXY_PORT;
			if (CFG.NEED_PASS_FTP_PROXY) {
				whatadd->config.proxy_user.set(CFG.FTP_PROXY_USER);
				whatadd->config.proxy_pass.set(CFG.FTP_PROXY_PASS);
			};
		};
		break;
	};
	case D_PROTO_HTTP:{
		if (CFG.USE_PROXY_FOR_HTTP) {
			whatadd->config.proxy_host.set(CFG.HTTP_PROXY_HOST);
			whatadd->config.proxy_port=CFG.HTTP_PROXY_PORT;
			if (CFG.NEED_PASS_HTTP_PROXY) {
				whatadd->config.proxy_user.set(CFG.HTTP_PROXY_USER);
				whatadd->config.proxy_pass.set(CFG.HTTP_PROXY_PASS);
			};
		};
		break;
	};
	};

	addr=NULL;
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
		ftpsearch=new tFtpSearchCtrl;
		init_face(argv,argc);
		ftpsearch->init(FSearchCList,this,MainLog);
		DOWNLOAD_QUEUES[DL_COMPLETE]->init_pixmap(PIX_COMPLETE);
		DOWNLOAD_QUEUES[DL_RUN]->init_pixmap(PIX_RUN_PART);
		DOWNLOAD_QUEUES[DL_WAIT]->init_pixmap(PIX_WAIT);
		DOWNLOAD_QUEUES[DL_PAUSE]->init_pixmap(PIX_PAUSE);
		DOWNLOAD_QUEUES[DL_STOP]->init_pixmap(PIX_STOP);
		DOWNLOAD_QUEUES[DL_STOPWAIT]->init_pixmap(PIX_STOP_WAIT);
	};
	init_main_log();
	MainLog->add(VERSION_NAME,LOG_WARNING);
			  
	COOKIES=new tCookiesTree;
	COOKIES->load_cookies();
	if (CFG.WITHOUT_FACE==0){
		list_of_downloads_set_height();
		fs_list_set_size();
	};
	load_defaults();
	if (CFG.WITHOUT_FACE==0){
		prepare_buttons();
		init_timeouts();
	};
	parse_command_line_postload(argv,argc);
	server->run_thread();
	LastTime=get_precise_time();
	var_check_all_limits();
	MainLog->add(_("Normally started"),LOG_WARNING);
	check_for_remote_commands();
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
	while(1){
		check_for_remote_commands();
		main_circle();
		sleep(1);
		TIME_FOR_SAVING-=1;
		if (!TIME_FOR_SAVING) {
			if (CFG.SAVE_LIST) {
				save_list();
			};
			TIME_FOR_SAVING=CFG.SAVE_LIST_INTERVAL * 60;
		};
		if (CFG.EXIT_COMPLETE && aa.complete()){
			COMPLETE_INTERVAL-=1;
			if (COMPLETE_INTERVAL<0){
				break;
			};
		}else{
			COMPLETE_INTERVAL=CFG.EXIT_COMPLETE_TIME * 60;
		};
	};
	save_list();
	save_limits();
	save_passwords(PasswordsForHosts);
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
	MainLog->myprintf(LOG_OK,_("Added downloading of file %s from %s [by user]"),what->info->file.get(),what->info->host.get());
};

void tMain::done() {
	/* There are  we MUST stop all threads!!!
	 */
	server->stop_thread();

	/* removing ftpsearch before removing all queues
	   to avoid segfault at host-limit checks */
	if (ftpsearch) delete(ftpsearch);
	SOUND_SERVER->stop_thread();
	tDownload *tmp=DOWNLOAD_QUEUES[DL_RUN]->last();
	while (tmp) {
		stop_download(tmp);
		tmp=DOWNLOAD_QUEUES[DL_RUN]->last();
	};
	tDownload *temp=DOWNLOAD_QUEUES[DL_STOPWAIT]->last();
	do{
		while(temp) {
			tDownload *temp1=DOWNLOAD_QUEUES[DL_STOPWAIT]->next();
			if (temp->status==DOWNLOAD_REAL_STOP ||
			        temp->status==DOWNLOAD_COMPLETE  ||
			        temp->status==DOWNLOAD_FATAL) {
				prepare_for_stoping(temp,DOWNLOAD_QUEUES[DL_STOPWAIT]);
				real_stop_thread(temp);
				DOWNLOAD_QUEUES[DL_PAUSE]->insert(temp);
			};
			temp=temp1;
		};
		temp=DOWNLOAD_QUEUES[DL_STOPWAIT]->last();
		if (temp==NULL) break;
		sleep(1);
	}while(1);

	for (int i=DL_ALONE+1;i<DL_TEMP;i++)
		delete(DOWNLOAD_QUEUES[i]);
	MainScheduler->save();
	delete(MainScheduler);
	delete(GlobalMeter);
	delete(LocalMeter);
	delete(ALL_DOWNLOADS); // delete or not delete that is the question :-)
	delete(COOKIES);
	MainLog->init_list(NULL);
	MainLog->myprintf(LOG_OK,_("%lu bytes loaded during the session"),GVARS.READED_BYTES);
	MainLog->add(_("Downloader exited normaly"),LOG_OK);
	delete(MainLog);
	delete(LimitsForHosts);
	delete(SpeedScheduler);
	delete(PasswordsForHosts);

	close(LOCK_FILE_D);
	delete (server);
	delete (MsgQueue);
	FILTERS_DB->save_to_ntrc();
	delete(FILTERS_DB);
	/*
	for (int i=URL_HISTORY;i<LAST_HISTORY;i++)
		if (ALL_HISTORIES[i]) delete(ALL_HISTORIES[i]);
	*/
	delete(SOUND_SERVER);
	delete(GVARS.SOCKETS);
	if (LOCK_FILE) remove(LOCK_FILE);
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
		if (what->config.log_save_path.get()){
			char *real_path=parse_save_path(what->config.log_save_path.get(),what->info->file.get());
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
		if (what->config.proxy_host.get()  &&
		    (what->config.proxy_type || addr->proto==D_PROTO_HTTP)) {
			what->who=new tProxyDownload;
			what->download_http();
			pthread_exit(NULL);
			return NULL;
		};
		switch(addr->proto){
		case D_PROTO_SEARCH:
			if (what->who==NULL){
				if (what->config.proxy_host.get())
					what->who=new tProxyDownload;
				else
					what->who=new tHttpDownload;
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

