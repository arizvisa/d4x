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

#ifndef __DOWNLOADER_SEGMENTS_HEADER__
#define __DOWNLOADER_SEGMENTS_HEADER__

#include "mutex.h"
#include "queue.h"
typedef unsigned long long segoff_t;

struct tSegment{
	segoff_t  begin,end;
	long offset_in_file;
	tSegment *next,*prev;
	tSegment();
	void print();
	int save(int fd);
	~tSegment();
};

class tSegmentator{
	int autosave_counter;
	tSegment *FIRST,*LAST;
	tSegment *HEAP;
	d4x::Mutex lockmutex;
	int fd;
	char *filename;
	segoff_t total;
	int load();
	void lock();
	void unlock();
	tSegment *seg_alloc();
	void seg_free(tSegment *seg);
 public:
	tSegmentator();
	tSegmentator(char *path);
	void init(char *path);
	void print();
	int insert(segoff_t begin, segoff_t end);
	void truncate(segoff_t shift);
	tSegment *get_first();
	tSegment *to_holes(segoff_t size);
	int one_segment();
	segoff_t get_total();
	void done();
	int save();
	void complete();
	void lock_public();
	void unlock_public();
	~tSegmentator();
};

#endif
