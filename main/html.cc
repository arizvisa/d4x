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
//	{"body",	HTML_FIELDS_NAMES[HF_STYLE],		0},

	{"table",	HTML_FIELDS_NAMES[HF_BACKGROUND],	0},
//	{"table",      	HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"th",		HTML_FIELDS_NAMES[HF_BACKGROUND],	0},
//	{"th",		HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"td",		HTML_FIELDS_NAMES[HF_BACKGROUND],	0},
//	{"td",		HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"col",		HTML_FIELDS_NAMES[HF_BACKGROUND],	0},
//	{"col",		HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"thead",	HTML_FIELDS_NAMES[HF_BACKGROUND],	0},
//	{"thead",	HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"tfoot",	HTML_FIELDS_NAMES[HF_BACKGROUND],	0},
//	{"tfoot",	HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"tbody",	HTML_FIELDS_NAMES[HF_BACKGROUND],	0},
//	{"tbody",	HTML_FIELDS_NAMES[HF_STYLE],		0},

	{"a",		HTML_FIELDS_NAMES[HF_HREF],		0},
//	{"a",		HTML_FIELDS_NAMES[HF_STYLE],		0},
//	{"address",	HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"area",	HTML_FIELDS_NAMES[HF_HREF],		0},
//	{"area",	HTML_FIELDS_NAMES[HF_STYLE],		0},

	{"img",		HTML_FIELDS_NAMES[HF_SRC],		0},
	{"img",		HTML_FIELDS_NAMES[HF_LOWSRC],		0},
	{"img",		HTML_FIELDS_NAMES[HF_LONGDESC],		0},
//	{"img",		HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"img",		HTML_FIELDS_NAMES[HF_USEMAP],		0},

	{"link",	HTML_FIELDS_NAMES[HF_HREF],		0},
//	{"link",	HTML_FIELDS_NAMES[HF_STYLE],		0},

//	{"input",	HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"input",	HTML_FIELDS_NAMES[HF_SRC],		0},
	{"input",	HTML_FIELDS_NAMES[HF_USEMAP],		0},

	{"applet",	HTML_FIELDS_NAMES[HF_CODEBASE],		0},
//	{"applet",	HTML_FIELDS_NAMES[HF_STYLE],		0},

	{"iframe",	HTML_FIELDS_NAMES[HF_LONGDESC],		0},
//	{"iframe",	HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"iframe",	HTML_FIELDS_NAMES[HF_SRC],		0},
	{"frame",	HTML_FIELDS_NAMES[HF_LONGDESC],		0},
//	{"frame",	HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"frame",	HTML_FIELDS_NAMES[HF_SRC],		0},

	{"sound",	HTML_FIELDS_NAMES[HF_SRC],		0},
//	{"sound",	HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"bgsound",	HTML_FIELDS_NAMES[HF_SRC],		0},
//	{"bgsound",	HTML_FIELDS_NAMES[HF_STYLE],		0},

	{"script",	HTML_FIELDS_NAMES[HF_SRC],		0},

	{"embed",	HTML_FIELDS_NAMES[HF_SRC],		0},
//	{"embed",	HTML_FIELDS_NAMES[HF_STYLE],		0},

	{"fig",		HTML_FIELDS_NAMES[HF_SRC],		0},
//	{"fig",		HTML_FIELDS_NAMES[HF_STYLE],		0},

	{"overlay",	HTML_FIELDS_NAMES[HF_SRC],		0},
//	{"overlay",	HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"layer",	HTML_FIELDS_NAMES[HF_SRC],		0},
//	{"layer",	HTML_FIELDS_NAMES[HF_STYLE],		0},

	{"ins",		HTML_FIELDS_NAMES[HF_CITE],		0},
//	{"ins",		HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"del",		HTML_FIELDS_NAMES[HF_CITE],		0},
//	{"del",		HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"q",		HTML_FIELDS_NAMES[HF_CITE],		0},
//	{"q",		HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"blockqute",	HTML_FIELDS_NAMES[HF_CITE],		0},
//	{"blockqute",	HTML_FIELDS_NAMES[HF_STYLE],		0},

	{"span",	HTML_FIELDS_NAMES[HF_HREF],		0},
