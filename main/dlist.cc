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

#include "dlist.h"
#include "ftpd.h"
#include "locstr.h"
#include "string.h"
#include "html.h"

#include "face/lod.h"
#include "face/edit.h"
#include "var.h"
#include "ntlocale.h"
#include "main.h"
#include "httpd.h"
#include "savedvar.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <utime.h>
#include <unistd.h>
#include <strings.h>
#include "signal.h"
#include "ping.h"
#include "filter.h"

extern tMain aa;

void tTriger::reset() {
	old=curent;
};

void tTriger::clear() {
	old=-1;
	curent=0;
};

void tTriger::set(fsize_t a ) {
	curent=a;
};

void tTriger::update() {
	old=curent-1;
};

int tTriger::change() {
	return old==curent?0:1;
};

/* file locking functions */
/* return zero id all ok */
int d4x_f_lock(int fd){
	struct flock a;
	a.l_type=F_WRLCK;
	a.l_whence=SEEK_SET;
	a.l_start=0;
	a.l_len=1;
	if (fcntl(fd,F_SETLK,&a)==-1){
		switch(errno){
		case EINVAL: //not supported by fs
		case ENOLCK: //too many locks
			return(-1);
		};
		return(1);
	};
	return(0);
};

void d4x_f_unlock(int fd){
	struct flock a;
	a.l_type=F_UNLCK;
	a.l_whence=SEEK_SET;
	a.l_start=0;
	a.l_len=1;
	fcntl(fd,F_SETLK,&a);
};

/*-------------------------------------------------------------
  tDefaultWL
 --------------------------------------------------------------*/

tDefaultWL::tDefaultWL(){
	fd=-1;
	LOG=NULL;
	segments=NULL;
	fdlock=0;
	overlap_flag=0;
};

fsize_t tDefaultWL::read(void *dst,fsize_t len){
	if (fd>=0){
		fsize_t loaded_size=::read(fd,dst,len);
		return(loaded_size);
	};
	return(0);
};

int tDefaultWL::is_overlaped(){
	return(overlap_flag);
};

fsize_t tDefaultWL::write(const void *buff, fsize_t len){
	DBC_RETVAL_IF_FAIL(buff!=NULL,0);
	if (fd>=0){
		fsize_t cur=(fsize_t)(lseek(fd,0,SEEK_CUR));
		fsize_t saved_size=::write(fd,buff,len);
		if (saved_size<len)
			log(LOG_ERROR,_("Can't write to file"));
		
		if (segments && saved_size){
			overlap_flag=segments->insert(cur,cur+saved_size);
		};
		return (saved_size);
	};
	return 0;
};

void tDefaultWL::set_log(tLog *log){
	LOG=log;
};

void tDefaultWL::unlock_fd(){
	if (fdlock)
		d4x_f_unlock(fd);
	fdlock=0;
};

int tDefaultWL::lock_fd(){
	/*trying to lock*/
	switch(d4x_f_lock(fd)){
	case 0:
		fdlock=1;
		break;
	case 1:
		log(LOG_ERROR,_("File is already opened by another download!"));
		return(-1);
	case -1:
		log(LOG_WARNING,_("Filesystem do not support advisory record locking!"));
		log(LOG_WARNING,_("Will proceed without it but beware that you might have problems."));
	};
	return(0);
	/*end of trying */
};

void tDefaultWL::set_segments(tSegmentator *newseg){
	segments=newseg;
};

void tDefaultWL::fd_close(){
	if (fd>=0){
		unlock_fd();
		close(fd);		
	};
};

void tDefaultWL::set_fd(int newfd,int lockstate){
	if (fd>=0) fd_close();
	fdlock=lockstate;
	fd=newfd;
};


int tDefaultWL::get_fd(){
	return(fd);
};

fsize_t tDefaultWL::shift(fsize_t len,int mode){
	if (fd>=0){
//		printf("Shift to %i\n",shift);
		return(lseek(fd,len,mode));
	};
	return 0;
};

void tDefaultWL::truncate(){
	if (fd>=0){
		off_t a=lseek(fd,0,SEEK_CUR);
		ftruncate(fd,a);
		if (segments)
			segments->truncate(a);
//		log_printf(LOG_WARNING,"truncate file to %i",a);
	};
};

void tDefaultWL::log(int type,const char *str){
	DBC_RETURN_IF_FAIL(str!=NULL);
	if (LOG){
		LOG->add(str,type);
	};
};

void tDefaultWL::cookie_set(tCookie *cookie){
	COOKIES->lock.lock();
	tCookie *temp=COOKIES->find_exact(cookie);
	if (temp){
		if (cookie->get_time()<time(NULL)){
			COOKIES->del(temp);
			delete(temp);
		}else{
			temp->value.set(cookie->value.get());
			temp->set_time(cookie->get_time());
			temp->myown=1;
		};
		COOKIES->lock.unlock();
		delete(cookie);
		return;
	};
	cookie->myown=1;
	COOKIES->add(cookie);
	COOKIES->lock.unlock();
};


char *tDefaultWL::cookie(const char *host, const char *path){
	DBC_RETVAL_IF_FAIL(host!=NULL,NULL);
	DBC_RETVAL_IF_FAIL(path!=NULL,NULL);
	COOKIES->lock.lock();
	tCookie *temp=COOKIES->find(host);
	char *request_string=NULL;
	tDownload **dwn=my_pthread_key_get();
	if (dwn && *dwn && (*dwn)->config->cookie.get()){
		COOKIES->lock.unlock();
		return(copy_string((*dwn)->config->cookie.get()));
	};
	while (temp){
//		temp->print();
		if (begin_string(path,temp->path.get())){
			char *tmp = request_string==NULL?copy_string(""):request_string;
			request_string=sum_strings(tmp, temp->name.get(),
						   "=", temp->value.get(),
						   ";", NULL);
			delete[] tmp;
		};
		if (temp->get_time()<time(NULL)){
			COOKIES->del(temp);
			delete(temp);
			temp=COOKIES->find(host);
		}else
			temp=COOKIES->find(temp,host);
	};
	COOKIES->lock.unlock();
	return(request_string);
};


tDefaultWL::~tDefaultWL(){
	fd_close();
};

/*************Split Information****************/

d4xCondition::d4xCondition(){
	pthread_mutex_init(&my_mutex,NULL);
};

void d4xCondition::set_value(int val){
	pthread_mutex_lock(&my_mutex);
	value=val;
	pthread_mutex_unlock(&my_mutex);
};

int d4xCondition::get_value(){
	pthread_mutex_lock(&my_mutex);
	int val=value;
	pthread_mutex_unlock(&my_mutex);
	return(val);
};

int d4xCondition::dec(){
	pthread_mutex_lock(&my_mutex);
	value-=1;
	int val=value;
	pthread_mutex_unlock(&my_mutex);
	return(val);
};

int d4xCondition::inc(){
	pthread_mutex_lock(&my_mutex);
	value+=1;
	int val=value;
	pthread_mutex_unlock(&my_mutex);
	return(val);	
};

d4xCondition::~d4xCondition(){
	pthread_mutex_destroy(&my_mutex);
};


tSplitInfo::tSplitInfo(){
	FirstByte=LastByte=-1;
	next_part=parent=grandparent=NULL;
	thread_num=1;
	cond=NULL;
	reset();
};

void tSplitInfo::reset(){
	failed=prepared=run=0;
	stopcount=runcount=0;
};

tSplitInfo::~tSplitInfo(){
	if (next_part)	delete(next_part);
};

/**********************************************/
tDownload::tDownload() {
	sizequery=0;
	list_iter=NULL;
	fsearch=0;
	restart_from_begin=0;
	regex_match=NULL;
	config=NULL;
	next2stop=prev2update=next2update=NULL;
	protect=0;
	next=prev=NULL;
	split=NULL;
	who=NULL;
	info=NULL;
	CurrentLog=LOG=NULL;
	WL=NULL;
	editor=NULL;
//	config->ftp_recurse_depth=config->http_recurse_depth=1;
	SpeedLimit=NULL;
	finfo.size=-1;
	finfo.type=T_NONE;
	DIR=NULL;
	finfo.perm=S_IWUSR | S_IRUSR;
	Start=Pause=Difference=0;
	Percent=0;
	Attempt.clear();
	ActStatus.clear();
	Size.clear();
	Speed.clear();
	Remain.clear();
	myowner=NULL;
	thread_id=0;
	DIR=NULL;
	action=ACTION_NONE;
	ScheduleTime=0;
	segments=NULL;
	ALTS=NULL;
};

