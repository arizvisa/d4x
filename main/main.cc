/*	WebDownloader for X-Window
 *	Copyright (C) 1999 Koshelev Maxim
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
#include <sys/msg.h>
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
#include "config.h"
#include "ntlocale.h"

tTwoStrings::tTwoStrings() {
	one=two=NULL;
};

void tTwoStrings::zero() {
	one=two=NULL;
};

tTwoStrings::~tTwoStrings() {}
;

int get_port_by_proto(char *proto) {
	if (proto) {
		if (equal_uncase(proto,"ftp")) return 21;
		if (equal_uncase(proto,"http")) return 80;
	};
	return 21;
};

int amount_of_downloads_in_queues(){
	return RunList->count()+StopList->count()+CompleteList->count()+PausedList->count()+WaitList->count()+WaitStopList->count();
};
//**********************************************/

tMLog *MainLog=NULL;

tDList *RunList,*StopList,*CompleteList,*PausedList,*WaitStopList;
tDList *WaitList;
tMeter *GlobalMeter=NULL;
tMeter *LocalMeter=NULL;
key_t LogsMsgQueue;


void tMain::init() {
	WaitList=new tDList(DL_WAIT);
	WaitStopList=new tDList(DL_STOPWAIT);
	RunList=new tDList(DL_RUN);
	StopList=new tDList(DL_STOP);
	PausedList=new tDList(DL_PAUSE);
	CompleteList=new tDList(DL_COMPLETE);
	WaitList->init(0);
	WaitStopList->init(0);
	RunList->init(0);
	StopList->init(0);
	PausedList->init(0);
	CompleteList->init(0);
	GlobalMeter=new tMeter;
	GlobalMeter->init(METER_LENGTH);
	LocalMeter=new tMeter;
	LocalMeter->init(METER_LENGTH);

	LimitsForHosts=new tHostsLimits;
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
		if (msgrcv(MsgQueue,(msgbuf *)&Msg,sizeof(Msg)-sizeof(long),1,IPC_NOWAIT)>0 ||
		        msgrcv(MsgQueue,(msgbuf *)&Msg,sizeof(Msg)-sizeof(long),2,IPC_NOWAIT)>0)
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
					tDownload *download=WaitList->last();
					download->ScheduleTime=temp3->temp;
					download->config.set_flags(temp2->temp);
					switch (temp->temp) {
						case 0: break; //wait list
						case 1:
							{ //completed
								WaitList->del(download);
								CompleteList->insert(download);
								break;
							};
						case 2:
						default:
							{ //stopped
								WaitList->del(download);
								PausedList->insert(download);
								break;
							};
						case 3:
							{ //failed
								WaitList->del(download);
								StopList->insert(download);
								break;
							};
						case 4:
							{ //runing
								time_t NOW;
								time(&NOW);
								if (RunList->count()<CFG.MAX_THREADS && download->ScheduleTime<=NOW) {
									tSortString *tmp=LimitsForHosts->find(download->info->host,download->info->port);
									if ((tmp==NULL || tmp->curent<tmp->upper) && run_new_thread(download)==0) {
										if (tmp) tmp->curent+=1;
										WaitList->del(download);
										RunList->insert(download);
									};
								};
							};
					};
				};
			};
		};
		temp=what->next();
	};
	main_menu_prepare();
};

void tMain::init_main_log() {
	MainLog=new tMLog;
	MainLog->init(CFG.MAX_MAIN_LOG_LENGTH);
	MainLog->init_list(GTK_CLIST(MainLogList));
	MainLog->reinit_file();
	MainLog->add("----------------------------------------",LOG_FROM_SERVER);
	MainLog->add(VERSION_NAME,LOG_WARNING);
	MainLog->add(_("Normally started"),LOG_WARNING);
};

