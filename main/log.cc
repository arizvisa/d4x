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

#include "log.h"
#include <time.h>
#include <stdio.h>
//for messages queue
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "dlist.h"
#include "face/log.h"
#include "var.h"


tLogString::tLogString() {
	// body initialized by tString constructor
	time=0;
	type=LOG_FROM_SERVER;
};

tLogString::tLogString(char *where,int len,int tp):tString(where,len) {
	time=::time(NULL);
	type=tp;
};

void tLogString::print() {
	printf("%s ",ctime(&time));
	tString::print();
};

tLogString::~tLogString() {
	// body will be deleted by tString destructor
};
//******************************************//
tLog::tLog() {
	start=time(NULL);
	Window=NULL;
	for (int i=0;i<4;i++)
		geometry[i]=0;
	zebra=0;
};

void tLog::store_geometry(int *a) {
	for (int i=0;i<4;i++)
		geometry[i]=a[i];
};

void tLog::get_geometry(int *a) {
	for (int i=0;i<4;i++)
		a[i]=geometry[i];
};

void tLog::send_msg(int type,tLogString *what) {
	MaxNum=CFG.MAX_LOG_LENGTH;
	if (Window) {
		mbuf Msg;
		Msg.mtype=type;
		Msg.what=what;
		Msg.which=this;
		msgsnd(MsgQueue,(msgbuf *)&Msg,sizeof(mbuf)-sizeof(long),0);
	};
};

void tLog::print() {
	if (!Window) return;
	tLogString *prom=(tLogString *)First;
	while (prom) {
		log_window_add_string(this,prom);
		prom=(tLogString *)prom->prev;
	};
};


void tLog::add(char *str,int len,int type) {
	tLogString *temp=new tLogString(str,len,type);
	temp->time=time(NULL);
	insert(temp);
	send_msg(1,temp);
	Size+=len;
};

void tLog::add(char *str,int type) {
	int len=strlen(str);
	tLogString *ins=new tLogString(str,len,type);
	send_msg(1,ins);
	insert(ins);
	Size+=len;
};

void tLog::add(char *str) {
	int len=strlen(str);
	tLogString *ins=new tLogString(str,len,LOG_FROM_SERVER);
	send_msg(1,ins);
	insert(ins);
	Size+=len;
};

void tLog::dispose() {
	send_msg(2,NULL);
	tStringList::dispose();
};

tLogString *tLog::last() {
	return (tLogString *)tStringList::last();
};

tLogString *tLog::next() {
	return (tLogString *)tStringList::next();
};

tLogString *tLog::first() {
	return (tLogString *)tStringList::first();
};

tLog::~tLog() {
	destroy_log_window_by_log(this);
	// done(); will be used by tStringList::~tStringList();
};