fsize_t tDownload::filesize(){
	if (finfo.size==0 && who)
		finfo.size=who->another_way_get_size();
	return(finfo.size>0?finfo.size:0);
};

fsize_t tDownload::get_loaded(){
	if (segments){
		return(segments->get_total());
	};
	return(0);
};

int tDownload::cmp(tAbstractSortNode *b){
	DBC_RETVAL_IF_FAIL(b!=NULL,0);
	tDownload *bb=(tDownload*)b;
	int r=strcmp(info->file.get(),bb->info->file.get());
	if (r)
		return r;
	r=strcmp(info->path.get(),bb->info->path.get());
	if (r)
		return r;
	if (info->params.get()==NULL){
		if (bb->info->params.get())
			return 1;
		return 0;
	};
	if (bb->info->params.get()==NULL)
		return -1;
	return strcmp(info->params.get(),bb->info->params.get());
};

void tDownload::delete_editor() {
	if (editor)
		delete editor;
};

void tDownload::print_error(int err){
	switch(err){
	case ERROR_ACCESS:
		WL->log_printf(LOG_ERROR,
			      _("You have no permissions to create file at path %s"),
			      config->save_path.get());
		break;
	case ERROR_NO_SPACE:{
		WL->log_printf(LOG_ERROR,
			      _("You have no space at path %s for creating file"),
			      config->save_path.get());
		break;
	};
	case ERROR_DIRECTORY:{
		WL->log(LOG_ERROR,_("Directory already created!:))"));
		break;
	};
	};
};

void tDownload::print() {
	//do nothing
};

int tDownload::owner(){
	if (myowner)
		return(myowner->get_key());
	return(DL_ALONE);
};

void tDownload::remove_tmp_files(){
	if (info) {
		char *name;
		char *path=make_path_to_file();
		if (Name2Save.get()){
			name=sum_strings(path,Name2Save.get(),NULL);
		}else{
			if (info->file.get() && *(info->file.get())){
				name=sum_strings(path,".",info->file.get(),NULL);
			}else{
				name=sum_strings(path,".",CFG.DEFAULT_NAME,NULL);
			};
		};
		if (info->params.get()){
			char *tmp=sum_strings(name,"?",info->params.get(),NULL);
			delete[] name;
			name=tmp;
		};
		delete[] path;
		remove(name);
		char *segname=sum_strings(name,".segments",NULL);
		delete[] name;
		remove(segname);
		delete[] segname;
	};
};

int tDownload::delete_file() {
	if (who==NULL) return(0);
	int rvalue=0;
	tFileInfo *D_FILE=who->get_file_info();
	if (D_FILE->type==T_FILE) {
		char *name,*guess;
		make_file_names(&name,&guess);
		if (remove(guess) && remove(name))
			rvalue=-1;
		delete[] name;
		delete[] guess;
	};
	return rvalue;
};

void tDownload::make_file_names(char **name, char **guess){
	DBC_RETURN_IF_FAIL(name!=NULL);
	DBC_RETURN_IF_FAIL(guess!=NULL);
	char *real_path=parse_save_path(config->save_path.get(),info->file.get());
	if (Name2Save.get() && strlen(Name2Save.get()))
		who->make_full_pathes(real_path,
				      Name2Save.get(),
				      name,guess);
	else
		who->make_full_pathes(real_path,name,guess);
	delete[] real_path;
};

int tDownload::try_to_lock_fdesc(){
	if (im_first) return(((tDefaultWL*)(WL))->lock_fd());
	return(0);
};

fsize_t tDownload::init_segmentator(int fdesc,fsize_t cursize,char *name){
	fsize_t rvalue=cursize;
	im_first=0;
	((tDefaultWL*)(WL))->set_fd(fdesc);
	if (segments==NULL){
		im_first=1;
		if (try_to_lock_fdesc()) return(-1);
		segments=new tSegmentator;
		char *segname=sum_strings(name,".segments",NULL);
		segments->init(segname);
		delete[] segname;
		tSegment *first_seg=segments->get_first();
		if (first_seg && first_seg->next!=NULL &&
		    (unsigned long int)rvalue<first_seg->end){
			WL->log(LOG_WARNING,"Segmentation info is wrong!");
			rvalue=0;
			ftruncate(fdesc,0);
			segments->truncate(0);
		}else{
			if ((first_seg==NULL || first_seg->next==NULL)){
				if (rvalue>0)
					segments->insert(0,rvalue);
				segments->truncate(rvalue);
			};
		};
		first_seg=segments->get_first();
		rvalue=first_seg?first_seg->end:0;
	};
	((tDefaultWL*)(WL))->set_segments(segments);
	return rvalue;
};

fsize_t tDownload::create_file() {
	if (!who) return -1;
	tFileInfo *D_FILE=who->get_file_info();
	int fdesc=-1;
	fsize_t rvalue=0;
	char *name,*guess;
	make_file_names(&name,&guess);

	make_dir_hier_without_last(name);
	switch (D_FILE->type) {
	case T_LINK:{ //this is a link
		WL->log(LOG_WARNING,_("Trying to create a link"));
		int err=symlink(D_FILE->body.get(),guess);
		if (err) {
			switch (errno){
			case EEXIST:
				WL->log(LOG_ERROR,_("Link already created!:))"));
				break;
			case EACCES:
				print_error(ERROR_ACCESS);
				break;
			default:
				WL->log(LOG_ERROR,_("Can't create link"));
				rvalue=-1;
				break;
			};
		};
		chmod(guess,D_FILE->perm  | S_IWUSR);
		break;
	};
	case T_FILE:{ //this is a file
		WL->log(LOG_WARNING,_("Trying to create a file"));
		if (restart_from_begin){
			fdesc=open(guess,O_RDWR|O_TRUNC,S_IRUSR | S_IWUSR );
		}else
			fdesc=open(guess,O_RDWR,S_IRUSR | S_IWUSR );
		if (fdesc<0) {
			if (restart_from_begin){
				fdesc=open(name,O_RDWR|O_CREAT|O_TRUNC,S_IRUSR | S_IWUSR );
			}else
				fdesc=open(name,O_RDWR|O_CREAT,S_IRUSR | S_IWUSR );
			if (fdesc<0) {
				switch (errno){
				case EACCES:
					print_error(ERROR_ACCESS);
					rvalue=-1;
					break;
				case ENOSPC:
					print_error(ERROR_NO_SPACE);
					rvalue=-1;
					break;
				default:
					WL->log(LOG_ERROR,_("Can't create file at the path:"));
					WL->log(LOG_ERROR,config->save_path.get());
					WL->log(LOG_ERROR,_("which has name:"));
					WL->log(LOG_ERROR,name);
					rvalue=-1;
				};
				break;
			};
			need_to_rename=1;
		}else{
			need_to_rename=0;
		};
		restart_from_begin=0;
		WL->log(LOG_OK,_("File was created!"));
		rvalue=init_segmentator(fdesc,lseek(fdesc,0,SEEK_END),name);
		break;
	};
	case T_DIR:{ //this is a directory
		WL->log(LOG_WARNING,_("Trying to create a dir"));
		if (strlen(info->file.get())==0){
			print_error(ERROR_DIRECTORY);
			break;
		};
		int temp=0;
		if (strlen(guess))
			temp=mkdir(guess,S_IRWXU);
		if (temp) {
			if (errno!=EEXIST) {
				WL->log(LOG_ERROR,_("Can't create directory!"));
				rvalue=-1;
				break;
			};
			print_error(ERROR_DIRECTORY);
		};
		chmod(guess,D_FILE->perm | S_IWUSR |S_IXUSR);
		break;
	};
	case T_DEVICE:{ //this is device
		WL->log(LOG_WARNING,_("Downloader can't create devices..."));
		break;
	};
	default:{
		who->print_error(ERROR_UNKNOWN);
	};
	};
	delete[] name;
	delete[] guess;
	return rvalue;
};

