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

#include "main.h"
#include "eff.h"
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include "face/mywidget.h"

d4xEffString::d4xEffString():tAbstractSortNode(){
	body=NULL;
};

d4xEffString::d4xEffString(const char *a):tAbstractSortNode(){
	body=copy_string(a);
};

void d4xEffString::print(){
	printf("%s\n",body);
};

int d4xEffString::cmp(tAbstractSortNode *what){
	d4xEffString *a=(d4xEffString *)what;
	if (a->body==NULL){
		if (body) return(1);
		return(0);
	};
	if (body==NULL) return(-1);
	return(strcmp(body,a->body));
};

d4xEffString::~d4xEffString(){
	if (body)
		delete[] body;
};

/***********************************************/

tUrlParser::tUrlParser(const char *filename){
	DBC_RETURN_IF_FAIL(filename!=NULL);
	fd=open(filename,O_RDONLY);
	full=current=0;
	if (fd>=0){
		full=lseek(fd,0,SEEK_END);
		lseek(fd,0,SEEK_SET);
	};
	*buf=0;
	tree=NULL;
};

int tUrlParser::sequence(unsigned char *where, char *str){
	DBC_RETVAL_IF_FAIL(where!=NULL,0);
	DBC_RETVAL_IF_FAIL(str!=NULL,0);

	while(read(fd,where,1)>0){
		if (*str){
			if (*str!=*where){
				*buf=*where;
				return 0;
			};
		}else
			break;
		where+=1;
		str+=1;
		if (*str==0) break;
	};
	return 1;
};

int tUrlParser::read_url(unsigned char *where){
	DBC_RETVAL_IF_FAIL(where!=NULL,0);

	char *bad_chars="[]()\"'`*><,";
	while(read(fd,where,1)>0){
		if (*where<=' ' || *where>127 || index(bad_chars,*where)){
			*where=0;
			d4xEffString *str=new d4xEffString((char*)buf);
			if (tree->find(str)==NULL)
				tree->add(str);
			else
				delete(str);
			return 0;
		};
		if (where>=buf+MAX_LEN) return 0;
		where+=1;
	};
	return 1;
};

tAbstractSortTree *tUrlParser::parse(){
	if (tree) delete tree;
	tree=new tAbstractSortTree;
	if (read(fd,buf,1)<=0) return(tree);
	while (1){
		switch(*buf){
		case 'f':{
			if (sequence(buf+1,"tp://")){
				if (read_url(buf+6))
					return (tree);
			};
			break;
		};
		case 'h':{
			if (sequence(buf+1,"ttp://")){
				if (read_url(buf+7))
					return (tree);
			};
			break;
		};
		default:
			current=lseek(fd,0,SEEK_CUR);
			fflush(stdout);
			if (read(fd,buf,1)<=0) return(tree);
		};
	};
	return(tree);
};

tAbstractSortTree *tUrlParser::get_list(){
	return(tree);
};

tUrlParser::~tUrlParser(){
	if (fd>=0) close(fd);
	if (tree) delete(tree);
};

/***************************************************************/

static pthread_t LOAD_THREAD_ID;
static int LOAD_STATUS=0;
static tUrlParser *LOAD_PARSER=NULL;
extern tMain aa;

static void *thread_for_parse(void *what){
	sigset_t oldmask,newmask;
	sigemptyset(&newmask);
	sigaddset(&newmask,SIGTERM);
	sigaddset(&newmask,SIGINT);
	sigaddset(&newmask,SIGUSR2);
	sigaddset(&newmask,SIGUSR1);
	pthread_sigmask(SIG_BLOCK,&newmask,&oldmask);
	tUrlParser *parser=(tUrlParser *)what;
	if (parser){
		parser->parse();
	};
	LOAD_STATUS=2;
	pthread_exit(NULL);
	return NULL;
};

int thread_for_parse_txt(tUrlParser *parser){
	pthread_attr_t attr_p;
	pthread_attr_init(&attr_p);
	pthread_attr_setdetachstate(&attr_p,PTHREAD_CREATE_JOINABLE);
	pthread_attr_setscope(&attr_p,PTHREAD_SCOPE_SYSTEM);
	LOAD_PARSER=parser;
	LOAD_STATUS=1;
	return (pthread_create(&LOAD_THREAD_ID,&attr_p,thread_for_parse,(void *)parser));
};

float thread_for_parse_percent(){
	if (LOAD_PARSER)
		return(float(LOAD_PARSER->current)/float(LOAD_PARSER->full));
	return 0;
};

int thread_for_parse_txt_status(){
	return LOAD_STATUS;
};

int thread_for_parse_full(){
	if (LOAD_PARSER){
		tAbstractSortTree *tree=LOAD_PARSER->get_list();
		if (tree && tree->count()>0) return(1);
	};
	return(0);
};

static void tread_for_parse_foreach(tAbstractSortNode *node,
				    tAbstractSortTree *tree,
				    void *data){
	d4xEffString *str=(d4xEffString *)node;
	d4xLinksSel *sel=(d4xLinksSel *)data;
	d4x_links_sel_add(sel,str->body);
	tree->del(node);
	delete(node);
};

void thread_for_parse_add(d4xLinksSel *sel){
	if (LOAD_PARSER){
		tAbstractSortTree *tree=LOAD_PARSER->get_list();
		if (tree){
			tree->foreach(tread_for_parse_foreach,sel);
		};
		if (LOAD_PARSER){
			delete(LOAD_PARSER);
			LOAD_PARSER=NULL;
		};
	};
	LOAD_STATUS=0;
};

void thread_for_parse_stop(){
	if (LOAD_PARSER){
		if (pthread_cancel(LOAD_THREAD_ID))
			return;
		LOAD_THREAD_ID = 0;
		LOAD_STATUS = 0;
		if (LOAD_PARSER) delete(LOAD_PARSER);
		LOAD_PARSER=NULL;
	};
};
