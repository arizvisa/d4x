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
#ifndef T_HTTP_CLIENT
#define T_HTTP_CLIENT
#include "client.h"
#include "liststr.h"

class tHttpClient:public tClient{
 protected:
	fsize_t Offset;
	int pass_first;
	int send_request(char *request);
	int send_request(char *begin, char *center,char *end);
	fsize_t read_data(char *where,fsize_t len);
	int read_answer(tStringList *list);
	char *user_agent,*referer;
	void send_cookies(char *host,char *path);
	virtual fsize_t get_size_sub(tStringList *list);
 public:
	int CHUNKED,HTTP_VER,HTTP_SUBVER;
	int ERROR_CODE;
	tHttpClient();
	tHttpClient(tCfg *cfg,tSocket *ctrl=(tSocket *)NULL);
	void init(char *host,tWriterLoger *log,int prt,int time_out);
	void set_user_agent(char *agent,char *refer);
	void set_offset(fsize_t a);
	void pass_first_segment();
	int registr(char *user,char *password);
	virtual fsize_t get_size_only(char *filename,tStringList *list);
	fsize_t get_size(char *filename,tStringList *list);
	fsize_t get_file_from(char *what,fsize_t begin,fsize_t len);
	void down();
        void done();
        ~tHttpClient();
};

#endif
