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

#include "srvclt.h"
#include "var.h"
#include "main.h"
#include "locstr.h"
#include "signal.h"
#include <gtk/gtk.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>
#include <string.h>
#include "face/lod.h"


#if defined(sun)
typedef int socklen_t;
#endif

void server_thread_stop(int i){
	pthread_exit(NULL);
};

static void *server_thread_run(void *what){
	struct sigaction action,old_action;
	action.sa_handler=server_thread_stop;
	action.sa_flags=0;
	sigaction(SIGUSR2,&action,&old_action);

	sigset_t oldmask,newmask;
	sigemptyset(&newmask);
	sigaddset(&newmask,SIGTERM);
	sigaddset(&newmask,SIGINT);
	sigaddset(&newmask,SIGUSR1);
	pthread_sigmask(SIG_BLOCK,&newmask,&oldmask);
	tMsgServer *srv=(tMsgServer *)what;
	srv->run();
	return NULL;
};


tMsgServer::tMsgServer(){
	list=new tStringList;
	list->init(0);
	file=NULL;
	fd=newfd=0;
};

tMsgServer::~tMsgServer(){
    if (fd>0) close(fd);
    if (newfd>0) close(newfd);
    delete(list);
    unlink(file);
    delete[] file;
};

void tMsgServer::run_thread(){
	pthread_attr_t attr_p;
	pthread_attr_init(&attr_p);
	pthread_attr_setdetachstate(&attr_p,PTHREAD_CREATE_JOINABLE);
	pthread_attr_setscope(&attr_p,PTHREAD_SCOPE_SYSTEM);
	pthread_create(&thread_id,&attr_p,server_thread_run,this);
};

void tMsgServer::stop_thread(){
	void *rc;
	pthread_kill(thread_id,SIGUSR2);
	pthread_join(thread_id,(void **)&rc);
};

int tMsgServer::init(){

	struct sockaddr_un saddr;
	fd=socket(AF_UNIX,SOCK_STREAM,0);
	if (fd == -1)
		return(-1);
	
	saddr.sun_family = AF_UNIX;
	sprintf(saddr.sun_path, "%s/downloader_for_x_sock_%s", g_get_tmp_dir(), g_get_user_name());
	unlink(saddr.sun_path);
	file=copy_string(saddr.sun_path);
	if (bind(fd,(struct sockaddr *)&saddr,sizeof(saddr)) == -1)
		return(-1);
	listen(fd,5);
	return(0);
};

void tMsgServer::cmd_return_int(int what){
	tPacket packet;
	packet.type=PACKET_ACK;
	packet.len=sizeof(int);
	write(newfd,&packet,sizeof(packet));
	write(newfd,&what,sizeof(what));
};

void tMsgServer::cmd_ack(){
	cmd_return_int(0);
};

void tMsgServer::write_dwn_status(tDownload *dwn,int full=0){
	tPacketStatus status;
	if (dwn){
		if (full) status.url=dwn->info->url();
		status.Size=dwn->finfo.size;
		status.Download=dwn->Size.curent;
		status.Time=dwn->Start;
		status.Speed=dwn->Speed.curent;
		status.Status=dwn->owner(); /* FIXME: possible race condition */
		status.Attempt=dwn->Attempt.curent;
		status.MaxAttempt=dwn->config?dwn->config->number_of_attempts:CFG.DEFAULT_CFG.number_of_attempts;
	};
	
	ALL_DOWNLOADS->unlock();
	
	tPacket packet;
	packet.type=PACKET_ACK;
	if (dwn){
		packet.len=sizeof(tPacketStatus)-sizeof(char*);
		if (full)
			packet.len+=strlen(status.url)+1;
		write(newfd,&packet,sizeof(packet));
		write(newfd,&status,sizeof(status)-sizeof(char*));
		if (full)
			write(newfd,status.url,strlen(status.url)+1);
	}else{
		packet.len=0;
		write(newfd,&packet,sizeof(packet));
	};
};

void tMsgServer::cmd_ls(int len,int type){
	if (len){
		char *temp=new char[len+1];
		if (read(newfd,temp,len)==len){
			temp[len]=0;
			tDownload *dl=new tDownload;
			dl->info=new tAddr(temp);
			ALL_DOWNLOADS->lock();
			tDownload *answer=ALL_DOWNLOADS->find(dl);
			delete(dl);
			write_dwn_status(answer);
		};
		delete[] temp;
	}else{ // output whole list
		ALL_DOWNLOADS->lock();
		/* FIXME: when implement many queues */
		d4xWFNode *node=(d4xWFNode *)(D4X_QUEUE->qv.ListOfDownloadsWF.first());
		while (node) {
			d4xWFNode *next=(d4xWFNode *)(node->prev);
			if (node->dwn){
				write_dwn_status(node->dwn,1);
				ALL_DOWNLOADS->lock();
			};
			if (next==NULL || next->next==node)
				node=next;
			else
				break;
		};
		ALL_DOWNLOADS->unlock();
	};
};

void tMsgServer::cmd_add(int len,int type){
	char *temp=new char[len+1];
	if (read(newfd,temp,len)==len){
		temp[len]=0;
		tString *newstr=new tString;
		newstr->body=temp;
		newstr->temp=type;
		lock.lock();
		list->insert(newstr);
		lock.unlock();
		cmd_ack();
	}else
		delete[] temp;
};

