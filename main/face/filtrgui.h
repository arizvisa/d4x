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
#ifndef _D4X_FILTERS_INTERFACE_HEADER_
#define _D4X_FILTERS_INTERFACE_HEADER_

#include <gtk/gtk.h>
#include "../filter.h"

void d4x_filters_window_add(d4xFNode *filter);
void d4x_filters_window_init();
void d4x_filters_window_destroy();


#endif
