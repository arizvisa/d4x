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

//**********************************************/

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
	LogsMsgQueue=100;
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
	MainLog->add(_("Append list to curent queue of downloads"),LOG_OK|LOG_DETAILED);
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
	list_of_downloads_del_list(list_to_delete);
	list_to_delete=NULL;
};

void tMain::del_completed() {
	MainLog->add(_("Delete completed downloads"),LOG_OK|LOG_DETAILED);
	del_all_from_list(DOWNLOAD_QUEUES[DL_COMPLETE]);
	go_to_delete();
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

void tMain::stop_download(tDownload *what) {
	if (DOWNLOAD_QUEUES[DL_STOPWAIT]->owner(what) && what->action!=ACTION_DELETE) {
		what->action=ACTION_STOP;
		return;
	};
	if (DOWNLOAD_QUEUES[DL_RUN]->owner(what)) {
		DOWNLOAD_QUEUES[DL_RUN]->del(what);
		MainLog->myprintf(LOG_WARNING,_("Downloading of file %s from %s was terminated [by user]"),what->info->get_file(),what->info->get_host());
		if (!stop_thread(what)) {
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
	MainLog->myprintf(LOG_WARNING,_("Delete file %s from queue of downloads"),what->info->get_file());
	absolute_delete_download(NULL,what);
	return 1;
};

int tMain::try_to_run_download(tDownload *what){
	tSortString *tmp=LimitsForHosts->find(what->info->get_host(),what->info->port);
	time_t NOW;
	time(&NOW);
	if (DOWNLOAD_QUEUES[DL_RUN]->count()<CFG.MAX_THREADS && what->ScheduleTime<=NOW
	    && (tmp==NULL || tmp->curent<tmp->upper)
	    && run_new_thread(what)==0) {
		if (tmp) tmp->increment();
		return 1;
	};
	return 0;
};

void tMain::continue_download(tDownload *what) {
	if (DOWNLOAD_QUEUES[DL_STOPWAIT]->owner(what)) {
		if (what->action!=ACTION_DELETE) what->action=ACTION_CONTINUE;
		return;
	};
	if (DOWNLOAD_QUEUES[DL_RUN]->owner(what)) {
		stop_download(what);
		if (DOWNLOAD_QUEUES[DL_STOPWAIT]->owner(what)){
			what->action=ACTION_CONTINUE;
			return;
		};
	};

	MainLog->myprintf(LOG_OK,_("Continue downloading of file %s from %s..."),what->info->get_file(),what->info->get_host());

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

void tMain::print_info(tDownload *what) {
	char data[MAX_LEN];
	switch(what->finfo.type) {
		case T_FILE:
			{
				if (what->finfo.type!=what->finfo.oldtype) list_of_downloads_change_data(what->GTKCListRow,FILE_TYPE_COL,_("file"));
				int REAL_SIZE=what->finfo.size;
				if (REAL_SIZE==0 && what->who!=NULL)
					what->finfo.size=REAL_SIZE=what->who->another_way_get_size();
				make_number_nice(data,REAL_SIZE);
				list_of_downloads_change_data(what->GTKCListRow,FULL_SIZE_COL,data);
				if (what->who) what->Size.set(what->who->get_readed());
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
					sprintf(title,"%i%% %i/%i %s",temp,what->Size.curent,REAL_SIZE,what->info->get_file());
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
		case T_DIR:
			{
				if (what->finfo.type!=what->finfo.oldtype) list_of_downloads_change_data(what->GTKCListRow,FILE_TYPE_COL,_("dir"));
				break;
			};
		case T_LINK:
			{
				if (what->finfo.type!=what->finfo.oldtype) list_of_downloads_change_data(what->GTKCListRow,FILE_TYPE_COL,_("link"));
				break;
			};
		case T_DEVICE:
			{
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
		delete(what->who);
		what->who = NULL;
	};
	if (newurl) {
		tDownload *temp=DOWNLOAD_QUEUES[DL_WAIT]->first();
		if (temp)
			DOWNLOAD_QUEUES[DL_WAIT]->insert_before(what,temp);
		else
			DOWNLOAD_QUEUES[DL_WAIT]->insert(what);
		tAddr *addr=new tAddr(newurl);
		delete(newurl);
		ALL_DOWNLOADS->del(what);
		if (what->info) delete (what->info);
		what->info=addr;
		if (ALL_DOWNLOADS->find(what)) {
			DOWNLOAD_QUEUES[DL_WAIT]->del(what);
			list_to_delete=g_list_insert_sorted(list_to_delete,what,_compare_nodes);
			return;
		};
		ALL_DOWNLOADS->insert(what);
//		normalize_path(what->get_SavePath());
		what->finfo.type=what->status=0;
		what->finfo.size=-1;
		char *URL=what->info->url();
		list_of_downloads_change_data(what->GTKCListRow,URL_COL,URL);
		delete (URL);
		list_of_downloads_change_data(what->GTKCListRow,FILE_COL,what->info->get_file());
		for (int i=FILE_TYPE_COL;i<URL_COL;i++)
			list_of_downloads_change_data(what->GTKCListRow,i,"");
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
	list->del(what);
};

void tMain::case_download_completed(tDownload *what){
	prepare_for_stoping(what,DOWNLOAD_QUEUES[DL_RUN]);
	if (what->finfo.type==T_REDIRECT) {
		MainLog->myprintf(LOG_OK,_("Redirect from %z"),what);
		redirect(what);
	}else{
		if (what->who->file_type()==T_DIR) {
			MainLog->myprintf(LOG_OK,_("Downloading of directory %z was completed"),what);
			if (what->config.ftp_recurse_depth!=1) add_dir(what);
		} else {
			int bytes = what->finfo.size==0 ? what->who->get_readed():what->finfo.size;
			MainLog->myprintf(LOG_OK,_("Downloading of file %z (%i bytes) was completed at speed %i bytes/sec"),what,bytes,what->Speed.curent);
			if (what->config.http_recurse_depth!=1 && what->DIR)
				add_dir(what);
		};
		if (CFG.DELETE_COMPLETED ) {
			MainLog->myprintf(LOG_WARNING|LOG_DETAILED,_("%z was deleteted from queue of downloads as completed download"),what);
			absolute_delete_download(NULL,what);
		} else {
			if (what->who) delete what->who;
			what->who=NULL;
			DOWNLOAD_QUEUES[DL_COMPLETE]->insert(what);
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
		if (what->who) delete what->who;
		what->who=NULL;
		DOWNLOAD_QUEUES[DL_STOP]->insert(what);
	};
};

void tMain::main_circle() {
/* look for stopped threads */
	tDownload *temp=DOWNLOAD_QUEUES[DL_STOPWAIT]->last();
	while(temp) {
		tDownload *temp1=DOWNLOAD_QUEUES[DL_STOPWAIT]->next();
		if (temp->status==DOWNLOAD_REAL_STOP ||
		        temp->status==DOWNLOAD_COMPLETE  ||
		        temp->status==DOWNLOAD_FATAL) {
			real_stop_thread(temp);
			prepare_for_stoping(temp,DOWNLOAD_QUEUES[DL_STOPWAIT]);
			DOWNLOAD_QUEUES[DL_PAUSE]->insert(temp);
			if (temp->action==ACTION_DELETE) {
				delete_download(temp);
			} else {
				if (temp->action==ACTION_CONTINUE)
					continue_download(temp);
				else
					stop_download(temp);
				temp->action=ACTION_NONE;
			};
		};
		temp=temp1;
	};
/* look for completeted or faild threads */
	temp=DOWNLOAD_QUEUES[DL_RUN]->last();
	while(temp) {
		tDownload *temp1=DOWNLOAD_QUEUES[DL_RUN]->next();
		print_info(temp);
		switch(temp->status) {
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
/* look for added remotely */
//	check_for_remote_commands();
/* look for run new */
	temp=DOWNLOAD_QUEUES[DL_WAIT]->first();
	while(temp && DOWNLOAD_QUEUES[DL_RUN]->count()<CFG.MAX_THREADS) {
		if (try_to_run_download(temp)) {
			DOWNLOAD_QUEUES[DL_WAIT]->del(temp);
			DOWNLOAD_QUEUES[DL_RUN]->insert(temp);
			temp=DOWNLOAD_QUEUES[DL_WAIT]->first();
		} else
			temp=DOWNLOAD_QUEUES[DL_WAIT]->prev();
	};
/* various stuff */
	go_to_delete();
	speed();
	prepare_buttons();
	CFG.NICE_DEC_DIGITALS.reset();
};

void tMain::check_for_remote_commands(){
	tString *addnew=server->get_string();
	int i=0;
	while (addnew){
		switch (addnew->temp){
		case PACKET_ADD_OPEN:{
			init_add_dnd_window(addnew->body);
			break;
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
			set_speed_buttons();
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
			main_window_iconify();
			break;
		case PACKET_POPUP:
			main_window_popup();
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
	if (ALL_DOWNLOADS->find(what)) 
		return 1;
	if (what->info->get_username()==NULL){
		tUserPass *tmp=PasswordsForHosts->find(what->info->proto,
						       what->info->get_host());
		if (tmp){
			what->info->set_username(tmp->get_user());
			what->info->set_pass(tmp->get_pass());
		};
	};
	ALL_DOWNLOADS->insert(what);
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
	if (!addr->get_host()) return -1;
	tDownload *whatadd=new tDownload;
	whatadd->info=addr;
	if (where!=NULL && strlen(where)>0) {
		whatadd->config.set_save_path(where);
	} else
		whatadd->config.set_save_path(CFG.GLOBAL_SAVE_PATH);
	if (strlen(addr->get_file())==0) {
		whatadd->finfo.type=T_DIR;
		whatadd->finfo.size=0;
	};

	if (add_downloading(whatadd)) {
		delete(whatadd);
		return -1;
	};
//	normalize_path(whatadd->get_SavePath());

	if (name && strlen(name))
		whatadd->config.set_save_name(name);
	whatadd->set_default_cfg();

	whatadd->config.proxy_type=CFG.FTP_PROXY_TYPE;
	switch(whatadd->info->proto){
	case D_PROTO_FTP:{
		if (CFG.USE_PROXY_FOR_FTP) {
			whatadd->config.set_proxy_host(CFG.FTP_PROXY_HOST);
			whatadd->config.proxy_port=CFG.FTP_PROXY_PORT;
			if (CFG.NEED_PASS_FTP_PROXY) {
				whatadd->config.set_proxy_user(CFG.FTP_PROXY_USER);
				whatadd->config.set_proxy_pass(CFG.FTP_PROXY_PASS);
			};
		};
		break;
	};
	case D_PROTO_HTTP:{
		if (CFG.USE_PROXY_FOR_HTTP) {
			whatadd->config.set_proxy_host(CFG.HTTP_PROXY_HOST);
			whatadd->config.proxy_port=CFG.HTTP_PROXY_PORT;
			if (CFG.NEED_PASS_HTTP_PROXY) {
				whatadd->config.set_proxy_user(CFG.HTTP_PROXY_USER);
				whatadd->config.set_proxy_pass(CFG.HTTP_PROXY_PASS);
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
	init_face(argv,argc);
	DOWNLOAD_QUEUES[DL_COMPLETE]->init_pixmap(PIX_COMPLETE);
	DOWNLOAD_QUEUES[DL_RUN]->init_pixmap(PIX_RUN_PART);
	DOWNLOAD_QUEUES[DL_WAIT]->init_pixmap(PIX_WAIT);
	DOWNLOAD_QUEUES[DL_PAUSE]->init_pixmap(PIX_PAUSE);
	DOWNLOAD_QUEUES[DL_STOP]->init_pixmap(PIX_STOP);
	DOWNLOAD_QUEUES[DL_STOPWAIT]->init_pixmap(PIX_STOP_WAIT);
	init_main_log();
	MainLog->add(VERSION_NAME,LOG_WARNING);
	COOKIES=new tCookiesTree;
	COOKIES->load_cookies();
	list_of_downloads_set_height();
	load_defaults();
	prepare_buttons();
	init_timeouts();
	parse_command_line_postload(argv,argc);
	run_msg_server();
	LastTime=get_precise_time();
	var_check_all_limits();
	MainLog->add(_("Normally started"),LOG_WARNING);
	check_for_remote_commands();
	gtk_main();
};

void tMain::run_after_quit(){
	if (CFG.EXEC_WHEN_QUIT && strlen(CFG.EXEC_WHEN_QUIT))
		system(CFG.EXEC_WHEN_QUIT);
};

void tMain::add_download_message(tDownload *what) {
	if (!what) return;
	MainLog->myprintf(LOG_OK,_("Added downloading of file %s from %s [by user]"),what->info->get_file(),what->info->get_host());
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
	if (LOCK_FILE) remove(LOCK_FILE);
};

void tMain::reinit_main_log() {
	MainLog->reinit(CFG.MAX_MAIN_LOG_LENGTH);
	MainLog->reinit_file();
};
//*****************************************************************/
static void download_completed(tDownload *what) {
	what->who->done();
	what->LOG->add(_("Downloading was successefully completed!"),LOG_OK);
	what->make_file_visible();
	what->status=DOWNLOAD_COMPLETE;
	pthread_exit(NULL);
};

static void download_failed(tDownload *what) {
	if (what->who)
		what->who->done();
	what->LOG->add(_("Downloading was failed..."),LOG_ERROR);
	what->status=DOWNLOAD_FATAL;
	pthread_exit(NULL);
};

static void recurse_http(tDownload *what) {
	tHttpDownload *httpd=(tHttpDownload *)(what->who);
	char *type=httpd->get_content_type();
	if (what->config.http_recurse_depth!=1 && type &&
	    begin_string_uncase(type,"text/html")){
		httpd->analize_html();
		what->convert_list_to_dir2();
	};
};

void download_http(tDownload *what) {
	if (!what->who) what->who=new tHttpDownload;
	tAddr *addr=what->info;
	if (what->who->init(addr,what->LOG,&(what->config))) {
		download_failed(what);
		return;
	};
	what->who->init_download(addr->get_path(),addr->get_file());
	/* We need to know size of already loaded file
	 * but I think if file not found we need to delete it
	 * because in http name of file may be specify 
	 * in http answer
	 */
	int CurentSize=what->create_file();
	if (CurentSize<0) {
		download_failed(what);
		return;
	};
	what->who->set_data(CurentSize);
	what->who->rollback_before();
	int size=what->who->get_size();
	/* In the case if file already loaded
	 */
	if (size==CurentSize && size!=0) {
		what->finfo.size=size;
		what->finfo.type=T_FILE;
		recurse_http(what);
		download_completed(what);
		return;
	};
	/* There are must be procedure for removing file
	 * wich execute if CurentSize==0
	 */
	if (CurentSize==0) {
		if (what->who->delete_file())
			what->LOG->add(_("It is strange that we can't delete file which just created..."),LOG_WARNING);
	};

	if (size<-1) {
		what->LOG->add(_("File not found"),LOG_WARNING);
		download_failed(what);
		return;
	};
	if (size==-1) {
		what->finfo.type=T_REDIRECT;
		what->who->done();
		what->status=DOWNLOAD_COMPLETE;
		what->LOG->add(_("Redirect detected..."),LOG_WARNING);
		pthread_exit(NULL);
		return;
	};
	what->finfo.size=size;
	what->finfo.type=T_FILE;
	what->Start=what->Pause=time(NULL);
	/* there we need to create file again
	 * if CurentSize==0
	 */
	if (CurentSize==0) CurentSize=what->create_file();
	what->status=DOWNLOAD_GO;
	if (what->who->download(CurentSize,what->finfo.size-CurentSize)) {
		download_failed(what);
		return;
	};
	what->set_date_file();
	recurse_http(what);
	download_completed(what);
};

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
			what->LOG->add(_("Can't open messages queue!"),LOG_ERROR);
			what->status=DOWNLOAD_FATAL;
			pthread_exit(NULL);
			return NULL;
		};
		if (addr->proto==D_PROTO_UNKNOWN){
			what->LOG->add(_("Such protocol is not supported!"),LOG_ERROR);
			download_failed(what);
			return NULL;
		};
		if (what->config.get_proxy_host()  &&
		    (what->config.proxy_type || addr->proto==D_PROTO_HTTP)) {
			what->who=new tProxyDownload;
			download_http(what);
			pthread_exit(NULL);
			return NULL;
		};
		if (addr->proto==D_PROTO_HTTP){
			download_http(what);
			pthread_exit(NULL);
			return NULL;
		};
		if (!what->who) what->who=new tFtpDownload;
		what->LOG->add(_("Was Started!"),LOG_WARNING);

		if (what->finfo.type==T_LINK && !what->config.link_as_file) {
			what->LOG->add(_("It is a link and we already load it"),LOG_WARNING);
			what->who->short_init(what->LOG,&(what->config));
			what->who->init_download(addr->get_path(),addr->get_file());
			what->who->set_file_info(&(what->finfo));
			what->create_file();
			what->set_date_file();
			download_completed(what);
			return NULL;
		};

		if (what->who->init(addr,what->LOG,&(what->config))) {
			download_failed(what);
			return NULL;
		};

		what->who->init_download(addr->get_path(),addr->get_file());
		if (what->finfo.size<0) {
			what->status=DOWNLOAD_SIZE_WAIT;
			int size=what->who->get_size();
			if (size<0) {
				what->LOG->add(_("File not found"),LOG_ERROR);
				download_failed(what);
				return NULL;
			};
			what->finfo.size=size;
			what->finfo.type=what->who->file_type();
		} else {
			what->who->set_file_info(&(what->finfo));
		};
		if (what->finfo.type==T_LINK)
			what->finfo.size=0;
		int CurentSize=0;
		if (what->info->mask==0){
			CurentSize=what->create_file();
			 //if it was link
			what->finfo.type=what->who->file_type();
		};

		if (what->finfo.type==T_DEVICE) {
			download_completed(what);
			return NULL;
		};

		if (CurentSize<0) {
			download_failed(what);
			return NULL;
		};

		what->status=DOWNLOAD_GO;
		what->Start=what->Pause=time(NULL);
		if (what->who->download(CurentSize,what->finfo.size-CurentSize)) {
			download_failed(what);
			return NULL;
		};
		what->set_date_file();
		what->who->done();
		if (what->finfo.type==T_DIR && what->config.ftp_recurse_depth!=1) what->convert_list_to_dir();
		what->LOG->add(_("Downloading was successefully completed!"),LOG_OK);
		what->status=DOWNLOAD_COMPLETE;
		what->make_file_visible();
	};
	pthread_exit(NULL);
	return NULL;
};
//------------------------------------------------------------------------

