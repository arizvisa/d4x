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
#if (defined(__unix__) || defined(unix)) && !defined(USG)
#include <sys/param.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>
#include "signal.h"
#include "ping.h"
#include "socket.h"
#include "ntlocale.h"

d4xPing::d4xPing(){
	data=NULL;
	pf=NULL;
	size=0;
};

/* tDownload::status is used for wieght of speed
   tDownload::action is used for connection state
*/

void d4xPing::run(tDList *list,tWriterLoger *WL){
	size=list->count();
	if (size<=0) return;
	TOTAL=0;
	data=new d4xAccessSpeed[size];
	download_set_block(1);
	pf=new pollfd[size];
	for (int i=0;i<size;i++)
		pf[i].fd=-1;
	download_set_block(0);
	tDownload *tmp=list->last();
	WL->log(LOG_OK,_("Resolving hosts..."));
	while(tmp){
		data[TOTAL].ref=NULL;
		tmp->status=0;
		tmp->action=0;
		tmp->Attempt.curent+=1; // Used for calculation number of attempts
		int len=my_get_host_by_name(tmp->info->host.get(),tmp->info->port,
					    &info,&hp,
					    buf,MAX_LEN,&rval);
		if (len<0 || (pf[TOTAL].fd = socket(info.sin_family,SOCK_STREAM, 0)) < 0){
//			printf("Can't allocate socket\n");
		}else{
	    		fcntl(pf[TOTAL].fd,F_SETFL,O_NONBLOCK);
			connect(pf[TOTAL].fd, (struct sockaddr *)&(info), len);
/*
	    		if (errno!=EINPROGRESS){
				printf("Problems with connect()\n");
				close(pf[TOTAL].fd);
			}else{
*/
				data[TOTAL].ref=tmp;
//				printf("Connected to %s\n",tmp->info->host.get());
				pf[TOTAL].events=POLLOUT|POLLIN;
				TOTAL+=1;
//			};
		};
		tmp=list->next();
	};

	WL->log(LOG_OK,_("Pinging  (it takes 30 seconds maximum)..."));
	int step=0;
	int connected=0;
	while(step<300){
		if (connected==TOTAL)
			break;
		poll(pf,TOTAL,0);
    		for (int i=0;i<TOTAL;i++){
			pf[i].events=POLLOUT|POLLIN;
			if (pf[i].revents&POLLOUT){
				data[i].ref->status+=300-step;
			};
			if (pf[i].revents&POLLIN){
				char a;
				while (read(pf[i].fd,&a,1)==1);
				if (data[i].ref->action==0){
					data[i].ref->action=1;
					connected+=1;
					data[i].ref->status+=(301-step)*150;
				};
			};
			/* FIXME: what about errors?
			if (pf[i].revents&POLLHUP)
				printf("POLLHUP ");
			if (pf[i].revents&POLLERR){
				printf("POLLERR");
			};
			*/
		};
		step+=1;
		usleep(100000);
	};
	for (int i=0;i<TOTAL;i++){
		data[i].ref->status+=((300-step)*(300-step+1))/2;
		data[i].ref->Percent+=(float(data[i].ref->status)*float(100))/float(90300);
		close(pf[i].fd);
	};
};

d4xPing::~d4xPing(){
	if (data) delete[] data;
	if (pf){
		for (int i=0;i<size;i++)
			if (pf[i].fd>=0)
				close(pf[i].fd);
		delete[] pf;
	};
};
