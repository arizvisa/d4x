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

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>

#if (defined(BSD) && (BSD >= 199306))
#include <sys/msgbuf.h>
#else
#include <sys/msg.h>
#endif

#if (defined(__unix__) || defined(unix)) && !defined(USG)
#include <sys/param.h>
#endif

#include <sys/timeb.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
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
#include "config.h"
#include "ntlocale.h"


tMLog *MainLog=NULL;

tDList *DOWNLOAD_QUEUES[DL_TEMP];
tMeter *GlobalMeter=NULL;
tMeter *LocalMeter=NULL;
key_t LogsMsgQueue;

int amount_of_downloads_in_queues(){
	int num=0;
	for (int i=DL_ALONE+1;i<DL_TEMP;i++)
		num+=DOWNLOAD_QUEUES[i]->count();
	return num;
};
//**********************************************/


void tMain::init() {
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
	PasswordsForHosts=load_passwords();
	read_limits();
	SpeedScheduler=new tSpeedQueue;
	SpeedScheduler->init(0);

	ALL_DOWNLOADS=new tDB;

	LastReadedBytes=0;
	/* Create msgqueue for logs update */
	LogsMsgQueue=100+int(getuid());
	while ((MsgQueue=msgget(LogsMsgQueue,IPC_CREAT|0600))<0) {
		LogsMsgQueue+=1;
	};
	/* Clearing msgqueue */
	int complete=0;
	while (!complete) {
		mbuf Msg;
		complete=1;
		if (msgrcv(MsgQueue,(struct msgbuf *)&Msg,sizeof(Msg)-sizeof(long),1,IPC_NOWAIT)>0 ||
		        msgrcv(MsgQueue,(struct msgbuf *)&Msg,sizeof(Msg)-sizeof(long),2,IPC_NOWAIT)>0)
			complete=0;
	};
	struct sigaction action,old_action;
	action.sa_handler=my_main_quit;
	action.sa_flags=0;//SA_NOCLDSTOP;
	sigaction(SIGINT,&action,&old_action);
//	signal(SIGTERM,my_main_quit);
};

void tMain::load_defaults() {
	MainLog->add(_("Loading default list of downloads"),LOG_OK|LOG_DETAILED);
	tStringList *list=new tStringList;
	list->init(0);
	read_list(list);
	append_list(list);
	delete(list);
	read_list();
};

void tMain::append_list(tStringList *what) {
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
	mbuf Msg;
	int limit = 0;
	while (limit<99) {
		limit++;
		if (msgrcv(MsgQueue,(struct msgbuf *)&Msg,sizeof(Msg)-sizeof(long),1,IPC_NOWAIT)>0) {
			if (Msg.what){
				Msg.which->lock();
				log_window_add_string(Msg.which,Msg.what);
				Msg.which->unlock();
			}else
				del_first_from_log(Msg.which);
		}else
			break;
	};
};

static gint _compare_nodes(gconstpointer a,gconstpointer b){
	gint aa=((tDownload *)(a))->GTKCListRow;
	gint bb=((tDownload *)(b))->GTKCListRow;
	return aa-bb;
};

void tMain::absolute_delete_download(tDList *where,tDownload *what) {
	list_to_delete=g_list_insert_sorted(list_to_delete,what,_compare_nodes);
	if (where) where->del(what);
	ALL_DOWNLOADS->del(what);
};

void tMain::go_to_delete(){
	if (CFG.WITHOUT_FACE==0){
		list_of_downloads_del_list(list_to_delete);
	}else{
		while(list_to_delete){
			tDownload *tmp=(tDownload *)(list_to_delete->data);
			delete(tmp);
			list_to_delete=g_list_remove_link(list_to_delete,list_to_delete);
		};
	};
	list_to_delete=NULL;
};

void tMain::del_completed() {
	MainLog->add(_("Delete completed downloads"),LOG_OK|LOG_DETAILED);
	del_all_from_list(DOWNLOAD_QUEUES[DL_COMPLETE]);
	go_to_delete();
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
	go_to_delete();
};

