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
#ifndef MY_HTML_PARSE
#define MY_HTML_PARSE
#include "liststr.h"
#include "client.h"
#include "addr.h"

struct tHtmlTagField:public tNode{
	char *name;
	char *value;
	int saved;
	tHtmlTagField();
	void print();
	~tHtmlTagField();
};

struct tHtmlUrl:public tNode{
	tAddr *info;
	tHtmlUrl();
	void print();
	~tHtmlUrl();
};

struct tHtmlTag:public tNode{
	char *name;
	tQueue *fields;
	tHtmlTag();
	void print();
	~tHtmlTag();
};

class tHtmlParser{
	tWriterLoger *WL;
	char *base;
	char *get_string_back(int len,int shift);
	char *get_word(int shift);
	char *get_word_o(int shift);
	char *get_word();
	char *get_word_icommas2();
	char *get_word_icommas();
	void get_fields(tHtmlTag *tag);
	char *extract_from_icommas(char *str);
	void compact_string(char *str);
	void look_for_meta_content(tHtmlTagField *where,tQueue *list,tAddr *papa);
	tHtmlTag *get_tag();
	void fix_url(char *url,tQueue *list,tAddr *papa);
	void write_left_fields(tHtmlTag *tag);
 public:
	int out_fd,leave;
	void parse(tWriterLoger *wl, tQueue *list,tAddr *papa);
};

tAddr *fix_url_global(char *url,tAddr *papa,int out_fd,int leave);

#endif
