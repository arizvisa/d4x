/*	WebDownloader for X-Window
 *	Copyright (C) 1999 Koshelev Maxim
 *	This Program is free but not GPL!!! You can't modify it
 *	without agreement with autor. You can't distribute modified
 *	program but you can distribute unmodified program.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <string.h>
#include <stdio.h>

#include "var.h"

char *copy_string(const char *src,int len) {
	char *temp=new char[len+1];
	strncpy(temp,src,len);
	temp[len]=0;
	return temp;
};

char *copy_string(const char *src) {
	if (!src) return NULL;
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

int equal_first(const char *a,const char *b) {
	if (a==NULL || b==NULL) return 0;
	int la=strlen(a);
	int lb=strlen(b);
	if (la==0 || lb==0) return 0;
	int min=la<lb?la:lb;
	return !strncmp(a,b,min);
};

int equal_first_uncase(const char *a,const char *b) {
	if (a==NULL || b==NULL) return 0;
	int la=strlen(a);
	int lb=strlen(b);
	if (la==0 || lb==0) return 0;
	int min=la<lb?la:lb;
	return !strncasecmp(a,b,min);
};


int begin_string(const char *str,const char *begin) {
	if (str==NULL || begin==NULL) return 0;
	if (equal_first(str,begin) && strlen(str)>=strlen(begin)) return 1;
	return 0;
};

int begin_string_uncase(const char *str,const char *begin) {
	if (str==NULL || begin==NULL) return 0;
	if (equal_first_uncase(str,begin) && strlen(str)>=strlen(begin)) return 1;
	return 0;
};

char *sum_strings(const char *a,const char *b) {
	int la=0;
	int lb=0;
	if (a) la=strlen(a);
	if (b) lb=strlen(b);
	char *temp=new char[la+lb+1];
	*temp=0;
	if (a) strcat(temp,a);
	if (b) strcat(temp,b);
	return temp;
};

char *sum_strings(const char *a,const char *b,const char *c) {
	int la=0;
	int lb=0;
	int lc=0;
	if (a) la=strlen(a);
	if (b) lb=strlen(b);
	if (c) lc=strlen(c);
	char *temp=new char[la+lb+lc+1];
	*temp=0;
	if (a) strcat(temp,a);
	if (b) strcat(temp,b);
	if (c) strcat(temp,c);
	return temp;
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
	if (a==NULL) return 0;
	int len=strlen(a);
	for (int i=0;i<len;i++,a++) {
		if (*a!=' ' && *a!='\n' && *a!='\r')
			return 0;
	};
	return 1;
};

void convert_int_to_2(int what,char *where) {
	char tmp[MAX_LEN];
	*where=0;
	sprintf(tmp,"%i",what);
	if (what<10) strcat(where,"0");
	strcat(where,tmp);
};

void convert_time(int what,char *where) {
	int hours=what/int(3600);
	int mins=(what - hours*3600)/int(60);
	int secs= what-mins*60-hours*3600;
	char tmp[MAX_LEN];
	*where=0;
	if (hours>0) {
		convert_int_to_2(hours,tmp);
		strcat(where,tmp);
		strcat(where,":");
	};
	convert_int_to_2(mins,tmp);
	strcat(where,tmp);
	if (!CFG.TIME_FORMAT) {
		strcat(where,":");
		convert_int_to_2(secs,tmp);
		strcat(where,tmp);
	};
};

void string_to_low(char *what) {
	if (what==NULL) return;
	while (*what) {
		if (*what>='A' && *what<='Z')
			*what+='a'-'A';
		what+=1;
	};
};

void string_to_low(char *what,char delim) {
	if (what==NULL) return;
	char *temp=index(what,delim);
	if (temp) {
		while (what!=temp) {
			if (*what>='A' && *what<='Z')
				*what+='a'-'A';
			what+=1;
		};
	} else {
		string_to_low(what);
	};
};

int convert_from_hex(char what) {
	if (what>='0' && what<='9')
		return(what-'0');
	if (what>='a' && what<='f')
		return (10+what-'a');
	if (what>='A' && what<='F')
		return (10+what-'A');
	return 0;
};

char *parse_percents(char *what) {
	/* In the case if string not needed to correct
	 */
	if (what==NULL || index(what,'%')==NULL) return NULL;
	/* Next string is too ugly because I use "new" in separate thread;
	 */
	char *temp=new char[strlen(what)];
	char *percent,*where=temp,*old=what;
	while ((percent=index(old,'%'))) {
		strncpy(where,old,percent-old);
		where+=percent-old;
		old=percent+1;
		int num=0;
		if (*old) {
			num=convert_from_hex(*old);
			old++;
			if (*old) {
				num=num *16 + convert_from_hex(*old);
				old++;
			};
		};
		*where=char(num);
		where+=1;
	};
	strncpy(where,old,strlen(old));
	where[strlen(old)]=0;
	return temp;
};

