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
#include "sortstr.h"
#include <string.h>
#include <stdio.h>

tSortString::tSortString() {
	body=NULL;
};

void tSortString::print(){
	if (body) printf("%i \t %s",key,body);
};

int tSortString::size(){
	return body?strlen(body):0;
};

tSortString::tSortString(char *what,int len) {
	body=new char[len+2];
	strncpy(body,what,len);
	body[len]=0;
};

tSortString::~tSortString() {
	if (body) delete(body);
};
//---------------------------------------------------------

int tStrSortTree::compare_nodes(tAbstractSortNode *a,tAbstractSortNode *b){
	int r=strcmp(((tSortString*)a)->body,((tSortString*)b)->body);
	if (r==0)
		return(((tSortString*)a)->key - ((tSortString*)b)->key);
	return r;
};

int tStrSortTree::compare_nodes(tAbstractSortNode *a,char *b,int key){
	int r=strcmp(((tSortString*)a)->body,b);
	if (r==0)
		return(((tSortString*)a)->key - key);
	return r;
};

tSortString *tStrSortTree::find(char *what,int key) {
	tSortString **temp=(tSortString **)&Top;
	while (*temp) {
		int a=compare_nodes(*temp,what,key);
		if (a<0)
			temp=(tSortString **)&((*temp)->more);
		else {
			if (a==0) {
				return *temp;
			};
			temp=(tSortString **)&((*temp)->less);
		};
	};
	return NULL;
};


/* ---------------------------------------------------------------
 */

tHostsLimits::tHostsLimits() {
	init(0);
	Size=0;
	tree=new tStrSortTree;
};

void tHostsLimits::add(char *str,int port,int curent,int upper) {
	int len=strlen(str);
	tSortString *ins=new tSortString(str,len);
	insert(ins);
	Size+=len;
	ins->key=port;
	ins->curent=curent;
	ins->upper=upper;
	tree->add(ins);
};

void tHostsLimits::del(tSortString *what) {
	Num-=1;
	tree->del(what);
	if (what->prev)
		what->prev->next=what->next;
	else
		Last=what->next;
	if (what->next)
		what->next->prev=what->prev;
	else
		First=what->prev;
};

void tHostsLimits::decrement(tDownload *what){
	tSortString *tmp=find(what->info->host,what->info->port);
	if (tmp) tmp->curent-=1;
};

tSortString *tHostsLimits::last() {
	return (tSortString *)(Curent=Last);
};

tSortString *tHostsLimits::first() {
	return (tSortString *)(Curent=First);
};

tSortString *tHostsLimits::next() {
	return (tSortString *)(tQueue::next());
};

tSortString *tHostsLimits::prev() {
	return (tSortString *)(tQueue::prev());
};


void tHostsLimits::dispose() {
	tSortString *temp=(tSortString*)(First);
	tree->del(temp);
	Size-=temp->size();
	tQueue::dispose();
};

tSortString *tHostsLimits::find(char *host,int port) {
	return tree->find(host,port);
};

tHostsLimits::~tHostsLimits() {
	delete tree;
};
