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

#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <time.h>
#include <glib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include "var.h"
#include "dbc.h"

/*  char *copy_string();
    return the same string allocated in new area;
 */

char *copy_string2(const char *src,int len) {
	if (src==NULL) return NULL;
	char *temp=new char[len+1];
	if (temp){
		strncpy(temp,src,len);
		temp[len]=0;
	};
	return temp;
};

char *copy_string(const char *src) {
	if (src==NULL) return NULL;
	return copy_string2(src,strlen(src));
};

/*  int equal(const char, const char)
    params: two strings
    return: zero if a!=b and nonzero if a==b
 */

int equal(const char *a,const char *b) {
	if (!a) {
		if (b) return 0;
		return 1;
	};
	if (!b) return 0;
	while (*a==*b && *a!=0) {
		a++;
		b++;
	};
	if (*b==*a) return 1;
	return 0;
};

/* the same as previos one but case insensetive
 */

int equal_uncase(const char *a,const char *b) {
	if (!a) {
		if (b) return 0;
		return 1;
	};
	if (!b) return 0;
	if (strcasecmp(a,b)) return 0;
	return 1;
};

/* int equal_first()
    params: two strings
    return: non zero if first string is begining of second one
	    else return value is zero;
 */

int equal_first(const char *a,const char *b) {
	DBC_RETVAL_IF_FAIL(a!=NULL,0);
	DBC_RETVAL_IF_FAIL(b!=NULL,0);
	int la=strlen(a);
	int lb=strlen(b);
	if (la==0 || lb==0) return 0;
	int min=la<lb?la:lb;
	return !strncmp(a,b,min);
};

int equal_first_uncase(const char *a,const char *b) {
	DBC_RETVAL_IF_FAIL(a!=NULL,0);
	DBC_RETVAL_IF_FAIL(b!=NULL,0);
	int la=strlen(a);
	int lb=strlen(b);
	if (la==0 || lb==0) return 0;
	int min=la<lb?la:lb;
	return !strncasecmp(a,b,min);
};


/* int begin_string()
    params: char *str, char *begin
    return: zero if `begin' is not begining of `str'
	    non zero if `begin' is begining of `str'
 */

int begin_string(const char *str,const char *begin) {
	DBC_RETVAL_IF_FAIL(str!=NULL,0);
	DBC_RETVAL_IF_FAIL(begin!=NULL,0);
	if (equal_first(str,begin) && strlen(str)>=strlen(begin)) return 1;
	return 0;
};

int begin_string_uncase(const char *str,const char *begin) {
	DBC_RETVAL_IF_FAIL(str!=NULL,0);
	DBC_RETVAL_IF_FAIL(begin!=NULL,0);
	if (equal_first_uncase(str,begin) && strlen(str)>=strlen(begin)) return 1;
	return 0;
};

/* char *sum_strings()
    params: list if strings ended by NULL;
    return: cotatenated string
 */
 
char *sum_strings(const char *a,...){
	DBC_RETVAL_IF_FAIL(a!=NULL,NULL);
	va_list args;

	int l=strlen(a)+1;
	va_start(args,a);
	char *s=va_arg(args, char*);
	while(s){
		l+=strlen(s);
		s=va_arg(args, char*);
	};

	char *r=new char[l];
	if (r){
		*r=0;
		strcat(r,a);
		va_start(args,a);
		s=va_arg(args, char*);
		while(s){
			strcat(r,s);
			s=va_arg(args, char*);
		};
	};
	return r;
};

char *compose_strings_array(int *len,const char *a,int la,const char *b){
	DBC_RETVAL_IF_FAIL(a!=NULL,NULL);
	int lb=(b?strlen(b)+1:0);
	char *r=new char[la+lb];
	memcpy(r,a,la);
	if (b)
		memcpy(r+la,b,lb);
	*len=la+lb;
	return r;
};


// next function reallocate memory for string and return new size
int reallocate_string(char **what, int len){
	int newlen=len+MAX_LEN+1;
	char *temp=new char[newlen];
	strncpy(temp,*what,len);
	temp[len+1]=0;
	delete *what;
	*what=temp;
	return newlen;
};

/* int empty_string();
    params: string
    return: non zero if string contains only spaces
	    zero if not only spaces
 */

int empty_string(char *a) {
	DBC_RETVAL_IF_FAIL(a!=NULL,0);
	int len=strlen(a);
	for (int i=0;i<len;i++,a++) {
		if (!isspace(*a))
			return 0;
	};
	return 1;
};

/* str_replace();
   params: string, string to search, string to put on the place of second string
   return: new allocated string
   action: replace all occurrences of the second parameter in the first paramter by the third parameter
 */

