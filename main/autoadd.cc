/*	WebDownloader for X-Window
 *	Copyright (C) 1999-2001 Koshelev Maxim
 *	This Program is free but not GPL!!! You can't modify it
 *	without agreement with author. You can't distribute modified
 *	program but you can distribute unmodified program.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include "autoadd.h"
#include "dbc.h"
#include "locstr.h"

struct d4xAASubStr:tNode{
	int type;
	int left_int,right_int,cur_int,int_len;
	char *left_str,*right_str,*cur_str;
	int str_len;
	int end_flag;
	int period;
	d4xAASubStr();
	void print();
	char *first();
	char *next();
	char *scan(char *str);
	~d4xAASubStr();
private:
	int sub_scan(char **cur);
	void next_str();
};

enum D4X_AUTO_ADD_ENUM{
	DAA_STR,
	DAA_INT_INT,
	DAA_STR_STR,
	DAA_STRS,
	DAA_UNKNOWN
};

/******************************************************************/

d4xAASubStr::d4xAASubStr(){
	type=DAA_UNKNOWN;
	period=1;
	left_int=right_int=cur_int=int_len=0;
	left_str=right_str=cur_str=NULL;
};

void d4xAASubStr::print(){
	switch(type){
	case DAA_STR:
		printf("%s",left_str);
		break;
	case DAA_INT_INT:
		printf("%i - %i",left_int,right_int);
		break;
	case DAA_STR_STR:
		printf("%s - %s",left_str,right_str);
		break;
	case DAA_STRS:{
		char *cur=left_str;
		for (int i=0;i<left_int;i++){
			printf("%s",cur);
			cur+=strlen(cur)+1;
		};
		break;
	};
	};
	if (period)
		printf(" / %i",period);
	printf("\n");
};


static char *str_skip_digits_or_(char *cur,char orwhat){
	while(*cur){
		if (!isdigit(*cur) && *cur!=orwhat)
			break;
		cur++;
	};
	return(cur);
};

int d4xAASubStr::sub_scan(char **cur){
	switch (**cur){
	case '}':
		break;
	case '/':{
		*cur+=1;
		if (sscanf(*cur,"%i",&period)!=1 || period<=1)
			return(1);
		*cur=str_skip_digits_or_(*cur,0);
		if (**cur!='}') return(1);
		break;
	};
	default:
		return(1);
	};
	return(0);
};

char *d4xAASubStr::scan(char *str){
	DBC_RETVAL_IF_FAIL(str!=NULL,str);
	char *cur=str;
	if (*cur!='{' && *cur!='['){
		type=DAA_STR;
		while(*cur && *cur!='{' && *cur!='[')
			cur++;
		int len=cur-str;
		left_str=new char[len+1];
		memcpy(left_str,str,len);
		left_str[len]=0;
		return(cur);
	};
	if (*cur=='['){
		cur+=1;
		if (*cur=='['){
			type=DAA_STR;
			left_str=new char[2];
			left_str[0]='[';
			left_str[1]=0;
			return(cur+1);
		};
		char *end=index(cur,']');
		if (end==NULL || end-cur==1 || *cur==',')
			return(str);
		str_len=end-cur;
		left_str=new char[str_len+1];
		memcpy(left_str,cur,str_len);
		left_str[str_len]=0;
		cur=left_str;
		left_int=1;
		while (*cur){
			if (*cur==','){
				*cur=0;
				left_int+=1;
			};
			cur+=1;
		};
		type=DAA_STRS;
		return(end+1);
	};
	cur+=1;
	if (*cur=='{'){
		type=DAA_STR;
		left_str=new char[2];
		left_str[0]='{';
		left_str[1]=0;
		return(cur+1);
	};
	if (sscanf(cur,"%i-%i",&left_int,&right_int)==2){
		char a[100];
		sprintf(a,"%i-%i",left_int,right_int);
		if (!begin_string(cur,a)){
			char *b=index(cur,'-');
			if (b)
				int_len=b-cur;
			else
				int_len=0;
		};
		cur=str_skip_digits_or_(cur,'-');
		if (sub_scan(&cur)) return(str);
		type=DAA_INT_INT;
		return(cur+1);
	};
	char *end=index_mult(cur,"}/");
	if (end){
		char *tmp=cur;
		for (tmp=cur;tmp<end;tmp++)
			if (*tmp=='-') break;
		if (*tmp!='-') return(str);
		if (tmp-cur!=end-tmp-1) return(str);
		str_len=tmp-cur;
		type=DAA_STR_STR;
		left_str=new char[str_len+1];
		right_str=new char[str_len+1];
		memcpy(left_str,cur,str_len);
		memcpy(right_str,tmp+1,str_len);
		left_str[str_len]=right_str[str_len]=0;
		if (sub_scan(&end)) return(str);
		return(end+1);
	};
	return(str);
};

