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

#include <unistd.h>
#include "ftp.h"
#include "client.h"
#include "liststr.h"
#include "locstr.h"
#include "var.h"
#include "ntlocale.h"

char *FTP_SERVER_OK="220";
char *FTP_USER_OK="331";
char *FTP_PASS_OK="230";
char *FTP_PASV_OK="227";
char *FTP_PORT_OK="200";
char *FTP_CWD_OK="250";
char *FTP_RETR_OK="150";
char *FTP_QUIT_OK="221";
char *FTP_READ_OK="226";
char *FTP_ABOR_OK="225";
char *FTP_REST_OK="350";

int tFtpClient::accepting() {
	if (passive) return RVALUE_OK;
	DSFlag=1;
	if (DataSocket.accepting(hostname)) {
		Status=STATUS_TRIVIAL;
		LOG->add(_("Accepting faild"),LOG_ERROR);
		return RVALUE_TIMEOUT;
	};
	return RVALUE_OK;
};

int  tFtpClient::send_command(char * comm,char *argv) {
	int len=strlen(comm)+2;
	if (argv) len+=strlen(argv)+1;
	char *data=new char[strlen(comm)+(argv?strlen(argv)+1:0)+1+strlen("\r\n")];
	data[0]=0;
	strcat(data,comm);
	if (argv && strlen(argv)) {
		strcat(data," ");
		strcat(data,argv);
	};
	strcat(data,"\r\n");
	if (equal(comm,"PASS"))
		LOG->add("PASS ***",LOG_TO_SERVER);
	else
		LOG->add(data,len,LOG_TO_SERVER);
	if ((Status=CtrlSocket.send_string(data,timeout))) {
		if (Status==STATUS_TIMEOUT)
			LOG->add(_("Timeout send through control socket."),LOG_ERROR);
		LOG->add(_("Control connection is lost"),LOG_WARNING);
		delete data;
		return RVALUE_TIMEOUT;
	};
	delete data;
	return RVALUE_OK;
};

int tFtpClient::read_data(char *where,int len) {
	int all=DataSocket.rec_string(where,len,timeout);
	if (socket_err_handler(all)) {
		LOG->add(_("Data socket is lost!"),LOG_WARNING);
		return RVALUE_TIMEOUT;
	};
	return all;
};

int tFtpClient::read_control() {
	if (read_string(&CtrlSocket,LOG,MAX_LEN)<0) {
		LOG->add(_("Control socket lost!"),LOG_WARNING);
		return RVALUE_TIMEOUT;
	};
	return RVALUE_OK;
};

int tFtpClient::analize(char *how) {
	tLogString *log=LOG->last();
	if (log) {
		int ind=0;
		while(how[ind]!=0) {
			if(how[ind]!=log->body[ind])
				return 1;
			ind+=1;
		};
		return 0;
	};
	return 1;
};

int tFtpClient::analize_ctrl(int argc,char **argv) {
	int ok;
	do {
		do {
			if (read_control()) return RVALUE_TIMEOUT;
			ok=1;
			ok=analize("2");
			for (int i=0;i<argc;i++) {
				ok=(ok && analize(argv[i]));
			};
			ok=(ok && analize("5") && analize("4"));
		} while (ok);
		if (!analize("4")){
			Status=STATUS_UNSPEC_ERR;
			return RVALUE_UNSPEC_ERR;
		};
		if (!analize("5")){
			Status=STATUS_CMD_ERR;
			return RVALUE_BAD_COMMAND;
		};
	} while (!last_answer());
	return RVALUE_OK;
};

int tFtpClient::last_answer() {
	tLogString *test=LOG->last();
	if (test->body[3]!=' ' &&  test->body[3]!='\n') return 0;
	return 1;
};

int tFtpClient::rest(int offset) {
	if (offset && ReGet) {
		char data[MAX_LEN];
		sprintf(data,"%i",offset);
		send_command("REST",data);
		int a=analize_ctrl(1,&FTP_REST_OK);
		if (a==RVALUE_TIMEOUT || a==RVALUE_UNSPEC_ERR) return(a);
		if (a==RVALUE_BAD_COMMAND) ReGet=0;
		else ReGet=1;
	};
	return RVALUE_OK;
};
//**************************************************/

tFtpClient::tFtpClient() {
	hostname=userword=username=buffer=NULL;
	passive=0;
	TEMP_SIZE=0;
};

void tFtpClient::init(char *host,tLog *log,int prt,int time_out) {
	tClient::init(host,log,prt,time_out);
	DSFlag=0;
	BuffSize=MAX_LEN;
	passive=0;
	buffer=new char[BuffSize];
};

int tFtpClient::reinit() {
	int rvalue=0;
	if ((rvalue=tClient::reinit())==0) {
		return analize_ctrl(5,&FTP_SERVER_OK);
	};
	return(rvalue);
};


int tFtpClient::registr(char *user,char *password) {
	username=user;
	userword=password;
	return 0;
};

int tFtpClient::connect() {
	ReGet=1;
	char *temp[2];
	temp[0]=FTP_PASS_OK;
	temp[1]=FTP_USER_OK;

	send_command("USER",username);
	if (analize_ctrl(2,temp)) return -1;
	if (analize(FTP_PASS_OK)){
		send_command("PASS",userword);
		if (analize_ctrl(2,temp)) return -2;
	};
	return 0;
};

