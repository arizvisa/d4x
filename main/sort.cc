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

#include "sort.h"
#include "stdio.h"
tAbstractSortNode::tAbstractSortNode(){
	less=more=NULL;
};

tAbstractSortNode::~tAbstractSortNode(){
	//do nothing it is abstract
};

void tSortNode::print(){
	printf("%i",key);
};
// Abstractions

tAbstractSortTree::tAbstractSortTree() {
	Top=NULL;
};

void tAbstractSortTree::init() {
	Top=NULL;
};

void tAbstractSortTree::simple_add(tAbstractSortNode **where,tAbstractSortNode *what) {
	tAbstractSortNode **temp=where;
	while (*temp) {
		if (compare_nodes(*temp,what)<0)
			temp=&((*temp)->more);
		else
			temp=&((*temp)->less);
	};
	*temp=what;
};

void tAbstractSortTree::add(tAbstractSortNode *what) {
	simple_add(&Top,what);
	what->less=what->more=NULL;
};

int tAbstractSortTree::empty(){
	return Top?0:1;
};

tAbstractSortNode *tAbstractSortTree::find(tAbstractSortNode *what) {
	tAbstractSortNode **temp=&Top;
	while (*temp) {
		int a=compare_nodes(*temp,what);
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
	tAbstractSortNode **temp=&Top;
	while (*temp && *temp!=what) {
		if (compare_nodes(*temp,what)<0)
			temp=&((*temp)->more);
		else
			temp=&((*temp)->less);
	};
	*temp=what->less;
	if (what->more) simple_add(temp,what->more);
};

tAbstractSortNode *tAbstractSortTree::max() {
	tAbstractSortNode *temp=Top;
	while (temp->more) {
		temp=temp->more;
	};
	return temp;
};

tAbstractSortTree::~tAbstractSortTree() {
	//do nothing %))
};

// going from abstraction to reality
 
int tSortTree::compare_nodes(tAbstractSortNode *a,tAbstractSortNode *b){
	return(((tSortNode *)a)->key - ((tSortNode *)b)->key);
};