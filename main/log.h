/*	WebDownloader for X-Window
 *	Copyright (C) 1999 Koshelev Maxim
 *	This Program is free but not GPL!!! You can't modify it
 *	without agreement with author. You can't distribute modified
 *	program but you can distribute unmodified program.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */
#ifndef T_LOG_STRING
#define T_LOG_STRING
#include "liststr.h"
#include <pthread.h>
#include <time.h>

struct tLogString:public tString{
	int type;
	time_t time;
	tLogString();
	tLogString(char *where, int len,int tp);
	void print();
	~tLogString();
};

class tLog:public tStringList{
	protected:
	time_t start;
	pthread_mutex_t mutex;
	int key;
	void send_msg(int type,tLogString *what);
	int geometry[4];
	public:
		int MsgQueue;
		void *Window;
		tLog();
		void store_geometry(int *a);
		void get_geometry(int *a);
		void print();
		void add(char *str,int len,int type);
		void add(char *str,int type);
		void add(char *str);
		void dispose();
		void lock();
		void unlock();
		tLogString *last();
		tLogString *next();
		tLogString *first();
		~tLog();
};

struct mbuf{
	long mtype;
	tLogString *what;
	tLog *which;
};

enum{
	LOG_OK=0,
	LOG_WARNING,
	LOG_FROM_SERVER,
	LOG_TO_SERVER,
	LOG_ERROR,
	LOG_DETAILED=64
};
#endif