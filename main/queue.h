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
#ifndef T_QUEUE
#define T_QUEUE

struct tNode{
    tNode *next,*prev;
    tNode();
    virtual void print()=0;
    virtual ~tNode();
};

class tQueue{
    protected:
    int MaxNum,Num;
    tNode *First,*Last,*Curent;
    public:
    	tQueue();
    	virtual void init(int n);
    	virtual void insert(tNode *what);
    	virtual void insert_before(tNode *what,tNode *where);
    	virtual void del(tNode *what);
    	int count();
    	virtual tNode *last();
    	virtual tNode *first();
    	virtual tNode *next();
    	virtual tNode *prev();
    	virtual void dispose();
    	virtual void done();
    	virtual ~tQueue();
};

#endif