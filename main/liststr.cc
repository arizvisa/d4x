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

#include <string.h>
#include <strings.h>
#include <stdio.h>
#include "liststr.h"
#include "var.h"
tString::tString(){
	body=NULL;
	temp=0;
};

tString::tString(const char *what,int len) {
	body=new char[len+1];
	strncpy(body,what,len);
	body[len]=0;
};

void tString::print() {
	if (body) puts(body);
};

int tString::size() {
	return(body?strlen(body):0);
};

tString::~tString() {
	if (body) delete[] body;
};

/*************************************************/
tString *tMemory::add(){
	tString *temp=new tString;
	insert(temp);
	return temp;
};

void tMemory::del(tString *a){
	tStringList::del(a);
	delete(a);
};

/*************************************************/
tStringList::tStringList():tQueue(){
	init(CFG.MAX_LOG_LENGTH);
	Size=0;
};

tString *tStringList::last() {
	return (tString *)(Curent=Last);
};

tString *tStringList::first() {
	return (tString *)(Curent=First);
};

tString *tStringList::next() {
	return (tString *)(tQueue::next());
};

tString *tStringList::prev() {
	return (tString *)(tQueue::prev());
};


int tStringList::add_strings(char *what,int len) {
	DBC_RETVAL_IF_FAIL(what!=NULL,0);
	int length=1;
	char *where=what,*begin=what;
	while(len--) {
		if (*where=='\n' || *where==0) {
			tString *ins=new tString(begin,length);
			insert(ins);
			length=1;
			where++;
			begin=where;
		} else {
			where++;
			length++;
		};
	};
	return(length-1);
};

void tStringList::add(const char *str,int len) {
	DBC_RETURN_IF_FAIL(str!=NULL);
	tString *ins=new tString(str,len);
	insert(ins);
	Size+=len;
};

void tStringList::add(const char *str) {
	DBC_RETURN_IF_FAIL(str!=NULL);
	int len=strlen(str);
	tString *ins=new tString(str,len);
	insert(ins);
	Size+=len;
};

int tStringList::size() {
	return Size;
};


void tStringList::print() {
	tNode *prom=First;
	while (prom) {
		prom->print();
		prom=prom->prev;
	};
};

void tStringList::done() {
	Size=0;
	tQueue::done();
};

void tStringList::dispose() {
	tString *temp=(tString*)(First);
	Size-=temp->size();
	tQueue::dispose();
};

tStringList::~tStringList() {
	done();
};
