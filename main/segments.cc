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
};

void tSegment::print(){
	printf("%.12li %.12li\n",begin,end);
};

int tSegment::save(int fd){
	DBC_RETVAL_IF_FAIL(fd>=0,-1);
	if (write(fd,&begin,sizeof(begin))!=int(sizeof(begin)) ||
	    write(fd,&end,sizeof(end))!=int(sizeof(end)))
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
	autosave_counter=50;
	filename=copy_string(path);
	fd=open(path, O_CREAT|O_RDWR,S_IRUSR | S_IWUSR);
	load();
//	print();
};

void tSegmentator::print(){
	tSegment *tmp=FIRST;
	while(tmp){
		tmp->print();
		tmp=tmp->next;
	};
	printf("total %li -----------------\n",total);
};

unsigned long int tSegmentator::get_total(){
	return(total);
};

/* tSegmentator::insert
   return values:
      zero - if inserted zone does not overlap any exist zone
      non zero - if it overlaps :-)
 */

int tSegmentator::insert(unsigned long int begin, unsigned long int end){
	if (begin>=end) return(0); //simple case :-)
	//FIXME: overlapping only below, overlapping above is not overlapping;
	//    [100-200] + [201-300] = not overlapped
	//    [100-200] + [0-99]    = not overlapped
	//    [100-200] + [100-300] = not overlapped
	//    [100-200] + [101-102] = overlapped
	//    [100-200] + [50-150]  = overlapped
	lock();
	int rval=0;
	if (FIRST){
		tSegment *cur=FIRST;
		tSegment *prev=NULL;
		while(cur && cur->end+1<begin){
			prev=cur;
			cur=cur->next;
		};
		if (cur){ //somewhere in the list
			if (cur->begin>end+1){ //just insert new element here
				tSegment *add=seg_alloc();
				if ((add->prev=prev))
					prev->next=add;
				else
					FIRST=add;
				add->next=cur;
				cur->prev=add;
				add->begin=begin;
				add->end=end;
				total+=end-begin;
			}else{
				int dec=cur->end-cur->begin;
				if ((cur->begin>=begin && cur->begin<end)||
				    cur->end>end)
					rval=1;
				if (cur->begin>begin)
					cur->begin=begin;
				if (cur->end<end)
					cur->end=end;
				//proceed compactifation
				while(cur->next){
					tSegment *next=cur->next;
					if (next->begin<=cur->end+1){ //remove next element as overlaped
						dec+=next->end-next->begin;
						if (cur->end<next->end)
							cur->end=next->end;
						if ((cur->next=next->next))
							cur->next->prev=cur;
						else
							LAST=cur;
						seg_free(next);
					}else
						break;
				};
				total+=cur->end-cur->begin-dec;
			};
		}else{ // at the end of the list
			tSegment *add=seg_alloc();
			add->next=NULL;
			LAST->next=add;
			add->prev=LAST;
			LAST=add;
			add->begin=begin;
			add->end=end;
			total+=end-begin;
		};
	}else{
		FIRST=LAST=seg_alloc();
		FIRST->next=FIRST->prev=NULL;
		FIRST->begin=begin;
		FIRST->end=end;
		total=end-begin;
	};
	if (autosave_counter--<0){
		autosave_counter=50;
		save();
	};
	unlock();
	return(rval);
};

tSegment *tSegmentator::get_first(){
	return(FIRST);
};

int tSegmentator::one_segment(){
	if (FIRST==NULL || FIRST->next==NULL) return 1;
	return 0;
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
	while(read(fd,&begin,sizeof(begin))==sizeof(begin) &&
	      read(fd,&end,sizeof(end))==sizeof(end)){
		insert(begin,end);
	};
	return (0);
};

int tSegmentator::save(){
	DBC_RETVAL_IF_FAIL(fd>=0,-1);
	tSegment *tmp=FIRST;
	ftruncate(fd,0);
	while(tmp){
		if (tmp->save(fd)) return(-1);
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
