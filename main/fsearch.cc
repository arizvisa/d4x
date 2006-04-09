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
#include "fsearch.h"
#include "dbc.h"
#include "signal.h"
#include "var.h"
#include "face/fsface.h"
#include "face/lod.h"
#include "main.h"
#include "xml.h"
#include "ntlocale.h"
#include "face/themes.h"

using namespace d4x;

d4xEnginesList D4X_SEARCH_ENGINES;

char *cut_string(const char *str,int a,int b){
	if (b<a) return NULL;
	char *rval=new char[b-a+1];
	memcpy(rval,str+a,b-a);
	rval[b-a]=0;
	return(rval);
};

char *str_regex_replace(const char *str){
	char *badchars="[]\\.(){}:=";
	int len=strlen(str);
	const char *c=str;
	while(*c){
		if (index(badchars,*c))
			len++;
		c++;
	};
	char *rval=new char[len+1];
	char *b=rval;
	c=str;
	while(*c){
		if (index(badchars,*c)){
			*b='\\';
			b++;
		};
		*b=*c;
		b++;
		c++;
	};
	*b=0;
	return(rval);
};

/****************************************************************/

void d4xSearchEngine::prepare_url(d4x::URL &adr,int size,const char *file,int num){
	char data[100];
	char *tmp=NULL,*tmp1;
	char *f=unparse_percents((char*)file);
	sprintf(data,"%i",num);
	if (size>0){
		tmp=str_replace(urlsize.get(),"[:num:]",data);
		sprintf(data,"%li",size);
		tmp1=str_replace(tmp,"[:size:]",data);
		delete[] tmp;
		tmp=str_replace(tmp1,"[:file:]",f);
	}else{
		tmp=str_replace(urlnosize.get(),"[:num:]",data);
		tmp1=str_replace(tmp,"[:file:]",f);
		delete[] tmp;
		tmp=tmp1;
	};
	delete[] f;
	adr=std::string(tmp);
	delete[] tmp;
};

d4xSearchEngine *d4xEnginesList::first(){
	return((d4xSearchEngine*)(tQueue::first()));
};

d4xSearchEngine *d4xEnginesList::next(){
	return((d4xSearchEngine*)(tQueue::next()));
};

d4xSearchEngine *d4xEnginesList::prev(){
	return((d4xSearchEngine*)(tQueue::prev()));
};

int d4xEnginesList::load(){
	char *tmp=sum_strings(D4X_SHARE_PATH,"/ftpsearch.xml",NULL);
	tQueue *data=d4x_xml_parse_file(tmp);
	delete[] tmp;
	char *cur=CFG.SEARCH_ENGINES;
	if (data){
		d4xXmlObject *obj=NULL;
		while ((obj=(d4xXmlObject *)(data->first()))){
			data->del(obj);
			if (equal_uncase(obj->name.get(),"ftpsearch")){
				d4xXmlField *fld=obj->get_attr("name");
				d4xXmlObject *urlsize=obj->find_obj("urlsize");
				d4xXmlObject *urlnosize=obj->find_obj("urlnosize");
				d4xXmlObject *match=obj->find_obj("match");
				if (fld && urlsize && urlnosize && match){
					d4xSearchEngine *engine=new d4xSearchEngine;
					engine->name.set(fld->value.get());
					engine->urlsize.set(urlsize->value.get());
					engine->urlnosize.set(urlnosize->value.get());
					engine->match.set(match->value.get());
					d4xFtpRegex test;
					regex_t regs[2];
					if (test.compile(match->value.get(),"test") &&
					    test.compile_regexes(regs)){
						if (*cur){
							if (*cur=='1')
								engine->used=1;
							cur++;
						};
						insert(engine);
						regfree(regs);
						regfree(regs+1);
					}else
						delete(engine);
				};
			};
			delete(obj);
		};
		delete(data);
	};
	return(0);
};

d4xSearchEngine *d4xEnginesList::get_by_num(int num){
	tNode *a=First;
	while(a && num>0){
		num--;
		a=a->prev;
	};
	return((d4xSearchEngine*)a);
};

void d4xEnginesList::names2array(char **arr){
	if (First){
		int i=0;
		tNode *a=First;
		while(a){
			arr[i++]=((d4xSearchEngine*)a)->name.get();
			a=a->prev;
		};
		
	}else{
		arr[0]=_("Check your installation of the Downloader");
	};
};

