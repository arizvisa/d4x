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
#include "html.h"
#include "locstr.h"
#include "var.h"
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <stdio.h>

struct tHtmlTegTable{
	char *tag,*field;
	int mod;
};
enum HTML_FIELDS_TYPES{
	HF_TYPE_LINK,
	HF_TYPE_BASE,
	HF_TYPE_BASE_CLOSE,
	HF_TYPE_META
};
	
enum HTML_FIELDS{
	HF_PROFILE,
	HF_STYLE,
	HF_BACKGROUND,
	HF_HREF,
	HF_SRC,
	HF_USEMAP,
	HF_LONGDESC,
	HF_LOWSRC,
	HF_CONTENT,
	HF_CITE,
	HF_DATA,
	HF_HT,
	HF_CODEBASE,
	HF_ACTION,
	HF_HTTP_EQUIV
};

char *HTML_FIELDS_NAMES[]={
	"profile",
	"style",
	"background",
	"href",
	"src",
	"usemap",
	"longdesc",
	"lowsrc",
	"content",
	"cite",
	"data",
	"ht",
	"codebase",
	"action",
	"http-equiv"
};

tHtmlTegTable HTML_TEGS[]={
	{"head",	HTML_FIELDS_NAMES[HF_PROFILE],		0},

	{"body",	HTML_FIELDS_NAMES[HF_BACKGROUND],	0},
	{"body",	HTML_FIELDS_NAMES[HF_STYLE],		0},

	{"table",	HTML_FIELDS_NAMES[HF_BACKGROUND],	0},
	{"table",      	HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"th",		HTML_FIELDS_NAMES[HF_BACKGROUND],	0},
	{"th",		HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"td",		HTML_FIELDS_NAMES[HF_BACKGROUND],	0},
	{"td",		HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"col",		HTML_FIELDS_NAMES[HF_BACKGROUND],	0},
	{"col",		HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"thead",	HTML_FIELDS_NAMES[HF_BACKGROUND],	0},
	{"thead",	HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"tfoot",	HTML_FIELDS_NAMES[HF_BACKGROUND],	0},
	{"tfoot",	HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"tbody",	HTML_FIELDS_NAMES[HF_BACKGROUND],	0},
	{"tbody",	HTML_FIELDS_NAMES[HF_STYLE],		0},

	{"a",		HTML_FIELDS_NAMES[HF_HREF],		0},
	{"a",		HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"address",	HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"area",	HTML_FIELDS_NAMES[HF_HREF],		0},
	{"area",	HTML_FIELDS_NAMES[HF_STYLE],		0},

	{"img",		HTML_FIELDS_NAMES[HF_SRC],		0},
	{"img",		HTML_FIELDS_NAMES[HF_LOWSRC],		0},
	{"img",		HTML_FIELDS_NAMES[HF_LONGDESC],		0},
	{"img",		HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"img",		HTML_FIELDS_NAMES[HF_USEMAP],		0},

	{"link",	HTML_FIELDS_NAMES[HF_HREF],		0},
	{"link",	HTML_FIELDS_NAMES[HF_STYLE],		0},

	{"input",	HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"input",	HTML_FIELDS_NAMES[HF_SRC],		0},
	{"input",	HTML_FIELDS_NAMES[HF_USEMAP],		0},

	{"applet",	HTML_FIELDS_NAMES[HF_CODEBASE],		0},
	{"applet",	HTML_FIELDS_NAMES[HF_STYLE],		0},

	{"iframe",	HTML_FIELDS_NAMES[HF_LONGDESC],		0},
	{"iframe",	HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"iframe",	HTML_FIELDS_NAMES[HF_SRC],		0},
	{"frame",	HTML_FIELDS_NAMES[HF_LONGDESC],		0},
	{"frame",	HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"frame",	HTML_FIELDS_NAMES[HF_SRC],		0},

	{"sound",	HTML_FIELDS_NAMES[HF_SRC],		0},
	{"sound",	HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"bgsound",	HTML_FIELDS_NAMES[HF_SRC],		0},
	{"bgsound",	HTML_FIELDS_NAMES[HF_STYLE],		0},

	{"script",	HTML_FIELDS_NAMES[HF_SRC],		0},

	{"embed",	HTML_FIELDS_NAMES[HF_SRC],		0},
	{"embed",	HTML_FIELDS_NAMES[HF_STYLE],		0},

	{"fig",		HTML_FIELDS_NAMES[HF_SRC],		0},
	{"fig",		HTML_FIELDS_NAMES[HF_STYLE],		0},

	{"overlay",	HTML_FIELDS_NAMES[HF_SRC],		0},
	{"overlay",	HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"layer",	HTML_FIELDS_NAMES[HF_SRC],		0},
	{"layer",	HTML_FIELDS_NAMES[HF_STYLE],		0},

	{"ins",		HTML_FIELDS_NAMES[HF_CITE],		0},
	{"ins",		HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"del",		HTML_FIELDS_NAMES[HF_CITE],		0},
	{"del",		HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"q",		HTML_FIELDS_NAMES[HF_CITE],		0},
	{"q",		HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"blockqute",	HTML_FIELDS_NAMES[HF_CITE],		0},
	{"blockqute",	HTML_FIELDS_NAMES[HF_STYLE],		0},

	{"span",	HTML_FIELDS_NAMES[HF_HREF],		0},
	{"span",	HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"div",		HTML_FIELDS_NAMES[HF_HREF],		0},
	{"div",		HTML_FIELDS_NAMES[HF_STYLE],		0},

	{"object",	HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"object",	HTML_FIELDS_NAMES[HF_USEMAP],		0},
	{"object",	HTML_FIELDS_NAMES[HF_DATA],		0},

	{"center",	HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"h1",		HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"h2",		HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"h3",		HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"h4",		HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"h5",		HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"h6",		HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"hr",		HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"isindex",	HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"p",		HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"pre",		HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"noscript",	HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"dir",		HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"dl",		HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"dt",		HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"dd",		HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"li",		HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"menu",	HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"ol",		HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"ul",		HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"caption",	HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"colgroup",	HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"optgroup",	HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"form",	HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"fieldset",	HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"button",	HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"legend",	HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"label",	HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"select",	HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"option",	HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"textarea",	HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"bdo",		HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"br",		HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"font",	HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"map",		HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"sub",		HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"sup",		HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"abbr",	HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"acronym",	HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"cite",	HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"code",	HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"dfn",		HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"em",		HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"samp",	HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"strong",	HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"var",		HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"b",		HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"big",		HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"i",		HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"s",		HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"small",	HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"strike",	HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"tt",		HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"u",		HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"csobj",	HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"noframes",	HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"frameset",	HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"base",	HTML_FIELDS_NAMES[HF_HREF],		HF_TYPE_BASE},
	{"/base",	NULL,					HF_TYPE_BASE_CLOSE},
	{"meta",	HTML_FIELDS_NAMES[HF_HTTP_EQUIV],	HF_TYPE_META}
};
const int HTML_TEGS_NUM=sizeof(HTML_TEGS)/sizeof(tHtmlTegTable);

