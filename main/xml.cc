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
#include "xml.h"
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

void d4xXmlField::print(){
	printf("%s=\"%s\"",name.get(),value.get());
};

void d4xXmlObject::print(){
	printf("<%s",name.get());
	d4xXmlField *fld=(d4xXmlField *)(fields.first());
	while(fld){
		printf(" ");
		fld->print();
		fld=(d4xXmlField *)(fld->prev);
	};
	printf(">\n%s",value.get());
	printf("</>");
};

void d4xXmlObject::print_rec(int depth){
	int i;
	for (i=0;i<depth;i++) printf(" ");
	printf("<%s",name.get());
	d4xXmlField *fld=(d4xXmlField *)(fields.first());
	while(fld){
		printf(" ");
		fld->print();
		fld=(d4xXmlField *)(fld->prev);
	};
	printf(">\n");
	d4xXmlObject *obj=(d4xXmlObject *)objects.first();
	while(obj){
		obj->print_rec(depth+1);
		obj=(d4xXmlObject *)(obj->prev);
	};
	printf("%s\n",value.get());
	for (i=0;i<depth;i++) printf(" ");
	printf("</>\n");
};

d4xXmlField *d4xXmlObject::get_attr(char *name){
	d4xXmlField *fld=(d4xXmlField *)fields.first();
	while(fld){
		if (equal_uncase(name,fld->name.get()))
			return(fld);
		fld=(d4xXmlField *)(fld->prev);
	};
	return(NULL);
};

d4xXmlObject *d4xXmlObject::find_obj(char *name){
	return(d4x_xml_find_obj(&objects,name));
};
/*************************************************************/

int d4x_xml_skip_for(int fd,char *b){
//	printf("d4x_xml_skip_for!\n");
	char a=0;
	int i=1;
	while(read(fd,&a,sizeof(char))==sizeof(char)){
		if (index(b,a)){
			i=0;
			break;
		};
	};
	return(i);
};

void d4x_xml_read_grow(int fd,tPStr *str,char *b,char *stopchars){
//	printf("d4x_xml_read_grow!\n");
	char buf[101];
	char *cur=buf;
	*b=0;
	while(read(fd,cur,1)==1){
		if (index(stopchars,*cur)){
			*b=*cur;
			*cur=0;
			cur=sum_strings(str->get(),buf,NULL);
			str->set(cur);
			delete[] cur;
			return;
		};
		cur++;
		if (cur-buf>=100){
			*cur=0;
			cur=sum_strings(str->get(),buf,NULL);
			str->set(cur);
			delete[] cur;
			cur=buf;
		};
	};
};

int d4x_xml_read_value(int fd,tPStr *fld){
//	printf("d4x_xml_read_value!\n");
	unsigned char b[101];
	unsigned char *cur=b;
	if (fld->get()==NULL)
		fld->set("");
	while(read(fd,cur,1)==1){
		if (*cur<' ') return(-1);
		switch(*cur){
		case '\\':{
			if (read(fd,cur,1)!=1 || *cur<' ')
				return(-1);
			switch(*cur){
			case 'n':
				*cur='\n';
				break;
			case 'b':
				*cur='\b';
				break;
			case 't':
				*cur='\t';
				break;
			};
			break;
		};
		case '\"':{
			*cur=0;
			cur=(unsigned char *)sum_strings(fld->get(),b,NULL);
			fld->set((char*)cur);
			delete[] cur;
			return(0);
		};
		};
		cur++;
		if (cur-b>=100){
			*cur=0;
			cur=(unsigned char*)sum_strings(fld->get(),b,NULL);
			fld->set((char*)cur);
			delete[] cur;
			cur=b;
		};
	};
	return(-1);
};

int d4x_xml_read_fields(int fd,d4xXmlObject *obj){
//	printf("d4x_xml_read_fields <%s>!\n",obj->name.get());
	char b[2]={0,0};
	while(read(fd,b,1)==1){
		if (!isspace(*b)){
			if (*b=='/'){
				if (read(fd,b,1)==1 && *b=='>')
					return(1);
				return(-1);
			};
			if (*b=='>') return(0);
			d4xXmlField *fld=new d4xXmlField;
			fld->name.set(b);
			d4x_xml_read_grow(fd,&(fld->name),b," \n\t\r<>=");
			if (*b=='>'){
				delete(fld);
				return(0);
			};
			if (*b==0 || *b=='<'){
				delete(fld);
				return(-1);
			};
			while(*b!='='){
				if (read(fd,b,1)!=1 || (!isspace(*b) && *b!='=')){
					delete(fld);
					return(-1);
				};
			};
			if (read(fd,b,1)!=1){
				delete(fld);
				return(-1);
			};
			while(isspace(*b)){
				if (read(fd,b,1)!=1){
					delete(fld);
					return(-1);
				};
			};
			if (*b!='\"' || d4x_xml_read_value(fd,&(fld->value))){
				delete(fld);
				return(-1);
			};
			obj->fields.insert(fld);
		};
	};
	return(-1);
};