void tMain::del_all_from_list(tDList *list){
	tDownload *temp=list->first();
	/* delete from begin for speed, becouse
	 * after deleting from GtkClist list of downloads
	 * will be recalculated
	 */
	while (temp) {
		absolute_delete_download(list,temp);
		temp=list->first();
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
		temp->action=ACTION_DELETE;
		temp=DOWNLOAD_QUEUES[DL_STOPWAIT]->prev();
	};
	del_all_from_list(DOWNLOAD_QUEUES[DL_PAUSE]);
	del_all_from_list(DOWNLOAD_QUEUES[DL_WAIT]);
	del_all_from_list(DOWNLOAD_QUEUES[DL_STOP]);
	del_all_from_list(DOWNLOAD_QUEUES[DL_COMPLETE]);
	go_to_delete();
};


int tMain::run_new_thread(tDownload *what) {
	pthread_attr_t attr_p;
	what->status=READY_TO_RUN;
	if (!what->LOG) {
		what->LOG=new tLog;
		what->LOG->init(CFG.MAX_LOG_LENGTH);
	};
	what->WL=new tDefaultWL;
	((tDefaultWL *)(what->WL))->set_log(what->LOG);	
	what->update_trigers();

	what->SpeedLimit=new tSpeed;
	what->SpeedLimit->base = GLOBAL_SLEEP_DELAY * what->config.speed;
	if (CFG.SPEED_LIMIT<3 && CFG.SPEED_LIMIT>0)
		what->SpeedLimit->bytes = 512; //allow to read 512 bytes
	else
		what->SpeedLimit->bytes = what->SpeedLimit->base;

	SpeedScheduler->insert(what->SpeedLimit);
	if (what->editor) what->editor->disable_ok_button();

	MainLog->myprintf(LOG_OK|LOG_DETAILED,_("Run new thread for %z"),what);

	pthread_attr_init(&attr_p);
	pthread_attr_setdetachstate(&attr_p,PTHREAD_CREATE_JOINABLE);
	pthread_attr_setscope(&attr_p,PTHREAD_SCOPE_SYSTEM);
	return (pthread_create(&what->thread_id,&attr_p,download_last,(void *)what));
};

void tMain::stop_split(tDownload *what){
	tDownload *tmp=what->split->next_part;
	what->split->status=1;
	while(tmp){
		stop_thread(tmp);
		tmp=tmp->split?tmp->split->next_part:NULL;
	};
};

void tMain::stop_download(tDownload *what) {
	if (DOWNLOAD_QUEUES[DL_STOPWAIT]->owner(what) && what->action!=ACTION_DELETE) {
		what->action=ACTION_STOP;
		return;
	};
	if (DOWNLOAD_QUEUES[DL_RUN]->owner(what)) {
		DOWNLOAD_QUEUES[DL_RUN]->del(what);
		MainLog->myprintf(LOG_WARNING,_("Downloading of file %s from %s was terminated [by user]"),
				  what->info->file.get(),
				  what->info->host.get());
		if (what->split)
			stop_split(what);
		if (!stop_thread(what) || what->split) {
			DOWNLOAD_QUEUES[DL_STOPWAIT]->insert(what);
		} else {
			LimitsForHosts->decrement(what);
			DOWNLOAD_QUEUES[DL_PAUSE]->insert(what);
		};
		what->Status.clear();
	} else {
		if (DOWNLOAD_QUEUES[DL_WAIT]->owner(what)) {
			DOWNLOAD_QUEUES[DL_WAIT]->del(what);
			DOWNLOAD_QUEUES[DL_PAUSE]->insert(what);
			what->Status.clear();
		};
	};
};

int tMain::delete_download(tDownload *what) {
	if (!what) return 0;
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
	absolute_delete_download(NULL,what);
	return 1;
};

