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
#ifndef T_FTP_CLIENT
#define T_FTP_CLIENT
#include "client.h"

class tFtpClient:public tClient{
 protected:
	int DSFlag;
	int passive;
	fsize_t TEMP_SIZE;
	int CON_FLAGS;
	int DONT_SEND_QUIT;
	int METHOD_TO_LIST;
	int RETRY_IF_NO_REGET;
	/* to avoid memory leaks next variable is global */
	char *FIRST_REPLY;
	tStringList *CTRL;
	tSocket DataSocket;
	int send_command(char *comm,char *argv);
	int read_data(char *where,fsize_t len);
	int read_control();
	int analize_ctrl(int argc,char **argv);
	int analize(char *how);
	int accepting();
	int last_answer(char *first);
	int is_valid_answer(char *what);
	int rest(int offset);
	void vdisconnect();
 public:
	tFtpClient();
	void init(char *host,tWriterLoger *log,int prt,int time_out);
	void set_passive(int a);
	void set_retry(int a);
	void set_dont_set_quit(int a);
	int reinit();
	int connect();
	int registr(char *user,char *password);
        int stand_data_connection();
	int change_dir(char *where);
	fsize_t get_size(char *filename,tStringList *list);
	int get_file_from(char *what,unsigned int begin,fsize_t len);
        int read_block(char *where,int size);
	void quit();
	void down();
    	void done();
	int another_way_get_size();
	~tFtpClient();
};

enum CONNECTED_FLAGS_ENUM{
	CON_FLAG_CONNECTED = 1 << 0,
	CON_FLAG_LOGGED =    1 << 1,
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