tHtmlTagField::tHtmlTagField(){
	name=value=0;
};
void tHtmlTagField::print(){
	printf("Name:\t%s\n",name==NULL?"*NULL*":name);
	printf("Value:\t%s\n",value==NULL?"*NULL*":value);
};

tHtmlTagField::~tHtmlTagField(){
	if (name) delete(name);
	if (value) delete(value);
};

tHtmlTag::tHtmlTag(){
	name=NULL;
	fields=NULL;
};

void tHtmlTag::print(){
	printf("Name:\t%s\n",name==NULL?"*NULL*":name);
};

tHtmlTag::~tHtmlTag(){
	if (name) delete(name);
	if (fields) delete(fields);
};
/* Will be parse htmls in next assumption:
   <TAG[spaces]FIELD[spaces]=[spaces]VALUEspacesFIELD[spaces]=[spaces]VALUE...[spaces]>
 */

char *tHtmlParser::get_string_back(int len,int shift){
	WL->shift(-(len+shift),SEEK_CUR);
	char *temp=new char[len+1];
	WL->read(temp,len);
	temp[len]=0;
	return temp;
};

char *tHtmlParser::get_word(){
	int i=0;
	char p;
	while(WL->read(&p,sizeof(p))>0){
		if (isspace(p) || p=='>'){
			return(get_string_back(i,1));
		};
		i++;
	};
	return NULL;
};

char *tHtmlParser::get_word(int shift){
	int i=shift;
	char p;
	while(WL->read(&p,sizeof(p))>0){
		if (isspace(p) || p=='=' || p=='>'){
			return(get_string_back(i,1));
		};
		i++;
	};
	return NULL;
};

char *tHtmlParser::get_word_icommas(){
	int i=1;
	char p;
	while(WL->read(&p,sizeof(p))>0){
		if (p=='>'){
			return(get_string_back(i,1));
		};
		i++;
		if (p=='\"'){
			return(get_string_back(i,0));
		};
	};
	return NULL;
};

