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

#include "srvclt.h"
#include "var.h"
#include "main.h"
#include "locstr.h"
#include <gtk/gtk.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>
#include <string.h>

#if defined(sun)
typedef int socklen_t;
#endif

void *server_thread_run(void *what){
    tMsgServer *srv=(tMsgServer *)what;
    srv->init();
    srv->run();
    return NULL;
};

void server_thread_stop(int i){
    pthread_exit(NULL);
};

tMsgServer::tMsgServer(){
    list=new tStringList;
    list->init(0);
    file=NULL;
    fd=newfd=0;
    pthread_mutex_init(&lock,NULL);
};

tMsgServer::~tMsgServer(){
    if (fd>0) close(fd);
    if (newfd>0) close(newfd);
    pthread_mutex_destroy(&lock);
    delete(list);
    unlink(file);
    delete file;
};

void tMsgServer::init(){
	struct sigaction action,old_action;
	action.sa_handler=server_thread_stop;
	action.sa_flags=0;
	sigaction(SIGUSR2,&action,&old_action);

	sigset_t oldmask,newmask;
	sigemptyset(&newmask);
	sigaddset(&newmask,SIGINT);
	sigaddset(&newmask,SIGUSR1);
	pthread_sigmask(SIG_BLOCK,&newmask,&oldmask);

	struct sockaddr_un saddr;
	if ((fd=socket(AF_UNIX,SOCK_STREAM,0)) != -1){
		saddr.sun_family = AF_UNIX;
		sprintf(saddr.sun_path, "%s/downloader_for_x_sock_%s", g_get_tmp_dir(), g_get_user_name());
		unlink(saddr.sun_path);
		file=copy_string(saddr.sun_path);
		if (bind(fd,(struct sockaddr *)&saddr,sizeof(saddr)) != -1){
			listen(fd,5);
		};
	};
};

void tMsgServer::cmd_return_int(int what){
	tPacket packet;
	packet.type=PACKET_ACK;
	packet.len=what;
	write(newfd,&packet,sizeof(packet));
};

void tMsgServer::cmd_ack(){
	cmd_return_int(0);
};

void tMsgServer::cmd_add(int len,int type){
	char *temp=new char[len+1];
	if (read(newfd,temp,len)==len){
		temp[len]=0;
		tString *newstr=new tString;
		newstr->body=temp;
		newstr->temp=type;
		pthread_mutex_lock(&lock);
		list->insert(newstr);
		pthread_mutex_unlock(&lock);
		cmd_ack();
	}else
		delete temp;
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
					case PACKET_SET_SPEED_LIMIT:
					case PACKET_ADD:{
						if (packet.len>0){
							cmd_add(packet.len,packet.type);
							break;
						};
					};
					case PACKET_ASK_SPEED:{
						cmd_return_int(GlobalMeter->last_value());
						break;
					};
					case PACKET_ASK_RUN:{
						cmd_return_int(RunList->count());
						break;
					};
					case PACKET_ASK_STOP:{
						cmd_return_int(StopList->count());
						break;
					};
					case PACKET_ASK_PAUSE:{
						cmd_return_int(PausedList->count()+WaitStopList->count());
						break;
					};
					case PACKET_ASK_COMPLETE:{
						cmd_return_int(CompleteList->count());
						break;
					};
					case PACKET_ACK:{
						cmd_ack();
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
    pthread_mutex_lock(&lock);
    tString *temp=list->first();
    if (temp){
		list->del(temp);
    };
    pthread_mutex_unlock(&lock);
    return temp;
};
/* tMsgClient, send command if Downloader already run
 */
tMsgClient::tMsgClient(){
    fd=0;
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
	answer_int=command.len;
    return 0;
};

int tMsgClient::get_answer_int(){
	return answer_int;
};

void tMsgClient::done(){
    if (fd>0){
		close(fd);
		fd=0;
    };
};

tMsgClient::~tMsgClient(){
    done();
};
