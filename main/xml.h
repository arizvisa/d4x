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
#include "addr.h"
#include "locstr.h"
#include "var.h"
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "dbc.h"
#ifndef _D4X_XML_HEADER_
#define _D4X_XML_HEADER_

#include "locstr.h"
#include "queue.h"
#include <string>

struct d4xXmlField:public tNode{
	tPStr name,value;
	void print();
};

struct d4xXmlObject:public tNode{
	tPStr name;
	tPStr value;
	tQueue fields;
	tQueue objects;
	void print();
	void print_rec(int depth);
	d4xXmlField *get_attr(const char *name);
	d4xXmlObject *find_obj(const char *name);
};

tQueue *d4x_xml_parse_fd(int fd);
tQueue *d4x_xml_parse_file(const char *filename);
d4xXmlObject *d4x_xml_find_obj(tQueue *q,const std::string &name);
char *d4x_xml_find_obj_value(tQueue *q,const std::string &path);
void d4x_xml_out(tQueue *q);

#endif// _D4X_XML_HEADER_
