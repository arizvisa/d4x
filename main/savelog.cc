/*	WebDownloader for X-Window
 *	Copyright (C) 1999 Koshelev Maxim
 *	This Program is free but not GPL!!! You can't modify it
 *	without agreement with autor. You can't distribute modified
 *	program but you can distribute unmodified program.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "dlist.h"
#include "config.h"
#include "liststr.h"
#include "var.h"
#include "locstr.h"
#include "savelog.h"
#include "ntlocale.h"
#include "main.h"

tDownload *get_download_from_clist(int row);

const char *LIST_FILE="list";

void write_one_node(int fd,tDownload *what) {
	tDownload *temp=what;
	write(fd,temp->info->protocol,strlen(temp->info->protocol));
	write(fd,"://",strlen("://"));
	if (temp->info->username && !equal(temp->info->username,"anonymous")) {
		write(fd,temp->info->username,strlen(temp->info->username));
		write(fd,":",strlen(":"));
		write(fd,temp->info->pass,strlen(temp->info->pass));
		write(fd,"@",strlen("@"));
	};
	write(fd,temp->info->host,strlen(temp->info->host));
	char data[MAX_LEN];
	sprintf(data,":%i",temp->info->port);
	write(fd,data,strlen(data));

	write(fd,temp->info->path,strlen(temp->info->path));
	if (temp->info->path[strlen(temp->info->path)-1]!='/')
		write(fd,"/",1);
	write(fd,temp->info->file,strlen(temp->info->file));
	char zero=0;
	write(fd,&zero,sizeof(zero));
	sprintf(data,"%i",int(temp->ScheduleTime));
	write(fd,data,strlen(data));
	write(fd,"\n",strlen("\n"));
	write(fd,temp->get_SavePath(),strlen(temp->get_SavePath()));
	write(fd,"\n",strlen("\n"));
	if (temp->get_SaveName()) write(fd,temp->get_SaveName(),strlen(temp->get_SaveName()));
	write(fd,&zero,sizeof(zero));
	sprintf(data,"0");
	if (CompleteList->owner(what))
		sprintf(data,"1");
	if (PausedList->owner(what) || WaitStopList->owner(what))
		sprintf(data,"2");
	if (StopList->owner(what))
		sprintf(data,"3");
	if (RunList->owner(what))
		sprintf(data,"4");
	write(fd,data,strlen(data));
	write(fd,"\n",strlen("\n"));
};

void save_list() {
	if (!HOME_VARIABLE) {
		MainLog->add(_("Can't save default list of downloads!!! Becouse can't find HOME variable"),LOG_ERROR);
		return;
	};
	char *path=new char[strlen(LIST_FILE)+strlen(HOME_VARIABLE)+strlen(CFG_DIR)+3];
	*path=0;
	strcat(path,HOME_VARIABLE);
	strcat(path,"/");
	strcat(path,CFG_DIR);
	strcat(path,"/");
	strcat(path,LIST_FILE);
	if (save_list_to_file(path)) {
		MainLog->add(_("Can't save default list of downloads!!!"),LOG_ERROR);
	};
	delete(path);
};

int save_list_to_file(char *path) {
	remove(path);
	int fd=open(path,O_TRUNC | O_CREAT |O_RDWR,S_IRUSR | S_IWUSR);
	if (fd<0) return -1;
	int i=0;
	tDownload *temp=get_download_from_clist(i);
	// *** Calculate amount of elements ***
	while (temp) {
		i++;
		temp=get_download_from_clist(i);
	};
	// *** Saving them in reverse order  ***
	for (int a=i-1;a>=0;a--) {
		temp=get_download_from_clist(a);
		write_one_node(fd,temp);
	};
	close(fd);
	return 0;
};

void read_list(tStringList *where) {
	if (!HOME_VARIABLE) return;
	char *path=new char[strlen(LIST_FILE)+strlen(HOME_VARIABLE)+strlen(CFG_DIR)+3];
	*path=0;
	strcat(path,HOME_VARIABLE);
	strcat(path,"/");
	strcat(path,CFG_DIR);
	strcat(path,"/");
	strcat(path,LIST_FILE);
	if (read_list_from_file(path,where)) {
		char *error_msg=_("Can't load default list of downloads!!!");
		if (MainLog)
			MainLog->add(error_msg,LOG_ERROR);
		else
			puts(error_msg);
	};
	delete(path);
};

int read_list_from_file(char *path,tStringList *where) {
	int fd=open(path,O_RDONLY,S_IRUSR | S_IWUSR);
	if (fd>0) {
		char temp[MAX_LEN];
		char *cur=temp;
		while(read(fd,cur,1)>0) {
			char *label=NULL;
			if (*cur==0) label=cur;
			while(*cur!='\n') {
				cur++;
				if (read(fd,cur,1)==0) break;
				if (*cur==0) label=cur;
			};
			*cur=0;
			where->add(temp);
			tString *str=where->last();
			str->temp=0;
			if (label) {
				sscanf(label+1,"%i",&(str->temp));
			};
			cur=temp;
		};
		close(fd);
		return 0;
	};
	return -1;
};
