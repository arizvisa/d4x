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
	for (int i=0;i<256;i++)
		nodes[i]=NULL;
};

void tStringHostNode::print(){
	if (body) puts(body);
};

int tStringHostNode::cmp(tAbstractSortNode *b){
	return strcmp(body,((tStringHostNode*)b)->body);
};

tStringHostNode::~tStringHostNode(){
	if (body) delete[] body;
};
/* tHostTree
 */

tStringHostNode *tHostTree::find(char *what){
	DBC_RETVAL_IF_FAIL(what!=NULL,NULL);
	tStringHostNode temp;
	temp.body=what;
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
	pthread_mutex_init(&mylock,NULL);
	for (int i=0;i<5;i++,b++){
		if (*b==0) break;
		a+=*b;
	};
	return(&(temp->nodes[a]));
};

void tDB::insert(tDownload *what) {
	DBC_RETURN_IF_FAIL(what!=NULL);
	lock();
	tStringHostNode *temp=tree->find(what->info->host.get());
	if (!temp){
		temp=new tStringHostNode;
		temp->body=copy_string(what->info->host.get());
		tree->add(temp);
	};
	tDownloadTree **point=hash(temp,what);
	if (*point==NULL){
		*point = new tDownloadTree;
		temp->filled_num+=1;
	};
	(*point)->add(what);
	unlock();
};

tDownload *tDB::find(tDownload *what) {
	DBC_RETVAL_IF_FAIL(what!=NULL,NULL);
	DBC_RETVAL_IF_FAIL(what->info!=NULL,NULL);
	tStringHostNode *temp=tree->find(what->info->host.get());
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
	lock();
	tStringHostNode *temp=tree->find(what->info->host.get());
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
	unlock();
};

void tDB::lock(){
	pthread_mutex_lock(&mylock);
};

void tDB::unlock(){
	pthread_mutex_unlock(&mylock);
};

tDB::~tDB() {
	pthread_mutex_destroy(&mylock);
	delete tree;
};