//	{"span",	HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"div",		HTML_FIELDS_NAMES[HF_HREF],		0},
//	{"div",		HTML_FIELDS_NAMES[HF_STYLE],		0},

//	{"object",	HTML_FIELDS_NAMES[HF_STYLE],		0},
	{"object",	HTML_FIELDS_NAMES[HF_USEMAP],		0},
	{"object",	HTML_FIELDS_NAMES[HF_DATA],		0},
/*
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
*/
	{"base",	HTML_FIELDS_NAMES[HF_HREF],		HF_TYPE_BASE},
	{"/base",	NULL,					HF_TYPE_BASE_CLOSE},
	{"meta",	HTML_FIELDS_NAMES[HF_HTTP_EQUIV],	HF_TYPE_META}
};
const int HTML_TEGS_NUM=sizeof(HTML_TEGS)/sizeof(tHtmlTegTable);

tHtmlTagField::tHtmlTagField(){
	name=value=NULL;
	saved=0;
};
void tHtmlTagField::print(){
	printf("Name:\t%s\n",name==NULL?"*NULL*":name);
	printf("Value:\t%s\n",value==NULL?"*NULL*":value);
};

tHtmlTagField::~tHtmlTagField(){
	if (name) delete[] name;
	if (value) delete[] value;
};

tHtmlTag::tHtmlTag(){
	name=NULL;
	fields=NULL;
};

void tHtmlTag::print(){
	printf("Name:\t%s\n",name==NULL?"*NULL*":name);
};

tHtmlTag::~tHtmlTag(){
	if (name) delete[] name;
	if (fields) delete(fields);
};
/********************************************************/
tHtmlUrl::tHtmlUrl(){
	info=NULL;
};

void tHtmlUrl::print(){
};

tHtmlUrl::~tHtmlUrl(){
	if (info) delete(info);
};


/* Will be parse htmls in next assumption:
   <TAG[spaces]FIELD[spaces][=[spaces]VALUE]spacesFIELD[spaces][=[spaces]VALUE]...[spaces]>
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

char *tHtmlParser::get_word_o(int shift){
	int i=shift;
	char p;
	while(WL->read(&p,sizeof(p))>0){
		if (isspace(p) ||  p=='>'){
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
		if (isspace(p) || p=='=' ||  p=='>'){
			return(get_string_back(i,1));
		};
		i++;
	};
	return NULL;
};

/* compact_string(char *str)
    before exec:    str="\naa\nbb\ncc"
    after execution:    str="aabbcc"
 */
