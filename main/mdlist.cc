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

#include "mdlist.h"

void tDListMutex::init(int n) {
	tDList::init(n);
	pthread_mutex_init(&mutex,NULL);
};

void tDListMutex::insert(tNode *what) {
	pthread_mutex_lock(&mutex);
	tQueue::insert(what);
	pthread_mutex_unlock(&mutex);
};

void tDListMutex::del(tNode *what) {
	pthread_mutex_lock(&mutex);
	tQueue::del(what);
	pthread_mutex_unlock(&mutex);
};

void tDListMutex::dispose() {
	pthread_mutex_lock(&mutex);
	tQueue::dispose();
	pthread_mutex_unlock(&mutex);
};

tDownload *tDListMutex::last() {
	pthread_mutex_lock(&mutex);
	tDownload *rvalue=tDList::last();
	pthread_mutex_unlock(&mutex);
	return rvalue;
};

tDownload *tDListMutex::first() {
	pthread_mutex_lock(&mutex);
	tDownload *rvalue=tDList::first();
	pthread_mutex_unlock(&mutex);
	return rvalue;
};

tDownload *tDListMutex::next() {
	pthread_mutex_lock(&mutex);
	tDownload *rvalue=tDList::next();
	pthread_mutex_unlock(&mutex);
	return rvalue;
};
