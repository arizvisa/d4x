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

#include <unistd.h>
#include "ftp.h"
#include "client.h"
#include "liststr.h"
#include "locstr.h"
#include "var.h"
#include "ntlocale.h"

int tFtpClient::accepting() {
	if (passive) return 0;
	DSFlag=1;
	if (DataSocket.accepting(hostname)) {
		Status=STATUS_TRIVIAL;
		LOG->add(_("Accepting faild"),LOG_ERROR);
		return -1;
	};
	return 0;
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
		LOG->add(_("Control conection lost"),LOG_WARNING);
		delete data;
		return -1;
	};
	delete data;
	return 0;
};

int tFtpClient::read_data(char *where,int len) {
	int all=DataSocket.rec_string(where,len,timeout);
	if (socket_err_handler(all)) {
		LOG->add(_("Data socket lost!"),LOG_WARNING);
		return -1;
	};
	return all;
};

int tFtpClient::read_control() {
	if (read_string(&CtrlSocket,LOG,MAX_LEN)<0) {
		LOG->add(_("Control socket lost!"),LOG_WARNING);
		return -1;
	};
	return 0;
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
			if (read_control()) return -1;
			ok=1;
			ok=analize("2");
			for (int i=0;i<argc;i++) {
				ok=(ok && analize(argv[i]));
			};
			ok=(ok && analize("5") && analize("4"));
		} while (ok);
		if (!analize("4") || !analize("5")) {
			Status=STATUS_CMD_ERR;
			return -2;
		};
	} while (!last_answer());
	return 0;
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
		if (a==-1) return -1;
		if (a==-2) ReGet=0;
		else ReGet=1;
	};
	return 0;
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
	if (tClient::reinit()==0) {
		return analize_ctrl(5,&FTP_SERVER_OK);
	};
	return -1;
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
	return 0;
}


int tFtpClient::get_size(char *filename,tStringList *list) {
	if (rest(list->size())) return -1;
	if (!ReGet) list->done();
	send_command("LIST -la",filename);
	if (analize_ctrl(1,&FTP_RETR_OK)) return -1;
	if (accepting()) return -1;
	while (1) {
		int a=read_string(&DataSocket,list,MAX_LEN);
		DSize=list->size();
		if(a<0) return -1;
		if (a>0) break;
	};
	if (analize_ctrl(1,&FTP_READ_OK)) return -1;
	return 0;
};

int tFtpClient::get_file_from(char *what,unsigned int begin,int fd) {
	FileLoaded=begin;
	char data[25];
	data[0]=0;
	sprintf(data,"%u",begin);
	send_command("TYPE","I");
	if (analize_ctrl(1,&FTP_PORT_OK)) return -1;
	if (rest(begin)) return -1;
	if (!ReGet) {
		lseek(fd,0,SEEK_SET);
		FileLoaded=0;
	};
	send_command("RETR",what);
	if (analize_ctrl(1,&FTP_RETR_OK)) return -1;
	// Trying to determine file size
	tLogString *log=LOG->last();
	TEMP_SIZE=0;
	if (log) {
		char *str=rindex(log->body,'(');
		if (str) sscanf(str+1,"%i",&TEMP_SIZE);
	};

	if (accepting()) return -1;
	if (Status) return -1;
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
