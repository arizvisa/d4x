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
#ifndef SORT_LIST
#define SORT_LIST
#include "queue.h"

struct tAbstractSortNode:public tNode{
	tAbstractSortNode *less,*more;
	tAbstractSortNode();
	virtual int cmp(tAbstractSortNode *what)=0;
	virtual ~tAbstractSortNode();
};

struct tSortNode:public tAbstractSortNode{
	int key;
	void print();
	int cmp(tAbstractSortNode *what);
};

class tAbstractSortTree{
	protected:
	tAbstractSortNode *Top;
	int NUM;
	void simple_add(tAbstractSortNode **where,tAbstractSortNode *what);
	public:
		tAbstractSortTree();
		void init();
		virtual void add(tAbstractSortNode *what);
		virtual void del(tAbstractSortNode *what);
		int count();
		int empty();
		virtual tAbstractSortNode *find(tAbstractSortNode *what);
		tAbstractSortNode *max();
		virtual ~tAbstractSortTree();
};


class tSortTree:public tAbstractSortTree{
	protected:
};

#endif