void tMain::try_to_run_split(tDownload *what){
	if (what->status==DOWNLOAD_GO || what->status==DOWNLOAD_COMPLETE){
		if (what->finfo.size>MINIMUM_SIZE_TO_SPLIT){
			int part=what->split->NumOfParts-1;
			tSortString *tmp=LimitsForHosts->find(what->info->host.get(),what->info->port);
			tDownload *dwn=what;
			while (part>0){
				if (tmp && tmp->curent>=tmp->upper)
					return;
				dwn->prepare_next_split();
				if (run_new_thread(dwn->split->next_part)){
					delete(dwn->split->next_part);
					dwn->split->next_part=NULL;
					return;
				};
				if (tmp) tmp->increment();
				dwn=dwn->split->next_part;
				part-=1;
			};
			what->split->status=1;
		}else{
			what->split->status=1;
		};
	};
};

int tMain::try_to_run_download(tDownload *what){
	tSortString *tmp=LimitsForHosts->find(what->info->host.get(),what->info->port);
	time_t NOW;
	time(&NOW);
	if (DOWNLOAD_QUEUES[DL_RUN]->count()<50 && what->ScheduleTime<=NOW
	    && (tmp==NULL || tmp->curent<tmp->upper)) {
		if (what->split){
			what->finfo.size=-1;
			what->split->FirstByte=0;
			what->split->LastByte=-1;
			what->split->status=0;
			what->config.rollback=0;
			what->config.ftp_recurse_depth=0;
			what->config.http_recurse_depth=0;
		};
		if (run_new_thread(what)) return -1;
		if (tmp) tmp->increment();
		return 1;
	};
	return 0;
};

void tMain::continue_download(tDownload *what) {
	if (!what) return;
	switch (what->owner) {
	case DL_STOPWAIT:
		if (what->action!=ACTION_DELETE) what->action=ACTION_CONTINUE;
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
		if (try_to_run_download(what)) {
			DOWNLOAD_QUEUES[DL_RUN]->insert(what);
		} else {
			tDownload *temp=DOWNLOAD_QUEUES[DL_WAIT]->last();
			if (!temp || temp->GTKCListRow < what->GTKCListRow)
				DOWNLOAD_QUEUES[DL_WAIT]->insert(what);
			else {
				temp=DOWNLOAD_QUEUES[DL_WAIT]->first();
				while (temp && temp->GTKCListRow < what->GTKCListRow)
					temp=DOWNLOAD_QUEUES[DL_WAIT]->prev();
				DOWNLOAD_QUEUES[DL_WAIT]->insert_before(what,temp);
			};
		};
	};
	what->Attempt.clear();
};

int tMain::complete() {
	return((DOWNLOAD_QUEUES[DL_WAIT]->count()+DOWNLOAD_QUEUES[DL_RUN]->count()+DOWNLOAD_QUEUES[DL_STOPWAIT])>0?0:1);
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
	switch(what->finfo.type) {
	case T_FILE:{
		int REAL_SIZE=what->finfo.size;
		if (REAL_SIZE==0 && what->who!=NULL)
			what->finfo.size=REAL_SIZE=what->who->another_way_get_size();
		if (what->who) what->Size.set(what->who->get_readed());
		what->Remain.set(REAL_SIZE-what->Size.curent);
		int period=int(time(NULL)-what->Start);
		if (period!=0 && what->who) {
			float tmp=float(what->Size.curent-what->who->get_start_size());
			if (tmp>0) {
				what->Speed.set(int(tmp/period));
			};
		};
	};
	};
};

