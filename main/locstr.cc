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

#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <time.h>
#include <glib.h>
#include "var.h"

char *copy_string(const char *src,int len) {
	if (src==NULL) len=0;
	char *temp=new char[len+1];
	strncpy(temp,src,len);
	temp[len]=0;
	return temp;
};

char *copy_string(const char *src) {
	if (src==NULL) return NULL;
/*	g_return_val_if_fail(src!=NULL,NULL); */
	int len=strlen(src);
	return copy_string(src,len);
};

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

int equal_uncase(const char *a,const char *b) {
	if (!a) {
		if (b) return 0;
		return 1;
	};
	if (!b) return 0;
	if (strcasecmp(a,b)) return 0;
	return 1;
};

int equal_first(const char *a,const char *b) {
	g_return_val_if_fail(a!=NULL,0);
	g_return_val_if_fail(b!=NULL,0);
	int la=strlen(a);
	int lb=strlen(b);
	if (la==0 || lb==0) return 0;
	int min=la<lb?la:lb;
	return !strncmp(a,b,min);
};

int equal_first_uncase(const char *a,const char *b) {
	g_return_val_if_fail(a!=NULL,0);
	g_return_val_if_fail(b!=NULL,0);
	int la=strlen(a);
	int lb=strlen(b);
	if (la==0 || lb==0) return 0;
	int min=la<lb?la:lb;
	return !strncasecmp(a,b,min);
};


int begin_string(const char *str,const char *begin) {
	g_return_val_if_fail(str!=NULL,0);
	g_return_val_if_fail(begin!=NULL,0);
	if (equal_first(str,begin) && strlen(str)>=strlen(begin)) return 1;
	return 0;
};

int begin_string_uncase(const char *str,const char *begin) {
	g_return_val_if_fail(str!=NULL,0);
	g_return_val_if_fail(begin!=NULL,0);
	if (equal_first_uncase(str,begin) && strlen(str)>=strlen(begin)) return 1;
	return 0;
};

