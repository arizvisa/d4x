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
#ifndef THREAD_MANGER
#define THREAD_MANGER
#include "dlist.h"

void init_signal_handler();
void real_stop_thread(tDownload *what);
int stop_thread(tDownload *what);
void my_pthread_key_init();
tDownload **my_pthread_key_get();

#endif