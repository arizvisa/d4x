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
#ifndef T_GTK_LOG
#define T_GTK_LOG

void init_pixmaps_for_log();
void init_log_window(tDownload *what);
void log_window_add_string(tLog *log,tLogString *str);
int destroy_log_window(void *a);
void destroy_log_window_by_log(void *a);
void del_first_from_log(tLog *what);

#endif
