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

#include "queue.h"
#include <stdio.h>
#include "var.h"

tNode::tNode() {
	next=prev=NULL;
};

tNode::~tNode() {
	//do nothing
}
;

//************************************/
tQueue::tQueue() {
	MaxNum=100;
	Num=0;
};


void tQueue::init(int n) {
	MaxNum=n;
	Num=0;
	Curent=First=Last=NULL;
};

void tQueue::insert(tNode *what) {
	if (MaxNum && Num>=MaxNum) {
		while (Num>=MaxNum)
			dispose();
	};
	Num+=1;
	what->next=Last;
	what->prev=NULL;
	if (Last) {
		Last->prev=what;
	} else {
		First=what;
	};
	Curent=Last=what;
};

void tQueue::insert_before(tNode *what,tNode *where) {
	/* what and where != NULL
	 * if First than .next==NULL
	 * if Last than .prev==NULL
	 */
	if (MaxNum && Num==MaxNum) {
		while (Num>=MaxNum)
			dispose();
	};
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
	if(First->prev) First->prev->next=NULL;
	else Last=NULL;
	tNode *prom=First;
	First=First->prev;
	delete prom;
	Num-=1;
};

int tQueue::count() {
	return Num;
};

void tQueue::done() {
	while (First)
		dispose();
};

tQueue::~tQueue() {
	done();
};
//************************************/
