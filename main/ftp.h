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
#ifndef T_FTP_CLIENT
#define T_FTP_CLIENT
#include "client.h"

class tFtpClient:public tClient{
    protected:
    int DSFlag;
    int passive;
    int TEMP_SIZE;
/*    int PASSIVE_ADDR[6];*/
    tSocket DataSocket;
    int send_command(char *comm,char *argv);
    int read_data(char *where,int len);
    int read_control();
    int analize_ctrl(int argc,char **argv);
    int analize(char *how);
    int accepting();
    int last_answer();
    int rest(int offset);
    public:
		tFtpClient();
		void init(char *host,tLog *log,int prt,int time_out);
		void set_passive(int a);
		int reinit();
        int connect();
		int registr(char *user,char *password);
        int stand_data_connection();
		int change_dir(char *where);
		int get_size(char *filename,tStringList *list);
		int get_file_from(char *what,unsigned int begin,int fd);
        int read_block(char *where,int size);
		void quit();
		void down();
    	void done();
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