char *str_replace(const char *str,const char *where,const char *what){
	char *a=NULL;
	int len1=strlen(where);
	int len2=strlen(what);
	int len3=strlen(str);
	char *rval=copy_string(str);
	char *cur=rval;
	while ((a=strstr(cur,where))){
		int len=len3-len1+len2;
		char *old=rval;
		rval=new char[len+1];
		int l=a-old;
		memcpy(rval,old,l);
		memcpy(rval+l,what,len2);
		cur=rval+l+len2;
		memcpy(rval+l+len2,a+len1,len3-l-len1);
		len3+=len2-len1;
		rval[len3]=0;
		delete[]old;
	};
	return rval;
};


/* convert_int_to_2()
    params: integer and pointer to buffer
    return: none
    action: convert integer to hexademal pair of chars
 */

template <typename Integral>
inline std::string format_int_xx(Integral  what) {
	if (what>=10)
		return boost::lexical_cast<std::string>(what);
	return std::string("0")+boost::lexical_cast<std::string>(what);
};

/* convert_time();
    params: number of seconds, pointer to buffer
    return: none
    action: convert number of seconds to nice string HH:MM:SS
 */

std::string convert_time(int what,int TIME_FORMAT) {
	int hours = what/int(3600);
	int mins = (what%3600)/int(60);
	if (!TIME_FORMAT) {
		int secs = what%60;
		return format_int_xx(hours)+":"+format_int_xx(mins)+":"+format_int_xx(secs);
	};
	return format_int_xx(hours)+":"+format_int_xx(mins);
};

/* string_to_low();
    params: string;
    return: none;
    action: convert string to low case;
 */

void string_to_low(char *what) {
	DBC_RETURN_IF_FAIL(what!=NULL);
	while (*what) {
		if (*what>='A' && *what<='Z')
			*what+='a'-'A';
		what+=1;
	};
};

/* the same as previos but till delimeter caracter
 */

void string_to_low(char *what,char delim) {
	DBC_RETURN_IF_FAIL(what!=NULL);
	char *temp=index(what,delim);
	while (*what && what!=temp) {
		if (*what>='A' && *what<='Z')
			*what+='a'-'A';
		what+=1;
	};
};

char *index_mult(char *str,const char *what){
	DBC_RETVAL_IF_FAIL(str!=NULL,NULL);
	DBC_RETVAL_IF_FAIL(what!=NULL,NULL);
	while (*str){
		char *a=index(what,*str);
		if (a) return(str);
		str++;
	};
	return(NULL);
};

/* convert_from_hex()
    params: char
    return: integer equal to value in hexademal notation
	    of specifyed character
 */

int convert_from_hex(unsigned char what) {
	if (what>='0' && what<='9')
		return(what-'0');
	if (what>='a' && what<='f')
		return (10+what-'a');
	if (what>='A' && what<='F')
		return (10+what-'A');
	return -1;
};

int convert_from_hex(char what) {
	if (what>='0' && what<='9')
		return(int(what)-'0');
	if (what>='a' && what<='f')
		return (10+int(what)-'a');
	if (what>='A' && what<='F')
		return (10+int(what)-'A');
	return -1;
};

int convert_from_hex(char a,char b) {
	int aa=convert_from_hex(a);
	int bb=convert_from_hex(b);
	if (aa<0 || bb<0) return 1000;
	return (aa<<4)|bb;
};

/* convert_to_hex();
    params: char, pointer to buffer
    return: none
    action: coverts character to hexademal string (e.g. ' ' -> "20")
 */

static const char _dec_to_hex_[16]={
	'0','1','2','3','4','5','6','7','8','9',
	'A','B','C','D','E','F'
};

void convert_to_hex(unsigned char what,char *where) {
	DBC_RETURN_IF_FAIL(where!=NULL);
	*where = _dec_to_hex_[what/16];
	where[1] = _dec_to_hex_[what%16];
};

void convert_to_hex_s(char what,char *where) {
	unsigned char b=*((unsigned char*)&what);
	convert_to_hex(b,where);
};

/* parse_percents();
    params: string
    return: new allocated string in which all %xx codes from input string
	    are replaced.
 */

char *parse_percents(char *what) {
	DBC_RETVAL_IF_FAIL(what!=NULL,NULL);
	/* Next string is too ugly because I use "new" in separate threads;
	 */
	char *temp=new char[strlen(what)+1];
	if (temp==NULL) return(NULL);
	char *where=temp,*old=what;
	while (*old) {
		switch (*old){
		case '%':{
			int num=convert_from_hex(old[1]);
			if (num>=0 && old[2]) {
				int num1=convert_from_hex(old[2]);
				if (num1>=0){
					num=num *16 + num1;
					*where=char(num);
					old+=2;
					break;
				};
			};
		};
		default:{
			*where=*old;
		};
		};
		where+=1;
		old+=1;		      
	};
	*where=0;
	return temp;
};