char *sum_strings(const char *a,...){
	g_return_val_if_fail(a!=NULL,NULL);
	va_list args;

	int l=strlen(a)+1;
	va_start(args,a);
	char *s=va_arg(args, char*);
	while(s){
		l+=strlen(s);
		s=va_arg(args, char*);
	};

	char *r=new char[l];
	*r=0;
	strcat(r,a);
	va_start(args,a);
	s=va_arg(args, char*);
	while(s){
		strcat(r,s);
		s=va_arg(args, char*);
	};
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

int empty_string(char *a) {
	g_return_val_if_fail(a!=NULL,0);
	int len=strlen(a);
	for (int i=0;i<len;i++,a++) {
		if (!isspace(*a))
			return 0;
	};
	return 1;
};

void convert_int_to_2(int what,char *where) {
	g_return_if_fail(where!=NULL);
	char tmp[MAX_LEN];
	*where=0;
	sprintf(tmp,"%i",what);
	if (what<10) strcat(where,"0");
	strcat(where,tmp);
};

void convert_time(int what,char *where) {
	g_return_if_fail(where!=NULL);
	int hours=what/int(3600);
	int mins=(what - hours*3600)/int(60);
	int secs= what-mins*60-hours*3600;
	char tmp[MAX_LEN];
	*where=0;
	if (hours>0) {
		sprintf(where,"%i",hours);
		strcat(where,":");
		convert_int_to_2(mins,tmp);
		strcat(where,tmp);
	}else
		sprintf(where,"%i",mins);
	if (!CFG.TIME_FORMAT) {
		strcat(where,":");
		convert_int_to_2(secs,tmp);
		strcat(where,tmp);
	};
};

void string_to_low(char *what) {
	g_return_if_fail(what!=NULL);
	while (*what) {
		if (*what>='A' && *what<='Z')
			*what+='a'-'A';
		what+=1;
	};
};

void string_to_low(char *what,char delim) {
	g_return_if_fail(what!=NULL);
	char *temp=index(what,delim);
	while (*what && what!=temp) {
		if (*what>='A' && *what<='Z')
			*what+='a'-'A';
		what+=1;
	};
};

int convert_from_hex(char what) {
	if (what>='0' && what<='9')
		return(what-'0');
	if (what>='a' && what<='f')
		return (10+what-'a');
	if (what>='A' && what<='F')
		return (10+what-'A');
	return -1;
};

char *parse_percents(char *what) {
	g_return_val_if_fail(what!=NULL,NULL);
	/* Next string is too ugly because I use "new" in separate threads;
	 */
	char *temp=new char[strlen(what)+1];
	char *where=temp,*old=what;
	while (*old) {
		switch (*old){
		case '%':{
			int num=-1;
			num=convert_from_hex(*old);
			old++;
			if (num>=0 && *old) {
				num=num *16 + convert_from_hex(*old);
				old++;
				*where=char(num);
				break;
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

void convert_to_hex(char what,char *where) {
	g_return_if_fail(where!=NULL);
	char hi=what/16;
	char lo=(what-(hi*16));
	if (hi>9)
		*where='A'+hi-10;
	else
		*where='0'+hi;		
	if (lo>9)
		where[1]='A'+lo-10;
	else
		where[1]='0'+lo;		
};

char *unparse_percents(char *what) {
	g_return_val_if_fail(what!=NULL,NULL);
	const char *true_chars=".-+";
	char *temp=what;
	int unparsed_len=0;

	while (*temp){
		if ((*temp<'/' || *temp>'z') && index(true_chars,*temp)==NULL)
			unparsed_len+=3;
		else
			unparsed_len+=1;
		temp+=1;
	};
	char *rvalue=new char[unparsed_len+1];
	char *cur=rvalue;
	temp=what;
	while (*temp){
		if ((*temp<'/' || *temp>'z') && index(true_chars,*temp)==NULL){
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
	return rvalue;
};

void str_non_print_replace(char *what,char symbol){
	g_return_if_fail(what!=NULL);
	unsigned char *temp=(unsigned char *)what;
	while (*temp){
		if (*temp<' ') *temp=(unsigned char )symbol;
		temp+=1;
	};
};

void del_crlf(char *what) {
	g_return_if_fail(what!=NULL);
	char *tmp;
	while((tmp=rindex(what,'\n')) || (tmp=rindex(what,'\r'))) {
		*tmp=0;
	};
};

void make_number_nice(char *where,int num) {
	g_return_if_fail(where!=NULL);
	switch (CFG.NICE_DEC_DIGITALS.curent) {
		case 1:	{
				sprintf(where,"%i",num);
				int len=strlen(where);
				if (len<4) return;
				for (int i=len-3;i>0;i-=3,len++) {
					for (int a=len-1;a>=i;a--)
						where[a+1]=where[a];
					where[i]=' ';
				};
				where[len]=0;
				break;
			};
		case 2:	{
				int megs;
				megs=num/(1024*1024);
				if (megs==0) {
					int kils=num/1024;
					if (kils==0)
						sprintf(where,"%i",num);
					else
						sprintf(where,"%iK",kils);
				} else
					sprintf(where,"%iM",megs);
				break;
			};
		default:
			sprintf(where,"%i",num);
	};
};

int is_string(char *what){
	g_return_val_if_fail(what!=NULL,0);
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
	g_return_val_if_fail(what!=NULL,NULL);
	char *tmp=what;
	while (*tmp){
		if (isspace(*tmp)) return(tmp);
		tmp+=1;
	};
	return NULL;
};

char *extract_string(char *src,char *dst) {
	g_return_val_if_fail(src!=NULL,NULL);
	g_return_val_if_fail(dst!=NULL,NULL);
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
	g_return_val_if_fail(src!=NULL,NULL);
	g_return_val_if_fail(dst!=NULL,NULL);
	char *new_src=src;
	for (int i=0;i<num;i++){
		new_src=extract_string(new_src,dst);
	};
	return(new_src);
};

char *extract_from_prefixed_string(char *str,char *begin){
	g_return_val_if_fail(str!=NULL,NULL);
	g_return_val_if_fail(begin!=NULL,NULL);
	char *tmp=str+strlen(begin);
	while (isspace(*tmp)) tmp+=1;
	char *rvalue=copy_string(tmp);
	del_crlf(rvalue);
	return rvalue;
};

char *skip_spaces(char *src){
	g_return_val_if_fail(src!=NULL,NULL);
	char *tmp=src;
	while(isspace(*tmp))
		tmp+=1;
	return tmp;
};

char *skip_strings(char *src,int num){
	g_return_val_if_fail(src!=NULL,NULL);
	char *tmp=src;
	for (int i=0;i<num;i++) {
		char *tmp1=my_space_locate(tmp);
		if (tmp1) while (isspace(*tmp1)) tmp1++;
		else break;
		tmp=tmp1;
	};
	return tmp;
};


int convert_month(char *src) {
	g_return_val_if_fail(src!=NULL,0);
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

int ctime_to_time(char *src) {
	g_return_val_if_fail(src!=NULL,0);
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
			sscanf(data,"%i:%i:%i",&date.tm_hour,&date.tm_min,&date.tm_sec);
		};
	};
	return mktime(&date);
};

int check_mask(char *src,char *mask) {
	g_return_val_if_fail(mask!=NULL,0);
	g_return_val_if_fail(src!=NULL,0);
	char *m=mask;
	char *s=src;
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

void normalize_path(char *src) {
	g_return_if_fail(src!=NULL);
	int len=strlen(src);
	while (len>1 && src[len-1]=='/') {
		src[len-1]=0;
		//		len=strlen(src);
		len-=1;
	};
};

char *compose_path(const char *left,const char *right) {
	g_return_val_if_fail(left!=NULL,NULL);
	g_return_val_if_fail(right!=NULL,NULL);
/*	if (left==NULL || right==NULL) return NULL; */
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

char *subtract_path(const char *a,const char *b){
	g_return_val_if_fail(a!=NULL,NULL);
	g_return_val_if_fail(b!=NULL,NULL);
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

int global_url(char *url) {
	g_return_val_if_fail(url!=NULL,0);
	if (!begin_string_uncase(url,"ftp://") && !begin_string_uncase(url,"http://")
	        && !begin_string_uncase(url,"mailto:") && !begin_string_uncase(url,"news:")) return 0;
	return 1;
};

void scroll_string_left(char *str,unsigned int shift){
	g_return_if_fail(str!=NULL);
/*	if (str==NULL) return; */
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
	delete temp;
};
//void scroll_string_right();


int get_permisions_from_int(int a){
	int rvalue=0;
	int a1=a/(int)10;
	int a2=a/(int)100;
	rvalue=(a-a1*10)+(a1-a2*10)*8+a2*64;
	return rvalue;
};

/*this function will return 0 if 'what' was
  ended by 'edned', less than 0 if 'what' less than 'ended'
  or greater than 0 if vice versa
 */
int string_ended(char *ended, char *what){
	if (ended==NULL || what==NULL) return 0;
	int a=strlen(ended);
	int b=strlen(what);
	char *aa=ended+a;
	char *bb=what+b;
	while (aa!=ended && bb!=what){
		if (*aa!=*bb) return(*aa>*bb?-1:1);
		aa-=1;
		bb-=1;
	};
	if (aa!=ended) return -1;
	return 0;
};

/* primitive file operations
 */

int f_wstr(int fd,char *str){
	return(write(fd,str,strlen(str)));
};

int f_wstr_lf(int fd,char *str){
	int a=f_wstr(fd,str);
	if (a<0) return a;
	int b=f_wstr(fd,"\n");
	if (b<0) return b;
	return(a+b);
};

int f_rstr(int fd,char *where,int max) {
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

int write_named_string(int fd,char *name,char *str){
	if (name==NULL || str==NULL) return(0);
	if (f_wstr_lf(fd,name)<0) return(-1);
	if (f_wstr_lf(fd,str)<0) return(-1);
	return 0;
};

int write_named_integer(int fd,char *name,int num){
	if (name==NULL) return(0);
	if (f_wstr_lf(fd,name)<0) return(-1);
	char str[MAX_LEN];
	g_snprintf(str,MAX_LEN,"%d",num);
	if (f_wstr_lf(fd,str)<0) return(-1);
	return 0;
};

int write_named_time(int fd,char *name,time_t when){
	if (name==NULL) return(0);
	if (f_wstr_lf(fd,name)<0) return(-1);
	char str[MAX_LEN];
	g_snprintf(str,MAX_LEN,"%ld",when);
	if (f_wstr_lf(fd,str)<0) return(-1);
	return 0;
};

