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
#ifndef T_HTTP_CLIENT
#define T_HTTP_CLIENT
#include "client.h"
#include "liststr.h"

class tHttpClient:public tClient{
 protected:
	int Offset;
	int send_request(char *request);
	int send_request(char *begin, char *center,char *end);
	int read_data(char *where,int len);
	int read_answer(tStringList *list);
	char *user_agent,*referer;
	void send_cookies(char *host,char *path);
 public:
	tHttpClient();
	void init(char *host,tWriterLoger *log,int prt,int time_out);
	void set_user_agent(char *agent,char *refer);
	void set_offset(int a);
	int registr(char *user,char *password);
	int get_size(char *filename,tStringList *list);
	int get_file_from(char *what,unsigned int begin,int len);
	void down();
        void done();
        ~tHttpClient();
};

#endif