int tDownload::file_type() {
	if (!who) return(T_FILE);
	tFileInfo *D_FILE=who->get_file_info();
	return (D_FILE->type);
};

void tDownload::set_date_file() {
	if (!who) return;
	tFileInfo *D_FILE=who->get_file_info();
	if (config->get_date) {
		char *name,*guess;
		make_file_names(&name,&guess);
		struct utimbuf dates;
		dates.actime=dates.modtime=D_FILE->date;
		utime(name,&dates);
		utime(guess,&dates);
		delete[] name;
		delete[] guess;
	};
	if (config->permisions)
		fchmod(((tDefaultWL *)(WL))->get_fd(),D_FILE->perm);
	else
		fchmod(((tDefaultWL *)(WL))->get_fd(),get_permisions_from_int(CFG.DEFAULT_PERMISIONS));
};

void tDownload::update_trigers() {
	Speed.update();
	ActStatus.update();
	Size.update();
	Attempt.update();
	Remain.update();
	finfo.oldtype=finfo.type-1;
};

void tDownload::make_file_visible(){
	if (who && need_to_rename){
		tFileInfo *D_FILE=who->get_file_info();
		if (D_FILE->type==T_FILE) {
			char *oldname,*newname;
			make_file_names(&oldname,&newname);
			rename(oldname,newname);
			delete[] oldname;
			delete[] newname;
		};
	};
};

void tDownload::set_default_cfg(){
	config->copy_ints(&(CFG.DEFAULT_CFG));
	config->http_recursing=config->http_recurse_depth==1?0:1;
	config->user_agent.set(CFG.USER_AGENT);
	config->Filter.set(CFG.DEFAULT_FILTER);
	if (myowner && myowner->PAPA)
		config->save_path.set(myowner->PAPA->save_path.get());
	else
		config->save_path.set(CFG.GLOBAL_SAVE_PATH);
	if (CFG.SOCKS_HOST){
		config->socks_host.set(CFG.SOCKS_HOST);
		config->socks_port=CFG.SOCKS_PORT;
		if (CFG.SOCKS_USER && CFG.SOCKS_PASS){
			config->socks_user.set(CFG.SOCKS_USER);
			config->socks_pass.set(CFG.SOCKS_PASS);
		};
	};
	config->proxy.type=CFG.FTP_PROXY_TYPE;
	config->proxy.no_cache=CFG.PROXY_NO_CACHE;
	if (CFG.USE_PROXY_FOR_FTP) {
		config->proxy.ftp_host.set(CFG.FTP_PROXY_HOST);
		config->proxy.ftp_port=CFG.FTP_PROXY_PORT;
		if (CFG.NEED_PASS_FTP_PROXY) {
			config->proxy.ftp_user.set(CFG.FTP_PROXY_USER);
			config->proxy.ftp_pass.set(CFG.FTP_PROXY_PASS);
		};
	};
	if (CFG.USE_PROXY_FOR_HTTP) {
		config->proxy.http_host.set(CFG.HTTP_PROXY_HOST);
		config->proxy.http_port=CFG.HTTP_PROXY_PORT;
		if (CFG.NEED_PASS_HTTP_PROXY) {
			config->proxy.http_user.set(CFG.HTTP_PROXY_USER);
			config->proxy.http_pass.set(CFG.HTTP_PROXY_PASS);
		};
	};
	if (CFG.NUMBER_OF_PARTS>1 && split==NULL){
		split=new tSplitInfo;
		split->NumOfParts=CFG.NUMBER_OF_PARTS;
	};
};

void tDownload::copy(tDownload *dwn){
	if (info==NULL)
		info=new tAddr;
	if (dwn->info)
		info->copy(dwn->info);
	Description.set(dwn->Description.get());
	if (dwn->config){
		if (config==NULL) config=new tCfg;
		config->copy(dwn->config);
		restart_from_begin=dwn->restart_from_begin;
		config->referer.set(dwn->config->referer.get());
		Name2Save.set(dwn->Name2Save.get());
		config->save_path.set(dwn->config->save_path.get());
		config->log_save_path.set(dwn->config->log_save_path.get());
	}else{
		if (config) delete(config);
		config=NULL;
	};
	if (dwn->split==NULL && split)
		delete(split);
	if (dwn->split){
		if (split==NULL) split=new tSplitInfo;
		split->NumOfParts=dwn->split->NumOfParts;
	};
};

char *tDownload::make_path_to_file(){
	int noconfig=0;
	if (config==NULL){
	    noconfig=1;
	    config=new tCfg;
	    set_default_cfg();
	};
	char *real_path=parse_save_path(config->save_path.get(),info->file.get());
	char *rval=NULL;
	if (info && info->proto==D_PROTO_HTTP && config->http_recursing){
		if (config->leave_server){
			rval=sum_strings(real_path,"/",
					 info->host.get(),"/",
					 info->path.get(),"/",NULL);
		}else
			rval=sum_strings(real_path,"/",
					 info->path.get(),"/",NULL);
	}else{
		rval=sum_strings(real_path,"/",NULL);
	};
	delete[] real_path;
	if (noconfig){
	    delete(config);
	    config=NULL;
	};
	return(rval);
};

char *tDownload::create_new_file_path(){
	if (info->mask==0) 
		return (compose_path(info->path.get(),info->file.get()));
	return(copy_string(info->path.get()));
};

char *tDownload::create_new_save_path(){
	if (info->mask==0) {
		char *SaveName=Name2Save.get();
		if (config->save_path.get()) {
			if (SaveName && *SaveName)
				return(compose_path(config->save_path.get(),SaveName));
			else 
				return(compose_path(config->save_path.get(),info->file.get()));
		} else {
			if (SaveName && *SaveName)
				return(copy_string(SaveName));
			else
				return(copy_string(info->file.get()));
		};
	};
	return(copy_string(config->save_path.get()));
};

void tDownload::convert_list_to_dir() {
	if (who==NULL || info==NULL) {
		return;
	};
	tFtpDownload *tmp=(tFtpDownload *)(who);
	tStringList *dir=tmp->dir();
	if (DIR) {
		DIR->done();
	} else {
		DIR=new tDList(DL_TEMP);
		DIR->init(0);
	};
	if (dir==NULL || dir->first()==NULL) {
		return;
	};
	char *path=create_new_file_path();
	char *savepath=create_new_save_path();
	tString *temp=dir->last();
	tFileInfo *prom=new tFileInfo;
	while (temp) {
		ftp_cut_string_list(temp->body,prom,1);
		if (prom->name.get() && !equal(prom->name.get(),".")
		    && !equal(prom->name.get(),"..")
		    && (prom->type!=T_DIR || config->ftp_recurse_depth!=2)
		    && (prom->type==T_DIR || info->mask==0 || check_mask(prom->name.get(),info->file.get()))) {
			tAddr *addrnew=new tAddr;
			tDownload *onenew=new tDownload;
			onenew->config=new tCfg;
			onenew->config->isdefault=0;
			if (prom->type==T_DIR){
				if (info->mask){
					addrnew->compose_path(path,prom->name.get());
					addrnew->file.set(info->file.get());
				}else{
					addrnew->compose_path2(path,prom->name.get());
					addrnew->file.set("");
				};
				char *SavePath=compose_path(savepath,prom->name.get());
				onenew->config->save_path.set(SavePath);
				delete[] SavePath;
				addrnew->mask=info->mask;
			} else {
				addrnew->path.set(path);
				addrnew->file.set(prom->name.get());
				onenew->config->save_path.set(savepath);
			};
			addrnew->copy_host(info);

			onenew->info=addrnew;
			
			onenew->config->copy(config);
			onenew->config->ftp_recurse_depth = config->ftp_recurse_depth ? config->ftp_recurse_depth-1 : 0;
			onenew->config->http_recurse_depth = config->http_recurse_depth;
			onenew->set_split_count(split?split->NumOfParts:0);
			if (CFG.RECURSIVE_OPTIMIZE) {
				onenew->finfo.type=prom->type;
				onenew->finfo.size=prom->size;
				onenew->finfo.date=prom->date;
				if (config->permisions) onenew->finfo.perm=prom->perm;
				if (onenew->finfo.type==T_LINK) {
					if (config->follow_link==1){
						onenew->finfo.type=T_NONE;
						char *tmppath=compose_path(onenew->info->path.get(),prom->body.get());
						char *a=rindex(tmppath,'/');
						if (a){
							*a=0;
							onenew->info->file.set(a+1);
							onenew->info->path.set(tmppath);
						}else{
							onenew->info->file.set(tmppath);
							onenew->info->path.set("");
						};
						delete[] tmppath;
						onenew->finfo.size=0; //follow symbolik link size is unknown yet
						onenew->finfo.date=0; //date is unknown too :-(
					}else{
						onenew->finfo.body.set(prom->body.get());
					};
				};
			};
			if (addrnew->is_valid()){
				DIR->insert(onenew);
			}else
				delete(onenew);
		};
		dir->del(temp);
		delete(temp);
		temp=dir->last();
		prom->name.set(NULL);
		prom->body.set(NULL);
	};
	delete (prom);
	delete[] path;
	delete[] savepath;
};