void tMsgServer::run(){
    while(1){
		fd_set set;
		FD_ZERO(&set);
		FD_SET(fd,&set);
		timeval tv;
		tv.tv_sec=1;
		tv.tv_usec=0;
		if (select(fd+1,&set,NULL,NULL,&tv)>0){
			tPacket packet;
			struct sockaddr_un addr;
			int tmp=sizeof(addr);
			newfd=accept(fd,(sockaddr *)&addr,(socklen_t *)&tmp);
			if (newfd>0 && read(newfd,&packet,sizeof (packet))>=0){
				switch (packet.type){
				case PACKET_EXIT_TIME:
				case PACKET_RERUN_FAILED:
				case PACKET_ADD_OPEN:
				case PACKET_SET_MAX_THREADS:
				case PACKET_DEL_COMPLETED:
				case PACKET_SET_SAVE_PATH:
				case PACKET_SET_SPEED_LIMIT:
				case PACKET_ICONIFY:
				case PACKET_POPUP:
				case PACKET_MSG:
				case PACKET_STOP:
				case PACKET_DEL:
				case PACKET_ADD:{
					cmd_add(packet.len,packet.type);
					break;
				};
				case PACKET_ASK_SPEED:{
					cmd_return_int(GlobalMeter->last_value());
					break;
				};
				case PACKET_ASK_RUN:{
					cmd_return_int(D4X_QUEUE->count(DL_RUN));
					break;
				};
				case PACKET_ASK_STOP:{
					cmd_return_int(D4X_QUEUE->count(DL_STOP));
					break;
				};
				case PACKET_ASK_PAUSE:{
					cmd_return_int(D4X_QUEUE->count(DL_PAUSE)+D4X_QUEUE->count(DL_STOPWAIT));
					break;
				};
				case PACKET_ASK_COMPLETE:{
					cmd_return_int(D4X_QUEUE->count(DL_COMPLETE));
					break;
				};
				case PACKET_ASK_READED_BYTES:{
					cmd_return_int(GVARS.READED_BYTES);
					break;
				};
				case PACKET_ASK_FULLAMOUNT:{
					int a=D4X_QUEUE->count();
					cmd_return_int(a);
					break;
				};
				case PACKET_ACK:{
					cmd_ack();
					break;
				};
				case PACKET_LS:{
					cmd_ls(packet.len,packet.type);
					break;
				};
				default:
				case PACKET_NOP: break;
				};
			};
       		if (newfd>0) close(newfd);
		newfd=0;
        };
    };
    pthread_exit(NULL);
};

tString *tMsgServer::get_string(){
    lock.lock();
    tString *temp=list->first();
    if (temp){
		list->del(temp);
    };
    lock.unlock();
    return temp;
};
/* tMsgClient, send command if Downloader already run
 */
tMsgClient::tMsgClient(){
    fd=0;
    buf=NULL;
    bufsize=0;
};

int tMsgClient::init(){
    done();
    uid_t stored_uid, euid;
    struct sockaddr_un saddr;
    if ((fd=socket(AF_UNIX, SOCK_STREAM, 0)) > 0){
		saddr.sun_family = AF_UNIX;
		stored_uid = getuid();
		euid = geteuid();
		setuid(euid);
		sprintf(saddr.sun_path, "%s/downloader_for_x_sock_%s", g_get_tmp_dir(), g_get_user_name());
		setreuid(stored_uid, euid);
		if (connect(fd,(struct sockaddr *) &saddr, sizeof (saddr)) <0)
	    	return -1;
	    return 0;
    };
    return -1;
};

int tMsgClient::send_command(int cmd,char *data,int len){
	if (init()) return -1;
	tPacket command;
	command.type=cmd;
	command.len = data!=NULL ? len:0;
	
	write(fd, &command, sizeof (command));
	if (command.len) write(fd, data, command.len);
	if (read(fd,&command,sizeof(command))<0)
		return -1;
	if (command.type!=PACKET_ACK)
		return -1;
	if (buf)
		delete[] buf;
	if (command.len){
		buf=new char[command.len];
		read(fd,buf,command.len);
	}else
		buf=NULL;
	bufsize=command.len;
	return 0;
};

int tMsgClient::send_command_short(int cmd,char *data,int len){
	if (init()) return -1;
	tPacket command;
	command.type=cmd;
	command.len = data!=NULL ? len:0;
	
	write(fd, &command, sizeof (command));
	if (command.len) write(fd, data, command.len);
	return 0;
};

int tMsgClient::get_answer_int(){
	int tmp=0;
	if (bufsize==sizeof(int)){
		memcpy(&tmp,buf,sizeof(int));
	};
	return tmp;
};

int tMsgClient::get_answer_status(tPacketStatus *status){
	tPacket command;
	command.type=PACKET_NOP;
	if (read(fd,&command,sizeof(command))<=0)
		return 0;
	if (command.type!=PACKET_ACK)
		return 0;
	if (buf)
		delete[] buf;
	if (command.len){
		buf=new char[command.len];
		read(fd,buf,command.len);
	}else
		buf=NULL;
	bufsize=command.len;
	int size=sizeof(tPacketStatus)-sizeof(char *);
	if (bufsize>=size){
		memcpy(status,buf,size);
		if (bufsize>size){
			int len=bufsize-size;
			status->url=new char[len+1];
			memcpy(status->url,buf+size,len);
			status->url[len]=0;
		};
		return(1);
	};
	return(0);
};

void tMsgClient::done(){
	if (fd>0){
		close(fd);
		fd=0;
	};
};

tMsgClient::~tMsgClient(){
	done();
	if (buf) delete[] buf;
};
