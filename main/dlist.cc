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

void tTriger::set(int a ) {
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
};

fsize_t tDefaultWL::read(void *dst,fsize_t len){
	if (fd>=0){
		fsize_t loaded_size=::read(fd,dst,len);
		return(loaded_size);
	};
	return(0);
};

fsize_t tDefaultWL::write(const void *buff, fsize_t len){
	DBC_RETVAL_IF_FAIL(buff!=NULL,0);
	if (fd>=0){
		fsize_t cur=(fsize_t)(lseek(fd,0,SEEK_CUR));
		fsize_t saved_size=::write(fd,buff,len);
		if (saved_size<len)
			log(LOG_ERROR,_("Can't write to file"));
		
		if (segments && saved_size){
			segments->insert(cur,cur+saved_size);
		};
		return (saved_size);
	};
	return 0;
};

void tDefaultWL::set_log(tLog *log){
	LOG=log;
};

void tDefaultWL::set_segments(tSegmentator *newseg){
	segments=newseg;
};

void tDefaultWL::fd_close(){
	if (fd>=0){
		if (fdlock)
			d4x_f_unlock(fd);
		close(fd);		
	};
};

void tDefaultWL::set_fd(int newfd,int lockstate=0){
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

char *tDefaultWL::cookie(const char *host, const char *path){
	DBC_RETVAL_IF_FAIL(host!=NULL,NULL);
	DBC_RETVAL_IF_FAIL(path!=NULL,NULL);
	tCookie *temp=COOKIES->find(host);
	char *request_string=NULL;
	tDownload **dwn=my_pthread_key_get();
	if (dwn && *dwn && (*dwn)->config.cookie.get())
		return(copy_string((*dwn)->config.cookie.get()));
	while (temp){
//		temp->print();
		if (begin_string(path,temp->path.get())){
			char *tmp = request_string==NULL?copy_string(""):request_string;
			request_string=sum_strings(tmp, temp->name.get(),
						   "=", temp->value.get(),
						   ";", NULL);
			delete[] tmp;
		};
		temp=(tCookie *)(temp->next);
	};
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
	next_part=parent=NULL;
	status=0;
	cond=NULL;
};

tSplitInfo::~tSplitInfo(){
	if (next_part)	delete(next_part);
};

/**********************************************/
tDownload::tDownload() {
	protect=0;
	next=prev=NULL;
	split=NULL;
	who=NULL;
	info=NULL;
	CurrentLog=LOG=NULL;
	WL=NULL;
	editor=NULL;
	config.ftp_recurse_depth=config.http_recurse_depth=1;
	SpeedLimit=NULL;
	finfo.size=-1;
	finfo.type=T_NONE;
	DIR=NULL;
	finfo.perm=S_IWUSR | S_IRUSR;
	Start=Pause=Difference=0;
	Percent=0;
	Attempt.clear();
	Status.clear();
	Size.clear();
	Speed.clear();
	Remain.clear();
	myowner=NULL;
	thread_id=0;
	NanoSpeed=0;
	DIR=NULL;
	action=ACTION_NONE;
	ScheduleTime=0;
	segments=NULL;
	StartSize=0;
};

fsize_t tDownload::get_loaded(){
	if (segments){
		return(segments->get_total());
	};
	return(0);
};

fsize_t tDownload::start_size(){
	if (split) return(StartSize);
	if (who) return(who->get_start_size());
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

void tDownload::clear() {
	if (who) {
		delete(who);
		who=NULL;
	};
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
			      config.save_path.get());
		break;
	case ERROR_NO_SPACE:{
		WL->log_printf(LOG_ERROR,
			      _("You have no space at path %s for creating file"),
			      config.save_path.get());
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
		if (config.save_name.get()){
			name=sum_strings(path,config.save_name.get(),NULL);
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
	char *real_path=parse_save_path(config.save_path.get(),info->file.get());
	if (config.save_name.get() && strlen(config.save_name.get()))
		who->make_full_pathes(real_path,
				      config.save_name.get(),
				      name,guess);
	else
		who->make_full_pathes(real_path,name,guess);
	delete[] real_path;
};

fsize_t tDownload::init_segmentator(int fdesc,fsize_t cursize,char *name){
	fsize_t rvalue=cursize;
	im_first=0;
	if (segments==NULL){
		/*trying to lock*/
		switch(d4x_f_lock(fdesc)){
		case 1:
			WL->log(LOG_ERROR,_("File is already opened by another download!"));
			close(fdesc);
			return(-1);
		case -1:
			WL->log(LOG_WARNING,_("Filesystem do not support advisory record locking!"));
			WL->log(LOG_WARNING,_("Will proceed without it but beware that you might have problems."));
		};
		/*end of trying */
		segments=new tSegmentator;
		char *segname=sum_strings(name,".segments",NULL);
		segments->init(segname);
		delete[] segname;
		im_first=1;
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
					segments->insert(0,(unsigned long int)rvalue);
				segments->truncate(rvalue);
			};
		};
		first_seg=segments->get_first();
		rvalue=first_seg?first_seg->end:0;
		StartSize=segments->get_total();
	};
	((tDefaultWL*)(WL))->set_segments(segments);
	((tDefaultWL*)(WL))->set_fd(fdesc,im_first);
	return rvalue;
};

fsize_t tDownload::create_file() {
	if (!who) return -1;
	tFileInfo *D_FILE=who->get_file_info();
	if (D_FILE->type==T_LINK && config.link_as_file)
		D_FILE->type=T_FILE;
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
		if (config.restart_from_begin){
			fdesc=open(guess,O_RDWR|O_TRUNC,S_IRUSR | S_IWUSR );
		}else
			fdesc=open(guess,O_RDWR,S_IRUSR | S_IWUSR );
		if (fdesc<0) {
			if (config.restart_from_begin){
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
					WL->log(LOG_ERROR,config.save_path.get());
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
		config.restart_from_begin=0;
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
	if (config.get_date) {
		char *name,*guess;
		make_file_names(&name,&guess);
		struct utimbuf dates;
		dates.actime=dates.modtime=D_FILE->date;
		utime(name,&dates);
		utime(guess,&dates);
		delete[] name;
		delete[] guess;
	};
	if (config.permisions)
		fchmod(((tDefaultWL *)(WL))->get_fd(),D_FILE->perm);
	else
		fchmod(((tDefaultWL *)(WL))->get_fd(),get_permisions_from_int(CFG.DEFAULT_PERMISIONS));
};

void tDownload::update_trigers() {
	Speed.update();
	Status.update();
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
	config.copy_ints(&(CFG.DEFAULT_CFG));
	config.http_recursing=config.http_recurse_depth==1?0:1;
	config.user_agent.set(CFG.USER_AGENT);
	config.Filter.set(CFG.DEFAULT_FILTER);
	if (CFG.SOCKS_HOST){
		config.socks_host.set(CFG.SOCKS_HOST);
		config.socks_port=CFG.SOCKS_PORT;
		if (CFG.SOCKS_USER && CFG.SOCKS_PASS){
			config.socks_user.set(CFG.SOCKS_USER);
			config.socks_pass.set(CFG.SOCKS_PASS);
		};
	};
};

void tDownload::copy(tDownload *dwn){
	if (info==NULL)
		info=new tAddr;
	if (dwn->info)
		info->copy(dwn->info);
	config.copy(&(dwn->config));
	config.restart_from_begin=dwn->config.restart_from_begin;
	config.referer.set(dwn->config.referer.get());
	config.save_name.set(dwn->config.save_name.get());
	config.save_path.set(dwn->config.save_path.get());
	config.log_save_path.set(dwn->config.log_save_path.get());
};

char *tDownload::make_path_to_file(){
	char *real_path=parse_save_path(config.save_path.get(),info->file.get());
	char *rval=NULL;
	if (info && info->proto==D_PROTO_HTTP && config.http_recursing){
		if (config.leave_server){
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
	return(rval);
};

char *tDownload::create_new_file_path(){
	if (info->mask==0) 
		return (compose_path(info->path.get(),info->file.get()));
	return(copy_string(info->path.get()));
};

char *tDownload::create_new_save_path(){
	if (info->mask==0) {
		char *SaveName=config.save_name.get();
		if (config.save_path.get()) {
			if (SaveName && *SaveName)
				return(compose_path(config.save_path.get(),SaveName));
			else 
				return(compose_path(config.save_path.get(),info->file.get()));
		} else {
			if (SaveName && *SaveName)
				return(copy_string(SaveName));
			else
				return(copy_string(info->file.get()));
		};
	};
	return(copy_string(config.save_path.get()));
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
		    && (prom->type!=T_DIR || config.ftp_recurse_depth!=2)
		    && (prom->type==T_DIR || info->mask==0 || check_mask(prom->name.get(),info->file.get()))) {
			tAddr *addrnew=new tAddr;
			tDownload *onenew=new tDownload;
			if (prom->type==T_DIR && info->mask) {
				addrnew->compose_path(path,prom->name.get());
				addrnew->file.set(info->file.get());
				char *SavePath=compose_path(savepath,prom->name.get());
				onenew->config.save_path.set(SavePath);
				delete[] SavePath;
				addrnew->mask=info->mask;
			} else {
				addrnew->path.set(path);
				addrnew->file.set(prom->name.get());
				onenew->config.save_path.set(savepath);
			};
			addrnew->copy_host(info);

			onenew->info=addrnew;

			onenew->config.copy(&config);
			onenew->config.ftp_recurse_depth = config.ftp_recurse_depth ? config.ftp_recurse_depth-1 : 0;
			onenew->config.http_recurse_depth = config.http_recurse_depth;
			if (CFG.RECURSIVE_OPTIMIZE) {
				onenew->finfo.type=prom->type;
				onenew->finfo.size=prom->size;
				onenew->finfo.date=prom->date;
				if (config.permisions) onenew->finfo.perm=prom->perm;
				if (onenew->finfo.type==T_LINK) {
					onenew->finfo.body.set(prom->body.get());
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

int tDownload::http_check_settings(tAddr *what){
	DBC_RETVAL_IF_FAIL(what!=NULL,0);
	if (!equal(what->host.get(),info->host.get())){
		return (config.leave_server);
	};
	if (config.dont_leave_dir==0 || begin_string(what->path.get(),info->path.get()))
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
	d4xFilter *filter=config.Filter.get()?FILTERS_DB->find(config.Filter.get()):NULL;
	while (temp) {
		tDownload *onenew=new tDownload;
		onenew->config.save_path.set(config.save_path.get());
		if (onenew->config.save_path.get())
			normalize_path(onenew->config.save_path.get());
		
		onenew->config.http_recursing=1;
		
		onenew->config.copy(&config);
		onenew->config.http_recurse_depth = config.http_recurse_depth ? config.http_recurse_depth-1 : 0;
		onenew->config.ftp_recurse_depth = config.ftp_recurse_depth;
		onenew->config.referer.set(URL);
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
	config.save_to_config(fd);
	if (ScheduleTime)
		write_named_time(fd,"Time:",ScheduleTime);
	write_named_integer(fd,"State:",owner());
	if (finfo.size>0){
		write_named_time(fd,"size:",finfo.size);
		if (Size.curent>0)
			write_named_time(fd,"loaded:",Size.curent);
	};
	if (protect)
		write_named_integer(fd,"protect:",protect);
	f_wstr_lf(fd,"EndDownload:");
};

int tDownload::load_from_config(int fd){
	tSavedVar table_of_fields[]={
		{"Url:",	SV_TYPE_URL,	&(info)},
		{"State:",	SV_TYPE_INT,	&status},
		{"Time:",	SV_TYPE_TIME,	&ScheduleTime},
		{"Cfg:",	SV_TYPE_CFG,	&config},
		{"SavePath:",	SV_TYPE_PSTR,	&(config.save_path)},
		{"SaveName:",	SV_TYPE_PSTR,	&(config.save_name)},
		{"SplitTo:",	SV_TYPE_SPLIT,	&(split)},
		{"size:",	SV_TYPE_TIME,	&(finfo.size)},
		{"loaded:",	SV_TYPE_TIME,	&(Size.curent)},
		{"protect:",	SV_TYPE_INT,	&(protect)},
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
		};
		break;
	};
	case D_PROTO_FTP:{
		if (finfo.type==T_DIR && config.ftp_recurse_depth!=1)
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
		lockf(fd,F_LOCK,0);
		if (config.save_name.get())
			f_wstr(fd,config.save_name.get());
		else
			f_wstr(fd,info->file.get());
		f_wstr(fd," - ");
		if (config.Description.get()) {
			f_wstr(fd,config.Description.get());
			f_wstr(fd," [");
			info->save_to_description(fd);
			f_wchar(fd,']');
		} else {
			info->save_to_description(fd);
		};
		f_wchar(fd,'\n');
		lockf(fd,F_ULOCK,0);
		close(fd);
		download_set_block(0);
	};
	if (im_last){
		make_file_visible();
		set_date_file();
	};
	if (config.sleep_before_complete)
		sleep(config.time_for_sleep);
	status=DOWNLOAD_COMPLETE;
};

void tDownload::download_failed() {
	if (who)
		who->done();
	WL->log(LOG_ERROR,_("Downloading was failed..."));
	status=DOWNLOAD_FATAL;
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
	if ((config.change_links ||  config.http_recurse_depth!=1)
	    && type && begin_string_uncase(type,"text/html")){
		tQueue *dir=new tQueue;
		tHtmlParser *html=new tHtmlParser;
		pthread_cleanup_push(_html_parser_dir_destroy_,dir);
		pthread_cleanup_push(_html_parser_destroy_,html);
		dir->init(0);
		if (config.change_links){
			char *a=make_path_to_file();
			char *tmppath=sum_strings(a,"/",info->file.get(),
					    info->params.get()?"?":".fl",
					    info->params.get()?info->params.get():NULL,
					    info->params.get()?".fl":NULL,
					    NULL);
			delete[] a;
			html->out_fd=open(tmppath,O_RDWR|O_CREAT|O_TRUNC,S_IRUSR | S_IWUSR );
			html->leave=config.leave_server;
			html->parse(WL,dir,info);
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
			html->parse(WL,dir,info);
		};
		pthread_cleanup_pop(1);
		if (config.http_recurse_depth!=1){
			convert_list_to_dir2(dir);
		};
		pthread_cleanup_pop(1);
	};
};

void tDownload::export_socket(tDownloader *what){
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

void tDownload::http_check_redirect(){
	char *newurl=newurl=who->get_new_url();
	if (config.change_links && config.http_recurse_depth==1 &&
	    config.http_recursing==0){
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
					     config.leave_server);
		if (tmpadr) delete(tmpadr);
		WL->write(redirect_html_end,strlen(redirect_html_end));
		who->done();
		make_file_visible();
	}else{
		who->done();
		delete_file();
	};
	delete[] newurl;
	tAddr *addr=redirect_url();
	finfo.type=T_REDIRECT;
	if (addr){
		if (config.http_recursing ||config.http_recurse_depth!=1){
			if (!equal_uncase(addr->host.get(),info->host.get()) &&
			    config.leave_server){
				finfo.type=T_REDIRECT;
			}else{
				finfo.type=T_FILE;
//				WL->log(LOG_WARNING,"Redirection blocked by '%s'",_());
			};
		};
		d4xFilter *filter=config.Filter.get()?FILTERS_DB->find(config.Filter.get()):NULL;
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

void tDownload::download_http() {
	if (!who) who=new tHttpDownload(WL);
	config.split=split?1:0;
	if (who->init(info,&(config))) {
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
	if (split && !im_first)
		CurentSize=split->FirstByte;
	who->set_loaded(CurentSize);
	who->rollback();
	int size=who->get_size();
	/* In the case if file already loaded
	 */
	if (size==CurentSize && size>0 && config.rollback==0) {
		check_local_file_time();
		if (!who->remote_file_changed()){
			finfo.size=size;
			finfo.type=T_FILE;
			download_completed(D_PROTO_HTTP);
			return;
		};
	};
	/* There are must be procedure for removing file
	 * wich execute if CurentSize==0
	 */
	if (size==-1) {
		http_check_redirect();
		status=DOWNLOAD_COMPLETE;
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
		prepare_splits();
	};
	check_local_file_time();
	int SIZE_FOR_DOWNLOAD= (split && split->LastByte>0)?split->LastByte-split->FirstByte:0;
	Start=Pause=time(NULL);
	Difference=0;
	status=DOWNLOAD_GO;
	if (who->download(SIZE_FOR_DOWNLOAD)) {
		download_failed();
		return;
	};
	download_completed(D_PROTO_HTTP);
};

void tDownload::remove_links(){
	tDownload *tmp=DIR->last();
	while(tmp){
		tDownload *nexttmp=DIR->next();
		if (equal(tmp->info->file.get(),info->file.get())==0 ||
		    tmp->info->proto!=D_PROTO_FTP){
			DIR->del(tmp);
			delete(tmp);
		};
		tmp=nexttmp;
	};
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
	if (DIR->count()==0) return;
	WL->log(LOG_OK,_("Sorting started"));
	int i=0;
	while (i<CFG.SEARCH_PING_TIMES){
		WL->log_printf(LOG_OK,_("Pinging (atempt %i of %i)"),i+1,CFG.SEARCH_PING_TIMES);
		if (!Status.curent){ //clear previous percentage for non comulative ping
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
			Status.curent=1;
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
	config.number_of_attempts=5;
	while(tmp){
		tDownload *nexttmp=DIR->next();
		who=new tFtpDownload(WL);
		if (who->init(tmp->info,&(config))){
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
		tAddr *tmpinfo=new tAddr;
		pthread_cleanup_push(_tmp_info_remove_,tmpinfo);
		tmpinfo->proto=D_PROTO_HTTP;
		tmpinfo->port=get_port_by_proto(tmpinfo->proto);
		config.change_links=0;
		char data[MAX_LEN];
		switch (CFG.SEARCH_HOST){
		case 1:
			tmpinfo->host.set("www.filesearch.ru");
			tmpinfo->path.set("cgi-bin");
			tmpinfo->file.set("s");
			if (finfo.size>0)
				sprintf(data,"q=%s&w=a&t=f&e=on&m=%i&o=n&s1=%li&s2=%li&d=&p=&p2=&x=24&y=12",
					info->file.get(),
					CFG.SEARCH_ENTRIES,
					finfo.size,finfo.size);
			else
				sprintf(data,"q=%s&w=a&t=f&e=on&m=%i&o=n&d=&p=&p2=&x=24&y=12",
					info->file.get(),
					CFG.SEARCH_ENTRIES);
			tmpinfo->params.set(data);
			break;
		default:
			tmpinfo->host.set("download.lycos.com");
			tmpinfo->path.set("swadv");
			tmpinfo->file.set("AdvResults.asp");
			if (finfo.size>0)
				sprintf(data,"form=advanced&query=%s&doit=Search&type=Case+sensitive+glob+search&hits=%i&limsize1=%li&limsize2=%li",
					info->file.get(),
					CFG.SEARCH_ENTRIES,
					finfo.size,finfo.size);
			else
				sprintf(data,"form=advanced&query=%s&doit=Search&type=Case+sensitive+glob+search&hits=%i",
					info->file.get(),
					CFG.SEARCH_ENTRIES);
			tmpinfo->params.set(data);
		};
		if (who->init(tmpinfo,&(config))) {
			download_failed();
			return;
		};
		who->init_download(tmpinfo->path.get(),tmpinfo->file.get());
		who->set_loaded(0);
		int size=who->get_size();
		if (size<=-1) {
			WL->log(LOG_WARNING,_("Searching failed"));
			download_failed();
			return;
		};
		finfo.type=T_FILE;
		Start=Pause=time(NULL);
		Difference=0;
		status=DOWNLOAD_GO;
		if (who->download(0)) {
			download_failed();
			return;
		};
		pthread_cleanup_pop(1);
		config.http_recurse_depth=2;
		config.leave_server=1;
		http_recurse();
		who->done();
		remove_links();
	};
	if (finfo.size<0 && DIR->count()>0)
		ftp_search_sizes();
	sort_links();
	WL->log(LOG_OK,_("Search had been completed!"));
	status=DOWNLOAD_COMPLETE;
};


void tDownload::download_ftp(){
	WL->log(LOG_WARNING,_("Was Started!"));
	if (!who) who=new tFtpDownload(WL);

	if (finfo.type==T_LINK && !config.link_as_file) {
		WL->log(LOG_WARNING,_("It is a link and we already load it"));
		who->init_download(NULL,info->file.get());
		who->set_file_info(&finfo);
		create_file();
		set_date_file();
		download_completed(D_PROTO_FTP);
		return;
	};

	config.split=split?1:0;
	download_set_block(1);
	tSocket *s=GVARS.SOCKETS->find(info);
	download_set_block(0);
	if (who->init(info,&(config),s)) {
		download_failed();
		return;
	};
	who->init_download(info->path.get(),info->file.get());
	if (finfo.size<0 || finfo.type==T_NONE) {
		status=DOWNLOAD_SIZE_WAIT;
		int size=who->get_size();
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
	if (finfo.type==T_LINK)
		finfo.size=0;
	int CurentSize=0;
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
	
	if (split){
		if (im_first)
			prepare_splits();
		CurentSize=split->FirstByte;
	};
	check_local_file_time();
	who->set_loaded(CurentSize);
	if (split) WL->shift(CurentSize);
	int SIZE_FOR_DOWNLOAD= (split && split->LastByte>0)?split->LastByte-split->FirstByte:0;
	Start=Pause=time(NULL);
	Difference=0;
	status=DOWNLOAD_GO;
	if (who->download(SIZE_FOR_DOWNLOAD)) {
		download_failed();
		return;
	};
	if (config.dont_send_quit) export_socket(who);
	download_completed(D_PROTO_FTP);
};

#define SPLIT_MINIMUM_PART 2048

void tDownload::prepare_splits(){
 	DBC_RETURN_IF_FAIL(split!=NULL);
	DBC_RETURN_IF_FAIL(segments!=NULL);
	tSegment *holes=segments->to_holes(finfo.size);
	tSegment *tmp;
//	printf("split to %i parts\n",split->NumOfParts);
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
	split->cond=new d4xCondition;
	if (split->NumOfParts<holes->offset_in_file)
		split->NumOfParts=holes->offset_in_file;
	split->cond->set_value(split->NumOfParts);
	split->FirstByte=holes->begin;
	split->LastByte=holes->end;
//	printf("%li %li\n",split->FirstByte,split->LastByte);
	tmp=holes->next;
	delete(holes);
	holes=tmp;
	tSplitInfo *newsplit=split;
	tDownload *parent=this;
	char i='1';
	while(holes){
		tmp=holes->next;
		if (newsplit->next_part==NULL)
			newsplit->next_part=new tDownload;
		newsplit->cond=split->cond;
		tDownload *temp=newsplit->next_part;
		temp->status=DOWNLOAD_REAL_STOP;
		if (temp->split==NULL)
			temp->split=new tSplitInfo;
		newsplit=temp->split;
		newsplit->NumOfParts=split->NumOfParts;
		temp->split->parent=parent;
		parent=temp;
		temp->split->status=0;
		temp->split->FirstByte=holes->begin;
		temp->split->LastByte=holes->end;
//		printf("%li %li\n",newsplit->FirstByte,newsplit->LastByte);
		temp->segments=segments;
		temp->config.copy(&config);
		if (config.log_save_path.get()){
			char *tmppath=sum_strings(config.log_save_path.get(),"_ ",
						  NULL);
			tmppath[strlen(tmppath)-1]=i;
			temp->config.log_save_path.set(tmppath);
		};
		i+=1;
		temp->config.speed=(config.speed/split->NumOfParts)*temp->split->NumOfParts;
		temp->config.user_agent.set(config.user_agent.get());
		temp->config.referer.set(config.referer.get());
		temp->config.save_name.set(config.save_name.get());
		temp->config.save_path.set(config.save_path.get());
		temp->finfo.size=finfo.size;
		temp->finfo.type=finfo.type;
		temp->finfo.perm=finfo.perm;
		temp->finfo.date=finfo.date;
		if (temp->info==NULL)
			temp->info=new tAddr();
		temp->info->copy_host(info);
		temp->info->path.set(info->path.get());
		temp->info->file.set(info->file.get());
		temp->info->params.set(info->params.get());
		delete(holes);
		holes=tmp;
	};
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

tDownload::~tDownload() {
	if (myowner) myowner->del(this);
	if (who) delete who;
	if (info) delete info;
	if (editor) delete editor;
	if (LOG) LOG->ref_dec();
	if (DIR) delete DIR;
	if (WL) delete(WL);
	if (split) delete(split);
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
	if (Pixmap!=PIX_UNKNOWN)
		list_of_downloads_set_pixmap(what,Pixmap);
};

void tDList::insert_before(tDownload *what,tDownload *where) {
	DBC_RETURN_IF_FAIL(where!=NULL);
	DBC_RETURN_IF_FAIL(where->myowner==this);
	tQueue::insert_before(what,where);
	what->myowner=this;
	if (Pixmap!=PIX_UNKNOWN)
		list_of_downloads_set_pixmap(what,Pixmap);
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

void tDList::dispose(){
	ALL_DOWNLOADS->del((tDownload *)First);
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