/* unparse_percents();
    params: string;
    return: allocated string in which special simbols from input are
	    replaced by %xx;
 */

char *unparse_percents(char *what) {
	DBC_RETVAL_IF_FAIL(what!=NULL,NULL);
	const char *true_chars=".-+%";
	const char *false_chars=":<>~ ";
	char *temp=what;
	int unparsed_len=0;

	while (*temp){
		if ((*temp<'/' || *temp>'z' || index(false_chars,*temp)!=NULL) && index(true_chars,*temp)==NULL)
			unparsed_len+=3;
		else
			unparsed_len+=1;
		temp+=1;
	};
	char *rvalue=new char[unparsed_len+1];
	if (rvalue){
		char *cur=rvalue;
		temp=what;
		while (*temp){
			if ((*temp<'/' || *temp>'z'|| index(false_chars,*temp)!=NULL) && index(true_chars,*temp)==NULL){
				*cur='%';
				cur+=1;
				convert_to_hex(*temp,cur);
				cur+=1;
			}else
				*cur=*temp;
			cur+=1;
			temp+=1;
		};
		*cur=0;
	};
	return rvalue;
};

static const char *_url_true_chars_=".-+&";
static const char *_url_false_chars_=":<>~% ";

#include <iostream>
std::string hexed_string(const std::string &str){
	std::string rval(str);
	std::string::size_type len=rval.length(),pos=0;
	char buf[4]={'%',0,0,0};
	while(pos<len){
		char r=rval[pos];
		if ((r<'/' || r>'z' || index(_url_false_chars_,r))
		    && index(_url_true_chars_,r)==0){
			convert_to_hex_s(r,buf+1);
			rval.replace(pos,1,buf);
			pos+=2;
			len+=2;
		};
		pos++;
	};
	return rval;
};

std::string unhexed_string(const std::string &str){
	std::string::size_type pos=0;
	std::string::size_type len=str.length();
	std::string rval(str);
	while((pos=rval.find('%',pos))!=std::string::npos){
		if (len<pos+1) break;
		if (len>=pos+2){
			int b=convert_from_hex(rval[pos+1],rval[pos+2]);
			if (b<256){
				char buf[2]={b,0};
				rval.replace(pos,3,buf);
				len-=2;
			};
		};
		pos++;
	};
	return rval;
};

/* escape_char();
    params: string, char to search, escaped character;
    return: string
    action: allocate new string with char `what' escaped
	    by char `bywhat'
 */

char *escape_char(const char *where,char what,char bywhat){
	DBC_RETVAL_IF_FAIL(where!=NULL,NULL);
	int num=0;
	char *tmp=index(where,what);
	while(tmp){
		num+=1;
		tmp=index(tmp+1,what);
	};
	if (num){
		char *rvalue=new char[strlen(where)+num*2+1];
		*rvalue=0;
		char *r=rvalue;
		tmp=index(where,what);
		while(tmp){
			if (tmp-where)
				memcpy(r,where,tmp-where);
			r+=tmp-where;
			r[0]=bywhat;
			r[1]=what;
			r+=2;
			where=tmp+1;
			tmp=index(where,what);
		};
		*r=0;
		if (*where)
			memcpy(r,where,strlen(where)+1);
		return rvalue;
	};
	return(copy_string(where));
};

/* str_non_print_replace();
    params: string, char
    return: none
    action: replace all character<' ' in string `what' by `symbol'
 */

void str_non_print_replace(char *what,char symbol){
	DBC_RETURN_IF_FAIL(what!=NULL);
	unsigned char *temp=(unsigned char *)what;
	while (*temp){
		if (*temp<' ') *temp=(unsigned char )symbol;
		temp+=1;
	};
	while (temp > (unsigned char *)what){
		temp--;
		if (*temp==' ') *temp=0;
		else break;
	};
};

/*  del_crlf()
    params: string
    return: none
    action: cut string to '\n' or '\r'
 */

void del_crlf(char *what) {
	DBC_RETURN_IF_FAIL(what!=NULL);
	char *tmp;
	while((tmp=rindex(what,'\n')) || (tmp=rindex(what,'\r'))) {
		*tmp=0;
	};
};


/* is_string()
    params: string
    return: non zero if there are not only digits in string
	    zero if there are only digits in string
 */

int is_string(char *what){
	DBC_RETVAL_IF_FAIL(what!=NULL,0);
	char *current=what;
	while (*current){
		if (*current<'0' || *current>'9') return 1;
		current+=1;
	};
	return 0;
};