void convert_to_hex(char what,char *where) {
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
	if (what==NULL) return NULL;
	int len=strlen(what);
	int unparsed_len=0;
	for (int i=0;i<len;i++){
		if ((what[i]<'/' || what[i]>'z') && what[i]!='.')//|| (what[i]>'9' && what[i]<'A')
			unparsed_len+=3;
		else
			unparsed_len+=1;
	};
	char *rvalue=new char[unparsed_len+1];
	char *cur=rvalue;
	for (int i=0;i<len;i++){
		if ((what[i]<'/' || what[i]>'z') && what[i]!='.'){//|| (what[i]>'9' && what[i]<'A')
			*cur='%';
			cur+=1;
			convert_to_hex(what[i],cur);
			cur+=1;
		}else
			*cur=what[i];
		cur+=1;
	};
	*cur=0;
	return rvalue;
};

void del_crlf(char *what) {
	if (!what) return;
	char *tmp;
	while((tmp=rindex(what,'\n')) || (tmp=rindex(what,'\r'))) {
		*tmp=0;
	};
};

void make_number_nice(char *where,int num) {
	switch (CFG.NICE_DEC_DIGITALS.curent) {
		case 1:
			{
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
		case 2:
			{
				int megs,kils;
				kils=num/1024;
				megs=kils/1024;
				if (megs==0) {
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

/*
 * This function extract string from string with spaces
 * for example:
 * given the string '  aa  '
 * extracted value will be 'aa'
 */

int is_string(char *what){
	if (!what) return 0;
	char *curent=what;
	while (*curent){
		if (*curent<'0' || *curent>'9') return 1;
		curent+=1;
	};
	return 0;
};

char *extract_string(char *src,char *dst) {
	if (!src || !dst) return src;
	char *tmp=src;
	while (*tmp==' ') tmp++;
	char *space=index(tmp,' ');
	if (space) {
		while (tmp!=space) {
			*dst=*tmp;
			dst++;
			tmp++;
		};
		*dst=0;
	} else {
		int a=strlen(tmp);
		for (int i=0;i<a;i++) {
			*dst=*tmp;
			dst++;
			tmp++;
		};
		*dst=0;
	};
	return tmp;
};

char *extract_string(char *src,char *dst,int num){
	char *new_src=src;
	for (int i=0;i<num;i++){
		new_src=extract_string(new_src,dst);
	};
	return(new_src);
};

char *extract_from_prefixed_string(char *str,char *begin){
	char *tmp=str+strlen(begin);
	while (*tmp==' ') tmp+=1;
	char *rvalue=copy_string(tmp);
	del_crlf(rvalue);
	return rvalue;
};

int convert_month(char *src) {
	if (!src) return 0;
	switch(*src) {
		case 'j':
		case 'J':
			{
				if (src[1]=='a' || src[1]=='A') return 0;
				if (src[2]=='n' || src[2]=='N') return 5;
				return 6;
			};
		case 'a':
		case 'A':
			{
				if (src[1]=='u' || src[1]=='U') return 7;
				return 3;
			};
		case 'm':
		case 'M':
			{
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
	if (!mask || !src) return 0;
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
	if (*s) return 0;
	return 1;
};

void normalize_path(char *src) {
	if (!src) return;
	int len=strlen(src);
	while (len>1 && src[len-1]=='/') {
		src[len-1]=0;
		//		len=strlen(src);
		len-=1;
	}
	;
};

char *compose_path(const char *left,const char *right) {
	if (left==NULL || right==NULL) return NULL;
	unsigned int ll=strlen(left);
	unsigned int rl=strlen(right);
	char *updir="../";
	char *thisdir="./";
	if (begin_string(right,updir) || equal(right,"..")) {
		char *tmp=rindex(left,'/');
		if (left[ll-1]=='/'){
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
		return(sum_strings(left,"/",right));
	return(sum_strings(left,right));
};

char *subtract_path(const char *a,const char *b){
	if (a==NULL || b==NULL) return NULL;
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
	if (!url) return 0;
	if (!begin_string_uncase(url,"ftp://") && !begin_string_uncase(url,"http://")
	        && !begin_string_uncase(url,"mailto:") && !begin_string_uncase(url,"news:")) return 0;
	return 1;
};

void scroll_string_left(char *str,unsigned int shift){
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

