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
#ifndef __DOWNLOADER_MEM_WL_HEADER__
#define __DOWNLOADER_MEM_WL_HEADER__

#include "log.h"
#include "socket.h"
#include "client.h"

class tMemoryWL:public tWriterLoger{
	tLog *LOG;
	fsize_t cur,bufsize,filesize;
	char *buf;
	void grow();
 public:
	tMemoryWL();
	virtual fsize_t write(const void *buf,fsize_t len);
	virtual fsize_t shift(fsize_t len,int mode);
	virtual fsize_t read(void *dst,fsize_t len);
	virtual void log(int type,const char *str);
	virtual void truncate();
	void set_log(tLog *log);
	virtual ~tMemoryWL();
};

#endif