/*
 * This function extract string from string with spaces
 * for example:
 * given the string '  aa  blala'
 * extracted value will be 'aa'
 */

char *my_space_locate(char *what){
	DBC_RETVAL_IF_FAIL(what!=NULL,NULL);
	char *tmp=what;
	while (*tmp){
		if (isspace(*tmp)) return(tmp);
		tmp+=1;
	};
	return NULL;
};

char *extract_string(char *src,char *dst) {
	DBC_RETVAL_IF_FAIL(src!=NULL,NULL);
	DBC_RETVAL_IF_FAIL(dst!=NULL,NULL);
	char *tmp=src;
	while (*tmp && isspace(*tmp)) tmp++;
	char *space=my_space_locate(tmp);
	while (*tmp && tmp!=space) {
		*dst=*tmp;
		dst++;
		tmp++;
	};
	*dst=0;
	return tmp;
};

char *extract_string(char *src,char *dst,int num){
	DBC_RETVAL_IF_FAIL(src!=NULL,NULL);
	DBC_RETVAL_IF_FAIL(dst!=NULL,NULL);
	char *new_src=src;
	for (int i=0;i<num;i++){
		new_src=extract_string(new_src,dst);
	};
	return(new_src);
};

char *extract_from_prefixed_string(char *str,char *begin){
	DBC_RETVAL_IF_FAIL(str!=NULL,NULL);
	DBC_RETVAL_IF_FAIL(begin!=NULL,NULL);
	char *tmp=str+strlen(begin);
	while (isspace(*tmp)) tmp+=1;
	char *rvalue=copy_string(tmp);
	return rvalue;
};

/* skip_spaces();
    params: poiter to char
    return: pointer to first non space char
 */

char *skip_spaces(char *src){
	DBC_RETVAL_IF_FAIL(src!=NULL,NULL);
	char *tmp=src;
	while(*tmp && isspace(*tmp))
		tmp+=1;
	return tmp;
};

/* skip_strings()
    params: pointer to char, number of substring to skip
    return: pointer to char after last skipped string;
 */

char *skip_strings(char *src,int num){
	DBC_RETVAL_IF_FAIL(src!=NULL,NULL);
	char *tmp=src;
	for (int i=0;i<num;i++) {
		char *tmp1=my_space_locate(tmp);
		if (tmp1) while (isspace(*tmp1)) tmp1++;
		else break;
		tmp=tmp1;
	};
	return tmp;
};

/* convert_month()
    params: pointer to char
    return: number of month beging from 0;
 */

int convert_month(char *src) {
	DBC_RETVAL_IF_FAIL(src!=NULL,0);
	switch(*src) {
	case 'j':
	case 'J':{
		if (src[1]=='a' || src[1]=='A') return 0;
		if (src[2]=='n' || src[2]=='N') return 5;
		return 6;
	};
	case 'a':
	case 'A':{
		if (src[1]=='u' || src[1]=='U') return 7;
		return 3;
	};
	case 'm':
	case 'M':{
		if (src[2]=='r' || src[2]=='R') return 2;
		return 4;
	};
	case 'f':
	case 'F':
		return 1;
	case 's':
	case 'S':
		return 8;
	case 'o':
	case 'O':
		return 9;
	case 'n':
	case 'N':
		return 10;
	case 'd':
	case 'D':
		return 11;
	};
	return 0;
};

/* ctime_to_time();
    params: string
    return: time
    action: convert string in ctime() format to time_t
 */

int ctime_to_time(char *src) {
	DBC_RETVAL_IF_FAIL(src!=NULL,0);
	char data[MAX_LEN];
	char *tmp=extract_string(src,data);
	time_t NOW=time(NULL);
	struct tm date;
	localtime_r(&NOW,&date);
	date.tm_isdst=-1;
	if (tmp && *tmp) {
		tmp=extract_string(tmp,data);
		sscanf(data,"%i",&date.tm_mday);
	};
	if (tmp && *tmp) {
		tmp=extract_string(tmp,data);
		date.tm_mon=convert_month(data);
	};
	if (tmp && *tmp) {
		tmp=extract_string(tmp,data);
		sscanf(data,"%i",&date.tm_year);
		date.tm_year-=1900;
	};
	if (tmp && *tmp) {
		extract_string(tmp,data);
		if (index(data,':')) {
			char *tmpdata=data;
			sscanf_int(tmpdata,&(date.tm_hour));
			tmpdata+=3;
			sscanf_int(tmpdata,&(date.tm_min));
			tmpdata+=3;
			sscanf_int(tmpdata,&(date.tm_sec));
		};
	};
#ifdef HAVE_STRUCT_TM_TM_ZONE
	return(mktime(&date)+date.tm_gmtoff);
#else
	return(mktime(&date));
#endif
};

