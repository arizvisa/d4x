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
#include "db.h"
#include "locstr.h"
#include <stdio.h>
#include "dbc.h"

/* data base is used for avoiding adding the same URLs in Downloader */

tStringHostNode::tStringHostNode(){
	body=NULL;
	filled_num=0;
	port=0;
	for (int i=0;i<256;i++)
		nodes[i]=NULL;
};

void tStringHostNode::print(){
	if (body) printf("%s:",body);
	printf("%i\n",port);
};

int tStringHostNode::cmp(tAbstractSortNode *b){
	int a=strcmp(body,((tStringHostNode*)b)->body);
	return(a?a:((tStringHostNode*)b)->port-port);
};

tStringHostNode::~tStringHostNode(){
	if (body) delete[] body;
};
/* tHostTree
 */

tStringHostNode *tHostTree::find(char *what,int port){
	DBC_RETVAL_IF_FAIL(what!=NULL,NULL);
	tStringHostNode temp;
	temp.body=what;
	temp.port=port;
	tStringHostNode *rvalue=(tStringHostNode *)tAbstractSortTree::find((tAbstractSortNode *)(&temp));
	temp.body=NULL;
	return rvalue;
};
/* tDB
 */

tDB::tDB() {
	tree=new tHostTree;
};

int tDB::empty(){
	return tree->empty();
};


/*
tDownloadTree **tDB::hash(tStringHostNode *temp,tDownload *what){
	unsigned char *b=(unsigned char *)(what->info->file.get());
	return(&(temp->nodes[*b]));
};
*/

/* This implementation of hash function seems to be more suitable for
   downloads' db. Previous one generates too short range of values.
 */

tDownloadTree **tDB::hash(tStringHostNode *temp,tDownload *what){
	unsigned char *b=(unsigned char *)(what->info->path.get());
	unsigned char a=0;
	for (int i=0;i<5;i++,b++){
		if (*b==0) break;
		a+=*b;
	};
	return(&(temp->nodes[a]));
};

void tDB::insert(tDownload *what) {
	DBC_RETURN_IF_FAIL(what!=NULL);
	mylock.lock();
	tStringHostNode *temp=tree->find(what->info->host.get(),what->info->port);
	if (!temp){
		temp=new tStringHostNode;
		temp->body=copy_string(what->info->host.get());
		temp->port=what->info->port;
		tree->add(temp);
	};
	tDownloadTree **point=hash(temp,what);
	if (*point==NULL){
		*point = new tDownloadTree;
		temp->filled_num+=1;
	};
	(*point)->add(what);
	mylock.unlock();
};

tDownload *tDB::find(tAddr *addr){
	DBC_RETVAL_IF_FAIL(addr!=NULL,NULL);
	tDownload *tmp=new tDownload;
	tmp->info=addr;
	tDownload *rval=find(tmp);
	tmp->info=NULL;
	delete(tmp);
	return(rval);
};

tDownload *tDB::find(tDownload *what) {
	DBC_RETVAL_IF_FAIL(what!=NULL,NULL);
	DBC_RETVAL_IF_FAIL(what->info!=NULL,NULL);
	tStringHostNode *temp=tree->find(what->info->host.get(),what->info->port);
	if (temp){
		tDownloadTree **point=hash(temp,what);
		if (*point){
			return(tDownload*)((*point)->find(what));
		};
	};
	return NULL;
};

void tDB::del(tDownload *what) {
	DBC_RETURN_IF_FAIL(what!=NULL);
	DBC_RETURN_IF_FAIL(what->info!=NULL);
	mylock.lock();
	tStringHostNode *temp=tree->find(what->info->host.get(),what->info->port);
	if (temp){
		tDownloadTree **point=hash(temp,what);
		if (*point){
			(*point)->del(what);
			if ((*point)->empty()){
				delete (*point);
				*point=NULL;
				temp->filled_num-=1;
				if (temp->filled_num==0){
					tree->del(temp);
					delete(temp);
				};
			};
		};
	};
	mylock.unlock();
};

tDB::~tDB() {
	delete tree;
};