d4xXmlObject *d4x_xml_read_tag(int fd);

int d4x_xml_read_obj_body(int fd,d4xXmlObject *obj){
//	printf("d4x_xml_read_body! <%s>\n",obj->name.get());
	char b[2]={0,0};
	obj->value.set("");
	do {
		d4x_xml_read_grow(fd,&(obj->value),b,"<>");
		if (*b=='>' || *b==0) break;
		if (*b=='<'){
			d4xXmlObject *chld=d4x_xml_read_tag(fd);
			if (chld && *(chld->name.get())=='/'){
				delete(chld);
				return(0);
			};
			if (chld==NULL) return(-1);
			obj->objects.insert(chld);
		};
	}while(*b!=0);
	return(-1);
};

void d4x_xml_skip_for_str(int fd,char *str){
	int len=strlen(str);
	char *buf=new char[len+1];
	buf[len]=0;
	if (read(fd,buf,len)!=len){
		delete[] buf;
		return;
	};
	while(!equal(str,buf)){
		memmove(buf,buf+1,len-1);
		if (read(fd,buf+len-1,1)!=1) return;
	};
};

d4xXmlObject *d4x_xml_read_tag(int fd){
//	printf("d4x_xml_read_tag!\n");
	char b[2]={0,0};
	while(read(fd,b,1)==1){
		if (!isspace(*b))
			break;
		*b=0;
	};
	if (*b=='>' || *b==0) return(NULL);
	d4xXmlObject *obj=new d4xXmlObject;
	obj->name.set(b);
	d4x_xml_read_grow(fd,&(obj->name),b," \n\t\r<>/");
	if (*b=='<' || *b==0){
		delete(obj);
		return(NULL);
	};
	if (begin_string(obj->name.get(),"!--")){
		if (*b!='>' || string_ended("--",obj->name.get()))
			d4x_xml_skip_for_str(fd,"-->");
		delete(obj);
		if (d4x_xml_skip_for(fd,"<"))
			return(NULL);
		return(d4x_xml_read_tag(fd));
	};
	if (*b=='/'){
		if (read(fd,b,1)==1 && *b=='>')
			return(obj);
		delete(obj);
		return(NULL);
	};
	if (*b!='>'){
		switch(d4x_xml_read_fields(fd,obj)){
		case -1:
			delete(obj);
			return(NULL);
		case 1:
			return(obj);
		};
	};
	if (*(obj->name.get())=='/'){
		return(obj);
	};
	if (d4x_xml_read_obj_body(fd,obj)){
		delete(obj);
		return(NULL);
	};
	return(obj);
};

tQueue *d4x_xml_parse_fd(int fd){
	tQueue *rval=new tQueue;
	if (rval==NULL) return(NULL);
	if (d4x_xml_skip_for(fd,"<")){
		delete(rval);
		return(NULL);
	};
	d4xXmlObject *obj=d4x_xml_read_tag(fd);
	while(obj){
		rval->insert(obj);
		if (d4x_xml_skip_for(fd,"<"))
			break;
		obj=d4x_xml_read_tag(fd);
	};
	return(rval);
};

tQueue *d4x_xml_parse_file(const char *filename){
	int fd=open(filename,O_RDONLY);
	if (fd<0) return(NULL);
	tQueue *rval=d4x_xml_parse_fd(fd);
	close(fd);
	return(rval);
};

void d4x_xml_out(tQueue *q){
	d4xXmlObject *obj=(d4xXmlObject *)q->first();
	while(obj){
		obj->print_rec(0);
		obj=(d4xXmlObject *)(obj->prev);
	};
};

d4xXmlObject *d4x_xml_find_obj(tQueue *q,char *name){
	if (!q) return(NULL);
	char *n=copy_string(name);
	d4xXmlObject *obj=(d4xXmlObject *)q->first();
	char *space=index(n,' ');
	if (space) *space=0;
	while(obj){
		if (equal_uncase(n,obj->name.get())){
			if (space){
				*space=0;
				d4xXmlObject *rval=d4x_xml_find_obj(&(obj->objects),space+1);
				delete[] n;
				return(rval);
			};
			delete[] n;
			return(obj);
		};
		obj=(d4xXmlObject *)(obj->prev);
	};
	delete[] n;
	return(NULL);
};
