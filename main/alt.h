#ifndef _D4X_ALTERNATES_HEADER_
#define _D4X_ALTERNATES_HEADER_

#include "addr.h"
#include "mutex.h"
#include "face/mywidget.h"

struct d4xAlt{
	tAddr info;
	d4xAlt *next,*prev;
};

struct tDownload;

struct d4xAltList{
private:
	d4xLinksSel *edit;
	d4xStringEdit *add_edit,*mod_edit;
	int str2mod;
public:
	d4xMutex lock;
	d4xAlt *FIRST,*END;
	d4xAltList();
	~d4xAltList();
	void init_edit_mod(int str);
	void edit_mod_destroy();
	void edit_mod_ok();
	void lock_by_download();
	void unlock_by_download();
	void add(d4xAlt *alt);
	void del(d4xAlt *alt);
	void init_add();
	void add_edit_destroy();
	void add_edit_ok();
	void init_edit();
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
