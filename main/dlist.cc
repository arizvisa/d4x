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
	if (fcntl(fd,F_SETLK,&a)==-1)
		return(-1);
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
	while (temp){
//		temp->print();
		if (begin_string(path,temp->path.get())){
			char *tmp = request_string==NULL?copy_string(""):request_string;
			request_string=sum_strings(tmp, temp->name.get(),
						   "=", temp->value.get(),
						   ";", NULL);
			delete(tmp);
		};
		temp=(tCookie *)(temp->next);
	};
	return(request_string);
};


tDefaultWL::~tDefaultWL(){
	fd_close();
};

/*************Split Information****************/

tSplitInfo::tSplitInfo(){
	FirstByte=LastByte=-1;
	next_part=parent=NULL;
};

tSplitInfo::~tSplitInfo(){
	if (next_part)	delete(next_part);
};

/**********************************************/
tDownload::tDownload() {
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
	owner=DL_ALONE;
	thread_id=0;
	GTKCListRow=-1;
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
			delete(name);
			name=tmp;
		};
		delete(path);
		remove(name);
		char *segname=sum_strings(name,".segments",NULL);
		delete(name);
		remove(segname);
		delete(segname);
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
		delete name;
		delete guess;
	};
	return rvalue;
};

void tDownload::make_file_names(char **name, char **guess){
	DBC_RETURN_IF_FAIL(name!=NULL);
	DBC_RETURN_IF_FAIL(guess!=NULL);
	if (config.save_name.get() && strlen(config.save_name.get()))
		who->make_full_pathes(config.save_path.get(),
				 config.save_name.get(),
				 name,guess);
	else
		who->make_full_pathes(config.save_path.get(),name,guess);
};

