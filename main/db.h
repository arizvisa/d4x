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
#ifndef MY_GLOBAl_DB
#define MY_GLOBAl_DB

#include "queue.h"
#include "dlist.h"
#include "sort.h"

class tDownloadTree:public tAbstractSortTree{
	protected:
	int compare_nodes(tAbstractSortNode *a,tAbstractSortNode *b);
};

struct tStringHostNode:public tAbstractSortNode{
	char *body;
	int filled_num;
	tDownloadTree *nodes[256];
	tStringHostNode();
	void print();
	~tStringHostNode();
};

class tHostTree:public tAbstractSortTree{
	protected:
	int compare_nodes(tAbstractSortNode *a,tAbstractSortNode *b);
	int compare_nodes(tAbstractSortNode *a,char *b);
	public:
		tStringHostNode *find(char *what);
};

class tDB{
	tHostTree *tree;
	public:
		tDB();
		void insert(tDownload *what);
		void del(tDownload *what);
		tDownload *find(tDownload *what);
		~tDB();
};

#endif