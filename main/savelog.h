/*	WebDownloader for X-Window
 *	Copyright (C) 1999 Koshelev Maxim
 *	This Program is free but not GPL!!! You can't modify it
 *	without agreement with autor. You can't distribute modified
 *	program but you can distribute unmodified program.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */
#ifndef SAVE_LOG
#define SAVE_LOG
#include "liststr.h"

extern const char *LIST_FILE;
void save_list();
void read_list(tStringList *where);
int save_list_to_file(char *path);
int read_list_from_file(char *path,tStringList *where);

#endif