void tMain::redraw_logs() {
	mbuf Msg;
	int complete=0;
	int limit = 0;
	while (!complete && limit<99) {
		complete=1;
		limit++;
		if (msgrcv(MsgQueue,(msgbuf *)&Msg,sizeof(Msg)-sizeof(long),1,IPC_NOWAIT)>0) {
			complete=0;
			if (Msg.what){
			    Msg.which->lock();
			    log_window_add_string(Msg.which,Msg.what);
			    Msg.which->unlock();
			}else
			    del_first_from_log(Msg.which);
		};
	};
};

void tMain::split_string(char *what,char *delim,tTwoStrings *out) {
	char * where=strstr(what,delim);
	if (where) {
		int len=strlen(where),len1=strlen(delim);
		out->two=copy_string(where+len1);
		len1=strlen(what)-len;
		out->one=copy_string(what,len1);
	} else {
		out->two=copy_string(what);
		out->one=NULL;
	};
	delete(what);
};

void tMain::absolute_delete_download(tDList *where,tDownload *what) {
	list_of_downloads_del(what);
	if (where) where->del(what);
	ALL_DOWNLOADS->del(what);
	delete(what);
};

void tMain::del_completed() {
	MainLog->add(_("Delete completed downloads"),LOG_OK|LOG_DETAILED);
	tDownload *temp=CompleteList->last();
	while (temp) {
		absolute_delete_download(CompleteList,temp);
		temp=CompleteList->last();
	};
};

void tMain::del_fataled() {
	MainLog->add(_("Delete failed downloads"),LOG_OK|LOG_DETAILED);
	tDownload *temp=StopList->last();
	while (temp) {
		absolute_delete_download(StopList,temp);
		temp=StopList->last();
	};
};

void tMain::del_all() {
	if (PausedList->count() || WaitList->count() || RunList->count() ||StopList->count() || CompleteList->count())
		MainLog->add(_("Clear queue of downloads"),LOG_ERROR);
	del_completed();
	del_fataled();
	tDownload *temp=WaitList->first();
	/* delete from begin for speed, becouse
	 * after deleting from GtkClist list of downloads
	 * will be recalculated
	 */
	while (temp) {
		absolute_delete_download(WaitList,temp);
		temp=WaitList->first();
	};
	temp=PausedList->last();
	while (temp) {
		absolute_delete_download(PausedList,temp);
		temp=PausedList->last();
	};
	temp=RunList->last();
	while (temp) {
		delete_download(temp);
		temp=RunList->last();
	};
};

tAddr *tMain::analize(char *what) {
	tTwoStrings pair;
	split_string(what,"://",&pair);
	tAddr *out=new tAddr;
	if (pair.one) {
		out->protocol=pair.one;
	} else {
		out->protocol=copy_string(DEFAULT_PROTO);
	};
	out->host=pair.two;
	if (!out->host) {
		delete(out);
		return NULL;
	};
	split_string(out->host,"/",&pair);
	if (pair.one) {
		out->host=pair.one;
		out->file=pair.two;
	} else {
		out->host=pair.two;
		out->file=pair.one;
	};
	split_string(out->host,"@",&pair);
	out->host=pair.two;
	out->username=pair.one;
	if (out->username) {
		split_string(out->username,":",&pair);
		out->username=pair.one;
		out->pass=pair.two;
	} else {
		out->username=NULL;
		out->pass=NULL;
	};
	if (out->file) {
		char *tmp=parse_percents(out->file);
		if (tmp) {
			delete out->file;
			out->file=tmp;
		} else
			delete tmp;
		char *prom=rindex(out->file,'/');
		if (prom) {
			out->path=copy_string(prom+1);
			*prom=0;
			prom=out->path;
			out->path=sum_strings("/",out->file);
			delete out->file;
			out->file=prom;
		};
	} else {
		out->file=copy_string("");
	};
	if (!out->path) out->path=copy_string("/");
	split_string(out->host,":",&pair);
	if (pair.one) {
		sscanf(pair.two,"%i",&out->port);
		delete pair.two;
		out->host=pair.one;
	} else {
		out->port=0;
		out->host=pair.two;
	};
	if (equal_uncase(out->protocol,"ftp") && index(out->file,'*'))
		out->mask=1;
	/* Parse # in http urls
	 */
	if (equal_uncase(out->protocol,"http") && out->file!=NULL) {
		char *tmp=index(out->file,'#');
		if (tmp) {
			*tmp=0;
			tmp=out->file;
			out->file=copy_string(tmp);
			delete(tmp);
		};
	};
	if (out->port==0)
		out->port=get_port_by_proto(out->protocol);
	return out;
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
	what->SpeedLimit->base=2 * what->config.speed;
	SpeedScheduler->insert(what->SpeedLimit);
	if (what->editor) what->editor->disable_ok_button();

	char data[MAX_LEN];
	char *URL=make_simply_url(what);
	snprintf(data,MAX_LEN,_("Run new thread for %s"),URL);
	delete URL;
	MainLog->add(data,LOG_OK|LOG_DETAILED);

	pthread_attr_init(&attr_p);
	pthread_attr_setdetachstate(&attr_p,PTHREAD_CREATE_JOINABLE);
	pthread_attr_setscope(&attr_p,PTHREAD_SCOPE_SYSTEM);
	return (pthread_create(&what->thread_id,&attr_p,download_last,(void *)what));
};