void make_dir_hier(char *path) {
	DBC_RETURN_IF_FAIL(path!=NULL);
	char *temp=path;
	while (temp) {
		temp=index(temp+1,'/');
		if (temp) *temp=0;
		mkdir(path,S_IRWXU);
		if (temp) *temp='/';
	};
};

void make_dir_hier_without_last(char *path) {
	DBC_RETURN_IF_FAIL(path!=NULL);
	char *temp=rindex(path,'/');
	if (temp){
		*temp=0;
		make_dir_hier(path);
		*temp='/';
	};
};

static int is_subdir(char *path, char *subdir){
	if (begin_string(subdir,path)){
		int len=strlen(path);
		if (subdir[len]=='/' || subdir[len]==0)
			return(1);
	};
	return(0);
};

int tDownload::http_check_settings(tAddr *what){
	DBC_RETVAL_IF_FAIL(what!=NULL,0);
	if (!equal(what->host.get(),info->host.get())){
		return (config->leave_server);
	};
	if (config->dont_leave_dir==0 || is_subdir(info->path.get(),what->path.get()))
		return 1;
	return 0;
};

void tDownload::convert_list_to_dir2(tQueue *dir) {
	if (!dir) return;
	if (DIR) {
		DIR->done();
		DIR->init(0);
	} else {
		DIR=new tDList(DL_TEMP);
		DIR->init(0);
	};
	tHtmlUrl *temp=(tHtmlUrl *)dir->last();
	char *URL=info->url();
	d4xFilter *filter=config->Filter.get()?FILTERS_DB->find(config->Filter.get()):NULL;
	while (temp) {
		tDownload *onenew=new tDownload;
		onenew->config=new tCfg;
		onenew->config->isdefault=0;
		onenew->config->save_path.set(config->save_path.get());
		if (onenew->config->save_path.get())
			normalize_path(onenew->config->save_path.get());
		
		onenew->config->http_recursing=1;
		onenew->config->copy(config);
		onenew->set_split_count(split?split->NumOfParts:0);
		onenew->config->http_recurse_depth = config->http_recurse_depth ? config->http_recurse_depth-1 : 0;
		onenew->config->ftp_recurse_depth = config->ftp_recurse_depth;
		onenew->config->referer.set(URL);
		if (temp->info->is_valid() && http_check_settings(temp->info)){
			onenew->info=temp->info;
			temp->info=NULL;
			if (filter){
				if (filter->match(onenew->info)){
					onenew->info->tag.set(NULL); //this info is not needed any more
					DIR->insert(onenew);
				}else{
					delete(onenew);
				};
			}else{
				DIR->insert(onenew);
			};
		}else{
			delete(onenew);
		};
		dir->del(temp);
		delete(temp);
		temp=(tHtmlUrl *)dir->last();
	};
	if (filter) filter->unref();
	delete[] URL;
};

void tDownload::save_to_config(int fd){
	f_wstr_lf(fd,"Download:");
	if (info) info->save_to_config(fd);
	if (split)
		write_named_integer(fd,"SplitTo:",split->NumOfParts);
	if (config && config->isdefault==0)
		config->save_to_config(fd);
	if (ScheduleTime)
		write_named_time(fd,"Time:",ScheduleTime);
	int tmpid=owner();
	if (tmpid==DL_SIZEQUERY)
		write_named_integer(fd,"State:",action);
	else
		write_named_integer(fd,"State:",tmpid);
	if (finfo.size>0){
		write_named_fsize(fd,"size:",finfo.size);
		if (Size.curent>0)
			write_named_fsize(fd,"loaded:",Size.curent);
	};
	if (Description.get())
		write_named_string(fd,"Description:",Description.get());
	if (Name2Save.get())
		write_named_string(fd,"SaveName:",Name2Save.get());
	if (protect)
		write_named_integer(fd,"protect:",protect);
	if (ALTS)
		ALTS->save_to_config(fd);
	if (restart_from_begin)
		write_named_integer(fd,"restart_from_begin:",restart_from_begin);
	f_wstr_lf(fd,"EndDownload:");
};

int tDownload::load_from_config(int fd){
	tSavedVar table_of_fields[]={
		{"Url:",	SV_TYPE_URL,	&(info)},
		{"State:",	SV_TYPE_INT,	&status},
		{"Time:",	SV_TYPE_TIME,	&ScheduleTime},
		{"Cfg:",	SV_TYPE_CFG,	&config},
		{"SavePath:",	SV_TYPE_PSTR,	&(config->save_path)},
		{"SaveName:",	SV_TYPE_PSTR,	&(Name2Save)},
		{"SplitTo:",	SV_TYPE_SPLIT,	&(split)},
		{"size:",	SV_TYPE_LINT,	&(finfo.size)},
		{"loaded:",	SV_TYPE_LINT,	&(Size.curent)},
		{"protect:",	SV_TYPE_INT,	&(protect)},
		{"Description:",SV_TYPE_PSTR,	&(Description)},
		{"Alt:",	SV_TYPE_ALT,	&(ALTS)},
		{"restart_from_begin:",SV_TYPE_INT,&restart_from_begin},
		{"EndDownload:",SV_TYPE_END,	NULL}
	};
	char buf[MAX_LEN];
	while(f_rstr(fd,buf,MAX_LEN)>0){
		unsigned int i;
		for (i=0;i<sizeof(table_of_fields)/sizeof(tSavedVar);i++){
			if (equal_uncase(buf,table_of_fields[i].name)){
				if (table_of_fields[i].type==SV_TYPE_END){
					if (finfo.size>0)
						Percent=(float(Size.curent)*float(100))/float(finfo.size);
					return(0);
				}else{
					if (sv_parse_file(fd,&(table_of_fields[i]),buf,MAX_LEN))
						return(-1);
				};
			};
		};
	};
	return -1;
};

void tDownload::check_local_file_time(){
	if (split==NULL){
		struct stat tmpstat;
		fstat(((tDefaultWL *)WL)->get_fd(),&tmpstat);
		who->set_local_filetime(tmpstat.st_mtime);
	};
};

void tDownload::http_postload(){
	d4xContentDisposition *cd=((tHttpDownload *)who)->get_content_disp();
	if (cd){
		if (cd->filename.get()){
			download_set_block(1);
			char *buf=new char[1000];
			char *oldname,*newname;
			make_file_names(&oldname,&newname);
			char *tmp=rindex(newname,'/');
			if (tmp){
				*tmp=0;
				char *a=tmp;
				tmp=sum_strings(newname,"/",cd->filename.get(),NULL);
				*a='/';
			}else{
				tmp=copy_string(cd->filename.get());
			};
			char *cpfrom;
			if (need_to_rename){
				cpfrom=oldname;
			}else
				cpfrom=newname;
				 
			WL->log_printf(LOG_WARNING,_("Trying to copy %s to %s due 'Content-Disposition'"),
				       cpfrom,tmp);
			if (file_copy(cpfrom,tmp,buf,1000))
				WL->log(LOG_ERROR,_("Error during copying!"));
			delete[] buf;
			delete[] tmp;
			delete[] newname;
			delete[] oldname;
			download_set_block(0);
		};
	};
};

