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
#ifndef DOWNLOADER_H_PASS
#define DOWNLOADER_H_PASS

#include "sort.h"

class tUserPass:public tAbstractSortNode{
	char *host,*user,*pass;
 public:
	int proto;
	tUserPass();
	void set_host(char *a);
	void set_user(char *a);
	void set_pass(char *a);
	char *get_host();
	char *get_user();
	char *get_pass();
	void print();
	void save(int fd);
	int load(int fd);
	~tUserPass();
};

#include "face/passface.h"

class tFacePass;

class tUserPassTree:public tAbstractSortTree{
	void save_node(tUserPass *node,int fd);
	void fill_face_node(tUserPass *node,tFacePass *a);
 protected:
	int compare_nodes(tAbstractSortNode *a,tAbstractSortNode *b);
 public:
	void save(int fd);
	void fill_face(tFacePass *a);
	tUserPass *find(int proto,char *host);
};

tUserPassTree *load_passwords();
void save_passwords(tUserPassTree *tree);

#endif