/* check_mask();
    params: string, mask
    return: non zero if `src' is satisfied by `mask'
	    zero if `src' is not satisfied by `mask'
 */

int check_mask(const char *src,const char *mask) {
	DBC_RETVAL_IF_FAIL(mask!=NULL,0);
	DBC_RETVAL_IF_FAIL(src!=NULL,0);
	const char *m=mask;
	const char *s=src;
	while (*m) {
		if (*m=='*') {
			if (*s==0) return 0;
			m+=1;
			s+=1;
			while (*s) {
				if (check_mask(s,m)) return 1;
				s+=1;
			};
		} else {
			if (*s!=*m) return 0;
			s+=1;
			m+=1;
		};
	};
	return (!*s);
};

/* int check_mask2(char *,char *)
   almost the same as check_mask() but
   allow ? as any symbol
 */

int check_mask2(const char *src,const char *mask) {
	DBC_RETVAL_IF_FAIL(mask!=NULL,0);
	DBC_RETVAL_IF_FAIL(src!=NULL,0);
	const char *m=mask;
	const char *s=src;
	while (*m) {
		switch(*m){
		case '*':{
			if (*s==0) return 0;
			s+=1;m+=1;
			while (*s) {
				if (check_mask2(s,m)) return 1;
				s+=1;
			};
			break;
		};
		case '?':
			if (*s==0) return 0;
			s+=1;m+=1;
			break;
		default:
			if (*s!=*m) return 0;
			s+=1;m+=1;
		};
	};
	return (!*s);
};

/* int check_mask2_uncase(char *,char *)
   case insensetive variant of check_mask2()
 */


int check_mask2_uncase(const char *src,const char *mask) {
	DBC_RETVAL_IF_FAIL(mask!=NULL,0);
	DBC_RETVAL_IF_FAIL(src!=NULL,0);
	const char *m=mask;
	const char *s=src;
	while (*m) {
		switch(*m){
		case '*':{
			if (*s==0) return 0;
			s+=1;m+=1;
			while (*s) {
				if (check_mask2_uncase(s,m)) return 1;
				s+=1;
			};
			break;
		};
		case '?':
			if (*s==0) return 0;
			s+=1;m+=1;
			break;
		default:
			if (tolower(*s)!=tolower(*m)) return 0;
			s+=1;m+=1;
		};
	};
	return (!*s);
};

void normalize_path(char *src) {
	DBC_RETURN_IF_FAIL(src!=NULL);
	char *a=src,*b=src;
	while (*a) {
		if (*a=='/'){
			int need_exit=0;
			do {
				switch(a[1]){
				case '/':
					a+=1;
					break;
				case '.':{
					if (a[2]=='/' || a[2]==0){
						a+=2;
						break;
					};
					if (a[2]=='.'  && (a[3]=='/' || a[3]==0)){
						if (b>src) b-=1;
						while (b>src && *b!='/') b-=1;
						a+=3;
						break;
					};
				};
				default:
					need_exit=1;
				};
				if (*a==0) break;
			}while(!need_exit);
		};
		if ((*b=*a)==0) return;
		b+=1;
		a+=1;
	};
	*b=0;
};

char *normalize_path_full(char *src){
	DBC_RETVAL_IF_FAIL(src!=NULL,NULL);
	if (*src=='~'){
		struct passwd *p;
		char *cur=index(src,'/');
		if (src[1]=='/'){
			p=getpwuid(getuid());
		}else{
			if (cur) *cur=0;
			p=getpwnam(src+1);
			if (cur) *cur='/';
		};
		if (p){
			char *path=sum_strings(p->pw_dir,cur,NULL);
			normalize_path(path);
			return(path);
		};
	};
	char *rval=copy_string(src);
	normalize_path(rval);
	return(rval);
};

/* FIXME: compose_path() should be rewritten!!!
 */

char *compose_path(const char *left,const char *right){
	DBC_RETVAL_IF_FAIL(left!=NULL,NULL);
	DBC_RETVAL_IF_FAIL(right!=NULL,NULL);
	char *newpath=sum_strings(left,"/",right,NULL);
	normalize_path(newpath);
	return(newpath);
}; 