void tDownload::download_completed(int type) {
	who->done();
	im_last=1;
	if (split && split->cond && split->cond->dec()!=0)
		im_last=0;
	if (split==NULL)
		WL->truncate();
	switch (type){
	case D_PROTO_HTTP:{
		if (im_last){
			http_recurse();
			http_postload();
			if (split && split->grandparent!=this){
				if (split->grandparent->DIR) delete(split->grandparent->DIR);
				split->grandparent->DIR=DIR;
				DIR=NULL;
			};
		};
		break;
	};
	case D_PROTO_FTP:{
		if (finfo.type==T_DIR && config->ftp_recurse_depth!=1)
			convert_list_to_dir();
	};
	};
	WL->log(LOG_OK,_("Downloading was successefully completed!"));
	
	if (im_last && CFG.WRITE_DESCRIPTION && info->proto!=D_PROTO_SEARCH){
		/* add string into Descript.ion file */
		download_set_block(1);
		char *path=make_path_to_file();
		char *desc=sum_strings(path,"Descript.ion",NULL);
		delete[] path;
		int fd=open(desc,O_WRONLY|O_CREAT,S_IRUSR | S_IWUSR );
		delete[] desc;
		lseek(fd,0,SEEK_END);
		/* locking file exclusively */
		struct flock a;
		a.l_type=F_WRLCK;
		a.l_whence=SEEK_SET;
		a.l_start=0;
		a.l_len=1;
		fcntl(fd,F_SETLKW,&a);
		/* writing file */
		if (Name2Save.get())
			f_wstr(fd,Name2Save.get());
		else
			f_wstr(fd,info->file.get());
		f_wstr(fd," - ");
		if (Description.get()) {
			f_wstr(fd,Description.get());
			f_wstr(fd," [");
			info->save_to_description(fd);
			f_wchar(fd,']');
		} else {
			info->save_to_description(fd);
		};
		f_wchar(fd,'\n');
		/* unlocking file */
		a.l_type=F_UNLCK;
		fcntl(fd,F_SETLK,&a);
		close(fd);
		download_set_block(0);
	};
	if (im_last){
		make_file_visible();
		set_date_file();
	};
	if (config->sleep_before_complete)
		sleep(config->time_for_sleep);
	D4X_UPDATE.add(this,DOWNLOAD_COMPLETE);
};

void tDownload::download_failed() {
	if (who)
		who->done();
	if (segments)
		segments->save();
	WL->log(LOG_ERROR,_("Downloading was failed..."));
	D4X_UPDATE.add(this,DOWNLOAD_FATAL);
};

static void _html_parser_destroy_(void *a){
	tHtmlParser *b=(tHtmlParser *)a;
	if (b) delete(b);
};

static void _html_parser_dir_destroy_(void *a){
	tStringList *b=(tStringList *)a;
	if (b) delete(b);
};

void tDownload::http_recurse() {
	tHttpDownload *httpd=(tHttpDownload *)(who);
	char *type=httpd->get_content_type();
	if ((config->change_links || config->http_recurse_depth!=1) &&
	    type && begin_string_uncase(type,"audio/x-pn-realaudio")){
		char *a=make_path_to_file();
		char *tmppath=sum_strings(a,"/",info->file.get(),
					  info->params.get()?"?":NULL,
					  info->params.get()?info->params.get():NULL,
					  NULL);
		delete[] a;
		int fd=open(tmppath,O_RDWR,S_IRUSR | S_IWUSR);
		delete[] tmppath;
		if (fd>=0){
			char *buf=new char[MAX_LEN];
			*buf=0;
			f_rstr(fd,buf,MAX_LEN);
			tQueue *dir=new tQueue;
			pthread_cleanup_push(_html_parser_dir_destroy_,dir);
			tHtmlUrl *node=new tHtmlUrl;
			info->tag.set("");
			node->info=new tAddr(buf);
			delete[] buf;
			dir->insert(node);
			if (config->http_recurse_depth!=1)
				convert_list_to_dir2(dir);
			/* FIXME: what about changing link? */
			pthread_cleanup_pop(1);
		};
		close(fd);
	};
	if ((config->change_links || config->http_recurse_depth!=1) &&
	    type && begin_string_uncase(type,"text/html")){
		tQueue *dir=new tQueue;
		tHtmlParser *html=new tHtmlParser;
		pthread_cleanup_push(_html_parser_dir_destroy_,dir);
		pthread_cleanup_push(_html_parser_destroy_,html);
		dir->init(0);
		if (config->change_links){
			char *a=make_path_to_file();
			char *tmppath=sum_strings(a,"/",info->file.get(),
					    info->params.get()?"?":".fl",
					    info->params.get()?info->params.get():NULL,
					    info->params.get()?".fl":NULL,
					    NULL);
			delete[] a;
			html->out_fd=open(tmppath,O_RDWR|O_CREAT|O_TRUNC,S_IRUSR | S_IWUSR );
			html->leave=config->leave_server;
			html->parse(WL,dir,info,config->quest_sign_replace);
			if (html->out_fd){
				char *name,*guess;
				make_file_names(&name,&guess);
				if (need_to_rename){
					delete[] guess;
					remove(name);
					rename(tmppath,name);
					delete[] name;
				}else{
					delete[] name;
					remove(guess);
					rename(tmppath,guess);
					delete[] guess;
				};
				((tDefaultWL*)(WL))->set_fd(html->out_fd,0);
			};
			delete[] tmppath;
		}else{
			html->out_fd=-1;
			html->parse(WL,dir,info,config->quest_sign_replace);
		};
		pthread_cleanup_pop(1);
		if (config->http_recurse_depth!=1){
			convert_list_to_dir2(dir);
		};
		pthread_cleanup_pop(1);
	};
};

void tDownload::export_socket(tDownloader *what){
	if (WL->is_overlaped()) return;
	download_set_block(1);
	tSocket *sock=what->export_ctrl_socket();
	if (sock){
		tAddr *adr=new tAddr;
		adr->copy_host(info);
		d4xOldSocket *tmp=new d4xOldSocket(adr,sock);
		GVARS.SOCKETS->insert(tmp);
	};
	download_set_block(0);
};

void tDownload::http_check_redirect(bool removefiles){
	char *newurl=newurl=who->get_new_url();
	if (config->change_links && (config->http_recurse_depth!=1 ||
	    config->http_recursing)){
		/* wtrite simply HTML file for redirection */
		WL->shift(0);
		static char *redirect_html="<HTML><HEAD><TITLE>"
			"D4X Redirect page</TITLE><META http-equiv=\"Refresh\" "
			"CONTENT=\"1;url=";
		static char *redirect_html_end="\"></HEAD></HTML>";
		WL->write(redirect_html,strlen(redirect_html));
		tAddr *tmpadr=fix_url_global(newurl,
					     info,
					     ((tDefaultWL *)(WL))->get_fd(),
					     config->leave_server);
		if (tmpadr) delete(tmpadr);
		WL->write(redirect_html_end,strlen(redirect_html_end));
		who->done();
		make_file_visible();
	}else{
		who->done();
		if (removefiles)
			delete_file();
	};
	delete[] newurl;
	tAddr *addr=redirect_url();
	finfo.type=T_REDIRECT;
	if (addr){
		if (config->http_recursing ||config->http_recurse_depth!=1){
			if (equal_uncase(addr->host.get(),info->host.get()) ||
			    config->leave_server){
				finfo.type=T_REDIRECT;
			}else{
				finfo.type=T_FILE;
			};
		};
		d4xFilter *filter=config->Filter.get()?FILTERS_DB->find(config->Filter.get()):NULL;
		if (filter && finfo.type==T_REDIRECT){
			if (filter->match(addr))
				finfo.type=T_REDIRECT;
			else{
				finfo.type=T_FILE;
				WL->log(LOG_WARNING,_("Redirection blocked by filter"));
			};
			filter->unref();
		};
	};
	delete(addr);
	WL->log(LOG_WARNING,_("Redirect detected..."));
};

