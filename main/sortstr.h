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
#ifndef MY_SORT_STRINGS_TREE
#define MY_SORT_STRINGS_TREE

#include "liststr.h"
#include "sort.h"

struct tSortString: public tSortNode{
	char *body;
	int curent,upper;
	tSortString();
	tSortString(char *what,int len);
	void print();
	int size();
	~tSortString();
};

class tStrSortTree:public tAbstractSortTree{
	protected:
	int compare_nodes(tAbstractSortNode *a,tAbstractSortNode *b);
	int compare_nodes(tAbstractSortNode *a,char *b,int key);
	public:
		tSortString *find(char *what,int key);
};


class tHostsLimits:public tQueue{
	protected:
	int Size;
	tStrSortTree *tree;
	public:
		tHostsLimits();
    	virtual void add(char *str,int port,int curent,int upper);
    	void dispose();
		tSortString *find(char *what,int key);
		void del(tSortString *what);
    	tSortString *last();
    	tSortString *first();
    	tSortString *next();
    	tSortString *prev();
		~tHostsLimits();
};
#endif