fsize_t tDownload::init_segmentator(int fdesc,fsize_t cursize,char *name){
	fsize_t rvalue=cursize;
	im_first=0;
	if (segments==NULL){
		/*trying to lock*/
		if (d4x_f_lock(fdesc)){
			WL->log(LOG_ERROR,_("File is already opened by another download!"));
			close(fdesc);
			return(-1);
		};
		/*end of trying */
		segments=new tSegmentator;
		char *segname=sum_strings(name,".segments",NULL);
		segments->init(segname);
		delete(segname);
		im_first=1;
		tSegment *first_seg=segments->get_first();
		if (first_seg && (unsigned long int)rvalue<first_seg->end){
			WL->log(LOG_WARNING,"Segmentation info is wrong!");
			rvalue=0;
			ftruncate(fdesc,0);
			segments->truncate(0);
		}else{
			if ((first_seg==NULL || first_seg->next==NULL) &&
			    rvalue>0){
				segments->insert(0,(unsigned long int)rvalue);
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
		if (config.restart_from_begin)
			fdesc=open(guess,O_RDWR|O_TRUNC,S_IRUSR | S_IWUSR );
		else
			fdesc=open(guess,O_RDWR,S_IRUSR | S_IWUSR );
		if (fdesc<0) {
			if (config.restart_from_begin)
				fdesc=open(name,O_RDWR|O_CREAT|O_TRUNC,S_IRUSR | S_IWUSR );
			else
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
	delete name;
	delete guess;
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
		delete name;
		delete guess;
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
			delete oldname;
			delete newname;
		};
	};
};

void tDownload::set_default_cfg(){
	config.copy_ints(&(CFG.DEFAULT_CFG));
	config.http_recursing=config.http_recurse_depth==1?0:1;
	config.user_agent.set(CFG.USER_AGENT);
};

char *tDownload::make_path_to_file(){
	if (info && info->proto==D_PROTO_HTTP && config.http_recursing){
		if (config.leave_server){
			return(sum_strings(config.save_path.get(),"/",
					   info->host.get(),"/",
					   info->path.get(),"/",NULL));
		};
		return(sum_strings(config.save_path.get(),"/",
				   info->path.get(),"/",NULL));
	};
	return(sum_strings(config.save_path.get(),"/",NULL));
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
				delete(SavePath);
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
			if (addrnew->is_valid())
				DIR->insert(onenew);
			else
				delete(onenew);
		};
		dir->del(temp);
		delete(temp);
		temp=dir->last();
		prom->name.set(NULL);
		prom->body.set(NULL);
	};
	delete (prom);
	delete (path);
	delete (savepath);
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

void tDownload::convert_list_to_dir2(tStringList *dir) {
	if (!dir) {
		return;
	};
	if (DIR) {
		DIR->done();
		DIR->init(0);
	} else {
		DIR=new tDList(DL_TEMP);
		DIR->init(0);
	};
	tString *temp=dir->last();
	char *URL=info->url();
	while (temp) {
		if (!global_url(temp->body)) {
			tAddr *addrnew=new tAddr;
			tDownload *onenew=new tDownload;
			char *quest=index(temp->body,'?');
			if (quest){
				addrnew->params.set(quest+1);
				*quest=0;
			};
			if (info->proto==D_PROTO_FTP){
				quest=index(temp->body,';');
				if (quest)
					*quest=0;
			};
			/* %xx -> CHAR */
			char *tmp=parse_percents(temp->body);
			delete(temp->body);
			temp->body=tmp;
			/* end of small hack */
			tmp=rindex(temp->body,'/');
			onenew->config.save_path.set(config.save_path.get());
			if (tmp) {
				addrnew->file.set(tmp+1);
				*tmp=0;
				if (temp->body[0]=='/')
					addrnew->path.set(temp->body+1);
				else{
					addrnew->compose_path(info->path.get(),temp->body);
				};
				*tmp='/';
			} else {
				addrnew->path.set(info->path.get());
				addrnew->file.set(temp->body);
			};
/*
			if (addrnew->path[0]!='/'){
				tmp=compose_path("/",addrnew->path);
				delete(addrnew->path);
				addrnew->path=tmp;
			};
 */
			addrnew->file_del_sq();
			if (onenew->config.save_path.get())
				normalize_path(onenew->config.save_path.get());
			addrnew->copy_host(info);

			onenew->config.http_recursing=1;
			onenew->info=addrnew;

			onenew->config.copy(&config);
			onenew->config.http_recurse_depth = config.http_recurse_depth ? config.http_recurse_depth-1 : 0;
			onenew->config.ftp_recurse_depth = config.ftp_recurse_depth;
			onenew->config.referer.set(URL);

			if (addrnew->is_valid() && http_check_settings(addrnew))
				DIR->insert(onenew);
			else
				delete(onenew);
		}else{
			if (begin_string_uncase(temp->body,"http://") ||
			    begin_string_uncase(temp->body,"ftp://")){
				tAddr *addrnew=new tAddr(temp->body);
				if (info->proto==D_PROTO_FTP && addrnew->proto==D_PROTO_FTP){
					char *quest=index(addrnew->file.get(),';');
					if (quest)
						*quest=0;
				};
				if (addrnew->is_valid() && http_check_settings(addrnew)){
					tDownload *onenew=new tDownload;
					onenew->config.save_path.set(config.save_path.get());
					onenew->config.http_recursing=1;
					onenew->info=addrnew;
					
					onenew->config.copy(&config);
					onenew->config.referer.set(URL);
					onenew->config.http_recurse_depth = config.http_recurse_depth ? config.http_recurse_depth-1 : 0;
					onenew->config.ftp_recurse_depth = config.ftp_recurse_depth;
						DIR->insert(onenew);
				}else
					delete addrnew;
			};
		};
		dir->del(temp);
		delete temp;
		temp=dir->last();
	};
	delete(URL);
};

void tDownload::save_to_config(int fd){
	f_wstr_lf(fd,"Download:");
	if (info) info->save_to_config(fd);
	if (split)
		write_named_integer(fd,"SplitTo:",split->NumOfParts);
	config.save_to_config(fd);
	if (ScheduleTime)
		write_named_time(fd,"Time:",ScheduleTime);
	write_named_integer(fd,"State:",owner);
	if (Description.get())
		write_named_string(fd,"Description:",Description.get());
	f_wstr_lf(fd,"EndDownload:");
};

int tDownload::load_from_config(int fd){
	tSavedVar table_of_fields[]={
		{"Url:",	SV_TYPE_URL,	info},
		{"State:",	SV_TYPE_INT,	&owner},
		{"Time:",	SV_TYPE_TIME,	&ScheduleTime},
		{"Cfg:",	SV_TYPE_CFG,	&config},
		{"SavePath:",	SV_TYPE_PSTR,	&(config.save_path)},
		{"SaveName:",	SV_TYPE_PSTR,	&(config.save_name)},
		{"SplitTo:",	SV_TYPE_SPLIT,	NULL},
		{"Description:",SV_TYPE_PSTR,	&(Description)},
		{"EndDownload:",SV_TYPE_END,	NULL}
	};
	char buf[MAX_LEN];
	while(f_rstr(fd,buf,MAX_LEN)>0){
		unsigned int i;
		for (i=0;i<sizeof(table_of_fields)/sizeof(tSavedVar);i++){
			if (equal_uncase(buf,table_of_fields[i].name)){
				switch(table_of_fields[i].type){
				case SV_TYPE_INT:
					if (f_rstr(fd,buf,MAX_LEN)<0) return -1;
					sscanf(buf,"%d",(int *)(table_of_fields[i].where));
					break;
				case SV_TYPE_PSTR:
					if (f_rstr(fd,buf,MAX_LEN)<0) return -1;
					((tPStr *)(table_of_fields[i].where))->set(buf);
					break;
				case SV_TYPE_URL:
					if (f_rstr(fd,buf,MAX_LEN)<0) return -1;
					if (info) delete(info);
					info=new tAddr(buf);
					break;
				case SV_TYPE_TIME:
					if (f_rstr(fd,buf,MAX_LEN)<0) return -1;
					sscanf(buf,"%ld",(time_t *)(table_of_fields[i].where));
					break;
				case SV_TYPE_CFG:
					if (config.load_from_config(fd)<0) return -1;
					break;
				case SV_TYPE_SPLIT:
					if (f_rstr(fd,buf,MAX_LEN)<0) return -1;
					int tmp;
					sscanf(buf,"%d",&tmp);
					if (tmp>10) tmp=10;
					if (tmp>1){
						split=new tSplitInfo;
						split->NumOfParts=tmp;
					};
					break;
				case SV_TYPE_END:
					return info==NULL?-1:0;
				};
				break;
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

void tDownload::download_completed(int type) {
	who->done();
	if (split==NULL)
		WL->truncate();
	switch (type){
	case D_PROTO_HTTP:{
		recurse_http();
		break;
	};
	case D_PROTO_FTP:{
		if (finfo.type==T_DIR && config.ftp_recurse_depth!=1)
			convert_list_to_dir();
	};
	};
	WL->log(LOG_OK,_("Downloading was successefully completed!"));
	if (im_first && CFG.WRITE_DESCRIPTION && info->proto!=D_PROTO_SEARCH){
		/* add string into Descript.ion file */
		download_set_block(1);
		char *path=make_path_to_file();
		char *desc=sum_strings(path,"Descript.ion",NULL);
		delete(path);
		int fd=open(desc,O_WRONLY|O_CREAT,S_IRUSR | S_IWUSR );
		delete(desc);
		lseek(fd,0,SEEK_END);
		lockf(fd,F_LOCK,0);
		f_wstr(fd,info->file.get());
		f_wstr(fd," - ");
		if (Description.get())
			f_wstr(fd,Description.get());
		else{
			info->save_to_description(fd);
		};
		f_wstr(fd,"\n");
		lockf(fd,F_ULOCK,0);
		close(fd);
		download_set_block(0);
	};
	make_file_visible();
	set_date_file();
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

void tDownload::recurse_http() {
	tHttpDownload *httpd=(tHttpDownload *)(who);
	char *type=httpd->get_content_type();
	if (config.http_recurse_depth!=1 && type &&
	    begin_string_uncase(type,"text/html")){
		tHtmlParser html;
		tStringList *dir=new tStringList;
		dir->init(0);
		html.parse(WL,dir);
		convert_list_to_dir2(dir);
		delete(dir);
	};
};


void tDownload::download_http() {
	if (!who) who=new tHttpDownload;
	config.split=split?1:0;
	if (who->init(info,WL,&(config))) {
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
	
	if (size<0 && split==NULL && CurentSize==0) {
		if (delete_file())
			WL->log(LOG_WARNING,_("It is strange that we can't delete file which just created..."));
	};

	if (size<-1) {
		WL->log(LOG_WARNING,_("File not found"));
		download_failed();
		return;
	};
	if (size==-1) {
		finfo.type=T_REDIRECT;
		who->done();
		status=DOWNLOAD_COMPLETE;
		WL->log(LOG_WARNING,_("Redirect detected..."));
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

void tDownload::sort_links(){
	WL->log(LOG_OK,_("Sorting started"));
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
	tDownload *a=DIR->first();
	WL->log(LOG_OK,"Sorting");
	while (a && a->prev){
		tDownload *prev=(tDownload *)(a->prev);
		if (a->Percent/a->Attempt.curent>prev->Percent/prev->Attempt.curent){
			DIR->del(a);
			DIR->insert(a);
			a=DIR->first();
		}else{
			a=prev;
		};
	};
	download_set_block(0);
};

static void _tmp_info_remove_(void *addr){
	tAddr *info=(tAddr *)addr;
	delete(info);
};

void tDownload::ftp_search() {
	/* FIXME: prepare new url for ftp search */
	if (action!=ACTION_REPING){
		tAddr *tmpinfo=new tAddr;
		pthread_cleanup_push(_tmp_info_remove_,tmpinfo);
		tmpinfo->proto=D_PROTO_HTTP;
		tmpinfo->port=get_port_by_proto(tmpinfo->proto);
		tmpinfo->host.set("ftpsearch.lycos.com");
		tmpinfo->path.set("cgi-bin");
		tmpinfo->file.set("search");
		char data[MAX_LEN];
		sprintf(data,"form=advanced&query=%s&doit=Search&type=Case+sensitive+glob+search&hits=15&limsize1=%li&limsize2=%li",
			info->file.get(),finfo.size,finfo.size);
		tmpinfo->params.set(data);
		if (who->init(tmpinfo,WL,&(config))) {
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
		recurse_http();
		who->done();
		remove_links();
	};
	sort_links();
	WL->log(LOG_OK,_("Search had been completed!"));
	status=DOWNLOAD_COMPLETE;
};


void tDownload::download_ftp(){
	WL->log(LOG_WARNING,_("Was Started!"));
	if (!who) who=new tFtpDownload;

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
	if (who->init(info,WL,&(config))) {
		download_failed();
		return;
	};
	who->init_download(info->path.get(),info->file.get());
	if (finfo.size<0) {
		status=DOWNLOAD_SIZE_WAIT;
		int size=who->get_size();
		if (size<0) {
			WL->log(LOG_ERROR,_("File not found"));
			download_failed();
			return;
		};
		finfo.size=size;
		finfo.type=file_type();
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
	download_completed(D_PROTO_FTP);
};

#define SPLIT_MINIMUM_PART 2048

void tDownload::prepare_splits(){
 	DBC_RETURN_IF_FAIL(split!=NULL);
	DBC_RETURN_IF_FAIL(segments!=NULL);
	tSegment *holes=segments->to_holes(finfo.size);
	tSegment *tmp;
	if (split->NumOfParts>holes->offset_in_file){
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
			tmp->begin=largest->begin;
			tmp->end=largest->begin+(largest->end-largest->begin)/(split->NumOfParts-holes->offset_in_file+1);
			largest->begin=tmp->end;
			if (tmp->begin<holes->begin){
				tmp->offset_in_file=holes->offset_in_file+1;
				tmp->next=holes;
				holes=tmp;
			}else{
				holes->offset_in_file+=1;
				tmp->next=holes->next;
				holes->next=tmp;
			};
		};
	};
	if (split->NumOfParts<holes->offset_in_file)
		split->NumOfParts=holes->offset_in_file;
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
		tDownload *temp=newsplit->next_part;
		if (temp->split==NULL)
			temp->split=new tSplitInfo;
		newsplit=temp->split;
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
		delete who;
		who=NULL;
	};
};

tDownload::~tDownload() {
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

int tDList::owner(tDownload *which) {
	if (which && which->owner==OwnerKey) return 1;
	return 0;
};

void tDList::insert(tDownload *what) {
	DBC_RETURN_IF_FAIL(what!=NULL);
	if (Num==0 && non_empty!=NULL)
		non_empty();
	tQueue::insert(what);
	what->owner=OwnerKey;
	if (Pixmap!=PIX_UNKNOWN)
		list_of_downloads_set_pixmap(what->GTKCListRow,Pixmap);
};

void tDList::insert_before(tDownload *what,tDownload *where) {
	DBC_RETURN_IF_FAIL(where!=NULL);
	DBC_RETURN_IF_FAIL(where->owner==OwnerKey);
	tQueue::insert_before(what,where);
	what->owner=OwnerKey;
	if (Pixmap!=PIX_UNKNOWN)
		list_of_downloads_set_pixmap(what->GTKCListRow,Pixmap);
};

void tDList::del(tDownload *what) {
	DBC_RETURN_IF_FAIL(what->owner==OwnerKey);
	if (Num==1 && empty!=NULL)
		empty();
	tQueue::del(what);
	what->owner=DL_ALONE;
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
