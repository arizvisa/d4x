/*	WebDownloader for X-Window
 *	Copyright (C) 1999-2000 Koshelev Maxim
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
	tLogString(const char *where, int len,int tp);
	void print();
	~tLogString();
};

#define LOG_TIME_STR_LEN 40

class tMsgQueue;

class tLog:public tStringList{
	protected:
	char timebuf[LOG_TIME_STR_LEN]; //for outputing time into file
	time_t start;
	pthread_mutex_t mutex;
	int key;
	int current_row;
	void send_msg(int type,tLogString *what);
	int geometry[4];
	int ref_count;
	int fd;
	public:
		tMsgQueue *MsgQueue;
		int freezed_flag;
		void *Window;
		tLog();
		void store_geometry(int *a);
		void get_geometry(int *a);
		void print();
		void add(const char *str,int len,int type);
		void add(const char *str,int type);
		void add(const char *str);
		int init_save(char *path);
		void insert(tNode *what);
		void dispose();
		void lock();
		void unlock();
		void ref_inc();
		void ref_dec();
		tLogString *last();
		tLogString *next();
		tLogString *first();
		~tLog();
};

#endif
