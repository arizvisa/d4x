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

#include "meter.h"
#include <stdio.h>

//************************************************

tMeter::tMeter() {
	sort=new tSortTree;
	counter=0;
	mode=0;
	lastval=0;
};

void tMeter::set_mode(int m){
	mode=m;
};

void tMeter::add(int speed) {
	lastval=speed;
	if (mode && counter<10){
		counter+=1;
		tSortNode *temp=last();
		if (temp){
			sort->del(temp);
			temp->key+=speed;
			sort->add(temp);
			return;
		};
	};
	counter=0;
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
	if (temp) return(temp->key);
	return 0;
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

int tMeter::last_speed(){
	return(lastval);
};

tMeter::~tMeter() {
	delete(sort);
};
