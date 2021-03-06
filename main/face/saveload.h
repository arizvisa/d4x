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
#ifndef MY_SAVE_WINDOWS
#define MY_SAVE_WINDOWS

#include <vector>
#include <string>

void init_save_list(...);
void init_load_list(...);
void init_load_txt_list(...);
gint load_save_list_cancel();
void create_addlinks_with_referer(const std::vector<std::string> &v,const char *savepath);
#endif