void tHtmlParser::compact_string(char *str){
	char *a=str,*b;
	while (*a){
		if (*a=='\n' || *a=='\r')
			break;
		a+=1;
	};
	b=a;
	while (*a){
		*a=*b;
		if (*a!='\n' && *a!='\r')
			b+=1;
		a+=1;
	};
	*b=0;
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
			int eqsign=0;
			while(WL->read(&p,sizeof(p)>0)){
				if (p=='>') return;
				if (p=='=') eqsign=1;
				if (!isspace(p) && p!='='){
					if (!eqsign){
						WL->shift(-1,SEEK_CUR);
						break;
					};
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
						field->value=get_word_o(1);
					};
					if (field->value)
						compact_string(field->value);
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
		return(copy_string2(temp,end-temp));
	return(copy_string(temp));
};

tHtmlTag *tHtmlParser::get_tag(){
	char p;
	tHtmlTag *rvalue=NULL;
	while(WL->read(&p,sizeof(p))>0){
		if (p=='<'){
			char *name=NULL;
			if ((name=get_word())){
				if (name && equal(name,"!--")){
					if (out_fd>=0){
						f_wstr(out_fd,"<!--");
					};
					while(WL->read(&p,sizeof(p))>0){
						if (out_fd>=0)
							write(out_fd,&p,sizeof(p));
						if (p=='>') break;
					};
					rvalue=get_tag();
					delete[] name;
				}else{
					rvalue=new tHtmlTag;
					rvalue->name=name;
					get_fields(rvalue);
				};
			};
			break;
		}else{
			if (out_fd>=0) write(out_fd,&p,sizeof(p));
		};
	};
	return rvalue;
};

static void write_up_dirs(int out_fd,char *a){
	int depth=0;
	while(*a){
		depth=1;
		if(*a=='/'){
			f_wstr(out_fd,"../");
		};
		a++;
	};
	if (depth){
		f_wstr(out_fd,"../");
	};
};

tAddr *fix_url_global(char *url,tAddr *papa,int out_fd,int leave){
	if (url==NULL || *url==0) return(NULL);
	tAddr *info=NULL;
	char *html_shift=NULL;
	if (!global_url(url)) {
		info=new tAddr;
		html_shift=rindex(url,'#');
		if (html_shift){
			*html_shift=0;
			html_shift+=1;
		};
		char *quest=index(url,'?');
		if (quest){
			info->params.set(quest+1);
			*quest=0;
		};
		if (papa->proto==D_PROTO_FTP){
			quest=index(url,';');
			if (quest)
				*quest=0;
		};
		/* %xx -> CHAR */
		char *tmp=parse_percents(url);
		quest=rindex(tmp,'/');
		if (quest) {
			info->file.set(quest+1);
			*quest=0;
			if (*tmp=='/')
				info->path.set(tmp+1);
			else{
				if (tmp==quest)
					info->path.set("");
				else
					info->compose_path(papa->path.get(),tmp);
			};
			*quest='/';
		} else {
			info->path.set(papa->path.get());
			info->file.set(tmp);
		};
		delete[] tmp;
		info->file_del_sq();
		info->copy_host(papa);
	}else{
		if (begin_string_uncase(url,"http:") ||
		    begin_string_uncase(url,"ftp:") ||
		    begin_string_uncase(url,"https:")){
			html_shift=rindex(url,'#');
			if (html_shift)	html_shift+=1;
			info=new tAddr(url);
			if (papa->proto==D_PROTO_FTP && info->proto==D_PROTO_FTP){
				char *quest=index(info->file.get(),';');
				if (quest)
					*quest=0;
			};
		}else{
			if (out_fd>=0){
				f_wstr(out_fd,url);
			};
			return(NULL);
		};
	};
	if (out_fd>=0){
		if (!equal(info->host.get(),papa->host.get())){
			if (leave){
				f_wstr(out_fd,"../");
				write_up_dirs(out_fd,papa->path.get());
				f_wstr(out_fd,info->host.get());
				f_wchar(out_fd,'/');
				f_wstr(out_fd,info->path.get());
				f_wchar(out_fd,'/');
				f_wstr(out_fd,info->file.get());
				if (info->params.get()){
					f_wstr(out_fd,"%3f");
					f_wstr(out_fd,info->params.get());
				};
			}else{
				char *url=info->url();
				f_wstr(out_fd,url);
				delete[] url;
			};
		}else{
			char *a=papa->path.get();
			char *b=info->path.get();
			if (*a=='/') a+=1;
			if (*b=='/') b+=1;
			char *l=b;
			while(*a){
				if (*a==*b){
					if (*b=='/')
						l=b+1;
					a++;
					b++;
				}else
					break;
			};
			if (*a==*b)
				l=b;
			else
				if (*a==0 && *b=='/')
					l=b+1;
			write_up_dirs(out_fd,a);
			if (*l){
				f_wstr(out_fd,l);
				f_wchar(out_fd,'/');
			};
			f_wstr(out_fd,info->file.get());
			if (info->params.get()){
				f_wstr(out_fd,"%3f");
				f_wstr(out_fd,info->params.get());
			};
		};
		if (html_shift){
			f_wchar(out_fd,'#');
			f_wstr(out_fd,html_shift);
		};
	};
	return(info);
};

void tHtmlParser::fix_url(char *url,tQueue *list,tAddr *papa,const char *tag){
	if (out_fd>=0) f_wstr(out_fd,"=\"");
	tAddr *info=fix_url_global(url,papa,out_fd,leave);
	if (out_fd>=0) f_wchar(out_fd,'\"');
	if (info){
		tHtmlUrl *node=new tHtmlUrl;
		info->tag.set(tag);
		node->info=info;
		list->insert(node);
	};
};

void tHtmlParser::write_left_fields(tHtmlTag *tag){
	if (out_fd<0) return;
	if (tag->fields){
		tHtmlTagField *field=(tHtmlTagField *)(tag->fields->first());
		while(field){
			if (field->saved==0 && field->name){
				f_wchar(out_fd,' ');
				f_wstr(out_fd,field->name);
				if (field->value){
					f_wchar(out_fd,'=');
					f_wstr(out_fd,field->value);
				};
			};
			field=(tHtmlTagField *)(tag->fields->prev());
		};
	};
	f_wchar(out_fd,'>');
};

void tHtmlParser::look_for_meta_content(tHtmlTagField *where,
					tQueue *list,
					tAddr *papa,
					const char *tag){
	tHtmlTagField *field=(tHtmlTagField *)(where->prev);
	while (field){
		char *tmp=NULL;
		if (field->name && field->value &&
		    equal_uncase(field->name,HTML_FIELDS_NAMES[HF_CONTENT]))
			tmp=extract_from_icommas(field->value);
		if (tmp){
			/* parsing Refresh in form:
			   "  <int> <;> <url=> URL"
			 */
			char *url=skip_spaces(tmp);
			if (isdigit(*url)) url=index(url,';');
			if (url){
				if (*url==';') url+=1;
				url=skip_spaces(url);
				if (begin_string_uncase(url,"url")){
					url=index(url,'=');
					if (url) url+=1;
				};
				if (url){
					url=skip_spaces(url);
					fix_url(url,list,papa,tag);
				};
			};
			delete[] tmp;
		};
		field=(tHtmlTagField *)(field->prev);
	};
};

void tHtmlParser::parse(tWriterLoger *wl,tQueue *list,tAddr *papa){
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
		if (out_fd>=0){
			f_wchar(out_fd,'<');
			f_wstr(out_fd,temp->name);
		};
//		f_wchar(out_fd,' ');
		for (int i=0;i<HTML_TEGS_NUM;i++)
			if(equal_uncase(HTML_TEGS[i].tag,temp->name) &&
			   temp->fields){
				tHtmlTagField *field=(tHtmlTagField *)(temp->fields->first());
				while(field){
					if (field->value && equal_uncase(field->name,HTML_TEGS[i].field)){
						field->saved=1;
						char *tmp=extract_from_icommas(field->value);
						switch(HTML_TEGS[i].mod){
						case HF_TYPE_META:
							if (tmp && equal_uncase(tmp,"refresh"))
								look_for_meta_content(field,list,papa,temp->name);
							break;
						case HF_TYPE_BASE:
							if (base) delete[] base;
							base=copy_string(tmp);
							if (base){
								char *slash=rindex(base,'/');
								if (slash) *slash=0;
							};
							break;
						default:
							if (base!=NULL && !global_url(tmp)){
								char *tmp1=tmp;
								tmp=compose_path(base,tmp1);
								delete[] tmp1;
							};
							if (strlen(tmp)<MAX_LEN){
								if (out_fd>=0){
									f_wchar(out_fd,' ');
									f_wstr(out_fd,field->name);
								};
								fix_url(tmp,list,papa,temp->name);
							};
						};
						delete[] tmp;
					}else{
						if (HTML_TEGS[i].mod==HF_TYPE_BASE_CLOSE){
							field->saved=1;
							if (base) delete[] base;
							base=NULL;
						};
					};
					field=(tHtmlTagField *)(temp->fields->prev());
				};
			};
		write_left_fields(temp);
		delete(temp);
	};
	if (base) delete[] base;
};
