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
#include <package_config.h>
#include "sort.h"
#include "stdio.h"
#include "dbc.h"
tAbstractSortNode::tAbstractSortNode(){
	less=more=NULL;
};

tAbstractSortNode::~tAbstractSortNode(){
	//do nothing it is abstract
};

void tSortNode::print(){
	printf("%i",key);
};

int tSortNode::cmp(tAbstractSortNode *what){
	return(key - ((tSortNode *)what)->key);
};
// Abstractions

tAbstractSortTree::tAbstractSortTree() {
	Top=NULL;
	NUM=0;
};

void tAbstractSortTree::init() {
	Top=NULL;
	NUM=0;
};

int tAbstractSortTree::count() {
	return(NUM);
};

void tAbstractSortTree::simple_add(tAbstractSortNode **where,tAbstractSortNode *what) {
	DBC_RETURN_IF_FAIL(where!=NULL);
	DBC_RETURN_IF_FAIL(what!=NULL);
	tAbstractSortNode **temp=where;
	while (*temp) {
		if ((*temp)->cmp(what)<0)
			temp=&((*temp)->more);
		else
			temp=&((*temp)->less);
	};
	*temp=what;
};

void tAbstractSortTree::add(tAbstractSortNode *what) {
	DBC_RETURN_IF_FAIL(what!=NULL);
	if (what==NULL) return;
	NUM+=1;
	simple_add(&Top,what);
	what->less=what->more=NULL;
};

int tAbstractSortTree::empty(){
	return Top?0:1;
};

tAbstractSortNode *tAbstractSortTree::find(tAbstractSortNode *what) {
	if (what==NULL) return NULL;
	tAbstractSortNode **temp=&Top;
	while (*temp) {
		int a=(*temp)->cmp(what);
		if (a<0)
			temp=&((*temp)->more);
		else {
			if (a==0) {
				return *temp;
			};
			temp=&((*temp)->less);
		};
	};
	return NULL;
};


void tAbstractSortTree::del(tAbstractSortNode *what) {
	if (what==NULL) return;
	NUM-=1;
	tAbstractSortNode **temp=&Top;
	while (*temp && *temp!=what) {
		if ((*temp)->cmp(what)<0)
			temp=&((*temp)->more);
		else
			temp=&((*temp)->less);
	};
	*temp=what->less;
	if (what->more) simple_add(temp,what->more);
};

tAbstractSortNode *tAbstractSortTree::max() {
	if (Top==NULL) return NULL;
	tAbstractSortNode *temp=Top;
	while (temp->more) {
		temp=temp->more;
	};
	return temp;
};

void tAbstractSortTree::foreach_rec(tAbstractSortNode *node,
				    d4xSortTreeFunc doit,void *data){
	if (node->less)
		foreach_rec(node->less,doit,data);
	doit(node,this,data);
	if (node->more)
		foreach_rec(node->more,doit,data);
};

void tAbstractSortTree::foreach(d4xSortTreeFunc doit,void *data){
	if (Top)
		foreach_rec(Top,doit,data);
};

tAbstractSortTree::~tAbstractSortTree() {
	//do nothing %))
};

// going from abstraction to reality