int tMain::get_split_loaded(tDownload *what){
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
	char data[MAX_LEN];
	switch(what->finfo.type) {
	case T_FILE:{
		if (what->finfo.type!=what->finfo.oldtype) list_of_downloads_change_data(what->GTKCListRow,FILE_TYPE_COL,_("file"));
		int REAL_SIZE=what->finfo.size;
		if (REAL_SIZE==0 && what->who!=NULL)
			what->finfo.size=REAL_SIZE=what->who->another_way_get_size();
		if (REAL_SIZE<0) REAL_SIZE=0;
		make_number_nice(data,REAL_SIZE);		
		list_of_downloads_change_data(what->GTKCListRow,FULL_SIZE_COL,data);
		if (what->who){
			if (what->split)
				what->Size.set(get_split_loaded(what));
			else
				what->Size.set(what->who->get_readed());
		};
		what->Remain.set(REAL_SIZE-what->Size.curent);
		if (what->Remain.change() && what->Remain.curent>=0){
			make_number_nice(data,what->Remain.curent);
			list_of_downloads_change_data(what->GTKCListRow,REMAIN_SIZE_COL,data);
		};
		if (what->Size.change() || CFG.NICE_DEC_DIGITALS.change()) {
			make_number_nice(data,what->Size.curent);
			list_of_downloads_change_data(what->GTKCListRow,DOWNLOADED_SIZE_COL,data);
			what->Pause=time(NULL);
			list_of_downloads_change_data(what->GTKCListRow,PAUSE_COL,"");
			// calculating speed for graph
			unsigned int TimeLeft=get_precise_time()-LastTime;
			if (what->Size.old==0 && what->who) what->Size.old=what->who->get_start_size();
			if (TimeLeft!=0 && what->Size.curent>what->Size.old)
				what->NanoSpeed=((what->Size.curent-what->Size.old)*1000)/TimeLeft;
			else
				what->NanoSpeed=0;
			what->Size.reset();
		} else {
			what->NanoSpeed=0;
			if (what->status==DOWNLOAD_GO) {
				int Pause=time(NULL)-what->Pause;
				if (Pause>=30) {
					convert_time(Pause,data);
					list_of_downloads_change_data(what->GTKCListRow,PAUSE_COL,data);
				};
			};
		};
		if (what->Start>0) {
			convert_time(time(NULL)-what->Start,data);
			list_of_downloads_change_data(what->GTKCListRow,TIME_COL,data);
		};
		int temp;
		if (REAL_SIZE!=0)
			temp=int((float(what->Size.curent)/float(REAL_SIZE))*100);
		else
			temp=100;
/* setting new title of log*/
		if (CFG.USE_MAINWIN_TITLE){
			char title[MAX_LEN];
			sprintf(title,"%i%% %i/%i %s",temp,what->Size.curent,REAL_SIZE,what->info->file.get());
			log_window_set_title(what,title);
		};

		what->Percent.set(temp);
		sprintf(data,"%i",temp);
		if (what->Percent.change()) {
			list_of_downloads_change_data(what->GTKCListRow,PERCENT_COL,data);
			what->Percent.reset();
		};
				/*	Speed calculation
				 */
		int period=int(time(NULL)-what->Start);
		if (period!=0 && what->who) {
			float tmp=float(what->Size.curent-what->who->get_start_size());
			if (tmp>0) {
				what->Speed.set(int(tmp/period));
				if (what->Speed.change()) {
					sprintf(data,"%i",what->Speed.curent);
					list_of_downloads_change_data(what->GTKCListRow,SPEED_COL,data);
					what->Speed.reset();
				};
			};
			/*	Remaining time calculation
			 */
			if (tmp>0) {
				tmp=(float(REAL_SIZE-what->Size.curent)*float(period))/tmp;
				if (tmp>=0 && tmp<24*3660) {
					convert_time(int(tmp),data);
					list_of_downloads_change_data(what->GTKCListRow,ELAPSED_TIME_COL,data);
				} else
					list_of_downloads_change_data(what->GTKCListRow,ELAPSED_TIME_COL,"...");
			} else
				list_of_downloads_change_data(what->GTKCListRow,ELAPSED_TIME_COL,"...");
		};
		break;
	};
	case T_DIR:{
		if (what->finfo.type!=what->finfo.oldtype) list_of_downloads_change_data(what->GTKCListRow,FILE_TYPE_COL,_("dir"));
		break;
	};
	case T_LINK:{
		if (what->finfo.type!=what->finfo.oldtype) list_of_downloads_change_data(what->GTKCListRow,FILE_TYPE_COL,_("link"));
		break;
	};
	case T_DEVICE:{
		if (what->finfo.type!=what->finfo.oldtype) list_of_downloads_change_data(what->GTKCListRow,FILE_TYPE_COL,_("device"));
		break;
	};
	default:
		if (what->finfo.type!=what->finfo.oldtype) list_of_downloads_change_data(what->GTKCListRow,FILE_TYPE_COL,"???");
	};
	if (what->finfo.type==T_DIR || what->finfo.type==T_NONE){
		if (what->who) what->Size.set(what->who->get_readed());
		if (what->Size.change() || CFG.NICE_DEC_DIGITALS.change()) {
			char data[MAX_LEN];
			make_number_nice(data,what->Size.curent);
			list_of_downloads_change_data(what->GTKCListRow,DOWNLOADED_SIZE_COL,data);
			what->Size.reset();
		};				
	};
	if (what->who) {
		what->Status.set(what->who->get_status());
		if (what->Status.curent==D_DOWNLOAD && !what->who->reget())
			what->Status.set(D_DOWNLOAD_BAD);
		if (what->Status.change()) {
			what->Status.reset();
			list_of_downloads_set_run_icon(what);
		};
		what->Attempt.set(what->who->treat());
		if (what->Attempt.change()) {
			what->Attempt.reset();
			if (what->config.number_of_attempts > 0)
				sprintf(data,"%i/%i",what->Attempt.curent, what->config.number_of_attempts);
			else
				sprintf(data,"%i",what->Attempt.curent);
			list_of_downloads_change_data(what->GTKCListRow,TREAT_COL,data);
		};
	};
	what->finfo.oldtype=what->finfo.type;
};


