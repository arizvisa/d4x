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
	descr=name=NULL;
	fields=NULL;
};

void tHtmlTag::print(){
	printf("Name:\t%s\n",name==NULL?"*NULL*":name);
};

tHtmlTagField *tHtmlTag::find_field(const char *n){
	tHtmlTagField *fld=(tHtmlTagField *)(fields->first());
	while(fld){
		if (strcasecmp(fld->name,n)==0)
			return fld;
		fld=(tHtmlTagField *)(fields->prev());
	};
	return 0;
};

tHtmlTag::~tHtmlTag(){
	if (name) delete[] name;
	if (descr) delete[] descr;
	if (fields) delete(fields);
};
/********************************************************/
tHtmlUrl::tHtmlUrl(){
	descr=NULL;
};

void tHtmlUrl::print(){
};

tHtmlUrl::~tHtmlUrl(){
	if (descr) delete[] descr;
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

void tHtmlParser::get_tag_descr(tHtmlTag *tag){
	if (strcasecmp(tag->name,"img")==0){
		//get descr from ALT or TITLE field
		tHtmlTagField *fld=tag->find_field("alt");
		if (!fld)
			fld=tag->find_field("title");
		if (fld) tag->descr=copy_string(fld->value);
	}else if (strcasecmp(tag->name,"a")==0){
		//get descr from content of tag
		char *str=new char[51];
		char *p=str;
		while(p-str<50 && WL->read(p,sizeof(char))>0){
			if (*p=='<'){
				WL->shift(-1,SEEK_CUR);
				break;
			};
			p++;
		};
		*p=0;
		WL->shift(str-p,SEEK_CUR);
		tag->descr=str;
	};
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
					get_tag_descr(rvalue);
				};
			};
			break;
		}else{
			if (out_fd>=0) write(out_fd,&p,sizeof(p));
		};
	};
	return rvalue;
};

static void write_up_dirs(int out_fd,const char *a){
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

d4x::URL fix_url_global(char *url,const d4x::URL &papa,int out_fd,int leave,int quest_sign_replace){
	if (url==NULL || *url==0) return d4x::URL();
	d4x::URL info;
	char *html_shift=NULL;
	if (!global_url(url)) {
		html_shift=rindex(url,'#');
		if (html_shift){
			*html_shift=0;
			html_shift+=1;
		};
		char *quest=index(url,'?');
		if (quest){
			info.params=(quest+1);
			*quest=0;
		};
		if (papa.proto==D_PROTO_FTP){
			quest=index(url,';');
			if (quest)
				*quest=0;
		};
		/* %xx -> CHAR */
		char *tmp=parse_percents(url);
		quest=rindex(tmp,'/');
		if (quest) {
			info.file=(quest+1);
			*quest=0;
			if (*tmp=='/')
				info.path=(tmp+1);
			else{
				if (tmp==quest)
					info.path=("");
				else
					info.path=papa.path/std::string(tmp);
			};
			*quest='/';
		} else {
			info.path=papa.path;
			info.file=tmp;
		};
		delete[] tmp;
		info.file=info.file.substr(0,info.file.find('#'));
		info.copy_host(papa);
	}else{
		if (begin_string_uncase(url,"http:") ||
		    begin_string_uncase(url,"ftp:") ||
		    begin_string_uncase(url,"https:")){
			html_shift=rindex(url,'#');
			if (html_shift)	html_shift+=1;
			info=std::string(url);
			if (papa.proto==D_PROTO_FTP && info.proto==D_PROTO_FTP){
				info.file=info.file.substr(0,info.file.find(';'));
			};
		}else{
			if (out_fd>=0){
				f_wstr(out_fd,url);
			};
			return d4x::URL();
		};
	};
	if (out_fd>=0){
		if (info.host!=papa.host){
			if (leave){
				f_wstr(out_fd,"../");
				write_up_dirs(out_fd,papa.path.c_str());
				f_wstr(out_fd,info.host.c_str());
				f_wchar(out_fd,'/');
				f_wstr(out_fd,info.path.c_str());
				f_wchar(out_fd,'/');
				if (quest_sign_replace && info.file.c_str()[0]==0)
					f_wstr(out_fd,"index_html");
				else
					f_wstr(out_fd,info.file.c_str());
				if (!info.params.empty()){
					f_wstr(out_fd,quest_sign_replace?"%5f":"%3f");
					f_wstr(out_fd,info.params.c_str());
				};
			}else{
				f_wstr(out_fd,std::string(info).c_str());
			};
		}else{
			//FIXME: next code is too ugly :-)
			
			const char *a=papa.path.c_str(); 
			const char *b=info.path.c_str();
			if (*a=='/') a+=1;
			if (*b=='/') b+=1;
			const char *l=b;
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
			if (quest_sign_replace && info.file.empty())
				f_wstr(out_fd,"index.html");
			else
				f_wstr(out_fd,info.file.c_str());
			if (!info.params.empty()){
				f_wstr(out_fd,quest_sign_replace?"%5f":"%3f");
				f_wstr(out_fd,info.params.c_str());
			};
		};
		if (html_shift){
			f_wchar(out_fd,'#');
			f_wstr(out_fd,html_shift);
		};
	};
	return(info);
};

char *tHtmlParser::convert_to_utf8(const char *src){
	if (!src) return 0;
	if((g_utf8_validate(src, -1, NULL)) == TRUE)
		return copy_string(src);
	//to avoid different memory freing schemes
	char *tmp=g_convert(src,-1,"UTF-8",codepage.c_str(),NULL,NULL,NULL);
	char *rval=copy_string(tmp);
	g_free(tmp);
	return rval;
};

void tHtmlParser::fix_url(char *url,tQueue *list,const d4x::URL &papa,const char *tag,const char *descr){
	if (out_fd>=0) f_wstr(out_fd,"=\"");
	d4x::URL info=fix_url_global(url,papa,out_fd,leave,quest_sign_replace);
	if (out_fd>=0) f_wchar(out_fd,'\"');
	if (info.is_valid()){
		tHtmlUrl *node=new tHtmlUrl;
		info.tag=tag;
		node->info=info;
		node->descr=convert_to_utf8(descr);
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
					const d4x::URL &papa,
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

void tHtmlParser::set_content_type(const char *ct){
	//Example: text/html; charset=koi8-r
	char *a=index(ct,'=');
	if (a) codepage=a+1;
};


void tHtmlParser::get_charset_from_meta(tHtmlTagField *fld){
	if (fld && fld->value){
		char *val=extract_from_icommas(fld->value);
		set_content_type(val);
		delete [] val;
	};
};

void tHtmlParser::parse(tWriterLoger *wl,tQueue *list,const d4x::URL &papa,int qsignreplace){
	quest_sign_replace=qsignreplace;
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
							if (tmp && equal_uncase(tmp,"Content-Type"))
								get_charset_from_meta(temp->find_field("content"));
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
								fix_url(tmp,list,papa,temp->name,temp->descr);
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
