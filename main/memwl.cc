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
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include "dbc.h"
#include "memwl.h"
#include "ntlocale.h"
#include "string.h"

#define MWL_BLOCK_SIZE 5120
#define MWL_START_SIZE 15360

tMemoryWL::tMemoryWL(){
	cur=bufsize=filesize=0;
	LOG=NULL;
	buf=NULL;
};

tMemoryWL::~tMemoryWL(){
	if (buf) delete[] buf;
};

void tMemoryWL::grow(){
	if (buf){
		fsize_t newsize=bufsize+MWL_BLOCK_SIZE;
		char *newbuf=new char[newsize];
		memcpy(newbuf,buf,bufsize);
		delete[] buf;
		buf=newbuf;
		bufsize=newsize;
	}else{
		bufsize=MWL_START_SIZE;
		buf=new char[bufsize];
	};
};

fsize_t tMemoryWL::write(const void *src,fsize_t len){
	DBC_RETVAL_IF_FAIL(src!=NULL,-1);
	while(bufsize<cur+len)
		grow();
	memcpy(buf+cur,src,len);
	cur+=len;
	if (filesize<cur) filesize=cur;
	return(len);
};

fsize_t tMemoryWL::shift(fsize_t len,int mode){
	switch(mode){
	case SEEK_CUR:
		if (len+cur<0 || len+cur>filesize)
			return(-1);
		cur+=len;
		break;
	case SEEK_SET:
		if (len>filesize || len<0) return(-1);
		cur=len;
		break;
	case SEEK_END:
		if (filesize+len<0 || len>0)
			return(-1);
		cur=filesize+len;
		break;
	default:
		log(LOG_ERROR,_("BUG: Wrong mode of lseek!\n"));
	};
	return(cur);
};

fsize_t tMemoryWL::read(void *dst,fsize_t len){
	DBC_RETVAL_IF_FAIL(buf!=NULL,-1);
	DBC_RETVAL_IF_FAIL(dst!=NULL,-1);
	fsize_t real_len=cur+len>filesize?filesize-cur:len;
	if (real_len>0)
		memcpy(dst,buf+cur,real_len);
	cur+=real_len;
	return(real_len);
};

void tMemoryWL::truncate(){
	filesize=cur; //and nothing more :-)
};

void tMemoryWL::log(int type,const char *str){
	DBC_RETURN_IF_FAIL(str!=NULL);
	if (LOG){
//		printf("%s\n",str);
		LOG->add(str,type);
	};
};

void tMemoryWL::set_log(tLog *log){
	LOG=log;
};

