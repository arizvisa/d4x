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
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "pass.h"
#include "addr.h"
#include "locstr.h"
#include "var.h"

tUserPass::tUserPass(){
	host=user=pass=NULL;
	proto=D_PROTO_UNKNOWN;
};

tUserPass::~tUserPass(){
	if (host) delete(host);
	if (user) delete(user);
	if (pass) delete(pass);
};

void tUserPass::set_host(char *a){
	if (host) delete (host);
	host=copy_string(a);
};
void tUserPass::set_user(char *a){
	if (user) delete (user);
	user=copy_string(a);
};
void tUserPass::set_pass(char *a){
	if (pass) delete (pass);
	pass=copy_string(a);
};

char *tUserPass::get_host(){return(host);};
char *tUserPass::get_pass(){return(pass);};
char *tUserPass::get_user(){return(user);};

void tUserPass::print(){
	if (host) printf("host:%s://%s\n",get_name_by_proto(proto),host);
	if (user) printf("user:%s\n",user);
	if (pass) printf("pass:%s\n",pass);
};

void tUserPass::save(int fd){
	write(fd,"NewHost:\n",strlen("NewHost:\n"));
	write_named_integer(fd,"proto:",proto);
	write_named_string(fd,"host:",host);
	write_named_string(fd,"user:",user);
	write_named_string(fd,"pass:",pass);
	write(fd,"EndHost:\n",strlen("EndHost:\n"));
};

int tUserPass::load(int fd){
	char *a[]={
		"host:",
		"user:",
		"pass:",
		"proto:",
		"EndHost:"
	};
	char buf[MAX_LEN];
	while(f_rstr(fd,buf,MAX_LEN)>0){
		unsigned int i;
		for (i=0;i<sizeof(a)/sizeof(char *);i++){
			if (equal_uncase(buf,a[i])) break;
		};
		switch(i){
		case 0:{
			if (f_rstr(fd,buf,MAX_LEN)<0) return -1;
			set_host(buf);
			break;
		};
		case 1:{
			if (f_rstr(fd,buf,MAX_LEN)<0) return -1;
			set_user(buf);
			break;
		};
		case 2:{
			if (f_rstr(fd,buf,MAX_LEN)<0) return -1;
			set_pass(buf);
			break;
		};
		case 3:{
			if (f_rstr(fd,buf,MAX_LEN)<0) return -1;
			sscanf(buf,"%i",&proto);
			break;
		};
		case 4:{
			return 0;
		};
		};
	};
	return 1;
};
/*
 */

int tUserPassTree::compare_nodes(tAbstractSortNode *a,tAbstractSortNode *b){
	int r=strcmp(((tUserPass*)a)->get_host(),((tUserPass*)b)->get_host());
	if (r) return r;
	return(((tUserPass*)a)->proto - ((tUserPass*)b)->proto);
};

void tUserPassTree::save_node(tUserPass *node,int fd){
	if (node->less) save_node((tUserPass *)(node->less),fd);
	if (node->more) save_node((tUserPass *)(node->more),fd);
	node->save(fd);
};

void tUserPassTree::fill_face_node(tUserPass *node,tFacePass *a){
	if (node->less) fill_face_node((tUserPass *)(node->less),a);
	if (node->more) fill_face_node((tUserPass *)(node->more),a);
	a->add(node);
};

tUserPass *tUserPassTree::find(int proto,char *host){
	tUserPass *tmp=new tUserPass;
	tmp->set_host(host);
	tmp->proto=proto;
	tUserPass *found=(tUserPass *)(tAbstractSortTree::find(tmp));
	delete(tmp);
	return(found);
};

void tUserPassTree::save(int fd){
	if (Top) save_node((tUserPass *)Top,fd);
};

void tUserPassTree::fill_face(tFacePass *a){
	if (Top) fill_face_node((tUserPass *)Top,a);
};

/*
 */

char *CFG_PASSWORDS="passwords";

tUserPassTree *load_passwords(){
	tUserPassTree *tree=new(tUserPassTree);
	if (!HOME_VARIABLE) return tree;
	char *cfgpath=sum_strings(HOME_VARIABLE,"/",CFG_DIR,"/",CFG_PASSWORDS,NULL);
	int fd=open(cfgpath,O_RDONLY);
	delete(cfgpath);
	if (fd>=0){
		char buf[MAX_LEN];
		while(f_rstr(fd,buf,MAX_LEN)>0){
			if (equal_uncase(buf,"NewHost:")){
				tUserPass *tmp=new(tUserPass);
				if (tmp->load(fd))
					delete(tmp);
				else
					tree->add(tmp);
			};
		};
		close(fd);
	};
	return tree;
};

void save_passwords(tUserPassTree *tree){
	if (!HOME_VARIABLE) return;
	char *cfgpath=sum_strings(HOME_VARIABLE,"/",CFG_DIR,"/",CFG_PASSWORDS,NULL);
	int fd=open(cfgpath,O_TRUNC | O_CREAT |O_RDWR,S_IRUSR | S_IWUSR);
	delete(cfgpath);
	if (fd>=0){
		tree->save(fd);
		close(fd);
	};
};

