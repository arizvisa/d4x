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

#include "queue.h"
#include <stdio.h>
#include "var.h"

tNode::tNode() {
	next=prev=NULL;
};

tNode::~tNode() {
	//do nothing
};

//************************************/
tQueue::tQueue() {
	Curent=First=Last=NULL;
	MaxNum=0;
	Num=0;
};

void tQueue::free_to_limit(){
	if (MaxNum && Num>=MaxNum) {
		while (Num>=MaxNum)
			dispose();
	};
};

void tQueue::init(int n) {
	MaxNum=n;
	tNode *tmp=First;
	Num=0;
	while (tmp){
		Num+=1;
		tmp=tmp->prev;
	};
	free_to_limit();
};

void tQueue::insert(tNode *what) {
	DBC_RETURN_IF_FAIL(what!=NULL);
	free_to_limit();
	Num+=1;
	what->prev=NULL;
	if ((what->next=Last)) {
		Last->prev=what;
	} else {
		First=what;
	};
	Curent=Last=what;
};

void tQueue::insert_before(tNode *what,tNode *where) {
	DBC_RETURN_IF_FAIL(what!=NULL);
	DBC_RETURN_IF_FAIL(where!=NULL);
//	if (what==NULL || where==NULL) return;
	/* what and where != NULL
	 * if First than .next==NULL
	 * if Last than .prev==NULL
	 */
	free_to_limit();
	Num+=1;
	if ((what->next=where->next))
		what->next->prev=what;
	else
		First=what;
	what->prev=where;
	where->next=what;
};

tNode *tQueue::last() {
	return Curent=Last;
};

tNode *tQueue::first() {
	return Curent=First;
};

tNode *tQueue::next() {
	if (Curent) Curent=Curent->next;
	return Curent;
};

tNode *tQueue::prev() {
	if (Curent) Curent=Curent->prev;
	return Curent;
};

void tQueue::del(tNode *what) {
	DBC_RETURN_IF_FAIL(what!=NULL);
//	if (what==NULL) return;
	Num-=1;
	if (what->prev)
		what->prev->next=what->next;
	else
		Last=what->next;
	if (what->next)
		what->next->prev=what->prev;
	else
		First=what->prev;
};

void tQueue::dispose() {
	if (First){
		tNode *prom = First;
		First = First->prev;
		delete prom;
		if (First) First->next = NULL;
		else Last = NULL;
		Num -= 1;
	};
};

int tQueue::count() {
	return Num;
};

void tQueue::done() {
	while (First)
		dispose();
};

void tQueue::sort(d4xNodeCmpFunc cmpfunc){
	tNode *a=First;
	while (a && a->prev){
		tNode *prev=a->prev;
		if (cmpfunc(a,prev)>0){
			del(a);
			insert(a);
			a=First;
		}else{
			a=prev;
		};
	};
};

tQueue::~tQueue() {
	done();
};
//************************************/
