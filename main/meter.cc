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

#include "meter.h"
#include <stdio.h>

//************************************************

tMeter::tMeter() {
	sort=new tSortTree;
};

void tMeter::add(int speed) {
	tSortNode *temp=new tSortNode;
	temp->key=speed;
	insert(temp);
	sort->add(temp);
};

void tMeter::dispose() {
	sort->del((tSortNode *)First);
	tQueue::dispose();
};

tSortNode *tMeter::first() {
	return (tSortNode *)(Curent=First);
};

tSortNode *tMeter::last() {
	return (tSortNode *)(Curent=Last);
};

tSortNode *tMeter::next() {
	return (tSortNode *)(tQueue::next());
};

int tMeter::max() {
	tSortNode *temp=(tSortNode *)(sort->max());
	return temp->key;
};

int tMeter::first_value() {
	tSortNode *temp=first();
	if (temp) return temp->key;
	return 0;
};

int tMeter::last_value() {
	tSortNode *temp=last();
	if (temp) return temp->key;
	return 0;
};

int tMeter::next_value() {
	tSortNode *temp=next();
	if (temp)  return temp->key;
	return 0;
};

tMeter::~tMeter() {
	delete(sort);
};