/*
char *compose_path(const char *left,const char *right) {
	DBC_RETVAL_IF_FAIL(left!=NULL,NULL);
	DBC_RETVAL_IF_FAIL(right!=NULL,NULL);
	char *newpath=NULL;
	int len=strlen(left);
	if (*right!='/' && (len==0 || left[len-1]!='/'))
		newpath=sum_strings(left,"/",right, NULL);
	else
		newpath=sum_strings(left,right, NULL);
	len=strlen("/../");
	char *up=strstr(newpath,"/../");
	while (up){
		while (up>newpath && *(up-1)=='/') up-=1;
		*up=0;
		char *tmp=rindex(newpath,'/');
		*up='/';
		if (tmp){
			while (tmp>newpath && *(tmp-1)=='/') tmp-=1;
			memmove(tmp+1,up+len,strlen(up+len)+1);
		}else{
			memmove(newpath,up+len-1,strlen(up+len)+2);
		};
		up=strstr(newpath,"/../");
	};
	if (!string_ended("/..",newpath)){
		char *tmp=newpath+strlen(newpath)-3;
		while (tmp>newpath && *(tmp-1)=='/') tmp-=1;
		*tmp=0;
		tmp=rindex(newpath,'/');
		if (tmp){
			*tmp=0;
		}else{
			*newpath=0;
		};
	};
//	printf("%s + %s -> %s\n",left,right,newpath);
	return(newpath);
};
*/
/*
char *compose_path(const char *left,const char *right) {
	DBC_RETVAL_IF_FAIL(left!=NULL,NULL);
	DBC_RETVAL_IF_FAIL(right!=NULL,NULL);
	unsigned int ll=strlen(left);
	unsigned int rl=strlen(right);
	char *updir="../";
	char *thisdir="./";
	if (begin_string(right,updir) || equal(right,"..")) {
		char *tmp=rindex(left,'/');
		if (ll && left[ll-1]=='/'){
			char *tmp2=tmp;
			*tmp2=0;
			tmp=rindex(left,'/');
			*tmp2='/';
		};
		if (tmp) {
			*tmp=0;
			int dl=rl>strlen("..")?strlen(updir):strlen("..");
			char *tmp2=compose_path(left,right+dl);
			*tmp='/';
			return tmp2;
		};
	};
	if (begin_string(right,thisdir))
		return compose_path(left,right+strlen(thisdir));
	if (ll && left[ll-1]!='/' && rl && *right!='/')
		return(sum_strings(left,"/",right,NULL));
	return(sum_strings(left,right,NULL));
};

*/

char *subtract_path(const char *a,const char *b){
	DBC_RETVAL_IF_FAIL(a!=NULL,NULL);
	DBC_RETVAL_IF_FAIL(b!=NULL,NULL);
	int i=0;
	char *temp=index(b,'/');
	while (temp){
		while (*temp=='/') temp+=1;
		temp=index(temp,'/');
		i+=1;
	};
	i-=1;
	temp=rindex(a,'/');
	while (temp && i>0){
		*temp=0;
		char *tmp=rindex(a,'/');
		*temp='/';
		temp=tmp;
		i-=1;
		if (i==0){
			tmp=copy_string(a);
			return tmp;
		};
	};
	return copy_string("/");
};

void scroll_string_left(char *str,unsigned int shift){
	DBC_RETURN_IF_FAIL(str!=NULL);
	unsigned int len=strlen(str);
	if (len<=shift || shift==0) return;
	char *temp=new char[len+1];
	if (temp==NULL) return; // can't allocate memory
	temp[len]=0;
	for (unsigned int i=0;i<len;i++){
		unsigned int realshift=shift+i>=len?shift+i-len:shift+i;
		temp[i]=str[realshift];
	};
	for (unsigned int i=0;i<len;i++)
		str[i]=temp[i];
	delete[] temp;
};
//void scroll_string_right();


int get_permisions_from_int(int a){
	int rvalue=0;
	int a1=a/(int)100;
	int a2=(a-a1*100)/(int)10;
	int a3=(a-a1*100-a2*10);
	rvalue=(a1*64)+(a2*8)+(a3);
//	printf("%i:%i,%i,%i:%i(%i)\n",a,a1,a2,a3,rvalue,S_IRUSR|S_IWUSR);
	return rvalue;
};

/*this function will return 0 if 'what' was
  ended by 'edned', less than 0 if 'what' less than 'ended'
  or greater than 0 if vice versa
 */

int string_ended(const char *ended, const char *what){
	if (ended==NULL || what==NULL) return 0;
	int a=strlen(ended);
	int b=strlen(what);
	int min=a<b?a:b;
	char *aa=(char *)(ended+a-min);
	char *bb=(char *)(what+b-min);
	return strncmp(aa,bb,min);
};

int string_ended_uncase(const char *ended, const char *what){
	if (ended==NULL || what==NULL) return 0;
	int a=strlen(ended);
	int b=strlen(what);
	int min=a<b?a:b;
	char *aa=(char *)(ended+a-min);
	char *bb=(char *)(what+b-min);
	return strncasecmp(aa,bb,min);
};

std::string filename_extension(const std::string &name){
	std::string::size_type pos=name.rfind('.');
	if (pos!=std::string::npos){
		return name.substr(pos+1);
	};
	return std::string();
};