void tMain::stop_download(tDownload *what) {
	if (WaitStopList->owner(what) && what->action!=ACTION_DELETE) {
		what->action=ACTION_STOP;
		return;
	};
	if (RunList->owner(what)) {
		RunList->del(what);
		char data[MAX_LEN];
		sprintf(data,_("Downloading of file %s from %s was terminated [by user]"),what->info->file,what->info->host);
		MainLog->add(data,LOG_WARNING);
		if (!stop_thread(what)) {
			WaitStopList->insert(what);
		} else {
			LimitsForHosts->decrement(what);
			PausedList->insert(what);
		};
		what->Status.clear();
	} else {
		if (WaitList->owner(what)) {
			WaitList->del(what);
			PausedList->insert(what);
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
				StopList->del(what);
				break;
			};
		case DL_COMPLETE:{
				CompleteList->del(what);
				break;
			};
		case DL_RUN:{
				stop_download(what);
				if (what->owner==DL_PAUSE) {
					PausedList->del(what);
					break;
				};
			};
		case DL_STOPWAIT:{
				what->action=ACTION_DELETE;
				return 0;
			};
		case DL_PAUSE:{
				PausedList->del(what);
				break;
			};
		case DL_WAIT:
				WaitList->del(what);
	};
	char data[MAX_LEN];
	sprintf(data,_("Delete file %s from queue of downloads"),what->info->file);
	MainLog->add(data,LOG_WARNING);
	absolute_delete_download(NULL,what);
	return 1;
};

void tMain::continue_download(tDownload *what) {
	if (WaitStopList->owner(what)) {
		if (what->action!=ACTION_DELETE) what->action=ACTION_CONTINUE;
		return;
	};
	if (RunList->owner(what)) {
		stop_download(what);
		if (WaitStopList->owner(what)){
			what->action=ACTION_CONTINUE;
			return;
		};
	};

	char data[MAX_LEN];
	sprintf(data,_("Continue downloading of file %s from %s..."),what->info->file,what->info->host);
	MainLog->add(data,LOG_OK);

	if (PausedList->owner(what)) 	PausedList->del(what);
	if (StopList->owner(what)) 		StopList->del(what);
	if (CompleteList->owner(what)) 	CompleteList->del(what);
	if (WaitList->owner(what))		WaitList->del(what);
	tSortString *tmp=LimitsForHosts->find(what->info->host,what->info->port);
	time_t NOW;
	time(&NOW);
	if (RunList->count()<CFG.MAX_THREADS && what->ScheduleTime<=NOW && (tmp==NULL || tmp->curent<tmp->upper) && run_new_thread(what)==0) {
		if (tmp) tmp->curent+=1;
		RunList->insert(what);
	} else {
		tDownload *temp=WaitList->last();
		if (!temp || temp->GTKCListRow < what->GTKCListRow)
			WaitList->insert(what);
		else {
			temp=WaitList->first();
			while (temp && temp->GTKCListRow < what->GTKCListRow)
				temp=WaitList->prev();
			WaitList->insert_before(what,temp);
		};
	};
	what->Attempt.clear();
};

