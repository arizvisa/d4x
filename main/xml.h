#ifndef _D4X_XML_HEADER_
#define _D4X_XML_HEADER_

#include "locstr.h"
#include "queue.h"

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
	d4xXmlField *get_attr(char *name);
	d4xXmlObject *find_obj(char *name);
};

tQueue *d4x_xml_parse_fd(int fd);
tQueue *d4x_xml_parse_file(const char *filename);
d4xXmlObject *d4x_xml_find_obj(tQueue *q,char *name);
void d4x_xml_out(tQueue *q);

#endif// _D4X_XML_HEADER_
