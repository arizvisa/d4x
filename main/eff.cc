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

#include "main.h"
#include "eff.h"
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

tUrlParser::tUrlParser(const char *filename){
	fd=open(filename,O_RDONLY);
	full=current=0;
	if (fd>=0){
		full=lseek(fd,0,SEEK_END);
		lseek(fd,0,SEEK_SET);
	};
	*buf=0;
	list=NULL;
};

int tUrlParser::sequence(unsigned char *where, char *str){
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

int tUrlParser::read_url(unsigned char *where, tStringList *list){
	char *bad_chars="[]()\"'`*";
	while(read(fd,where,1)>0){
		if (*where<=' ' || *where>127 || index(bad_chars,*where)){
			*where=0;
			list->add((char *)buf);
			return 0;
		};
		if (where>=buf+MAX_LEN) return 0;
		where+=1;
	};
	return 1;
};

tStringList *tUrlParser::parse(){
	list=new tStringList;
	list->init(0);
	if (read(fd,buf,1)<=0) return(list);
	while (1){
		switch(*buf){
		case 'f':{
			if (sequence(buf+1,"tp://")){
				if (read_url(buf+6,list))
					return (list);
			};
			break;
		};
		case 'h':{
			if (sequence(buf+1,"ttp://")){
				if (read_url(buf+7,list))
					return (list);
			};
			break;
		};
		default:
			current=lseek(fd,0,SEEK_CUR);
			fflush(stdout);
			if (read(fd,buf,1)<=0) return(list);
		};
	};
	return(list);
};

tStringList *tUrlParser::get_list(){
	return(list);
};

tUrlParser::~tUrlParser(){
	if (fd>=0) close(fd);
	if (list) delete(list);
};

/***************************************************************/

static pthread_t LOAD_THREAD_ID;
static int LOAD_STATUS=0;
static tUrlParser *LOAD_PARSER=NULL;
extern tMain aa;

static void *thread_for_parse(void *what){
	sigset_t oldmask,newmask;
	sigemptyset(&newmask);
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

void thread_for_parse_add(){
	if (LOAD_PARSER){
		tStringList *list=LOAD_PARSER->get_list();
		if (list){
			tString *tmp=list->last();
			while (tmp){
				aa.add_downloading(tmp->body,(char *)NULL,(char *)NULL);
				tmp=list->next();
			};
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
