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

/*
namespace d4x{

	struct HTTPAnswerField{
		std::string name;
		std::string value;
	};
	
	struct HTTPAnswer:public std::list<HTTPAnswerField>{
		std::string head;
		std::list<std::string> lst;
	};
};
*/

class tHttpClient:public tClient{
 protected:
	
	fsize_t Offset;
	int pass_first;
	std::string user_agent,referer;
	
	int send_request(const std::string &name,const std::string &val);
	int send_request(const std::string &request);
	fsize_t read_data(char *where,fsize_t len);
	int read_answer(tStringList *list);
	void send_cookies(const std::string &host,const std::string &path);
	virtual fsize_t get_size_sub(tStringList *list);
 public:
	int CHUNKED,HTTP_VER,HTTP_SUBVER;
	int ERROR_CODE;
	tHttpClient();
	tHttpClient(tCfg *cfg,d4x::SocketPtr ctrl=d4x::SocketPtr());
	void init(const std::string &host,tWriterLoger *log,int prt,int time_out);
	void set_user_agent(char *agent,char *refer);
	void set_offset(fsize_t a);
	void pass_first_segment();
	int registr(const std::string &user,const std::string &password);
	virtual fsize_t get_size_only(const std::string &filename,tStringList *list);
	fsize_t get_size(const std::string &filename,tStringList *list);
	fsize_t get_file_from(const char *what,fsize_t begin,fsize_t len);
	void down();
        void done();
        ~tHttpClient();
};

#endif
