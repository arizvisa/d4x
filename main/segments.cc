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

#include "segments.h"
#include "dbc.h"
#include "signal.h"
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
	printf("savin %li %li\n",begin,end);
	lseek(fd,offset_in_file,SEEK_SET);
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
	FIRST=LAST=NULL;
	my_pthreads_mutex_init(&lockmutex);
	fd=-1;
};

tSegmentator::tSegmentator(char *path){
	FIRST=LAST=NULL;
	my_pthreads_mutex_init(&lockmutex);
	fd=-1;
	init(path);
};

void tSegmentator::init(char *path){
	if (fd>=0)
		close(fd);
	fd=open(path, O_CREAT|O_RDWR,S_IRUSR | S_IWUSR);
	load();
	save();
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
};

void tSegmentator::save_from(tSegment *what){
	DBC_RETURN_IF_FAIL(what!=NULL);
	what->save(fd);
	tSegment *tmp=what->next;
	long offset_in_file=lseek(fd,0,SEEK_CUR);
	if (tmp && tmp->offset_in_file!=offset_in_file){
		ftruncate(fd,offset_in_file);
		while(tmp){
			tmp->offset_in_file=offset_in_file;
			if (tmp->save(fd))
				break;
			offset_in_file=lseek(fd,0,SEEK_CUR);
    			tmp=tmp->next;
		};
	};
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
			if(what->end<next->end)
				what->end=next->end;
			remove(next);
			join(what);
		};
	};
	if (prev){
		if (prev->end+1>=what->begin){
			if (prev->end<what->end)
				prev->end=what->end;
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
	tSegment *tmp=FIRST,*prev=NULL;
	if (tmp){
		while (tmp){
			if (begin<tmp->begin){
				/* adding before */
				tSegment *a=new tSegment;
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
		tmp=new tSegment;
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
		FIRST=new tSegment;
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

void tSegmentator::done(){
	while(FIRST){
		tSegment *tmp=(tSegment*)FIRST->next;
		delete(FIRST);
		FIRST=tmp;
	};
};

tSegmentator::~tSegmentator(){
	if (fd>=0) close(fd);
	done();
	pthread_mutex_destroy(&lockmutex);
};

/* private methods */
int tSegmentator::load(){
	DBC_RETVAL_IF_FAIL(fd>=0,-1);
	lseek(fd,0,SEEK_SET);
	done();
	unsigned long int begin,end;
	long offset_in_file=0;
	while(read(fd,&begin,sizeof(begin))==sizeof(begin) &&
	      read(fd,&end,sizeof(end))==sizeof(end)){
		tSegment *tmp=new tSegment;
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
	while(tmp){
		tmp->offset_in_file=offset_in_file;
		if (tmp->save(fd))
			return(-1);
		offset_in_file=lseek(fd,0,SEEK_CUR);
		tmp=tmp->next;
	};
	return(0);
};

void tSegmentator::lock(){
	pthread_mutex_lock(&lockmutex);
};

void tSegmentator::unlock(){
	pthread_mutex_unlock(&lockmutex);
};
