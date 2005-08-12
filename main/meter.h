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
#ifndef SPEED_METER
#define SPEED_METER
#include "sort.h"
#include "queue.h"

class tMeter:public tQueue{
public:
	typedef unsigned long long BSize;
protected:
	tSortTree *sort;
	BSize MAX,NUM;
	int counter,mode;
	BSize lastval;
public:
	tMeter();
	void add(BSize speed);
	void dispose();
	tSortNode *first();
	tSortNode *last();
	tSortNode *next();
	BSize max();
	BSize last_speed();
	BSize first_value();
	BSize last_value();
	BSize next_value();
	void set_mode(int mode);
	~tMeter();
};

#endif
