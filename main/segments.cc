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
#include <package_config.h>
#include "segments.h"
#include "dbc.h"
#include "signal.h"
#include "locstr.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

tSegment::tSegment(){
	begin=end=0;
	offset_in_file=-1;
};

void tSegment::print(){
	printf("%.12li %.12li\n",begin,end);
};

int tSegment::save(int fd){
	DBC_RETVAL_IF_FAIL(fd>=0,-1);
	DBC_RETVAL_IF_FAIL(offset_in_file>=0,-1);
//	printf("savin %li %li\n",begin,end);
//	lseek(fd,offset_in_file,SEEK_SET);
	if (write(fd,&begin,sizeof(begin))<int(sizeof(begin)) ||
	    write(fd,&end,sizeof(end))<int(sizeof(end)))
		return(-1);
	return(0);
};

tSegment::~tSegment(){
//	do nothing?
};

/*************** Segmentator *************************************/

tSegmentator::tSegmentator(){
	FIRST=LAST=HEAP=NULL;
	fd=-1;
	filename=NULL;
};

tSegmentator::tSegmentator(char *path){
	FIRST=LAST=HEAP=NULL;
	filename=NULL;
	fd=-1;
	total=0;
	init(path);
};

tSegment *tSegmentator::seg_alloc(){
	if (HEAP){
		tSegment *rval=HEAP;
		HEAP=HEAP->next;
		return(rval);
	};
	return(new tSegment);
};

void tSegmentator::seg_free(tSegment *seg){
	seg->next=HEAP;
	HEAP=seg;
};

void tSegmentator::init(char *path){
	DBC_RETURN_IF_FAIL(path!=NULL);
	done();
	filename=copy_string(path);
	fd=open(path, O_CREAT|O_RDWR,S_IRUSR | S_IWUSR);
	lock();
	load();
	save();
//	print();
	unlock();
};

void tSegmentator::print(){
	tSegment *tmp=FIRST;
	while(tmp){
		tmp->print();
		tmp=tmp->next;
	};
	printf("-----------------\n");
};

void tSegmentator::remove(tSegment *what){
	DBC_RETURN_IF_FAIL(what!=NULL);
	if (what->prev)
		what->prev->next=what->next;
	else
		FIRST=what->next;
	if (what->next)
		what->next->prev=what->prev;
	else
		LAST=what->prev;
	seg_free(what);
};

void tSegmentator::save_from(tSegment *what){
	DBC_RETURN_IF_FAIL(what!=NULL);
	lseek(fd,what->offset_in_file,SEEK_SET);
	what->save(fd);
	tSegment *tmp=what->next;
	unsigned long int offset_in_file=what->offset_in_file+2*sizeof(unsigned long int);
	while(tmp){
		tmp->offset_in_file=offset_in_file;
		if (tmp->save(fd))
			break;
		offset_in_file+=2*sizeof(unsigned long int);
   		tmp=tmp->next;
	};
	ftruncate(fd,offset_in_file);
};

unsigned long int tSegmentator::get_total(){
	return(total);
};

int tSegmentator::join(tSegment *what){
	DBC_RETVAL_IF_FAIL(what!=NULL,1);
	/* joining near segments */
	tSegment *next=what->next;
	tSegment *prev=what->prev;
	int changed=0;
	if (next){
		if (what->end+1>=next->begin){
			changed=1;
			total-=what->end-what->begin;
			total-=next->end-next->begin;
			if(what->end<next->end)
				what->end=next->end;
			total+=what->end-what->begin;
			remove(next);
//			join(what);
		};
	};
	if (prev){
		if (prev->end+1>=what->begin){
			total-=what->end-what->begin;
			total-=prev->end-prev->begin;
			if (prev->end<what->end)
				prev->end=what->end;
			total+=prev->end-prev->begin;
			changed=2;
			remove(what);
		};
	};
	switch(changed){
	case 2:
		save_from(prev);
		break;
	case 1:
		save_from(what);
		break;
	};
	return(!changed);
};

void tSegmentator::insert(unsigned long int begin, unsigned long int end){
	DBC_RETURN_IF_FAIL(begin<end);
	lock();
	total+=end-begin;
	tSegment *tmp=FIRST,*prev=NULL;
	if (tmp){
		while (tmp){
			if (begin<tmp->begin){
				/* adding before */
				tSegment *a=seg_alloc();
				a->begin=begin;
				a->end=end;
				a->next=tmp;
				if ((a->prev=tmp->prev)){
					tmp->prev->next=a;
					a->offset_in_file=tmp->offset_in_file;
				}else{
					a->offset_in_file=0;
					FIRST=a;
				};
				tmp->prev=a;
				if (join(a))
					save_from(a);
				unlock();
				return;
			};
			prev=tmp;
			tmp=tmp->next;
		};
		/* adding to the end */
		tmp=seg_alloc();
		tmp->prev=prev;
		prev->next=tmp;
		tmp->next=NULL;
		tmp->begin=begin;
		tmp->end=end;
		tmp->offset_in_file=prev->offset_in_file
				    +sizeof(prev->begin)+sizeof(prev->end);
		LAST=tmp;
		if (join(tmp))
			save_from(tmp);
	}else{
		FIRST=seg_alloc();
		FIRST->next=FIRST->prev=NULL;
		FIRST->begin=begin;
		FIRST->end=end;
		LAST=FIRST;
		save();
	};
	unlock();
};