void tDownload::download_http_size(){
	WL->log(LOG_WARNING,_("Size detection only!"));
	download_set_block(1);
	tSocket *s=GVARS.SOCKETS->find(info);
	download_set_block(0);
	if (who->init(info,config,s)==0) {
		who->init_download(info->path.get(),info->file.get());
		finfo.size=who->get_size_only();
		finfo.type=T_FILE;
		if (((tHttpDownload*)who)->persistent())
			export_socket(who);
	};
	D4X_UPDATE.add(this,DOWNLOAD_COMPLETE);
};

void tDownload::download_ftp_size(){
	WL->log(LOG_WARNING,_("Size detection only!"));
	download_set_block(1);
	tSocket *s=GVARS.SOCKETS->find(info);
	download_set_block(0);
	if (who->init(info,config,s)==0) {
		who->init_download(info->path.get(),info->file.get());
		status=DOWNLOAD_SIZE_WAIT;
		fsize_t size=who->get_size();
		if (size>=0) {
			finfo.size=size;
			finfo.type=file_type();
		};
	};
	export_socket(who);
	D4X_UPDATE.add(this,DOWNLOAD_COMPLETE);
};

void tDownload::download_http() {
	if (!who) who=new tHttpDownload(WL);
	if (sizequery){
		sizequery=0;
		download_http_size();
		return;
	};
	config->split=split?1:0;
	download_set_block(1);
	tSocket *s=GVARS.SOCKETS->find(info);
	download_set_block(0);
	if (who->init(info,config,s)) {
		download_failed();
		return;
	};
	who->init_download(info->path.get(),info->file.get());
	/* We need to know size of already loaded file
	 * but I think if file not found we need to delete it
	 * because in http name of file may be specify 
	 * in http answer
	 */
	int CurentSize=create_file();
	if (CurentSize<0) {
		download_failed();
		return;
	};
	((tDefaultWL*)(WL))->unlock_fd();
	if (split && !im_first)
		CurentSize=split->FirstByte;
	fsize_t SizeDecrement=CurentSize>0 && segments->one_segment()?1:0;
	if (SizeDecrement) ((tHttpDownload*)who)->pass_first_segment();
	who->set_loaded(CurentSize-SizeDecrement);
	CurentSize=who->rollback();
	if (split) split->FirstByte=CurentSize;
	
	fsize_t size=who->get_size();
	if (!im_first && split && split->FirstByte>0 && who->reget()==0){
		WL->log(LOG_WARNING,_("Multithreaded downloading is not possible due to server limitations (resuming not supported)"));
		download_completed(D_PROTO_HTTP);
		return;
	};
	/* In the case if file already loaded
	 */
	if (size==CurentSize+SizeDecrement && size>0 && config->rollback==0) {
		check_local_file_time();
		if (!who->remote_file_changed()){
			finfo.size=size;
			finfo.type=T_FILE;
			WL->log(LOG_OK,_("Local file is seems to be equal to remote one"));
			download_completed(D_PROTO_HTTP);
			return;
		};
	};
	/* There are must be procedure for removing file
	 * wich execute if CurentSize==0
	 */
	if (size==-1) {
		http_check_redirect(CurentSize<=0);
		D4X_UPDATE.add(this,DOWNLOAD_COMPLETE);
		return;
	};
	if (im_first && ((tDefaultWL*)(WL))->lock_fd()){
		download_failed();
		return;
	};

	if (size<0 && split==NULL && CurentSize==0) {
		if (CurentSize==0 && segments) segments->complete();
		if (delete_file())
			WL->log(LOG_WARNING,_("It is strange that we can't delete file which just created..."));
	};

	if (size<-1) {
		WL->log(LOG_WARNING,_("File not found"));
		download_failed();
		return;
	};
	finfo.size=size;
	finfo.type=T_FILE;
	/* there we need to create file again
	 * if CurentSize==0
	 */
	if (split && im_first){
		if (who->reget())
			prepare_splits();
		else
			split->LastByte=size;
	};
	check_local_file_time();
	fsize_t SIZE_FOR_DOWNLOAD=who->reget()?size-CurentSize:size;
	SIZE_FOR_DOWNLOAD=(split && split->LastByte>0)?split->LastByte-split->FirstByte:SIZE_FOR_DOWNLOAD;
	Difference=0;
	status=DOWNLOAD_GO;
	if (who->download(SIZE_FOR_DOWNLOAD)) {
		download_failed();
		return;
	};
	if (!split && ((tHttpDownload*)who)->persistent())
		export_socket(who);
	download_completed(D_PROTO_HTTP);
};

void tDownload::remove_links(d4xSearchEngine *engine){
	d4xFtpRegex ftpr;
	regex_t regs[2];
	ftpr.compile(engine->match.get(),info->file.get());
	ftpr.compile_regexes(regs);
	tDownload *tmp=DIR->last();
	tDList *nDIR=new tDList;
	while(tmp){
		DIR->del(tmp);
		char *url=tmp->info->url();
		char *a=ftpr.cut(url,regs);
		delete[] url;
		if (a){
//			printf("%s\n",a);
			tmp->info->from_string(a);
			delete[] a;
			if (nDIR->find(tmp->info))
				delete(tmp);
			else{
				nDIR->insert(tmp);
			};
		}else{
			delete(tmp);
		};
		tmp=DIR->last();
	};
	delete(DIR);
	DIR=nDIR;
};

static void _tmp_sort_free_(void *buf){
	d4xPing *tmp=(d4xPing *)buf;
	delete(tmp);
};

static int _cmp_pinged_hosts_(tNode *a,tNode *b){
	tDownload *aa=(tDownload *)a;
	tDownload *bb=(tDownload *)b;
	float rval=(aa->Percent/aa->Attempt.curent)-(bb->Percent/bb->Attempt.curent);
	if (rval==0) return(0);
	return(rval>0?1:-1);
};

void tDownload::sort_links(){
	if (DIR==NULL || DIR->count()<=0) return;
	WL->log(LOG_OK,_("Sorting started"));
	int i=0;
	while (i<CFG.SEARCH_PING_TIMES){
		WL->log_printf(LOG_OK,_("Pinging (atempt %i of %i)"),i+1,CFG.SEARCH_PING_TIMES);
		if (!ActStatus.curent){ //clear previous percentage for non comulative ping
			tDownload *a=DIR->last();
			while(a){
				a->Percent=0;
				a->Attempt.curent=0;
				a=DIR->next();
			};
		};
		d4xPing *tmp=new d4xPing;
		pthread_cleanup_push(_tmp_sort_free_,tmp);
		tmp->run(DIR,WL);
		pthread_cleanup_pop(1);
		download_set_block(1);
		DIR->sort(_cmp_pinged_hosts_);
		download_set_block(0);
		if (!i)
			ActStatus.curent=1;
		i+=1;
	};
};

static void _tmp_info_remove_(void *addr){
	tAddr *info=(tAddr *)addr;
	delete(info);
};

void tDownload::ftp_search_sizes(){
	WL->log(LOG_WARNING,_("Trying to determine filesizes"));
	download_set_block(1);
	delete(who);
	who=NULL;
	download_set_block(0);

	tDownload *tmp=DIR->last();
	config->number_of_attempts=5;
	while(tmp){
		tDownload *nexttmp=DIR->next();
		who=new tFtpDownload(WL);
		if (who->init(tmp->info,config)){
			WL->log(LOG_ERROR,"Can't determine filesize");
			tmp->finfo.size=-1;
		}else{
			who->init_download(tmp->info->path.get(),
					   tmp->info->file.get());
			tmp->finfo.size=who->get_size();
		};
		who->done();
		download_set_block(1);
		delete(who);
		who=NULL;
		download_set_block(0);
		tmp=nexttmp;
	};
};