/* primitive file operations
 */

int f_wchar(int fd,char c){
	return(write(fd,&c,sizeof(char)));
};

int f_wstr(int fd,const char *str){
	DBC_RETVAL_IF_FAIL(str!=NULL,0);
	return(write(fd,str,strlen(str)));
};

int f_wstr_lf(int fd,const char *str){
	DBC_RETVAL_IF_FAIL(str!=NULL,0);
	int a=f_wstr(fd,str);
	if (a<0) return a;
	int b=f_wstr(fd,"\n");
	if (b<0) return b;
	return(a+b);
};

int f_rstr(int fd,char *where,int max) {
	DBC_RETVAL_IF_FAIL(where!=NULL,0);
	char *cur=where;
	max-=1;
	if (max>0){
		int i=max;
		while(read(fd,cur,1)>0 && i>0) {
			i-=1;
			if (*cur=='\n') break;
			cur+=1;
		};
		*cur=0;
		return max-i;
	};
	*where=0;
	return 0;
};

int write_named_string(int fd,const char *name,const char *str){
	if (name==NULL || str==NULL) return(0);
	if (f_wstr_lf(fd,name)<0) return(-1);
	if (f_wstr_lf(fd,str)<0) return(-1);
	return 0;
};

int write_named_integer(int fd,const char *name,int num){
	if (name==NULL) return(0);
	if (f_wstr_lf(fd,name)<0) return(-1);
	char str[MAX_LEN];
	g_snprintf(str,MAX_LEN,"%d",num);
	if (f_wstr_lf(fd,str)<0) return(-1);
	return 0;
};

int write_named_time(int fd,const char *name,time_t when){
	if (name==NULL) return(0);
	if (f_wstr_lf(fd,name)<0) return(-1);
	char str[MAX_LEN];
	g_snprintf(str,MAX_LEN,"%ld",(long int)(when));
	if (f_wstr_lf(fd,str)<0) return(-1);
	return 0;
};

int write_named_fsize(int fd,const char *name,fsize_t size){
	if (name==NULL) return(0);
	if (f_wstr_lf(fd,name)<0) return(-1);
	char str[MAX_LEN];
	g_snprintf(str,MAX_LEN,"%lli",size);
	if (f_wstr_lf(fd,str)<0) return(-1);
	return 0;
};

/* length of string with specified integer;
 */
int int_to_strin_len(int num){
	int len=num<0?2:1;
	num/=10;
	while (num){
		num/=10;
		len+=1;
	};
	return len;
};

/* scanf for int begined from zero */

int sscanf_int(char *str,int *where){
	DBC_RETVAL_IF_FAIL(str!=NULL,0);
	DBC_RETVAL_IF_FAIL(where!=NULL,0);
	if (str==NULL) return 0;
	while (*str=='0' && str[1]!='0')
		str+=1;
	return(sscanf(str,"%i",where));
};

/* covert path from /lalala/lalala/%[D]_%[M] to
   /lalala/lalala/12_02 where 12 is current day
   02 is current month
 */

enum DATE_FMTS_ENUM{
	DFMT_DAY,
	DFMT_MONTH,
	DFMT_YEAR,
	DFMT_HOURS,
	DFMT_MINS,
	DFMT_SECS,
	DFMT_TIME,
	DFMT_DATE,
	DFMT_EXT,
	DFMT_NONE
};

static char* date_fmts[]={
	"[D]", //day
	"[M]", //month
	"[Y]", //year
	"[h]", //hours
	"[m]", //minutes
	"[s]", //seconds
	"[T]", // time in format HH:MM
	"[DATE]", // date in format DD_MM_YEAR
	"[E]" // extension of a file in upper case
};

char *get_extension(const char *name){
	if (name==NULL) return(NULL);
	const char *a=name+strlen(name)-1;
	while(a>=name){
		if (*a=='.'){
			a+=1;
			if (*a==0) return(NULL);
			char *rval=copy_string(a);
			string_to_low(rval);
			return(rval);
		};
		if (*a=='/') return(NULL);
		a-=1;
	};
	return(NULL);
};