int tFtpClient::stand_data_connection() {
	if (DSFlag) DataSocket.down();
	if (passive) {
		send_command("PASV",NULL);
		if (analize_ctrl(1,&FTP_PASV_OK)) {
			passive=0;
			return -1;
		};
		tLogString *log=LOG->last();
		int PASSIVE_ADDR[6]={0,0,0,0,0,0};
		sscanf(index(log->body,'(')+1,"%i,%i,%i,%i,%i,%i",&PASSIVE_ADDR[0],&PASSIVE_ADDR[1],&PASSIVE_ADDR[2],&PASSIVE_ADDR[3],&PASSIVE_ADDR[4],&PASSIVE_ADDR[5]);
		char data[MAX_LEN];
		sprintf(data,_("try to connect to %i,%i,%i,%i,%i,%i"),PASSIVE_ADDR[0],PASSIVE_ADDR[1],PASSIVE_ADDR[2],PASSIVE_ADDR[3],PASSIVE_ADDR[4],PASSIVE_ADDR[5]);
		LOG->add(data,LOG_OK);
		if (DataSocket.open_port(PASSIVE_ADDR[0]+(PASSIVE_ADDR[1]<<8)+(PASSIVE_ADDR[2]<<16)+(PASSIVE_ADDR[3]<<24)/*hostname*/,PASSIVE_ADDR[4]*256+PASSIVE_ADDR[5])) {
			LOG->add(_("Cant stand data connection"));
			Status=STATUS_TRIVIAL;
			passive=0;
			return -1;
		};
	} else {
		int addr=CtrlSocket.get_addr();
		int ac=DataSocket.open_any(addr);
		if (ac) return ac;
		addr=DataSocket.get_addr();
		int port=DataSocket.get_port();
		addr=DataSocket.get_addr();
		unsigned char *b=(unsigned char *)&port;
		unsigned char *a=(unsigned char *)&addr;
		char data[30];
		sprintf(data,"%u,%u,%u,%u,%u,%u",
		        a[0],a[1],a[2],a[3],b[0],(unsigned int)b[1]);
		send_command("PORT",data);
		if (analize_ctrl(1,&FTP_PORT_OK)) {
			passive=1;
			return -1;
		};
		Status=DSFlag=0;
	};
	return Status;
};

int tFtpClient::change_dir(char *where) {
	if (strlen(where)) {
		send_command("CWD",where);
		return analize_ctrl(1,&FTP_CWD_OK);
	};
	return RVALUE_OK;
}


int tFtpClient::get_size(char *filename,tStringList *list) {
	int rvalue=0;;
	if ((rvalue=rest(list->size()))) return(rvalue);
	if (!ReGet) list->done();
	send_command("LIST -la",filename);
	if ((rvalue=analize_ctrl(1,&FTP_RETR_OK))) return(rvalue);
	if ((rvalue=accepting())) return(rvalue);
	while (1) {
		int a=read_string(&DataSocket,list,MAX_LEN);
		DSize=list->size();
		if (a<0) return(a);
		if (a==RVALUE_COMPLETED){
			DataSocket.down(); // Added by Terence Haddock
			break;
		};
	};
	return (analize_ctrl(1,&FTP_READ_OK));
};

int tFtpClient::get_file_from(char *what,unsigned int begin,int fd) {
	int rvalue=0;;
	FileLoaded=begin;
	char data[25];
	data[0]=0;
	sprintf(data,"%u",begin);
	send_command("TYPE","I");
	if ((rvalue=analize_ctrl(1,&FTP_PORT_OK))) return(rvalue);
	if ((rvalue=rest(begin))) return rvalue;
	if (!ReGet) {
		lseek(fd,0,SEEK_SET);
		FileLoaded=0;
	};
	send_command("RETR",what);
	if ((rvalue=analize_ctrl(1,&FTP_RETR_OK))) return(rvalue);
	// Trying to determine file size
	tLogString *log=LOG->last();
	TEMP_SIZE=0;
	if (log) {
		char *str=rindex(log->body,'(');
		if (str) sscanf(str+1,"%i",&TEMP_SIZE);
	};

	if ((rvalue=accepting())) return(rvalue);
	if (Status) return RVALUE_TIMEOUT;
	DSize=0;
	int complete;
	do {
		if ((complete=tClient::read_data())<0) {
			LOG->add(_("Data connection closed."),LOG_WARNING);
			break;
		};
		FileLoaded+=complete;
		if (write_buffer(fd)) {
			LOG->add(_("Can't write to file"),LOG_ERROR);
			Status=STATUS_FATAL;
			break;
		};
	} while (complete!=0);
	DataSocket.down(); // to prevent next ideas from guys of wu-ftpd's team
	if (Status) return DSize;
	analize_ctrl(1,&FTP_READ_OK);
	return DSize;
};

int tFtpClient::another_way_get_size() {
	return TEMP_SIZE;
};

void tFtpClient::set_passive(int a) {
	passive=a;
};

void tFtpClient::down() {
	CtrlSocket.down();
	DataSocket.down();
};


void tFtpClient::quit() {
	send_command("QUIT",NULL);
	analize_ctrl(1,&FTP_QUIT_OK);
};

void tFtpClient::done() {
	if (!Status) quit();
	down();
};

tFtpClient::~tFtpClient() {
	down();
	if (buffer)	delete (buffer);
};
