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
#ifndef MY_GLOBAl_DB
#define MY_GLOBAl_DB

#include "queue.h"
#include "dlist.h"
#include "sort.h"
#include "mutex.h"

class tDownloadTree:public tAbstractSortTree{
	protected:
};

struct tStringHostNode:public tAbstractSortNode{
	char *body;
	int port;
	int filled_num;
	tDownloadTree *nodes[256];
	tStringHostNode();
	void print();
	int cmp(tAbstractSortNode *b);
	~tStringHostNode();
};

class tHostTree:public tAbstractSortTree{
protected:
public:
	tStringHostNode *find(const char *what,int port);
	~tHostTree();
};

class tDB{
	tHostTree *tree;
	tDownloadTree **hash(tStringHostNode *temp,tDownload *what);
public:
	d4x::Mutex mylock;
	tDB();
	void insert(tDownload *what);
	void del(tDownload *what);
	int empty();
	void lock(){mylock.lock();};
	void unlock(){mylock.unlock();};
	tDownload *find(tDownload *what);
	tDownload *find(const d4x::URL &addr);
	~tDB();
};

#endif
