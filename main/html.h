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
#ifndef MY_HTML_PARSE
#define MY_HTML_PARSE
#include "liststr.h"

struct tHtmlTagField:public tNode{
	char *name;
	char *value;
	tHtmlTagField();
	void print();
	~tHtmlTagField();
};

struct tHtmlTag:public tNode{
	char *name;
	tQueue *fields;
	tHtmlTag();
	void print();
	~tHtmlTag();
};

class tHtmlParser{
	int fdesc;
	char *base;
	char *get_string_back(int len,int shift);
	char *get_word(int shift);
	char *get_word();
	char *get_word_icommas2();
	char *get_word_icommas();
	void get_fields(tHtmlTag *tag);
	char *extract_from_icommas(char *str);
	tHtmlTag *get_tag();
 public:
	void parse(int fd, tStringList *list);
};

#endif
