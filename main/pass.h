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
#ifndef DOWNLOADER_H_PASS
#define DOWNLOADER_H_PASS

#include "sort.h"
#include "locstr.h"

class tUserPass:public tAbstractSortNode{
 public:
	tPStr host,user,pass;
	int proto;
	tUserPass();
	void print();
	void save(int fd);
	int cmp(tAbstractSortNode *a);
	int load(int fd);
	~tUserPass();
};

#include "face/passface.h"

class tFacePass;

class tUserPassTree:public tAbstractSortTree{
	void save_node(tUserPass *node,int fd);
	void fill_face_node(tUserPass *node,tFacePass *a);
 protected:
 public:
	void save(int fd);
	void fill_face(tFacePass *a);
	tUserPass *find(int proto,char *host);
};

tUserPassTree *load_passwords();
void save_passwords(tUserPassTree *tree);

#endif