tSegment *tSegmentator::get_first(){
	return(FIRST);
};

void tSegmentator::truncate(unsigned long int shift){
	lock();
	tSegment *tmp=FIRST;
	total=0;
	while(tmp){
		tSegment *next=tmp->next;
		if (tmp->begin>=shift){
			if (tmp->prev)
				tmp->prev->next=NULL;
			else
				FIRST=NULL;
			seg_free(tmp);
		}else{
			if (tmp->end>=shift)
				tmp->end=shift;
			total+=tmp->end-tmp->begin;
		};
		tmp=next;
	};
	save();
	unlock();
};

void tSegmentator::done(){
	lock();
	total=0;
	if (filename){
		delete[] filename;
		filename=NULL;
	};
	while(FIRST){
		tSegment *tmp=(tSegment*)FIRST->next;
		seg_free(FIRST);
		FIRST=tmp;
	};
	if (fd>=0) close(fd);
	fd=-1;
	unlock();
};

void tSegmentator::complete(){
	if (filename)
		::remove(filename);
	if (FIRST){
		lock();
		tSegment *a=seg_alloc();
		a->begin=FIRST->begin;
		a->end=LAST->end;
		a->next=a->prev=NULL;
		unlock();
		done();
		FIRST=LAST=a;
		total=a->end-a->begin;
	};
};

tSegmentator::~tSegmentator(){
	done();
	while(HEAP){
		tSegment *tmp=HEAP->next;
		delete(HEAP);
		HEAP=tmp;
	};
	if (filename) delete[] filename;
};

/* private methods */
int tSegmentator::load(){
	DBC_RETVAL_IF_FAIL(fd>=0,-1);
	lseek(fd,0,SEEK_SET);
	unsigned long int begin,end;
	long offset_in_file=0;
	while(read(fd,&begin,sizeof(begin))==sizeof(begin) &&
	      read(fd,&end,sizeof(end))==sizeof(end)){
		tSegment *tmp=seg_alloc();
	    	tmp->begin=begin;
	        tmp->end=end;
	        tmp->prev=tmp;
	        if (FIRST){
			tmp->prev=LAST;
			LAST->next=tmp;
			tmp->next=NULL;
			LAST=tmp;
		}else{
			tmp->prev=tmp->next=NULL;
			FIRST=LAST=tmp;
		};
	        tmp->offset_in_file=offset_in_file;
	        offset_in_file=lseek(fd,0,SEEK_CUR);
	};
	tSegment *tmp=FIRST;
	while(tmp){
		join(tmp);
		tmp=tmp->next;
	};
	return (0);
};

int tSegmentator::save(){
	DBC_RETVAL_IF_FAIL(fd>=0,-1);
	tSegment *tmp=FIRST;
	ftruncate(fd,0);
	lseek(fd,0,SEEK_SET);
	long offset_in_file=0;
	total=0;
	while(tmp){
		total+=tmp->end-tmp->begin;
		tmp->offset_in_file=offset_in_file;
		if (tmp->save(fd)) return(-1);
		offset_in_file+=2*sizeof(unsigned long int);
		tmp=tmp->next;
	};
	return(0);
};

tSegment *tSegmentator::to_holes(unsigned long int size){
	lock();
	tSegment *tmp=FIRST;
	tSegment *rvalue=NULL;
	tSegment *last=NULL;
	int i=0;
	while(tmp && tmp->end<size){
		tSegment *tmp1=new tSegment;
		tmp1->begin=tmp->end;
		if (tmp->next){
			tmp1->end=tmp->next->begin;
		}else{
			tmp1->end=size;
		};
		i+=1;
		tmp1->next=NULL;
		if (last)
			last->next=tmp1;
		else
			rvalue=tmp1;
		last=tmp1;
		tmp=tmp->next;
	};
	if (rvalue==NULL){
		rvalue=new tSegment;
		rvalue->begin=0;
		rvalue->end=size;
		rvalue->offset_in_file=1;
		rvalue->next=NULL;
	}else{
		rvalue->offset_in_file=i;
	};
	unlock();
	return(rvalue);
};

void tSegmentator::lock_public(){
	lockmutex.lock();
};

void tSegmentator::unlock_public(){
	lockmutex.unlock();
};


void tSegmentator::lock(){
	download_set_block(1);
	lockmutex.lock();
};

void tSegmentator::unlock(){
	lockmutex.unlock();
	download_set_block(0);
};
