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

#include "log.h"
#include <time.h>
#include <stdio.h>
//for messages queue
#include <sys/types.h>
#include <sys/ipc.h>
#include <stdarg.h>

#if (defined(__unix__) || defined(unix)) && !defined(USG)
#include <sys/param.h>
#endif

#if (defined(BSD) && (BSD >= 199306))
#include <sys/msgbuf.h>
#else
#include <sys/msg.h>
#endif

#include "dlist.h"
#include "face/log.h"
#include "var.h"
#include "ntlocale.h"


tLogString::tLogString() {
	// body initialized by tString constructor
	time=0;
	type=LOG_FROM_SERVER;
};

tLogString::tLogString(const char *where,int len,int tp):tString(where,len) {
	time=::time(NULL);
	type=tp;
};

void tLogString::print() {
	printf("%s ",ctime(&time));
	tString::print();
};

tLogString::~tLogString() {
	if (body) delete body;
	body=NULL;
};
//******************************************//
tLog::tLog() {
	pthread_mutex_init(&mutex,NULL);
	start=time(NULL);
	Window=NULL;
	for (int i=0;i<4;i++)
		geometry[i]=0;

/* next string should be added quiet */
	current_row=0;
	char *msg=_("Log was started!");
	tLogString *temp=new tLogString(msg,strlen(msg),LOG_OK);
	temp->time=time(NULL);
	insert(temp);
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
//	if (what) printf("%s\n", what->body);
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
//	lock();
	tLogString *prom=(tLogString *)First;
	while (prom) {
		log_window_add_string(this,prom);
		prom=(tLogString *)prom->prev;
	};
//	unlock();
};

void tLog::insert(tNode *what){
	DBC_RETURN_IF_FAIL(what!=NULL);
	current_row+=1;
	((tLogString *)what)->temp=current_row;
	tQueue::insert(what);
};

void tLog::add(const char *str,int len,int type) {
	DBC_RETURN_IF_FAIL(str!=NULL);
	tLogString *temp=new tLogString(str,len,type);
	temp->time=time(NULL);
	lock();
	insert(temp);
	Size+=len;
	unlock();
	send_msg(1,temp);
};

void tLog::add(const char *str,int type) {
	DBC_RETURN_IF_FAIL(str!=NULL);
	int len=strlen(str);
	tLogString *ins=new tLogString(str,len,type);
	lock();
	insert(ins);
	Size+=len;
	unlock();
	send_msg(1,ins);
};

void tLog::add(const char *str) {
	DBC_RETURN_IF_FAIL(str!=NULL);
	int len=strlen(str);
	tLogString *ins=new tLogString(str,len,LOG_FROM_SERVER);
	lock();
	insert(ins);
	Size+=len;
	unlock();
	send_msg(1,ins);
};

void tLog::dispose() {
	unlock();
	send_msg(1,NULL);
	tStringList::dispose();
};

void tLog::unlock() {
    pthread_mutex_unlock(&mutex);
};

void tLog::lock() {
    pthread_mutex_lock(&mutex);
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
	log_window_destroy_by_log(this);
	done();// will be used by tStringList::~tStringList();
	pthread_mutex_destroy(&mutex);
};