int tMain::complete() {
	return (WaitList->count()+RunList->count())>0?0:1;
};

void tMain::add_dir(tDownload *parent) {
	if (parent->DIR==NULL) return;
	tDownload *temp=parent->DIR->last();
	while(temp) {
		parent->DIR->del(temp);
		if (ALL_DOWNLOADS->find(temp)) {
			delete temp;
		} else {
			WaitList->insert(temp);
			list_of_downloads_add(temp);
			ALL_DOWNLOADS->insert(temp);
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
					REAL_SIZE=what->who->another_way_get_size();
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
					if (TimeLeft && what->Size.curent>what->Size.old)
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
				if (REAL_SIZE)
					temp=int((float(what->Size.curent)/float(REAL_SIZE))*100);
				else
					temp=100;
/* setting new title of log*/
				if (CFG.USE_MAINWIN_TITLE){
					char title[MAX_LEN];
					sprintf(title,"%i%% %i/%i %s",temp,what->Size.curent,REAL_SIZE,what->info->file);
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
				if (period && what->who) {
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
			switch (what->Status.curent) {
				case D_QUERYING:
					{
						list_of_downloads_set_pixmap(what->GTKCListRow,PIX_RUN_PART);
						break;
					};
				case D_DOWNLOAD:
					{
						list_of_downloads_set_pixmap(what->GTKCListRow,PIX_RUN);
						break;
					};
				case D_DOWNLOAD_BAD:
					{
						list_of_downloads_set_pixmap(what->GTKCListRow,PIX_RUN_BAD);
						break;
					};
			};
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
	char *newurl;
	newurl=what->who->get_new_url();
	if (newurl) {
		tDownload *temp=WaitList->first();
		if (temp)
			WaitList->insert_before(what,temp);
		else
			WaitList->insert(what);
		tAddr *addr=aa.analize(newurl);
		ALL_DOWNLOADS->del(what);
		if (what->info) delete (what->info);
		what->info=addr;
		if (ALL_DOWNLOADS->find(what)) {
			WaitList->del(what);
			list_of_downloads_del(what);
			delete(what);
			return;
		};
		ALL_DOWNLOADS->insert(what);
		normalize_path(what->get_SavePath());
		what->finfo.type=what->status=0;
		what->finfo.size=-1;
		char *URL=make_simply_url(what);
		list_of_downloads_change_data(what->GTKCListRow,URL_COL,URL);
		delete (URL);
		//		list_of_downloads_change_data(what->GTKCListRow,URL_COL,newurl);
		list_of_downloads_change_data(what->GTKCListRow,FILE_COL,what->info->file);
		for (int i=FILE_TYPE_COL;i<URL_COL;i++)
			list_of_downloads_change_data(what->GTKCListRow,i,"");
		if (what->who) delete what->who;
		what->who=NULL;
	} else {
		CompleteList->insert(what);
		what->finfo.type=T_NONE;
	};
};

void tMain::prepare_for_stoping(tDownload *what,tDList *list) {
	char data[MAX_LEN];
	char *URL=make_simply_url(what);
	snprintf(data,MAX_LEN,_("Prepare [%s] for stoping"),URL);
	delete URL;
	MainLog->add(data,LOG_OK|LOG_DETAILED);

	LimitsForHosts->decrement(what);
	if (what->editor) what->editor->enable_ok_button();
	SpeedScheduler->del(what->SpeedLimit);
	delete (what->SpeedLimit);
	what->SpeedLimit=NULL;
	list->del(what);
};

void tMain::case_download_completed(tDownload *what){
	prepare_for_stoping(what,RunList);
	char data[MAX_LEN];
	char *URL=make_simply_url(what);
	if (what->who->file_type()==T_DIR) {
		sprintf(data,_("Downloading of directory %s was completed"),URL);
		if (what->config.ftp_recurse_depth!=1) add_dir(what);
	} else {
		if (what->finfo.type!=T_REDIRECT){
			int bytes = what->finfo.size==0 ? what->who->get_readed():what->finfo.size;
			sprintf(data,_("Downloading of file %s (%i bytes) was completed at speed %i bytes/sec"),
		        URL,bytes,what->Speed.curent);
		}else{
		 	sprintf(data,_("Redirect from %s"),URL);
		};
		if (what->config.http_recurse_depth!=1 && what->DIR)
			add_dir(what);
	};
	MainLog->add(data,LOG_OK);
	if (what->finfo.type==T_REDIRECT) {
		redirect(what);
	} else {
		if (CFG.DELETE_COMPLETED ) {
			absolute_delete_download(NULL,what);
			snprintf(data,MAX_LEN,_("%s was deleteted from queue of downloads as completed download"),URL);
			MainLog->add(data,LOG_WARNING|LOG_DETAILED);
		} else {
			if (what->who) delete(what->who);
			what->who=NULL;
			CompleteList->insert(what);
			main_menu_del_completed_set_state(TRUE);
		};
	};
	delete URL;
};

void tMain::case_download_failed(tDownload *what){
	char data[MAX_LEN];
	char *URL=make_simply_url(what);
	prepare_for_stoping(what,RunList);
	sprintf(data,_("Downloading of file %s was terminated by fatal error"),URL);
	MainLog->add(data,LOG_ERROR);
	if (CFG.DELETE_FATAL) {
		absolute_delete_download(NULL,what);
		snprintf(data,MAX_LEN,_("%s was deleteted from queue of downloads as failed download"),URL);
		MainLog->add(data,LOG_WARNING|LOG_DETAILED);
	} else {
		if (what->who) delete(what->who);
		what->who=NULL;
		StopList->insert(what);
		main_menu_del_failed_set_state(TRUE);
	};
	delete URL;
};

void tMain::main_circle() {
	list_of_downloads_freeze();
/* look for stopped threads */
	tDownload *temp=WaitStopList->last();
	while(temp) {
		tDownload *temp1=WaitStopList->next();
		if (temp->status==DOWNLOAD_REAL_STOP ||
		        temp->status==DOWNLOAD_COMPLETE  ||
		        temp->status==DOWNLOAD_FATAL) {
			real_stop_thread(temp);
			prepare_for_stoping(temp,WaitStopList);
			PausedList->insert(temp);
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
	temp=RunList->last();
	while(temp) {
		tDownload *temp1=RunList->next();
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
	check_for_remote_commands();
/* look for run new */
	temp=WaitList->first();
	time_t NOW;
	time(&NOW);
	while(temp && RunList->count()<CFG.MAX_THREADS) {
		tSortString *tmp=LimitsForHosts->find(temp->info->host,temp->info->port);
		if (temp->ScheduleTime<=NOW && (tmp==NULL || tmp->curent<tmp->upper)) {
			if (tmp) tmp->curent+=1;
			if (run_new_thread(temp))
				break;
			WaitList->del(temp);
			RunList->insert(temp);
			temp=WaitList->first();
		} else
			temp=WaitList->prev();
	};
/* various stuff */
	list_of_downloads_unfreeze();
	speed();
	prepare_buttons();
	CFG.NICE_DEC_DIGITALS.reset();
};

void tMain::check_for_remote_commands(){
	tString *addnew=server->get_string();
	while (addnew){
		switch (addnew->temp){
			case PACKET_ADD:{
				add_downloading(addnew->body,NULL,NULL);
				break;
			};
			case PACKET_SET_SPEED_LIMIT:{
				sscanf(addnew->body,"%i",&CFG.SPEED_LIMIT);
				if (CFG.SPEED_LIMIT>3) CFG.SPEED_LIMIT=3;
				if (CFG.SPEED_LIMIT<1) CFG.SPEED_LIMIT=1;
				set_speed_buttons();
				break;
			};
		};
		delete(addnew);
		addnew=server->get_string();
	};
};
//**********************************************/

int tMain::add_downloading(char *adr,char *where,char *name) {
	if (adr==NULL) return -1;
	char *temp=copy_string(adr);
	tAddr *addr=analize(temp);
	if (!addr) return -1;
	tDownload *whatadd=new tDownload;
	whatadd->info=addr;
	if (where!=NULL && strlen(where)>0) {
		whatadd->set_SavePath(where);
	} else
		whatadd->set_SavePath(CFG.GLOBAL_SAVE_PATH);
	if (strlen(addr->file)==0) {
		whatadd->finfo.type=T_DIR;
		whatadd->finfo.size=0;
	};

	if (ALL_DOWNLOADS->find(whatadd)) {
		delete(whatadd);
		return -1;
	} else {
		ALL_DOWNLOADS->insert(whatadd);
	};
	normalize_path(whatadd->get_SavePath());

	if (name && strlen(name))
		whatadd->set_SaveName(name);
	whatadd->set_default_cfg();

	if (equal_uncase(whatadd->info->protocol,"ftp")) {
		if (CFG.USE_PROXY_FOR_FTP) {
			whatadd->config.set_proxy_host(CFG.FTP_PROXY_HOST);
			whatadd->config.proxy_port=CFG.FTP_PROXY_PORT;
			if (CFG.NEED_PASS_FTP_PROXY) {
				whatadd->config.set_proxy_user(CFG.FTP_PROXY_USER);
				whatadd->config.set_proxy_pass(CFG.FTP_PROXY_PASS);
			};
		};
		whatadd->config.proxy_type=CFG.FTP_PROXY_TYPE;
	} else {
		if (CFG.USE_PROXY_FOR_HTTP) {
			whatadd->config.set_proxy_host(CFG.HTTP_PROXY_HOST);
			whatadd->config.proxy_port=CFG.HTTP_PROXY_PORT;
			if (CFG.NEED_PASS_HTTP_PROXY) {
				whatadd->config.set_proxy_user(CFG.HTTP_PROXY_USER);
				whatadd->config.set_proxy_pass(CFG.HTTP_PROXY_PASS);
			};
		};
	};

	list_of_downloads_add(whatadd);
	WaitList->insert(whatadd);
	addr=NULL;
	return 0;
};

unsigned int tMain::get_precise_time(){
	struct timeb tp;
	ftime(&tp);
	return(tp.time*1000+tp.millitm);
};

void tMain::speed() {
	unsigned int curent_time=get_precise_time();
	unsigned int TimeLeft=curent_time-LastTime;
	int readed_bytes=GVARS.READED_BYTES;
	int bytes=readed_bytes-LastReadedBytes;
	if (TimeLeft){
		int Speed=((bytes*1000)/TimeLeft);
		LastReadedBytes=readed_bytes;
		GlobalMeter->add(Speed);
		LastTime=curent_time;
	};
	int SPEED_LIMIT=0;
	switch (CFG.SPEED_LIMIT) {
		case 1:	{
				SPEED_LIMIT=CFG.SPEED_LIMIT_1;
				break;
			};
		case 2:	{
				SPEED_LIMIT=CFG.SPEED_LIMIT_2;
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
	CompleteList->init_pixmap(PIX_COMPLETE);
	RunList->init_pixmap(PIX_RUN_PART);
	WaitList->init_pixmap(PIX_WAIT);
	PausedList->init_pixmap(PIX_PAUSE);
	StopList->init_pixmap(PIX_STOP);
	WaitStopList->init_pixmap(PIX_STOP_WAIT);
	init_main_log();
	load_defaults();
	list_of_downloads_set_height();
	init_timeouts();
	parse_command_line_postload(argv,argc);
	run_msg_server();
	LastTime=get_precise_time();
	gtk_main();
};

void tMain::run_after_quit(){
	if (CFG.EXEC_WHEN_QUIT && strlen(CFG.EXEC_WHEN_QUIT))
		system(CFG.EXEC_WHEN_QUIT);
};

void tMain::add_download_message(tDownload *what) {
	if (!what) return;
	char data[MAX_LEN];
	sprintf(data,_("Added downloading of file %s from %s [by user]"),what->info->file,what->info->host);
	MainLog->add(data,LOG_OK);
};

void tMain::done() {
	/* There are  we MUST stop all threads!!!
	 */
	int *rc;
	pthread_kill(server_thread_id,SIGUSR2);
	pthread_join(server_thread_id,(void **)&rc);

	tDownload *tmp=RunList->last();
	while (tmp) {
		stop_download(tmp);
		tmp=RunList->last();
	};
	tDownload *temp=WaitStopList->last();
	do{
		while(temp) {
			tDownload *temp1=WaitStopList->next();
			if (temp->status==DOWNLOAD_REAL_STOP ||
			        temp->status==DOWNLOAD_COMPLETE  ||
			        temp->status==DOWNLOAD_FATAL) {
				real_stop_thread(temp);
				prepare_for_stoping(temp,WaitStopList);
				PausedList->insert(temp);
			};
			temp=temp1;
		};
		temp=WaitStopList->last();
		if (temp==NULL) break;
		sleep(1);
	}while(1);

	delete(WaitList);
	delete(StopList);
	delete(PausedList);
	delete(CompleteList);
	delete(RunList);
	delete(WaitStopList);
	delete(GlobalMeter);
	delete(LocalMeter);
	delete(LocalMeter);
	delete(ALL_DOWNLOADS);

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
	what->who->done();
	what->LOG->add(_("Downloading was failed..."),LOG_ERROR);
	what->status=DOWNLOAD_FATAL;
	pthread_exit(NULL);
};

static void recurse_http(tDownload *what) {
	tHttpDownload *httpd=(tHttpDownload *)(what->who);
	char *type=httpd->get_content_type();
	if (what->config.http_recurse_depth!=1 && type && equal_first(type,"text/html") && strlen(type)>=strlen("text/html")) {
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
	what->who->init_download(addr->path,addr->file);
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
	((tHttpDownload*)(what->who))->rollback_before();
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
	/* There are must be procedure of removing file
	 * wich execute if CurentSize==0
	 */
	if (CurentSize==0) {
		if (what->who->delete_file(what->get_SavePath()))
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

void download_proxy(tDownload *what) {
	if (!what->who) what->who=new tProxyDownload;
	tAddr *addr=what->info;
	if (what->who->init(addr,what->LOG,&(what->config))) {
		download_failed(what);
		return;
	};
	what->who->init_download(addr->path,addr->file);
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
	((tProxyDownload*)(what->who))->rollback_before();
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
	/* There are must be procedure of removing file
	 * wich execute if CurentSize==0
	 */
	if (CurentSize==0) {
		if (what->who->delete_file(what->get_SavePath()))
			what->LOG->add(_("It is strange that we can't delete file which just created..."),LOG_WARNING);
	};

	if (size<-1) {
		what->LOG->add(_("File not found"),LOG_ERROR);
		download_failed(what);
		return;
	};
	if (size==-1) {
		what->finfo.type=T_REDIRECT;
		what->who->done();
		what->status=DOWNLOAD_COMPLETE;
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
void *download_last(void *nothing) {
	tDownload *what=(tDownload *)nothing;
	my_pthread_key_init();
	*(my_pthread_key_get())=what;
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
		if (what->config.get_proxy_host() && (what->config.proxy_type || equal_uncase(addr->protocol,"http"))) {
			download_proxy(what);
			pthread_exit(NULL);
			return NULL;
		};
		if (addr==NULL || equal_uncase(addr->protocol,"ftp")) {
			if (!what->who) what->who=new tFtpDownload;
		} else {
			download_http(what);
			pthread_exit(NULL);
			return NULL;
		};
		what->LOG->add(_("Was Started!"),LOG_WARNING);

		if (what->finfo.type==T_LINK) {
			what->LOG->add(_("It is a link and we already load it"),LOG_WARNING);
			what->who->short_init(what->LOG);
			what->who->init_download(addr->path,addr->file);
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

		what->who->init_download(addr->path,addr->file);
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
		int CurentSize=0;
		if (what->info->mask==0)
			CurentSize=what->create_file();

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

