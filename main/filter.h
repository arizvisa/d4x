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
#ifndef _D4X_FILTER_HEADER_
#define _D4X_FILTER_HEADER_

#include "locstr.h"
#include "queue.h"
#include "addr.h"
#include "sort.h"
#include "mutex.h"

struct d4xRule:public tNode{
	tPStr path,file,host,params,tag;
	int proto;
	int include;
	d4xRule();
	void print();
	int match(tAddr *addr);
	void save(int fd);
	int load(int fd);
	~d4xRule();
};

struct d4xFilterEdit;

class d4xFilter:public tQueue{
	d4xMutex my_mutex;
 public:
	int refcount;
	int default_inc;
	tPStr name;
	d4xFilter();
	int match(tAddr *addr);
	void save(int fd);
	int load(int fd);
	void insert(tNode *node);
	void insert_before(tNode *node,tNode *where);
	void del(tNode *node);
	void print(d4xFilterEdit *edit);
	void ref();
	void unref();
	~d4xFilter();
};

struct d4xFNode:public tAbstractSortNode{
	d4xFilter *filter;
	d4xFNode();
	int cmp(tAbstractSortNode *what);
	void print(){};
	~d4xFNode();
};

struct d4xFilterSel;

class d4xFiltersTree:public tAbstractSortTree{
 protected:
	d4xMutex my_mutex;
	void save_recurse(int fd,d4xFNode *what);
	void print_recurse(d4xFNode *what);
	void print_recurse(d4xFilterSel *sel,d4xFNode *what);
 public:
	d4xFiltersTree();
	void init();
	virtual void add(tAbstractSortNode *what);
	virtual void del(tAbstractSortNode *what);
	d4xFilter *find(char *name);
	tAbstractSortNode *max();
	void save(int fd);
	int load(int fd);
	void load_from_ntrc();
	void save_to_ntrc();
	void print();
	void print(d4xFilterSel *sel);
	~d4xFiltersTree();
};

extern d4xFiltersTree *FILTERS_DB;

#endif