d4xSearchEngine *d4xEnginesList::get_next_used_engine(d4xSearchEngine *cur){
	d4xSearchEngine *b;
	if (cur)
		b=(d4xSearchEngine *)(cur->prev);
	else
		b=(d4xSearchEngine *)First;
	while (b && b->used==0){
		b=(d4xSearchEngine *)(b->prev);
	};
	return (b);
};

/****************************************************************/

d4xFtpRegex::d4xFtpRegex():left(NULL),center(NULL),right(NULL){};

int d4xFtpRegex::compile(const char *str,const char *file){
	char *f=str_regex_replace(file);
	char *s=str_replace(str,"[:file:]",f);
	delete[] f;
	char *a=strstr(s,"(:");
	if (!a){
		delete[] s;
		return(0);
	};
	char *b=strstr(a+2,":)");
	if (!b){
			delete[] s;
			return(0);
	};
	free();
	left=cut_string(s,0,a-s);
	center=cut_string(s,a-s+2,b-s);
	right=cut_string(s,b-s+2,strlen(s));
	delete[] s;
	return(1);
};

int d4xFtpRegex::compile_regexes(regex_t *regs){
	if (regcomp(regs,left,0))
		return(0);
	if (regcomp(regs+1,center,0)){
		regfree(regs);
		return(0);
	};
	return(1);
};

char *d4xFtpRegex::cut(const char *str,regex_t *regs){
	regmatch_t rval[10];
	regmatch_t rval2[10];
	if (regexec(regs,str,10,rval,0)==0){
		for (int i=0;i<100;i++){
			if (rval[i].rm_so==-1) break;
			if (regexec(regs+1,str+rval[i].rm_eo,10,rval2,0)==0){
				if (rval2[0].rm_so==0)
					return(cut_string(str+rval[i].rm_eo,0,rval2[0].rm_eo));
			};
		};
	};
	return(NULL);
};

void d4xFtpRegex::print(){
	printf("%s\n%s\n%s\n",left,center,right);
};

void d4xFtpRegex::free(){
	if (left) delete[] left;
	if (right) delete[] right;
	if (center) delete[] center;
	left=right=center=NULL;
};

d4xFtpRegex::~d4xFtpRegex(){
	free();
};

/****************************************************************/
tFtpSearchCtrl::tFtpSearchCtrl(){
	for (int i=0;i<DL_FS_LAST;i++){
		queues[i]=new tDList(i);
		queues[i]->init(0);
	};
	view=(GtkTreeView *)NULL;
	parent=(tMain*)NULL;
	log=(tMLog*)NULL;
};

void tFtpSearchCtrl::init(GtkTreeView *v, tMain *papa,tMLog *mylog){
	parent=papa;
	view=v;
	log=mylog;
};

void tFtpSearchCtrl::add(tDownload *what){
	DBC_RETURN_IF_FAIL(what!=NULL);
	what->info.proto=D_PROTO_SEARCH;
	what->action=ACTION_NONE; //reping only flag
	what->ActStatus=0; //cumulative reping flag
	queues[DL_FS_WAIT]->insert(what);
	if (view){
		fs_list_add(view,what);
		fs_list_show();
	};
};

void tFtpSearchCtrl::reping(tDownload *what){
	what->action=ACTION_REPING;
	what->myowner->del(what);
	if (what->config==NULL){
		what->config=new tCfg;
		what->set_default_cfg();
	};
	if (what->split){
		delete(what->split);
		what->split=NULL;
	};
	queues[DL_FS_WAIT]->insert(what);
	fs_list_set_icon(view,what,LPE_WAIT);
};

void tFtpSearchCtrl::remove(tDownload *what){
	DBC_RETURN_IF_FAIL(what!=NULL);
	switch(what->owner()){
	case DL_FS_RUN:
		stop_thread(what);
		what->action=ACTION_DELETE;
		fs_list_set_icon(view,what,LPE_STOP_WAIT);
		break;
	case DL_FS_STOP:
	case DL_FS_WAIT:
		what->myowner->del(what);
		remove_from_clist(what);
		delete(what);
		break;
	default:
		printf("WARNING: bug in ftp search!\n");
	};
};