char *d4xAASubStr::first(){
	end_flag=0;
	if (type==DAA_STR)
		return(copy_string(left_str));
	if (type==DAA_INT_INT){
		cur_int=left_int;
		char tmp[100];
		if (int_len){
			char a[100];
			sprintf(a,"%%0%ii",int_len);
			sprintf(tmp,a,cur_int);
		}else
			sprintf(tmp,"%i",cur_int);
		return(copy_string(tmp));
	};
	if (type==DAA_STR_STR){
		cur_str=copy_string(left_str);
		return(copy_string(left_str));
	};
	if (type==DAA_STRS){
		cur_int=0;
		return(copy_string(left_str));
	};
	return(NULL);
};

void d4xAASubStr::next_str(){
	for (int per=0;per<period;per++){
		for(int i=0;i<str_len;i++){
			if (cur_str[i]!=right_str[i]){
				cur_str[i]+=cur_str[i]>right_str[i]?-1:1;
				break;
			};
			if (cur_str[i]==right_str[i])
				cur_str[i]=left_str[i];
		};
		if (strcmp(cur_str,right_str)==0) break;
	};
};

char *d4xAASubStr::next(){
 	if (type==DAA_STR){
		end_flag=1;
		return(copy_string(left_str));
	};
	if (type==DAA_INT_INT){		
		if (cur_int!=right_int){
			for (int per=0;per<period && cur_int!=right_int;per++)
				cur_int+=right_int<cur_int?-1:1;
		}else{
			end_flag=1;
		};
		char tmp[100];
		if (int_len){
			char a[100];
			sprintf(a,"%%0%ii",int_len);
			sprintf(tmp,a,cur_int);
		}else
			sprintf(tmp,"%i",cur_int);
		return(copy_string(tmp));
	};
	if (type==DAA_STR_STR){
		if (strcmp(cur_str,right_str)!=0){
			next_str();
		}else
			end_flag=1;
		return(copy_string(cur_str));
	};
	if (type==DAA_STRS){
		char *rval=left_str;
		if (cur_int+1<left_int)
			cur_int+=1;
		for (int i=0;i<cur_int;i++){
			rval+=strlen(rval)+1;
		};
		return(copy_string(rval));
	};
	return(NULL);
};

d4xAASubStr::~d4xAASubStr(){
	if (left_str) delete[] left_str;
	if (right_str) delete[] right_str;
	if (cur_str) delete[] cur_str;
};

/******************************************************************/


d4xAutoGenerator::d4xAutoGenerator(){
	// do nothing
};

int d4xAutoGenerator::init(char *str){
	char *cur=str;
	while(*cur){
		d4xAASubStr *tmp=new d4xAASubStr;
		char *a=tmp->scan(cur);
		if (a==cur){
			delete(tmp);
			return(1);
		};
		list.insert(tmp);
		cur=a;
	};
//	print();
	return(0);
};

char *d4xAutoGenerator::first(){
	d4xAASubStr *tmp=(d4xAASubStr *)list.first();
	if (tmp==NULL) return(NULL);
	char *rval=tmp->first();
	tmp=(d4xAASubStr *)(tmp->prev);
	while(tmp){
		char *a=tmp->first();
		char *b=sum_strings(rval,a,NULL);
		delete[] rval;
		delete[] a;
		rval=b;
		tmp=(d4xAASubStr *)(tmp->prev);
	};
	return(rval);
};

char *d4xAutoGenerator::next(){
	d4xAASubStr *tmp=(d4xAASubStr *)list.first();
	if (tmp==NULL) return(NULL);
	char *rval=tmp->next();
	int end_flag=tmp->end_flag;
	tmp=(d4xAASubStr *)(tmp->prev);
	while(tmp){
		char *a=tmp->next();
		char *b=sum_strings(rval,a,NULL);
		delete[] rval;
		delete[] a;
		rval=b;
		if (tmp->end_flag==0) end_flag=0;
		tmp=(d4xAASubStr *)(tmp->prev);
	};
	if (end_flag){
		delete[] rval;
		rval=NULL;
	};
	return(rval);
};

void d4xAutoGenerator::print(){
	d4xAASubStr *tmp=(d4xAASubStr *)list.first();
	while(tmp){
		tmp->print();
		tmp=(d4xAASubStr *)(tmp->prev);
	};
};

d4xAutoGenerator::~d4xAutoGenerator(){
	//do nothing?
};
