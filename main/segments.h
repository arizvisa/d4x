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

#ifndef __DOWNLOADER_SEGMENTS_HEADER__
#define __DOWNLOADER_SEGMENTS_HEADER__

#include <pthread.h>
#include "queue.h"

struct tSegment{
	unsigned long int begin,end;
	long offset_in_file;
	tSegment *next,*prev;
	tSegment();
	void print();
	int save(int fd);
	~tSegment();
};

class tSegmentator{
	tSegment *FIRST,*LAST;
	pthread_mutex_t lockmutex;
	int fd;
	char *filename;
	unsigned long int total;
	int load();
	int save();
	void lock();
	void unlock();
	int join(tSegment *what);
	void remove(tSegment *what);
	void save_from(tSegment *what);
 public:
	tSegmentator();
	tSegmentator(char *path);
	void init(char *path);
	void print();
	void insert(unsigned long int begin, unsigned long int end);
	void truncate(unsigned long int shift);
	tSegment *get_first();
	tSegment *to_holes(unsigned long int size);
	unsigned long int get_total();
	void done();
	void complete();
	~tSegmentator();
};

#endif