void tFtpSearchCtrl::remove_from_clist(tDownload *what){
	if (view){
		fs_list_remove(view,what);
		if (queues[DL_FS_WAIT]->count()+
		    queues[DL_FS_STOP]->count()+
		    queues[DL_FS_RUN]->count()==0)
			fs_list_hide();
	};
};

void tFtpSearchCtrl::stop_all_offline(){
	tDownload *tmp=queues[DL_FS_RUN]->last();
	while(tmp){
		tDownload *tmpnext=queues[DL_FS_RUN]->next();
		stop_thread(tmp);
		if (tmp->action!=ACTION_DELETE)
			tmp->action=ACTION_CONTINUE;
		tmp=tmpnext;
	};
};

void tFtpSearchCtrl::cycle(){
	/* stoping completed and failed */
	if (CFG.OFFLINE_MODE) return;
	tDownload *tmp=queues[DL_FS_RUN]->last();
	while (tmp){
		tDownload *tmpnext=queues[DL_FS_RUN]->next();
		fs_list_set_count(view,tmp);
		if (tmp->status==DOWNLOAD_REAL_STOP ||
		    tmp->status==DOWNLOAD_COMPLETE  ||
		    tmp->status==DOWNLOAD_FATAL) {
			queues[DL_FS_RUN]->del(tmp);
			parent->prepare_for_stoping(tmp);
			real_stop_thread(tmp);
			parent->post_stopping(tmp);
			switch(tmp->action){
			case ACTION_DELETE:{
				if (tmp->fsearch){
					tDownload *a=ALL_DOWNLOADS->find(tmp);
					if (a){
						if (a->ALTS!=NULL){
							a->ALTS->ftp_searching=0;
							a->ALTS->set_find_sens();
						};
					};
				};
				remove_from_clist(tmp);
				delete(tmp);
				break;
			};
			case ACTION_CONTINUE:{
				queues[DL_FS_WAIT]->insert(tmp);
				fs_list_set_icon(view,tmp,LPE_WAIT);
				if (tmp->config==NULL){
					tmp->config=new tCfg;
					tmp->set_default_cfg();
				};
				if (tmp->split){
					delete(tmp->split);
					tmp->split=NULL;
				};
				break;
			};
			default:{
				if (view){
					switch (tmp->status){
					case DOWNLOAD_COMPLETE:{
						fs_list_set_icon(view,tmp,LPE_COMPLETE);
						if (tmp->fsearch){
							tDownload *a=ALL_DOWNLOADS->find(tmp);
							if (a){
								if (a->ALTS==NULL) a->ALTS=new d4xAltList;
								a->ALTS->fill_from_ftpsearch(tmp);
								a->ALTS->ftp_searching=0;
								a->ALTS->set_find_sens();
								remove_from_clist(tmp);
								delete(tmp);
								tmp=NULL;
							};
						};
						break;
					};
					case DOWNLOAD_REAL_STOP:
						fs_list_set_icon(view,tmp,LPE_PAUSE);
						break;
					default:
						fs_list_set_icon(view,tmp,LPE_STOP);
					};
				};
				if (tmp) queues[DL_FS_STOP]->insert(tmp);
				break;
			};
			};
		};		
		tmp=tmpnext;
	};
	/* runing new */
	tmp=queues[DL_FS_WAIT]->last();
	while (tmp!=NULL && queues[DL_FS_RUN]->count()<5){
		tDownload *tmpnext=queues[DL_FS_WAIT]->next();
		if (parent->run_new_thread(tmp))
			break;
		queues[DL_FS_WAIT]->del(tmp);
		queues[DL_FS_RUN]->insert(tmp);
		if (view)
			fs_list_set_icon(view,tmp,LPE_RUN);
		tmp=tmpnext;
	};
};

tFtpSearchCtrl::~tFtpSearchCtrl(){
	tDownload *tmp=queues[DL_FS_STOP]->last();
	while(tmp){
		remove(tmp);
		tmp=queues[DL_FS_STOP]->last();
	};
	tmp=queues[DL_FS_WAIT]->last();
	while(tmp){
		remove(tmp);
		tmp=queues[DL_FS_WAIT]->last();
	};
	tmp=queues[DL_FS_RUN]->last();
	while(tmp){
		remove(tmp);
		tmp=queues[DL_FS_RUN]->next();
	};
	while(queues[DL_FS_RUN]->count()){
		cycle();
	};
	for (int i=0;i<DL_FS_LAST;i++){
		delete(queues[i]);
	};
};
