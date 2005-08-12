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
#ifndef MY_HTML_PARSE
#define MY_HTML_PARSE
#include "liststr.h"
#include "client.h"
#include "addr.h"
#include <string>

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
	char *descr;
	tHtmlUrl();
	void print();
	~tHtmlUrl();
};

struct tHtmlTag:public tNode{
	char *name;
	char *descr;
	tQueue *fields;
	tHtmlTag();
	tHtmlTagField *find_field(const char *name);
	void print();
	~tHtmlTag();
};

class tHtmlParser{
	tWriterLoger *WL;
	char *base;
	std::string codepage;
	int quest_sign_replace;
	char *get_string_back(int len,int shift);
	char *get_word(int shift);
	char *get_word_o(int shift);
	char *get_word();
	char *get_word_icommas2();
	char *get_word_icommas();
	void get_fields(tHtmlTag *tag);
	char *extract_from_icommas(char *str);
	void compact_string(char *str);
	void look_for_meta_content(tHtmlTagField *where,tQueue *list,
				   tAddr *papa,const char *tag);
	void get_charset_from_meta(tHtmlTagField *fld);
	tHtmlTag *get_tag();
	void get_tag_descr(tHtmlTag *tag);
	void fix_url(char *url,tQueue *list,tAddr *papa,const char *tag,const char *descr=NULL);
	void write_left_fields(tHtmlTag *tag);
	char *convert_to_utf8(const char *src);
 public:
	void set_content_type(const char *ct);
	int out_fd,leave;
	void parse(tWriterLoger *wl, tQueue *list,tAddr *papa,int qsignreplace=0);
};

tAddr *fix_url_global(char *url,tAddr *papa,int out_fd,int leave,int quest_sign_replace=0);

#endif
