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
#ifndef T_LIST_STRING
#define T_LIST_STRING
#include "queue.h"

struct tString:public tNode{
    char *body;
    int temp;
    tString();
    tString(char *what,int len);
    void print();
    int size();
    ~tString();
};

//*************************************************/

class tStringList:public tQueue{
    protected:
    int Size;
    public:
    	tStringList();
    	virtual void print();
    	virtual void add(char *str,int len);
    	virtual void add(char *str);
    	void dispose();
    	void done();
    	int size();
    	tString *last();
    	tString *first();
    	tString *next();
    	tString *prev();
    	int add_strings(char *what,int len);
    	~tStringList();
};

class tMemory:public tStringList{
	public:
		virtual tString *add();
		void del(tString *a);
};
#endif