void tDownload::ftp_search() {
	/* FIXME: prepare new url for ftp search */
	if (action!=ACTION_REPING){
		Size.set(0);
		d4xSearchEngine *engine=D4X_SEARCH_ENGINES.get_next_used_engine(NULL);
		tDList *TMP_DIR=NULL;
		while(engine){
			tAddr *tmpinfo=new tAddr;
			fsize_t size=0;
			int who_download_status=0;
			pthread_cleanup_push(_tmp_info_remove_,tmpinfo);
			config->change_links=0;
			engine->prepare_url(tmpinfo,finfo.size,info->file.get(),CFG.SEARCH_PERSERVER);
			pthread_cleanup_pop(0);
			if (who->init(tmpinfo,config)) {
				delete(tmpinfo);
				download_failed();
				return;
			};
			pthread_cleanup_push(_tmp_info_remove_,tmpinfo);
			who->init_download(tmpinfo->path.get(),tmpinfo->file.get());
			who->set_loaded(0);
			size=who->get_size();
			pthread_cleanup_pop(0);
			if (size<=-1) {
				WL->log(LOG_WARNING,_("Searching failed"));
				delete(tmpinfo);
				download_failed();
				return;
			};
			pthread_cleanup_push(_tmp_info_remove_,tmpinfo);
			finfo.type=T_FILE;
			Start=Pause=time(NULL);
			Difference=0;
			status=DOWNLOAD_GO;
			who_download_status=who->download(0);
			pthread_cleanup_pop(0);
			if (who_download_status) {
				delete(tmpinfo);
				download_failed();
				return;
			};
			pthread_cleanup_push(_tmp_info_remove_,tmpinfo);
			config->http_recurse_depth=2;
			config->leave_server=1;
			download_set_block(1);
			tAddr *a=info;
			info=tmpinfo;
			http_recurse();
			info=a;
			who->done();
			remove_links(engine);
			if (TMP_DIR){
				if (DIR){
					tDownload *dwn=DIR->first();
					while(dwn){
						DIR->del(dwn);
						TMP_DIR->insert_if_absent(dwn);
						dwn=DIR->first();
					};
					delete(DIR);
					DIR=NULL;
				};
			}else{
				TMP_DIR=DIR;
				DIR=NULL;
			};
			Size.set(TMP_DIR->count());
			engine=D4X_SEARCH_ENGINES.get_next_used_engine(engine);
			download_set_block(0);
			pthread_cleanup_pop(1);
			if (Size.curent>=CFG.SEARCH_ENTRIES)
				break;
		};
		if (TMP_DIR && !DIR)
			DIR=TMP_DIR;
	}else{
		Size.set(DIR->count());
	};
	if (finfo.size<0 && DIR!=NULL && DIR->count()>0)
		ftp_search_sizes();
	sort_links();
	WL->log(LOG_OK,_("Search had been completed!"));
	D4X_UPDATE.add(this,DOWNLOAD_COMPLETE);
};


void tDownload::download_ftp(){
	WL->log(LOG_WARNING,_("Was Started!"));
	if (!who) who=new tFtpDownload(WL);
	if (sizequery){
		sizequery=0;
		download_ftp_size();
		return;
	};

	if (finfo.type==T_LINK && config->follow_link!=2) {
		WL->log(LOG_WARNING,_("It is a link and we already load it"));
		who->init_download(NULL,info->file.get());
		who->set_file_info(&finfo);
		create_file();
		set_date_file();
		download_completed(D_PROTO_FTP);
		return;
	};

	config->split=split?1:0;
	download_set_block(1);
	tSocket *s=GVARS.SOCKETS->find(info);
	download_set_block(0);
	if (who->init(info,config,s)) {
		download_failed();
		return;
	};
	who->init_download(info->path.get(),info->file.get());
	if (finfo.size<0 || finfo.type==T_NONE) {
		status=DOWNLOAD_SIZE_WAIT;
		fsize_t size=who->get_size();
		if (size<0) {
			WL->log(LOG_ERROR,_("File not found"));
			if (info->mask){
				download_failed();
				return;
			};
			WL->log(LOG_ERROR,_("Trying to work without CWD"));
			((tFtpDownload *)(who))->dont_cwd();
			finfo.type=T_FILE;
		}else{
			finfo.size=size;
			finfo.type=file_type();
		};
	} else {
		who->set_file_info(&(finfo));
	};
	if (finfo.type==T_LINK){
		finfo.size=0;
		if (config->follow_link==2){
			tFileInfo *i=who->get_file_info();
			i->type=finfo.type=T_FILE;
		};
	};
	fsize_t CurentSize=0;
	if (info->mask==0){
		CurentSize=create_file();
		//if it was link
		finfo.type=file_type();
	};

	if (finfo.type==T_DEVICE) {
		download_completed(D_PROTO_FTP);
		return;
	};
	if (CurentSize<0) {
		download_failed();
		return;
	};
	if (finfo.size && CurentSize>finfo.size)
		CurentSize=finfo.size;
	
	if (split && finfo.type==T_FILE){
		if (im_first){
			if (who->reget())
				prepare_splits();
			else
				split->LastByte=CurentSize;
		}else{
			CurentSize=split->FirstByte;
		};
	};
	check_local_file_time();
	who->set_loaded(CurentSize);
	if (split) WL->shift(CurentSize);
	fsize_t SIZE_FOR_DOWNLOAD=(split && split->LastByte>0)?split->LastByte-split->FirstByte:0;
	Difference=0;
	status=DOWNLOAD_GO;
	if (who->download(SIZE_FOR_DOWNLOAD)) {
		download_failed();
		return;
	};
	if (config->dont_send_quit) export_socket(who);
	if (config->follow_link==1 && finfo.type==T_LINK)
		finfo.type=T_REDIRECT;
	download_completed(D_PROTO_FTP);
};

#define SPLIT_MINIMUM_PART 5120

int tDownload::find_best_split(){
	tSegment *holes=segments->to_holes(finfo.size);
	tSegment *tmp=holes->next;
	tSegment *best=holes;
	fsize_t maxlen=holes->end-holes->begin;
	while(tmp){
		fsize_t l=tmp->end-tmp->begin;
		if (l>maxlen){
			maxlen=l;
			best=tmp;
		};
		tmp=tmp->next;
	};
	split->FirstByte=split->LastByte=0;
	// if this part is already loading by another thread then we
	// need to load only part of this chunk
	tDownload *gp=split->grandparent;
	while (gp){
		if (gp!=this &&
		    ((gp->split->LastByte>=best->begin && gp->split->LastByte<=best->end) ||
		     (gp->split->FirstByte>=best->begin && gp->split->FirstByte<=best->end)||
		     (gp->split->FirstByte<=best->begin && gp->split->LastByte>=best->end)))
			break;
		gp=gp->split->next_part;
	};
	if (gp){
		split->FirstByte=best->begin+maxlen/2;
	}else{
		split->FirstByte=best->begin;
	};
	split->LastByte=best->end;
	int completed=0;
	if (best->begin==0) completed=1;
	while(holes){
		tmp=holes->next;
		delete(holes);
		holes=tmp;
	};
	int k=(info->proto==D_PROTO_FTP && (config==NULL || config->proxy.ftp_host.get()==NULL || config->proxy.type==0))?12:8;
	if (split->LastByte-split->FirstByte>SPLIT_MINIMUM_PART*k && completed==0){
		return 1;
	};
	split->FirstByte=split->LastByte=0;
	return 0;
};


