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
#include "face/lod.h"

const char *LIST_FILE="list";
const char *NEW_LIST_FILE="Default.dl";

void remove_old_file(){
	char *path=sum_strings(HOME_VARIABLE,"/",CFG_DIR,"/",LIST_FILE,NULL);
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

int save_list_to_file_current(char *path) {
	DBC_RETVAL_IF_FAIL(path!=NULL,-1);
	if (D4X_QUEUE==NULL) return(0);
	int fd=open(path,O_TRUNC | O_CREAT |O_RDWR,S_IRUSR | S_IWUSR);
	if (fd<0) return(-1);
	D4X_QUEUE->save_to_config_list(fd);
	close(fd);
	return(0);
};

int save_list_to_file(char *path) {
	DBC_RETVAL_IF_FAIL(path!=NULL,-1);
	remove(path);
	int fd=open(path,O_TRUNC | O_CREAT |O_RDWR,S_IRUSR | S_IWUSR);
	if (fd<0) return -1;
	d4xDownloadQueue *q=(d4xDownloadQueue *)(D4X_QTREE.first());
	while(q){
		q->save_to_config(fd);
		q=(d4xDownloadQueue *)(q->prev);
	};
	close(fd);
	return 0;
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
			if (equal(buf,"Queue:")){
				d4xDownloadQueue *temp=new d4xDownloadQueue;
				if (temp->load_from_config(fd)<0){
					delete(temp);
					break;
				}else
					D4X_QTREE.insert(temp);
			};
		};
		close(fd);
	}else
		return -1;
	return 0;
};


int read_list_from_file_current(char *path) {
	if (D4X_QUEUE==NULL) return(0);
	int fd=open(path,O_RDONLY,S_IRUSR | S_IWUSR);
	if (fd<0) return(-1);
	int r=D4X_QUEUE->load_from_config_list(fd);
	close(fd);
	_aa_.try_to_run_wait(D4X_QUEUE);
	return(r);
};
