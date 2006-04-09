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
#ifndef T_FTP_CLIENT
#define T_FTP_CLIENT
#include "client.h"

class tFtpClient:public tClient{
 protected:
	int DSFlag;
	int passive;
	fsize_t TEMP_SIZE,OLD_SIZE;
	fsize_t CUR_REST;
	int DONT_SEND_QUIT;
	int RETRY_IF_NO_REGET;
	int log_flag;
	/* to avoid memory leaks next variable is global */
	char *FIRST_REPLY;
	tStringList *CTRL;
	d4x::SocketPtr DataSocket;
	int send_command(const std::string &comm,const std::string &argv=std::string());
	fsize_t read_data(char *where,fsize_t len);
	int read_control();
	int analize_ctrl(int argc,char **argv);
	int analize(char *how);
	int accepting();
	int last_answer(char *first);
	int is_valid_answer(char *what);
	int rest(fsize_t offset);
	void vdisconnect();
 public:
	int METHOD_TO_LIST;
	tFtpClient();
	tFtpClient(tCfg *cfg,d4x::SocketPtr ctrl=d4x::SocketPtr());
	void init(const std::string &host,tWriterLoger *log,int prt,int time_out);
	void set_passive(int a);
	void set_retry(int a);
	void set_dont_set_quit(int a);
	int reinit();
	int connect();
	int registr(const std::string&user,const std::string &password);
        int stand_data_connection();
	int change_dir(const char *where);
	fsize_t get_size(const std::string &filename,tStringList *list);
	fsize_t get_file_from(const char *what,fsize_t begin,fsize_t len);
        int read_block(char *where,int size);
	void quit();
	void down();
    	void done();
	int force_reget();
	int another_way_get_size();
	~tFtpClient();
};

extern char *FTP_SERVER_OK;
extern char *FTP_USER_OK;
extern char *FTP_PASS_OK;
extern char *FTP_PASV_OK;
extern char *FTP_PORT_OK;
extern char *FTP_CWD_OK;
extern char *FTP_RETR_OK;
extern char *FTP_QUIT_OK;
extern char *FTP_READ_OK;
extern char *FTP_ABOR_OK;
extern char *FTP_REST_OK;

#endif