void tMain::redirect(tDownload *what) {
	char *newurl=NULL;
	if (what->who){
		newurl = what->who->get_new_url();
		what->delete_who();
	};
	if (newurl) {
		char *oldurl=what->info->url();
		tDownload *temp=DOWNLOAD_QUEUES[DL_WAIT]->first();
		if (temp)
			DOWNLOAD_QUEUES[DL_WAIT]->insert_before(what,temp);
		else
			DOWNLOAD_QUEUES[DL_WAIT]->insert(what);
		tAddr *addr=new tAddr(newurl);
		if (*newurl=='/'){
			addr->copy_host(what->info);
		};
		delete(newurl);
		ALL_DOWNLOADS->del(what);
		if (what->info) delete (what->info);
		what->info=addr;
		what->config.referer.set(oldurl);
		delete(oldurl);
		if (ALL_DOWNLOADS->find(what)) {
			DOWNLOAD_QUEUES[DL_WAIT]->del(what);
			list_to_delete=g_list_insert_sorted(list_to_delete,what,_compare_nodes);
			return;
		};
		ALL_DOWNLOADS->insert(what);
//		normalize_path(what->get_SavePath());
		what->finfo.type=what->status=0;
		what->finfo.size=-1;
		if (CFG.WITHOUT_FACE==0){
			char *URL=what->info->url();
			list_of_downloads_change_data(what->GTKCListRow,URL_COL,URL);
			delete (URL);
			list_of_downloads_change_data(what->GTKCListRow,FILE_COL,what->info->file.get());
			for (int i=FILE_TYPE_COL;i<URL_COL;i++)
				list_of_downloads_change_data(what->GTKCListRow,i,"");
		};
	} else {
		MainLog->myprintf(LOG_ERROR,_("Redirection from [%z] to nowhere!"),what);
		DOWNLOAD_QUEUES[DL_COMPLETE]->insert(what);
		what->finfo.type=T_NONE;
	};
};

void tMain::prepare_for_stoping(tDownload *what,tDList *list) {
	MainLog->myprintf(LOG_OK|LOG_DETAILED,_("Prepare [%z] for stoping"),what);
	LimitsForHosts->decrement(what);
	if (what->editor) what->editor->enable_ok_button();
	SpeedScheduler->del(what->SpeedLimit);
	delete (what->SpeedLimit);
	what->SpeedLimit=NULL;
	if (list) list->del(what);
	if (what->WL){
		delete(what->WL);
		what->WL=NULL;
	};
	if (what->split && what->split->next_part){
		prepare_for_stoping(what->split->next_part,NULL);
		delete(what->split->next_part);
		what->split->next_part=NULL;
	};
};

