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
#ifndef _D4X_AUTOADD_HEADER_
#define _D4X_AUTOADD_HEADER_

#include "queue.h"

class d4xAutoGenerator{
	tQueue list;
public:
	d4xAutoGenerator();
	int init(char *str);
	char *first();
	char *next();
	void print();
	~d4xAutoGenerator();
};

#endif