char *parse_save_path(const char *str,const char *file){
	DBC_RETVAL_IF_FAIL(str!=NULL,NULL);
	int len=0;
	char *rval;
	char *ext=get_extension(file);
	int extlen=0;
	const char *a=str;
	time_t tmptime=time(NULL);
	struct tm *tmp_tm=new tm;

	localtime_r(&tmptime,tmp_tm);
	tmp_tm->tm_year+=1900;
	tmp_tm->tm_mon+=1;
	/* calc length of returned string */
	while (*a){
		if (*a=='%'){
			unsigned int i;
			for (i=0;i<sizeof(date_fmts)/sizeof(char*);i++){
				if (equal_first(date_fmts[i],a+1))
					break;
			};
			switch(i){
			case DFMT_EXT:
				if (ext){
					extlen=strlen(ext);
					len+=extlen;
				};
				a+=strlen(date_fmts[i]);
				break;
			case DFMT_DAY:
			case DFMT_MONTH:
			case DFMT_HOURS:
			case DFMT_MINS:
			case DFMT_SECS:
				len+=2;
				a+=strlen(date_fmts[i]);
				break;
			case DFMT_TIME:
				len+=5;
				a+=strlen(date_fmts[i]);
				break;
			case DFMT_DATE:
				len+=6+int_to_strin_len(tmp_tm->tm_year);
				a+=strlen(date_fmts[i]);
				break;
			case DFMT_YEAR:
				len+=int_to_strin_len(tmp_tm->tm_year);
				a+=strlen(date_fmts[i]);
				break;
			default:
				len+=1;
				break;
			};
		}else{
			len+=1;
		};
		a+=1;
	};
	rval=new char[len+1];
	char *b=rval;
	a=str;

#define FMT_TMP_NEXT b+=2;a+=strlen(date_fmts[i]);break

	while (*a){
		if (*a=='%'){
			unsigned int i;
			for (i=0;i<sizeof(date_fmts)/sizeof(char*);i++){
				if (equal_first(date_fmts[i],a+1))
					break;
			};
			switch(i){
			case DFMT_EXT:
				if (ext){
					memcpy(b,ext,extlen);
					b+=extlen;
				};
				a+=strlen(date_fmts[i]);
				break;
			case DFMT_DAY:
				sprintf(b,"%02i",tmp_tm->tm_mday);
				FMT_TMP_NEXT;
			case DFMT_MONTH:
				sprintf(b,"%02i",tmp_tm->tm_mon);
				FMT_TMP_NEXT;
			case DFMT_HOURS:
				sprintf(b,"%02i",tmp_tm->tm_hour);
				FMT_TMP_NEXT;
			case DFMT_MINS:
				sprintf(b,"%02i",tmp_tm->tm_min);
				FMT_TMP_NEXT;
			case DFMT_SECS:
				sprintf(b,"%02i",tmp_tm->tm_sec);
				FMT_TMP_NEXT;
			case DFMT_TIME:
				sprintf(b,"%02i:%02i",
					tmp_tm->tm_hour,tmp_tm->tm_min);
				b+=5;
				a+=strlen(date_fmts[i]);
				break;
			case DFMT_DATE:
				sprintf(b,"%02i_%02i_%i",
					tmp_tm->tm_mday,tmp_tm->tm_mon,
					tmp_tm->tm_year);
				b+=6+int_to_strin_len(tmp_tm->tm_year);
				a+=strlen(date_fmts[i]);
				break;
			case DFMT_YEAR:
				sprintf(b,"%i",tmp_tm->tm_year);
				b+=int_to_strin_len(tmp_tm->tm_year);
				a+=strlen(date_fmts[i]);
				break;
			default:
				*b=*a;
				b+=1;
				break;
			};
		}else{
			*b=*a;
			b+=1;
		};
		a+=1;
	};
	delete(tmp_tm);
	if (ext) delete[] ext;
	*b=0;
	return(rval);
};

/* tPStr
   Protected String
 */

tPStr::tPStr(){
	a=NULL;
};

tPStr::tPStr(const char*s){
	a=copy_string(s);
};

bool tPStr::notempty(){
	return(a!=NULL && a[0]!=0);
};

void tPStr::set(const char *b){
	if (a) delete[] a;
	a=copy_string(b);
};

tPStr::~tPStr(){
	if (a)
		delete[] a;
};

/***************************************************************/

void make_proxy_host(const char *host,int port){
	char *tmp=new char[strlen(host)+10];
	char *b=tmp;
	while(*host!=':' && *host){
		*b=*host;
		host++;
		b++;
	};
	*b=':';
	b++;
	sprintf(b,"%i",port);
	if (*tmp!=':' && *tmp)
		ALL_HISTORIES[PROXY_HISTORY]->add(tmp);
	delete[] tmp;
};

int file_copy(char *from,char *to,char *buf,int size){
	FILE *src=fopen(from,"r");
	if (src==NULL){
		return(-1);
	};
	FILE *dst=fopen(to,"w+");
	if (dst==NULL){
		fclose(src);
		return(-1);
	};
	int len=0;
	int rval=0;
	while ((len=fread(buf,1,size,src))>0){
		if (fwrite(buf,len,1,dst)!=1){
			rval=-1;
			break;
		};
	};
	fclose(dst);
	fclose(src);
	return(rval);
};