void tDownload::prepare_splits(){
 	DBC_RETURN_IF_FAIL(split!=NULL);
	DBC_RETURN_IF_FAIL(segments!=NULL);
	tSegment *holes=segments->to_holes(finfo.size);
	tSegment *tmp;
/*
	printf("split to %i parts[holes->offest_in_file=%i]\n",split->NumOfParts,holes->offset_in_file);
	tmp=holes;
	while(tmp){
		printf("L[%i]:%li %li\n",holes->offset_in_file,holes->begin,holes->end);
		tmp=tmp->next;
	};
*/
	while(split->NumOfParts>holes->offset_in_file){
		tSegment *largest=holes;
		tmp=holes->next;
		while(tmp){
			if ((tmp->end-tmp->begin)>(largest->end-largest->begin))
				largest=tmp;
			tmp=tmp->next;
		};
		if (largest->end-largest->begin<SPLIT_MINIMUM_PART){
			WL->log(LOG_WARNING,_("Can't split file to specified number of parts!"));
			break;
		};
		tmp=new tSegment;
		tmp->end=largest->end;
		tmp->begin=largest->begin+(largest->end-largest->begin)/(split->NumOfParts-holes->offset_in_file+1);
		largest->end=tmp->begin;
		tmp->next=largest->next;
		largest->next=tmp;
		holes->offset_in_file+=1;
	};
	if (holes->next && holes->end==holes->next->begin){
		holes->end+=(holes->end-holes->begin)/10;
		holes->next->begin=holes->end;
	};
	split->cond=new d4xCondition;
//	if (split->NumOfParts<holes->offset_in_file)
		split->NumOfParts=holes->offset_in_file;
	split->cond->set_value(split->NumOfParts);
	split->FirstByte=holes->begin;
	split->LastByte=holes->end;
	split->thread_num=1;
//	printf("%li %li\n",split->FirstByte,split->LastByte);
	tmp=holes->next;
	delete(holes);
	holes=tmp;
	tSplitInfo *newsplit=split;
	tDownload *parent=this;
	char i='1';
	d4xAlt *alt=NULL;
	if (ALTS){
		ALTS->lock_by_download();
		alt=ALTS->FIRST;
	};
	split->alt=0;
	int alt_num=1;
	while(holes){
		tmp=holes->next;
//		printf("H:%li %li\n",holes->begin,holes->end);
		if (parent->split->thread_num<newsplit->NumOfParts){
			if (newsplit->next_part==NULL)
				newsplit->next_part=new tDownload;
			tDownload *temp=newsplit->next_part;
			temp->status=DOWNLOAD_REAL_STOP;
			if (temp->split==NULL)
				temp->split=new tSplitInfo;
			else
				temp->split->reset();
			newsplit=temp->split;
			newsplit->cond=split->cond;
			newsplit->NumOfParts=split->NumOfParts;
			temp->split->grandparent=this;
			temp->split->parent=parent;
			temp->split->thread_num=parent->split->thread_num+1;
			parent=temp;
			temp->split->FirstByte=holes->begin;
			temp->split->LastByte=holes->end;
//			printf("%li %li\n",newsplit->FirstByte,newsplit->LastByte);
			temp->segments=segments;
			if (temp->config==NULL) temp->config=new tCfg;
			temp->config->copy(config);
			if (config->log_save_path.get()){
				char *tmppath=sum_strings(config->log_save_path.get(),"_ ",
							  NULL);
				tmppath[strlen(tmppath)-1]=i;
				temp->config->log_save_path.set(tmppath);
			};
			i+=1;
			temp->config->speed=(config->speed/split->NumOfParts)*temp->split->NumOfParts;
			temp->config->user_agent.set(config->user_agent.get());
			temp->config->referer.set(config->referer.get());
			temp->config->save_path.set(config->save_path.get());
			temp->Name2Save.set(Name2Save.get());
			temp->finfo.size=finfo.size;
			temp->finfo.type=finfo.type;
			temp->finfo.perm=finfo.perm;
			temp->finfo.date=finfo.date;
			if (temp->info==NULL)
				temp->info=new tAddr();
			if (alt){
				temp->split->alt=alt_num;
				temp->info->copy(&(alt->info));
				alt->set_proxy_settings(temp);
			}else{
				temp->split->alt=0;
				temp->info->copy(info);
			};
			if (ALTS){
				if (alt->next){
					alt=alt->next;
					alt_num++;
				}else{
					alt=ALTS->FIRST;
					alt_num=1;
				};
			};
		};
		delete(holes);
		holes=tmp;
	};
	if (ALTS)
		ALTS->unlock_by_download();
// to avoid broken downloads when we can't detect resuming support at first request
// first thread always load from begining to end
	if (split->FirstByte==0) split->LastByte=finfo.size;
	split->prepared=1;
};


void tDownload::delete_who(){
	if (who){
		delete(who);
		who=NULL;
	};
};


tAddr *tDownload::redirect_url(){
	char *newurl=NULL;
	if (who)
		newurl=who->get_new_url();
	if (newurl) {
		tAddr *addr=fix_url_global(newurl,info,-1,0);
		delete[] newurl;
		return(addr);
	};
	return(NULL);
};

void tDownload::set_split_count(int num){
	if (num>1){
		if (num>10) num=10;
		if (split==NULL)
			split=new tSplitInfo;
		split->NumOfParts=num;
		split->grandparent=this;
	}else{
		if (split)
			delete(split);
		split=NULL;
	};
};

tDownload::~tDownload() {
	if (list_iter) gtk_tree_iter_free(list_iter);
	if (config) delete(config);
	if (myowner) myowner->del(this);
	if (who) delete who;
	if (info) delete info;
	if (editor) delete editor;
	if (LOG) LOG->ref_dec();
	if (DIR) delete DIR;
	if (WL) delete(WL);
	if (split) delete(split);
	if (ALTS) delete(ALTS);
};


//**********************************************/

tDList::tDList():tQueue(){
	Pixmap=PIX_UNKNOWN;
	empty=non_empty=NULL;
};

tDList::tDList(int key):tQueue(){
	Pixmap=PIX_UNKNOWN;
	OwnerKey=key;
	empty=non_empty=NULL;
};

void tDList::set_empty_func(void (*emp)(),void (*nonemp)()){
	empty=emp;
	non_empty=nonemp;
	if (Num && non_empty)
		non_empty();
	else if (empty)
		empty();
};

void tDList::init_pixmap(int a){
	Pixmap=a;
};

int tDList::get_key(){
	return(OwnerKey);
};

void tDList::insert(tDownload *what) {
	DBC_RETURN_IF_FAIL(what!=NULL);
	if (Num==0 && non_empty!=NULL)
		non_empty();
	tQueue::insert(what);
	what->myowner=this;
	if (Pixmap!=PIX_UNKNOWN && PAPA)
		PAPA->qv.set_pixmap(what,Pixmap);
};

void tDList::insert_before(tDownload *what,tDownload *where) {
	DBC_RETURN_IF_FAIL(where!=NULL);
	DBC_RETURN_IF_FAIL(where->myowner==this);
	tQueue::insert_before(what,where);
	what->myowner=this;
	if (Pixmap!=PIX_UNKNOWN && PAPA)
		PAPA->qv.set_pixmap(what,Pixmap);
};

void tDList::del(tDownload *what) {
	DBC_RETURN_IF_FAIL(what->myowner==this);
	if (Num==1 && empty!=NULL)
		empty();
	tQueue::del(what);
	what->myowner=NULL;
};

void tDList::forward(tDownload *what) {
	DBC_RETURN_IF_FAIL(what!=NULL);
	if (what->next) {
		tDownload *temp=(tDownload *)(what->next);
		if ((temp->prev=what->prev))
			what->prev->next=temp;
		else
			Last=what->next;
		what->prev=temp;
		if ((what->next=temp->next))
			temp->next->prev=what;
		else
			First=what;
		what->prev->next=what;
	};
};

void tDList::backward(tDownload *what) {
	DBC_RETURN_IF_FAIL(what!=NULL);
	if (what->prev) {
		tDownload *temp=(tDownload *)(what->prev);
		if ((temp->next=what->next))
			what->next->prev=temp;
		else
			First=what->prev;
		what->next=temp;
		if ((what->prev=temp->prev))
			temp->prev->next=what;
		else
			Last=what;
		what->next->prev=what;
	};
};

tDownload *tDList::find(tAddr *addr){
	tNode *a=First;
	while(a){
		if (((tDownload*)a)->info->cmp(addr)) return((tDownload*)a);
		a=a->prev;
	};
	return(NULL);
};

void tDList::insert_if_absent(tDownload *what){
	if (find(what->info))
		delete(what);
	else
		insert(what);
};

void tDList::dispose(){
//	ALL_DOWNLOADS->del((tDownload *)First);
	tQueue::dispose();
};

tDownload *tDList::last() {
	return (tDownload *)(Curent=Last);
};

tDownload *tDList::first() {
	return (tDownload *)(Curent=First);
};

tDownload *tDList::next() {
	return (tDownload *)tQueue::next();
};

tDownload *tDList::prev() {
	return (tDownload *)tQueue::prev();
};

tDList::~tDList() {
	done();
};
