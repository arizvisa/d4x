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

tStringHostNode::tStringHostNode(){
	body=NULL;
	filled_num=0;
	for (int i=0;i<256;i++)
		nodes[i]=NULL;
};

void tStringHostNode::print(){
	if (body) puts(body);
};

tStringHostNode::~tStringHostNode(){
	if (body) delete body;
};
/* tHostTree
 */
int tDownloadTree::compare_nodes(tAbstractSortNode *a,tAbstractSortNode *b){
	int r=strcmp(((tDownload*)a)->info->file.get(),((tDownload*)b)->info->file.get());
	if (r)
		return r;
	return strcmp(((tDownload*)a)->info->path.get(),((tDownload*)b)->info->path.get());
};

int tHostTree::compare_nodes(tAbstractSortNode *a,tAbstractSortNode *b){
	return strcmp(((tStringHostNode*)a)->body,((tStringHostNode*)b)->body);
};

int tHostTree::compare_nodes(tAbstractSortNode *a,char *b){
	return strcmp(((tStringHostNode*)a)->body,b);
};

tStringHostNode *tHostTree::find(char *what){
	tAbstractSortNode **temp=&Top;
	while (*temp) {
		int a=compare_nodes(*temp,what);
		if (a<0)
			temp=&((*temp)->more);
		else {
			if (a==0) {
				return (tStringHostNode *)(*temp);
			};
			temp=&((*temp)->less);
		};
	};
	return NULL;
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
