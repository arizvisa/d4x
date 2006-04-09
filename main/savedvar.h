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

#ifndef __DOWLOADER_SAVED_VARS__
#define __DOWLOADER_SAVED_VARS__

struct tSavedVar{
	char *name;
	int type;
	void *where;
};

enum SV_TYPES{
	SV_TYPE_INT,
	SV_TYPE_LINT,
	SV_TYPE_FSIZE_TRIGER,
	SV_TYPE_PSTR,
	SV_TYPE_STDSTR,
	SV_TYPE_SPLIT,
	SV_TYPE_CFG,
	SV_TYPE_URL,
	SV_TYPE_TIME,
	SV_TYPE_DOWNLOAD,
	SV_TYPE_QDOWNLOAD,
	SV_TYPE_QUEUE,
	SV_TYPE_QV,
	SV_TYPE_ALT,
	SV_TYPE_TMP,
	SV_TYPE_END
};

int sv_parse_file(int fd,tSavedVar *var,char *buf,int bufsize);

#endif
