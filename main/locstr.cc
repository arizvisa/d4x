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

char *sum_strings(const char *a,...){
	if (a==NULL) return NULL;
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
	if (a==NULL) return 0;
	int len=strlen(a);
	for (int i=0;i<len;i++,a++) {
		if (!isspace(*a))
			return 0;
	};
	return 1;
};

void convert_int_to_2(int what,char *where) {
	if (where==NULL) return;
	char tmp[MAX_LEN];
	*where=0;
	sprintf(tmp,"%i",what);
	if (what<10) strcat(where,"0");
	strcat(where,tmp);
};

void convert_time(int what,char *where) {
	if (where==NULL) return;
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
	return 0;
};

char *parse_percents(char *what) {
	/* In the case if string not needed to correct
	 */
	if (what==NULL || index(what,'%')==NULL) return NULL;
	/* Next string is too ugly because I use "new" in separate threads;
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
	if (where==NULL) return;
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

void del_crlf(char *what) {
	if (what==NULL) return;
	char *tmp;
	while((tmp=rindex(what,'\n')) || (tmp=rindex(what,'\r'))) {
		*tmp=0;
	};
};

void make_number_nice(char *where,int num) {
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
	if (what==NULL) return 0;
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
	if (what==NULL) return NULL;
	char *tmp=what;
	while (*tmp){
		if (isspace(*tmp)) return(tmp);
		tmp+=1;
	};
	return NULL;
};

char *extract_string(char *src,char *dst) {
	if (src==NULL || dst==NULL) return src;
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
	if (src==NULL || dst==NULL) return src;
	char *new_src=src;
	for (int i=0;i<num;i++){
		new_src=extract_string(new_src,dst);
	};
	return(new_src);
};

char *extract_from_prefixed_string(char *str,char *begin){
	if (str==NULL || begin==NULL) return NULL;
	char *tmp=str+strlen(begin);
	while (isspace(*tmp)) tmp+=1;
	char *rvalue=copy_string(tmp);
	del_crlf(rvalue);
	return rvalue;
};

char *skip_spaces(char *src){
	if (src==NULL) return NULL;
	char *tmp=src;
	while(isspace(*tmp))
		tmp+=1;
	return tmp;
};

char *skip_strings(char *src,int num){
	if (src==NULL) return NULL;
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
	if (src==NULL) return 0;
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
	if (src==NULL) return 0;
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
	if (mask==NULL || src==NULL) return 0;
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
	if (src==NULL) return;
	int len=strlen(src);
	while (len>1 && src[len-1]=='/') {
		src[len-1]=0;
		//		len=strlen(src);
		len-=1;
	};
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
		return(sum_strings(left,"/",right,NULL));
	return(sum_strings(left,right,NULL));
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
	if (url==NULL) return 0;
	if (!begin_string_uncase(url,"ftp://") && !begin_string_uncase(url,"http://")
	        && !begin_string_uncase(url,"mailto:") && !begin_string_uncase(url,"news:")) return 0;
	return 1;
};

void scroll_string_left(char *str,unsigned int shift){
	if (str==NULL) return;
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

/* parsing url */
struct tTwoStrings{
    char *one;
    char *two;
    tTwoStrings();
    void zero();
    ~tTwoStrings();
};

tTwoStrings::tTwoStrings() {
	one=two=NULL;
};

void tTwoStrings::zero() {
	one=two=NULL;
};

tTwoStrings::~tTwoStrings() {};

int get_port_by_proto(char *proto) {
	if (proto) {
		if (equal_uncase(proto,"ftp")) return 21;
		if (equal_uncase(proto,"http")) return 80;
	};
	return 21;
};

void split_string(char *what,char *delim,tTwoStrings *out) {
	char * where=strstr(what,delim);
	if (where) {
		int len=strlen(where),len1=strlen(delim);
		out->two=copy_string(where+len1);
		len1=strlen(what)-len;
		out->one=copy_string(what,len1);
	} else {
		out->two=copy_string(what);
		out->one=NULL;
	};
	delete(what);
};


tAddr *make_addr_from_url(char *what) {
	tTwoStrings pair;
	split_string(what,"://",&pair);
	tAddr *out=new tAddr;
	if (pair.one) {
		out->protocol=pair.one;
	} else {
		out->protocol=copy_string(DEFAULT_PROTO);
	};
	out->host=pair.two;
	if (!out->host) {
		delete(out);
		return NULL;
	};
	split_string(out->host,"/",&pair);
	if (pair.one) {
		out->host=pair.one;
		out->file=pair.two;
	} else {
		out->host=pair.two;
		out->file=pair.one;
	};
	split_string(out->host,"@",&pair);
	out->host=pair.two;
	out->username=pair.one;
	if (out->username) {
		split_string(out->username,":",&pair);
		out->username=pair.one;
		out->pass=pair.two;
	} else {
		out->username=NULL;
		out->pass=NULL;
	};
	if (out->file) {
		char *tmp=parse_percents(out->file);
		if (tmp) {
			delete out->file;
			out->file=tmp;
		} else
			delete tmp;
		char *prom=rindex(out->file,'/');
		if (prom) {
			out->path=copy_string(prom+1);
			*prom=0;
			prom=out->path;
//			if (out->file && out->file[0]=='~')
				out->path=copy_string(out->file);
//			else
//			out->path=sum_strings("/",out->file,NULL);
			delete out->file;
			out->file=prom;
		};
	} else {
		out->file=copy_string("");
	};
	if (!out->path) out->path=copy_string("");
	split_string(out->host,":",&pair);
	if (pair.one) {
		sscanf(pair.two,"%i",&out->port);
		delete pair.two;
		out->host=pair.one;
	} else {
		out->port=0;
		out->host=pair.two;
	};
	if (equal_uncase(out->protocol,"ftp") && index(out->file,'*'))
		out->mask=1;
	/* Parse # in http urls
	 */
	if (equal_uncase(out->protocol,"http") && out->file!=NULL) {
		char *tmp=index(out->file,'#');
		if (tmp) {
			*tmp=0;
			tmp=out->file;
			out->file=copy_string(tmp);
			delete(tmp);
		};
	};
	if (out->port==0)
		out->port=get_port_by_proto(out->protocol);
	return out;
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


int write_named_string(int fd,char *name,char *str){
	if (name==NULL || str==NULL) return(0);
	if (write(fd,name,strlen(name))<0) return(-1);
	if (write(fd,"\n",strlen("\n"))<0) return(-1);
	if (write(fd,str,strlen(str))<0) return(-1);
	if (write(fd,"\n",strlen("\n"))<0) return(-1);
	return 0;
};

int write_named_integer(int fd,char *name,int num){
	if (name==NULL) return(0);
	if (write(fd,name,strlen(name))<0) return(-1);
	if (write(fd,"\n",strlen("\n"))<0) return(-1);
	char str[MAX_LEN];
	snprintf(str,MAX_LEN,"%d",num);
	if (write(fd,str,strlen(str))<0) return(-1);
	if (write(fd,"\n",strlen("\n"))<0) return(-1);
	return 0;
};

int write_named_time(int fd,char *name,time_t when){
	if (name==NULL) return(0);
	if (write(fd,name,strlen(name))<0) return(-1);
	if (write(fd,"\n",strlen("\n"))<0) return(-1);
	char str[MAX_LEN];
	snprintf(str,MAX_LEN,"%ld",when);
	if (write(fd,str,strlen(str))<0) return(-1);
	if (write(fd,"\n",strlen("\n"))<0) return(-1);
	return 0;
};
