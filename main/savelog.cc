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

extern tMain aa;

tDownload *get_download_from_clist(int row);

const char *LIST_FILE="list";
const char *NEW_LIST_FILE="Default.dl";

void remove_old_file(){
	char *path=new char[strlen(LIST_FILE)+strlen(HOME_VARIABLE)+strlen(CFG_DIR)+3];
	sprintf(path,"%s/%s/%s",HOME_VARIABLE,CFG_DIR,LIST_FILE);
	remove(path);
	delete[] path;
};

void save_list() {
	remove_old_file();
	if (!HOME_VARIABLE) {
		MainLog->add(_("Can't save default list of downloads!!! Becouse can't find HOME variable"),LOG_ERROR);
		return;
	};
	char *path=new char[strlen(NEW_LIST_FILE)+strlen(HOME_VARIABLE)+strlen(CFG_DIR)+3];
	sprintf(path,"%s/%s/%s",HOME_VARIABLE,CFG_DIR,NEW_LIST_FILE);
	if (save_list_to_file(path)) {
		MainLog->add(_("Can't save default list of downloads!!!"),LOG_ERROR);
	};
	delete[] path;
};

int save_list_to_file(char *path) {
	DBC_RETVAL_IF_FAIL(path!=NULL,-1);
	remove(path);
	int fd=open(path,O_TRUNC | O_CREAT |O_RDWR,S_IRUSR | S_IWUSR);
	if (fd<0) return -1;
	int i=0;
	if (CFG.WITHOUT_FACE){
		for (int i=DL_ALONE+1;i<=DL_COMPLETE;i++){
			tDownload *temp=DOWNLOAD_QUEUES[i]->first();
			while(temp){
				temp->save_to_config(fd);
				temp=DOWNLOAD_QUEUES[i]->prev();
			};
		};
	}else{
		tDownload *temp=get_download_from_clist(i);
		while (temp) {
			i++;
			temp->save_to_config(fd);
			temp=get_download_from_clist(i);
		};
	};
	close(fd);
	return 0;
};

void read_list(tStringList *where) {
	DBC_RETURN_IF_FAIL(where!=NULL);
	if (!HOME_VARIABLE) return;
	char *path=new char[strlen(LIST_FILE)+strlen(HOME_VARIABLE)+strlen(CFG_DIR)+3];
	sprintf(path,"%s/%s/%s",HOME_VARIABLE,CFG_DIR,LIST_FILE);
	read_list_from_file_old(path,where) ;
	delete[] path;
};


void read_list() {
	if (!HOME_VARIABLE) return;
	char *path=new char[strlen(NEW_LIST_FILE)+strlen(HOME_VARIABLE)+strlen(CFG_DIR)+3];
	sprintf(path,"%s/%s/%s",HOME_VARIABLE,CFG_DIR,NEW_LIST_FILE);
	if (read_list_from_file(path)) {
		char *error_msg=_("Can't load default list of downloads!!!");
		if (MainLog)
			MainLog->add(error_msg,LOG_ERROR);
		else
			puts(error_msg);
	};
	delete[] path;
};

int read_list_from_file(char *path) {
	DBC_RETVAL_IF_FAIL(path!=NULL,-1);
	char buf[MAX_LEN];
	int fd=open(path,O_RDONLY,S_IRUSR | S_IWUSR);
	if (fd>=0) {
		while(f_rstr(fd,buf,MAX_LEN)>0){
			if (equal(buf,"Download:")){
				tDownload *temp=new tDownload;
				if (temp->load_from_config(fd)<0){
					delete(temp);
					break;
				}else
					aa.add_downloading_to(temp);
			};
		};
		close(fd);
	}else
		return -1;
	return 0;
};

int read_list_from_file_old(char *path,tStringList *where) {
	DBC_RETVAL_IF_FAIL(path!=NULL,-1);
	DBC_RETVAL_IF_FAIL(where!=NULL,-1);
	int fd=open(path,O_RDONLY,S_IRUSR | S_IWUSR);
	if (fd>=0) {
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