char *tHtmlParser::get_word_icommas2(){
	int i=1;
	char p;
	while(WL->read(&p,sizeof(p))>0){
		if (p=='>'){
			return(get_string_back(i,1));
		};
		i++;
		if (p=='\''){
			return(get_string_back(i,0));
		};
	};
	return NULL;
};

void tHtmlParser::get_fields(tHtmlTag *tag){
	char p;
	tag->fields=new tQueue;
	tag->fields->init(0);
	while(WL->read(&p,sizeof(p)>0)){
		if (p=='>') return;
		if (!isspace(p)){
			tHtmlTagField *field=new tHtmlTagField;
			field->name=get_word(1);
			tag->fields->insert(field);
			while(WL->read(&p,sizeof(p)>0)){
				if (p=='>') return;
				if (!isspace(p) && p!='='){
					switch(p){
					case '\"':{
						field->value=get_word_icommas();
						break;
					};
					case '\'':{
						field->value=get_word_icommas2();
						break;
					};
					default:
						field->value=get_word(1);
					};
					break;
				};
			};
		};
	};
};

char *tHtmlParser::extract_from_icommas(char *str){
	if (str==NULL) return NULL;
	char *temp=str;
	char *end=NULL;
	switch (*temp){
	case '\'':{
		temp+=1;
		end=index(temp,'\'');
		break;
	};
	case '\"':{
		temp+=1;
		end=index(temp,'\"');
		break;
	};
	};
	if (end)
		return(copy_string(temp,end-temp));
	return(copy_string(temp));
};

tHtmlTag *tHtmlParser::get_tag(){
	char p;
	tHtmlTag *rvalue=NULL;
	while(WL->read(&p,sizeof(p))>0){
		if (p=='<'){
			char *name=NULL;
			if ((name=get_word())){
				rvalue=new tHtmlTag;
				rvalue->name=name;
				if (name && equal(name,"!--")){
					while(WL->read(&p,sizeof(p))>0){
						if (p=='>') break;
					};
				}else
					get_fields(rvalue);
			};
			break;
		};
	};
	return rvalue;
};

void tHtmlParser::look_for_meta_content(tHtmlTagField *where,tStringList *list){
	tHtmlTagField *field=(tHtmlTagField *)(where->prev);
	while (field){
		char *tmp=NULL;
		if (field->name && field->value &&
		    equal_uncase(field->name,HTML_FIELDS_NAMES[HF_CONTENT]))
			tmp=extract_from_icommas(field->value);
		if (tmp){
			char *url=strstr(tmp,"url=");
			if (url)
				list->add(url+=4);
			delete(tmp);
		};
		field=(tHtmlTagField *)(field->prev);
	};
};

void tHtmlParser::parse(tWriterLoger *wl,tStringList *list){
	list->done();
	list->init(0);
	base=NULL;
	WL=wl;
	WL->shift(0);
	while(1){
		tHtmlTag *temp=get_tag();
		if (temp==NULL) return;
/*
		printf("Tag:\n");
		temp->print();
		if (temp->fields){
			printf("Fields:\n");
			tHtmlTagField *field=(tHtmlTagField *)(temp->fields->first());
			while(field){
				field->print();
				field=(tHtmlTagField *)(temp->fields->prev());
			};
			printf("EndFields:\n");
		};
		printf("EndTag:\n");
*/
		for (int i=0;i<HTML_TEGS_NUM;i++)
			if(equal_uncase(HTML_TEGS[i].tag,temp->name) &&
			   temp->fields){
				tHtmlTagField *field=(tHtmlTagField *)(temp->fields->first());
				while(field){
					if (field->value && equal_uncase(field->name,HTML_TEGS[i].field)){
						char *tmp=extract_from_icommas(field->value);
						switch(HTML_TEGS[i].mod){
						case HF_TYPE_META:
							if (tmp && equal_uncase(tmp,"refresh"))
								look_for_meta_content(field,list);
							break;
						case HF_TYPE_BASE:
							if (base) delete(base);
							base=copy_string(tmp);
							break;
						default:
							if (base!=NULL && !global_url(tmp)){
								char *tmp1=tmp;
								tmp=compose_path(base,tmp1);
								delete(tmp1);
							};
							if (strlen(tmp)<MAX_LEN)
								list->add(tmp);
						};
						delete(tmp);
					}else{
						if (HTML_TEGS[i].mod==HF_TYPE_BASE_CLOSE){
							if (base) delete(base);
							base=NULL;
						};
					};
					field=(tHtmlTagField *)(temp->fields->prev());
				};
			};
		delete(temp);
	};
	if (base) delete(base);
};
