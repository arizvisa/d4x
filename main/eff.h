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
#ifndef _DOWNLOADER_FILE_PARSER_
#define _DOWNLOADER_FILE_PARSER_

#include "sort.h"
#include "var.h"


struct d4xEffString:public tAbstractSortNode{
	char *body;
	d4xEffString();
	d4xEffString(const char *a);
	void print();
	int cmp(tAbstractSortNode *what);
	~d4xEffString();
};

class tUrlParser{
	int fd;
	unsigned char buf[MAX_LEN];
	tAbstractSortTree *tree;
	int sequence(unsigned char *where, char *str);
	int read_url(unsigned char *where);
 public:
	unsigned int full,current;
	tUrlParser(const char *filename);
	tAbstractSortTree *parse();
	tAbstractSortTree *get_list();
	~tUrlParser();
};

struct d4xLinksSel;

int thread_for_parse_txt(tUrlParser *parser);
float thread_for_parse_percent();
int thread_for_parse_txt_status();
void thread_for_parse_add(d4xLinksSel *sel);
void thread_for_parse_stop();
int thread_for_parse_full();

#endif
