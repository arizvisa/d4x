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
#include <package_config.h>
#include "history.h"
#include "locstr.h"
#include "dbc.h"
#include <stdio.h>

tHistory::tHistory():tStringList(){
	init(30);
};

void tHistory::insert(tNode *what) {
	DBC_RETURN_IF_FAIL(what!=NULL);	
	tString *tmp=last();
	while(tmp) {
		if (equal(tmp->body,((tString *)what)->body)) {
			del(tmp);
			delete(tmp);
			break;
		};
		tmp=next();
	};
	tQueue::insert(what);
};

tHistory::~tHistory() {
	done();
};
