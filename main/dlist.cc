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

#include "dlist.h"
#include "ftpd.h"
#include "locstr.h"
#include "string.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "face/edit.h"
#include "var.h"
#include "ntlocale.h"

tAddr::tAddr() {
	protocol=host=username=pass=path=file=NULL;
	port=0;
	mask=0;
};

void tAddr::print() {
	if (protocol) printf("protocol: %s\n",protocol);
	if (host) printf("host: %s\n",host);
	if (path) printf("path: %s\n",path);
	if (file) printf("file: %s\n",file);
	if (username) printf("username: %s\n",username);
	if (pass) printf("pass: %s\n",pass);
	printf("port: %i\n",port);
};

tAddr::~tAddr() {
	if (protocol) delete(protocol);
	if (path) delete(path);
	if (pass) delete(pass);
	if (username) delete(username);
	if (host) delete(host);
};

/**********************************************/
void make_url_from_download(tDownload *what,char *where) {
	*where=0;
	strcat(where,what->info->protocol);
	strcat(where,"://");
	if (what->info->username && !equal(what->info->username,"anonymous")) {
		strcat(where,what->info->username);
		strcat(where,":");
		if (what->info->pass) strcat(where,what->info->pass);
		strcat(where,"@");
	};
	strcat(where,what->info->host);
	if ((equal(what->info->protocol,"ftp")  && what->info->port!=21) ||
	        (equal(what->info->protocol,"http") && what->info->port!=80)) {
		char data[MAX_LEN];
		sprintf(data,":%i",what->info->port);
		strcat(where,data);
	};
	strcat(where,what->info->path);
	if (what->info->path[strlen(what->info->path)-1]!='/')
		strcat(where,"/");
	strcat(where,what->info->file);
};

char *make_simply_url(tDownload *what) {
	char *URL=new char[strlen(what->info->protocol)+strlen(what->info->host)+
	                   strlen(what->info->path)+strlen(what->info->file)+6];
	*URL=0;
	/* Easy way to make URL from info  field
	 */
	strcat(URL,what->info->protocol);
	strcat(URL,"://");
	strcat(URL,what->info->host);
	strcat(URL,what->info->path);
	if (what->info->path[strlen(what->info->path)-1]!='/')
		strcat(URL,"/");
	strcat(URL,what->info->file);
	return URL;
};
/**********************************************/
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


