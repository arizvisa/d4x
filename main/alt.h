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
#ifndef _D4X_ALTERNATES_HEADER_
#define _D4X_ALTERNATES_HEADER_

#include "addr.h"
#include "mutex.h"
#include "face/mywidget.h"

struct tDownload;

struct d4xAlt{
	tAddr info;
	tAddr proxy; /* ftp://user:pass@proxy.host.com:3128 */
	d4xAlt *next,*prev;
	void set_proxy_settings(tDownload *dwn);
};

struct tDownload;

struct d4xAltList{
private:
	d4xLinksSel *edit;
	d4xAltEdit *add_edit,*mod_edit;
	GtkTreeIter *str2mod;
public:
	d4xMutex lock;
	d4xAlt *FIRST,*END;
	int ftp_searching;
	d4xAltList();
	~d4xAltList();
	void set_find_sens();
	void init_edit_mod(GtkTreeIter *iter);
	void edit_mod_destroy();
	void edit_mod_ok();
	void lock_by_download();
	void unlock_by_download();
	void add(d4xAlt *alt);
	void del(d4xAlt *alt);
	void init_add();
	void add_edit_destroy();
	void add_edit_ok();
	void init_edit(tDownload *papa);
	void print2edit();
	void edit_destroy();
	void edit_remove();
	void clear();
	void fill_from_ftpsearch(tDownload *fs);
	void check(char *filename);
	int save_to_config(int fd);
	int load_from_config(int fd);
};

#endif //_D4X_ALTERNATES_HEADER_
