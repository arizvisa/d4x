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
	if (body) delete body;
};
/* tHostTree
 */

tStringHostNode *tHostTree::find(char *what){
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

void tDB::insert(tDownload *what) {
	DBC_RETURN_IF_FAIL(what!=NULL);

	tStringHostNode *temp=tree->find(what->info->host.get());
	if (!temp){
		temp=new tStringHostNode;
		temp->body=copy_string(what->info->host.get());
		tree->add(temp);
	};
	unsigned char *a=(unsigned char *)(what->info->file.get());
	if (temp->nodes[*a]==NULL){
		temp->nodes[*a]=new tDownloadTree;
		temp->filled_num+=1;
	};
	temp->nodes[*a]->add(what);
};

tDownload *tDB::find(tDownload *what) {
	DBC_RETVAL_IF_FAIL(what!=NULL,NULL);
	tStringHostNode *temp=tree->find(what->info->host.get());
	if (temp){
		unsigned char *a=(unsigned char *)(what->info->file.get());
		if (temp->nodes[*a]){
			return (tDownload*)(temp->nodes[*a]->find(what));
		};
	};
	return NULL;
};

void tDB::del(tDownload *what) {
	DBC_RETURN_IF_FAIL(what!=NULL);
	tStringHostNode *temp=tree->find(what->info->host.get());
	if (temp){
		unsigned char *file=(unsigned char *)(what->info->file.get());
		if (temp->nodes[file[0]]){
			temp->nodes[file[0]]->del(what);
			if (temp->nodes[file[0]]->empty()){
				delete (temp->nodes[file[0]]);
				temp->nodes[file[0]]=NULL;
				temp->filled_num-=1;
				if (temp->filled_num==0){
					tree->del(temp);
					delete(temp);
				};
			};
		};
	};
};

tDB::~tDB() {
	delete tree;
};