/**********************************************/
tDownload::tDownload() {
	next=prev=NULL;
	who=NULL;
	info=NULL;
	LOG=NULL;
	editor=NULL;
	SaveName=SavePath=NULL;
	config.ftp_recurse_depth=config.http_recurse_depth=1;
	SpeedLimit=NULL;
	finfo.body=NULL;
	finfo.size=-1;
	finfo.type=T_NONE;
	finfo.body=NULL;
	DIR=NULL;
	finfo.perm=S_IWUSR | S_IRUSR;
	Start=Pause=0;
	Percent.clear();
	Attempt.clear();
	Status.clear();
	Size.clear();
	Speed.clear();
	owner=DL_ALONE;
	thread_id=0;
	GTKCListRow=-1;
	NanoSpeed=0;
	DIR=NULL;
	action=ACTION_NONE;
	ScheduleTime=0;
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

void tDownload::print() {
	//do nothing
}
;

void tDownload::set_SavePath(char *what) {
	if (SavePath) delete(SavePath);
	SavePath=copy_string(what);
};

void tDownload::set_SaveName(char *what) {
	if (SaveName) delete SaveName;
	SaveName=copy_string(what);
};

int tDownload::create_file() {
	if (!who) return -1;
	return (who->create_file(SavePath,SaveName));
};

void tDownload::set_date_file() {
	if (!who) return;
	who->set_date_file(SavePath,SaveName);
};

void tDownload::update_trigers() {
	Percent.update();
	Speed.update();
	Status.update();
	Size.update();
	Attempt.update();
};

void tDownload::convert_list_to_dir() {
	if (!who) {
		return;
	};
	tFtpDownload *tmp=(tFtpDownload *)(who);
	tStringList *dir=tmp->dir();
	if (!dir) {
		return;
	};
	tString *temp=dir->first();
	if (!temp) {
		return;
	};
	if (DIR) {
		DIR->done();
	} else {
		DIR=new tDList(DL_TEMP);
		DIR->init(0);
	};
	char *path,*savepath;
	if (info->mask==0) {
		int len=strlen(info->path);
		/* Here we make new file's path
		 */
		path=compose_path(info->path,info->file);
		/* Here we make new path where files will be saved
		 */
		if (SavePath) {
			len=strlen(SavePath);
			if (SaveName && strlen(SaveName))
				savepath=compose_path(SavePath,SaveName);
			else {
				savepath=compose_path(SavePath,info->file);
			};
		} else {
			if (SaveName && strlen(SaveName))
				savepath=copy_string(SaveName);
			else
				savepath=copy_string(info->file);
		};
	} else {
		path=copy_string(info->path);
		savepath=copy_string(SavePath);
		normalize_path(savepath);
		normalize_path(path);
	};
	temp=dir->last();
	while (temp && !begin_string(temp->body,"total")) {
		tFileInfo prom;
		prom.name=NULL;
		prom.body=NULL;
		cut_string_list(temp->body,&prom,1);
		if (!equal(prom.name,".") && !equal(prom.name,"..") && (prom.type!=T_DIR || config.ftp_recurse_depth!=2)
		        && (prom.type==T_DIR || info->mask==0 || check_mask(prom.name,info->file))) {
			tAddr *addrnew=new tAddr;
			tDownload *onenew=new tDownload;
			if (prom.type==T_DIR && info->mask && prom.name) {
				addrnew->path=compose_path(path,prom.name);
				addrnew->file=copy_string(info->file);
				onenew->SavePath=compose_path(savepath,prom.name);
				delete prom.name;
				prom.name=NULL;
				mkdir(onenew->SavePath,prom.perm);
				addrnew->mask=info->mask;
			} else {
				addrnew->path=copy_string(path);
				addrnew->file=prom.name;
				onenew->SavePath=copy_string(savepath);
			};
			addrnew->protocol=copy_string(info->protocol);
			addrnew->host=copy_string(info->host);
			addrnew->pass=copy_string(info->pass);
			addrnew->username=copy_string(info->username);
			addrnew->port=info->port;

			onenew->info=addrnew;

			onenew->config.copy(&config);
			onenew->config.ftp_recurse_depth = config.ftp_recurse_depth ? config.ftp_recurse_depth-1 : 0;
			onenew->config.http_recurse_depth = config.http_recurse_depth;

			if (CFG.RECURSIVE_OPTIMIZE) {
				onenew->finfo.type=prom.type;
				onenew->finfo.size=prom.size;
				onenew->finfo.date=prom.date;
				if (config.permisions) onenew->finfo.perm=prom.perm;
				if (onenew->finfo.type==T_LINK) {
					onenew->finfo.body=prom.body;
				};
			} else {
				if (prom.body) delete prom.body;
			};
			DIR->insert(onenew);
		} else {
			if (prom.name) delete(prom.name);
			if (prom.body) delete(prom.body);
		};
		dir->del(temp);
		delete temp;
		temp=dir->last();
	};
	delete (path);
	delete (savepath);
};


static void make_dir_hier(char *path) {
	char *temp=path;
	while (temp) {
		temp=index(temp+1,'/');
		if (temp) *temp=0;
		mkdir(path,S_IRWXU);
		if (temp) *temp='/';
	};
};

void tDownload::convert_list_to_dir2() {
	if (!who) {
		return;
	};
	tStringList *dir=who->dir();
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
	while (temp) {
		if (!global_url(temp->body)) {
			tAddr *addrnew=new tAddr;
			tDownload *onenew=new tDownload;
			char *tmp=rindex(temp->body,'/');
			if (tmp) {
				addrnew->file=copy_string(tmp+1);
				*tmp=0;
				if (temp->body[0]=='/'){
					addrnew->path=copy_string(temp->body);
					onenew->SavePath=subtract_path(SavePath,temp->body);
				}else{
					addrnew->path=compose_path(info->path,temp->body);
					onenew->SavePath=compose_path(SavePath,temp->body);
				};
				*tmp='/';
			} else {
				addrnew->path=copy_string(info->path);
				addrnew->file=copy_string(temp->body);
				onenew->SavePath=copy_string(SavePath);
			};
			tmp=index(addrnew->file,'#');
			if (tmp) {
				*tmp=0;
				tmp=addrnew->file;
				addrnew->file=copy_string(tmp);
				delete(tmp);
			};
			normalize_path(onenew->SavePath);
			make_dir_hier(onenew->SavePath);
			addrnew->protocol=copy_string(info->protocol);
			addrnew->host=copy_string(info->host);
			addrnew->pass=copy_string(info->pass);
			addrnew->username=copy_string(info->username);
			addrnew->port=info->port;

			onenew->info=addrnew;

			onenew->config.copy(&config);
			onenew->config.http_recurse_depth = config.http_recurse_depth ? config.http_recurse_depth-1 : 0;
			onenew->config.ftp_recurse_depth = config.ftp_recurse_depth;

			tDownload *download=DIR->last();
			int flag=1;
			while (download) {
				if (equal(download->info->file,onenew->info->file) && equal(download->info->path,onenew->info->path)) {
					flag=0;
					break;
				};
				download=DIR->next();
			};
			if (flag) {
				DIR->insert(onenew);
			} else delete(onenew);
		};
		dir->del(temp);
		delete temp;
		temp=dir->last();
	};
};

tDownload::~tDownload() {
	if (who) delete who;
	if (info) delete info;
	if (LOG) delete LOG;
	if (SavePath) delete SavePath;
	if (SaveName) delete SaveName;
	if (finfo.body) delete finfo.body;
	if (editor) delete editor;
	if (DIR) delete DIR;
};


//**********************************************/

tDList::tDList() {
	set_pixmap=NULL;
};

tDList::tDList(int key) {
	set_pixmap=NULL;
	OwnerKey=key;
};

void tDList::init_set_pixmap(void (*a)(int)){
	set_pixmap=a;
};

int tDList::owner(tDownload *which) {
	if (which && which->owner==OwnerKey) return 1;
	return 0;
};

void tDList::insert(tDownload *what) {
	what->owner=OwnerKey;
	tQueue::insert(what);
	if (set_pixmap) set_pixmap(what->GTKCListRow);
};

void tDList::insert_before(tDownload *what,tDownload *where) {
	what->owner=OwnerKey;
	tQueue::insert_before(what,where);
	if (set_pixmap) set_pixmap(what->GTKCListRow);
};

void tDList::del(tDownload *what) {
	if (owner(what)) tQueue::del(what);
	else puts(_("***WARN***!!!"
		            "Attempt to delete download from list which does'nt contain it!"));
	what->owner=DL_ALONE;
};

void tDList::forward(tDownload *what) {
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

tDownload *tDList::last() {
	return (tDownload *)(Curent=Last);
};

tDownload *tDList::first() {
	return (tDownload *)(Curent=First);
};

tDownload *tDList::next() {
	Curent=Curent->next;
	return (tDownload *)Curent;
};

tDownload *tDList::prev() {
	Curent=Curent->prev;
	return (tDownload *)Curent;
};

tDList::~tDList() {
};
