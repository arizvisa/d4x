/*	WebDownloader for X-Window
 *	Copyright (C) 1999 Koshelev Maxim
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
	protected:
	tSortTree *sort;
	int MAX,NUM;
	public:
		tMeter();
		void add(int speed);
		void dispose();
		tSortNode *first();
		tSortNode *last();
		tSortNode *next();
		int max();
		int first_value();
		int last_value();
		int next_value();
		~tMeter();
};

#endif