void tMain::case_download_completed(tDownload *what){
	prepare_for_stoping(what,DOWNLOAD_QUEUES[DL_RUN]);
	printf("here");
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
		if (CFG.DELETE_COMPLETED ) {
			MainLog->myprintf(LOG_WARNING|LOG_DETAILED,_("%z was deleted from queue of downloads as completed download"),what);
			absolute_delete_download(NULL,what);
		} else {
			DOWNLOAD_QUEUES[DL_COMPLETE]->insert(what);
			what->delete_who();
		};
	};
};

void tMain::case_download_failed(tDownload *what){
	prepare_for_stoping(what,DOWNLOAD_QUEUES[DL_RUN]);
	MainLog->myprintf(LOG_ERROR,_("Downloading of file %z was terminated by fatal error"),what);
	if (CFG.DELETE_FATAL) {
		MainLog->myprintf(LOG_WARNING|LOG_DETAILED,_("%z was deleted from queue of downloads as failed download"),what);
		absolute_delete_download(NULL,what);
	} else {
		DOWNLOAD_QUEUES[DL_STOP]->insert(what);
		what->delete_who();
	};
};

int tMain::get_status_split(tDownload *what){
	tDownload *tmp=what;
	int status[3]={0,0,0};
	while (tmp){
		switch(tmp->status){
		case DOWNLOAD_COMPLETE:{
			status[0]+=1;
			break;
		};
		case DOWNLOAD_REAL_STOP:{
			status[1]+=1;
			break;
		};
		case DOWNLOAD_FATAL:{
			if (what->owner==DL_RUN){
				tmp->status=DOWNLOAD_REAL_STOP;
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
		tmp=tmp->split->next_part;
	};
	if (status[2]) return DOWNLOAD_GO;
	if (status[0]==0){
		return DOWNLOAD_REAL_STOP;
	};
	return DOWNLOAD_COMPLETE;
};

void tMain::main_circle_first(){
/* look for stopped threads */
	tDownload *temp=DOWNLOAD_QUEUES[DL_STOPWAIT]->last();
	while(temp) {
		tDownload *temp1=DOWNLOAD_QUEUES[DL_STOPWAIT]->next();
		if (temp->split) temp->status=get_status_split(temp);
		if (temp->status==DOWNLOAD_REAL_STOP ||
		        temp->status==DOWNLOAD_COMPLETE  ||
		        temp->status==DOWNLOAD_FATAL) {
			real_stop_thread(temp);
			prepare_for_stoping(temp,DOWNLOAD_QUEUES[DL_STOPWAIT]);
			DOWNLOAD_QUEUES[DL_PAUSE]->insert(temp);
			switch(temp->action){
			case ACTION_DELETE:
				delete_download(temp);
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
	while(temp) {
		tDownload *temp1=DOWNLOAD_QUEUES[DL_RUN]->next();
		int status=temp->status;
		if (temp->split){
			status=DOWNLOAD_GO;
			if (temp->split->status==0){
				try_to_run_split(temp);
			}else{
				status=get_status_split(temp);
			};
		};
		if (CFG.WITHOUT_FACE==0) print_info(temp);
		else speed_calculation(temp);
		switch(status) {
			case DOWNLOAD_COMPLETE:{
				case_download_completed(temp);
				break;
			};
			case DOWNLOAD_FATAL:{
				case_download_failed(temp);
			};
		};
		temp=temp1;
	};
};

void tMain::main_circle() {
	main_circle_first();
	main_circle_second();
/* look for run new */
	tDownload *temp=DOWNLOAD_QUEUES[DL_WAIT]->first();
	while(temp && DOWNLOAD_QUEUES[DL_RUN]->count()<CFG.MAX_THREADS) {
		tDownload *temp_next=DOWNLOAD_QUEUES[DL_WAIT]->prev();
		int rvalue=try_to_run_download(temp);
		if (rvalue<0){
			MainLog->add(_("Can't run new thread for downloading!"),LOG_ERROR);
			break;
		};
		if (rvalue) {
			DOWNLOAD_QUEUES[DL_WAIT]->del(temp);
			DOWNLOAD_QUEUES[DL_RUN]->insert(temp);
		};
		temp=temp_next;
	};
/* various stuff */
	go_to_delete();
	speed();
	if (CFG.WITHOUT_FACE==0)
		prepare_buttons();
	CFG.NICE_DEC_DIGITALS.reset();
};

void tMain::check_for_remote_commands(){
	tString *addnew=server->get_string();
	int i=0;
	while (addnew){
		switch (addnew->temp){
		case PACKET_RERUN_FAILED:{
			rerun_failed();
			break;
		};
		case PACKET_ADD_OPEN:{
			if (CFG.WITHOUT_FACE==0){
				init_add_dnd_window(addnew->body);
				break;
			};
		};
		case PACKET_ADD:{
			MainLog->myprintf(LOG_FROM_SERVER,_("Adding downloading via control socket [%s]"),addnew->body);
			add_downloading(addnew->body,CFG.LOCAL_SAVE_PATH,NULL);
			break;
		};
		case PACKET_SET_SPEED_LIMIT:{
			sscanf(addnew->body,"%i",&CFG.SPEED_LIMIT);
			if (CFG.SPEED_LIMIT>3) CFG.SPEED_LIMIT=3;
			if (CFG.SPEED_LIMIT<1) CFG.SPEED_LIMIT=1;
			if (CFG.WITHOUT_FACE==0) set_speed_buttons();
			MainLog->myprintf(LOG_FROM_SERVER|LOG_DETAILED,_("Set speed limitation to %s %s"),
					  _(SPEED_LIMITATIONS_NAMES[CFG.SPEED_LIMIT]),
					  _("[control socket]"));
			break;
		};
		case PACKET_SET_SAVE_PATH:{
			delete(CFG.LOCAL_SAVE_PATH);
			CFG.LOCAL_SAVE_PATH=copy_string(addnew->body);
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
		};
		delete(addnew);
		i+=1;
		if (i>10) break;
		addnew=server->get_string();
	};
};
//**********************************************/
int tMain::add_downloading(tDownload *what) {
	if (ALL_DOWNLOADS->find(what) || !what->info->is_valid()) 
		return 1;
	if (what->info->username.get()==NULL){
		tUserPass *tmp=PasswordsForHosts->find(what->info->proto,
						       what->info->host.get());
		if (tmp){
			what->info->username.set(tmp->user.get());
			what->info->pass.set(tmp->pass.get());
		};
	};
	ALL_DOWNLOADS->insert(what);
	if (CFG.WITHOUT_FACE==0)
		list_of_downloads_add(what);
	DOWNLOAD_QUEUES[DL_WAIT]->insert(what);
	return 0;
};

void tMain::add_downloading_to(tDownload *what) {
	int owner=what->owner;
	if (owner>DL_ALONE && owner<DL_TEMP){
		if (add_downloading(what)){
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

int tMain::add_downloading(char *adr,char *where,char *name) {
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
	return(tp.tv_sec*1000+tp.tv_usec);
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
				SPEED_LIMIT=GLOBAL_SLEEP_DELAY*CFG.SPEED_LIMIT_1;
				break;
			};
		case 2:	{
				SPEED_LIMIT=GLOBAL_SLEEP_DELAY*CFG.SPEED_LIMIT_2;
				break;
			};
		case 3:
		default:{
				SPEED_LIMIT=0;
		};
	};
	if (SPEED_LIMIT-bytes>0)
		SPEED_LIMIT+=SPEED_LIMIT-bytes;
	SpeedScheduler->schedule(SPEED_LIMIT);
};

void tMain::run_msg_server(){
	server=new tMsgServer;
	pthread_attr_t attr_p;
	pthread_attr_init(&attr_p);
	pthread_attr_setdetachstate(&attr_p,PTHREAD_CREATE_JOINABLE);
	pthread_attr_setscope(&attr_p,PTHREAD_SCOPE_SYSTEM);
	pthread_create(&server_thread_id,&attr_p,server_thread_run,server);
};

void tMain::run(int argv,char **argc) {
	if (CFG.WITHOUT_FACE==0){
		init_face(argv,argc);
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
	if (CFG.WITHOUT_FACE==0)
		list_of_downloads_set_height();
	load_defaults();
	if (CFG.WITHOUT_FACE==0){
		prepare_buttons();
		init_timeouts();
	};
	parse_command_line_postload(argv,argc);
	run_msg_server();
	LastTime=get_precise_time();
	var_check_all_limits();
	MainLog->add(_("Normally started"),LOG_WARNING);
	check_for_remote_commands();
	if (CFG.WITHOUT_FACE==0)
		gtk_main();
	else{
		run_without_face();
	};
};

void tMain::run_without_face(){
	int TIME_FOR_SAVING=CFG.SAVE_LIST_INTERVAL * 60;
	while(1){
		main_circle();
		sleep(1);
		TIME_FOR_SAVING-=1;
		if (!TIME_FOR_SAVING) {
			if (CFG.SAVE_LIST) {
				save_list();
			};
			TIME_FOR_SAVING=CFG.SAVE_LIST_INTERVAL * 60;
		};
		check_for_remote_commands();
	};
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
	int *rc;
	pthread_kill(server_thread_id,SIGUSR2);
	pthread_join(server_thread_id,(void **)&rc);

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
				real_stop_thread(temp);
				prepare_for_stoping(temp,DOWNLOAD_QUEUES[DL_STOPWAIT]);
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
	delete(GlobalMeter);
	delete(LocalMeter);
	delete(ALL_DOWNLOADS);
	delete(COOKIES);

	MainLog->add(_("Downloader exited normaly"),LOG_OK);
	delete(MainLog);
	delete(LimitsForHosts);
	delete(SpeedScheduler);

	msgctl(MsgQueue,IPC_RMID,NULL);
	close(LOCK_FILE_D);
	delete (server);
	/*
	for (int i=URL_HISTORY;i<LAST_HISTORY;i++)
		if (ALL_HISTORIES[i]) delete(ALL_HISTORIES[i]);
		*/
	if (LOCK_FILE) remove(LOCK_FILE);
};

void tMain::reinit_main_log() {
	MainLog->reinit(CFG.MAX_MAIN_LOG_LENGTH);
	MainLog->reinit_file();
};
//*****************************************************************/
//-----------------------------------------------------------------
/* to avoiding problem with stack better way is calling pthread_exit()
   from download_last().
 */
void *download_last(void *nothing) {
	tDownload *what=(tDownload *)nothing;
	my_pthread_key_init();
	my_pthread_key_set(what);
	init_signal_handler();
	if (what) {
		tAddr *addr=what->info;
		what->LOG->MsgQueue=msgget(LogsMsgQueue,0);
		if (what->LOG->MsgQueue<0) {
			what->WL->log(LOG_ERROR,_("Can't open messages queue!"));
			what->status=DOWNLOAD_FATAL;
			pthread_exit(NULL);
			return NULL;
		};
		if (addr->proto==D_PROTO_UNKNOWN){
			what->WL->log(LOG_ERROR,_("Such protocol is not supported!"));
			what->download_failed();
			pthread_exit(NULL);
			return NULL;
		};
		if (what->config.proxy_host.get()  &&
		    (what->config.proxy_type || addr->proto==D_PROTO_HTTP)) {
			what->who=new tProxyDownload;
			what->download_http();
			pthread_exit(NULL);
			return NULL;
		};
		if (addr->proto==D_PROTO_HTTP){
			what->download_http();
			pthread_exit(NULL);
			return NULL;
		};
		what->download_ftp();
	};
	pthread_exit(NULL);
	return NULL;
};
//------------------------------------------------------------------------

