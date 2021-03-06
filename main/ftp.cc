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

#include <unistd.h>
#include <ctype.h>
#include <strings.h>
#include "ftp.h"
#include "client.h"
#include "liststr.h"
#include "locstr.h"
#include "var.h"
#include "ntlocale.h"
#include "socks.h"

using namespace d4x;

char *FTP_CTRL_TIMEOUT="421";
char *FTP_SERVER_OK="220";
char *FTP_USER_OK="331";
char *FTP_PASS_OK="230";
char *FTP_PASV_OK="227";
char *FTP_PORT_OK="200";
char *FTP_CWD_OK="250";
char *FTP_DATA_OK="125";
char *FTP_RETR_OK="150";
char *FTP_QUIT_OK="221";
char *FTP_READ_OK="226";
char *FTP_ABOR_OK="225";
char *FTP_REST_OK="350";
char *FTP_LOGIN_OK[]={
	FTP_PASS_OK,
	FTP_USER_OK
};
char *FTP_EXIST_DATA[]={
	FTP_RETR_OK,
	FTP_DATA_OK
};

int tFtpClient::accepting() {
	if (passive) return RVALUE_OK;
	DSFlag=1;
	if (DataSocket->accepting(hostname.c_str())) {
		Status=STATUS_TRIVIAL;
		LOG->log(LOG_ERROR,_("Accepting faild"));
		return RVALUE_TIMEOUT;
	};
	return RVALUE_OK;
};

int  tFtpClient::send_command(const std::string &comm,const std::string &argv) {
	std::string send_str=comm;
	if (!argv.empty())
		send_str+=std::string(" ")+argv;
	if (comm=="PASS")
		LOG->log(LOG_TO_SERVER,"PASS ***");
	else
		LOG->log(LOG_TO_SERVER,send_str.c_str());
	send_str+=std::string("\r\n");
	Status=CtrlSocket->send_string(send_str.c_str(),timeout);
	if (Status) {
		if (Status==STATUS_TIMEOUT)
			LOG->log(LOG_ERROR,_("Timeout while sending through control socket."));
		LOG->log(LOG_ERROR,_("Control connection lost"));
		vdisconnect();
		return RVALUE_TIMEOUT;
	};
	return RVALUE_OK;
};

fsize_t tFtpClient::read_data(char *where,fsize_t len) {
	DBC_RETVAL_IF_FAIL(where!=NULL,RVALUE_TIMEOUT);
	fsize_t all=DataSocket->rec_string(where,len,timeout);
	if (socket_err_handler(all)) {
		LOG->log(LOG_WARNING,_("Data connection lost!"));
		vdisconnect();
		return RVALUE_TIMEOUT;
	};
	return all;
};

int tFtpClient::read_control() {
	CTRL->done();
	if (read_string(CtrlSocket,CTRL,MAX_LEN)<0) {
		LOG->log(LOG_WARNING,_("Control connection lost!"));
		vdisconnect();
		return RVALUE_TIMEOUT;
	};
	tString *log=CTRL->last();
	if (log){
		gchar *text_utf=g_convert_with_fallback(log->body,-1,"UTF-8","ISO8859-1",NULL,NULL,NULL,NULL);
		LOG->log(LOG_FROM_SERVER,text_utf);
		g_free(text_utf);
	}else{
//		vdisconnect();
		return RVALUE_TIMEOUT;
	};
	return RVALUE_OK;
};

int tFtpClient::analize(char *how) {
	DBC_RETVAL_IF_FAIL(how!=NULL,0);
	tString *log=CTRL->last();
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
		if (read_control())
			return RVALUE_TIMEOUT;
		if (!FIRST_REPLY)
			FIRST_REPLY = CTRL->last()?copy_string(CTRL->last()->body):NULL;
	} while (!last_answer(FIRST_REPLY));
	if (FIRST_REPLY) delete[] FIRST_REPLY;
	FIRST_REPLY = NULL;
	if (!analize(FTP_CTRL_TIMEOUT)){
		Status=STATUS_TIMEOUT;
		vdisconnect();
		return RVALUE_TIMEOUT;
	};
	if (!analize("4")){
		Status=STATUS_UNSPEC_ERR;
		return RVALUE_UNSPEC_ERR;
	};
	if (!analize("5")){
		Status=STATUS_CMD_ERR;
		return RVALUE_BAD_COMMAND;
	};
	ok=analize("2");
	for (int i=0;i<argc;i++) {
		ok=(ok && analize(argv[i]));
	};
	if (ok) return RVALUE_BAD_COMMAND;
	return RVALUE_OK;
};

int tFtpClient::is_valid_answer(char *what) {
	if (what==NULL) return 0;
	for (int i=0;i<3;i++){
		if (what[i]<'0' || what[i]>'9') return 0;
	};
	return 1;
};

int tFtpClient::last_answer(char *first) {
	tString *test=CTRL->last();
	if (test && test->body && strlen(test->body)>=3 && first &&
	    isspace(test->body[3]) && is_valid_answer(test->body)){
		if (first[0] == test->body[0] &&
		    first[1] == test->body[1] &&
		    first[2] == test->body[2])
			return 1;
	};
	return 0;
};

int tFtpClient::rest(fsize_t offset) {
	ReGet=1;
	if (offset || CUR_REST!=0) {
		send_command("REST",boost::lexical_cast<std::string>(offset));
		int a=analize_ctrl(1,&FTP_REST_OK);
		if (a==RVALUE_TIMEOUT) return(a);
		if (a!=RVALUE_OK){
			ReGet=0;
			LOG->log(LOG_WARNING,_("Reget is not supported!!!"));
			CUR_REST=0;
		}else
			CUR_REST=offset;
	};
	return RVALUE_OK;
};

int tFtpClient::force_reget(){
	return(rest(100));
};

//**************************************************/

tFtpClient::tFtpClient():tClient(),DataSocket(new tSocket){
	passive=0;
	TEMP_SIZE=OLD_SIZE=0;
	CTRL=new tStringList;
	CTRL->init(2);
	FIRST_REPLY = NULL;
	METHOD_TO_LIST=0;
	log_flag=0;
	CUR_REST=0;
};

tFtpClient::tFtpClient(tCfg *cfg,SocketPtr ctrl):tClient(cfg,ctrl){
	passive=cfg->passive;
	TEMP_SIZE=OLD_SIZE=0;
	CTRL=new tStringList;
	CTRL->init(2);
	FIRST_REPLY = NULL;
	METHOD_TO_LIST=0;
	if (cfg->socks_host.get() && cfg->socks_port){
		DataSocket.reset(new tSocksSocket(cfg->socks_host.get(),
						  cfg->socks_port,
						  cfg->socks_user.get(),
						  cfg->socks_pass.get()));
	}else
		DataSocket.reset(new tSocket);
	CUR_REST=0;
};

void tFtpClient::init(const std::string &host,tWriterLoger *log,int prt,int time_out) {
	tClient::init(host,log,prt,time_out);
	DSFlag=0;
	BuffSize=BLOCK_READ;
	passive=0;
	buffer=new char[BuffSize];
	vdisconnect();
};

int tFtpClient::reinit() {
	ReGet=1;
	CUR_REST=0;
	int rvalue=0;
	quit();
	vdisconnect();
	if ((rvalue=tClient::reinit())==0) {
		rvalue=analize_ctrl(1,&FTP_SERVER_OK);
	};
	return(rvalue);
};


int tFtpClient::registr(const std::string &user,const std::string &password) {
	username=user;
	userword=password;
	return 0;
};

int tFtpClient::connect() {
	send_command("USER",username);
	if (analize_ctrl(sizeof(FTP_LOGIN_OK)/sizeof(char *),FTP_LOGIN_OK)) return -1;
	if (analize(FTP_PASS_OK)){
		send_command("PASS",userword);
		if (analize_ctrl(1,&FTP_PASS_OK)) return -2;
	};
	log_flag=1;
	return 0;
};

static void d4x_ftp_parse_pasv(const char *str,int args[]){
	char *a=index(str,'(');
	if (a==NULL) return;
	a+=1;
	int i=0;
	while (*a && i<6){
		if (isdigit(*a)){
			sscanf_int(a,&(args[i]));
			while(isdigit(*a)) a+=1;
		};
		a+=1;
		i+=1;
	};
};

int tFtpClient::stand_data_connection() {
	if (DSFlag) DataSocket->down();
	if (passive) {
		send_command("PASV");
		if (analize_ctrl(1,&FTP_PASV_OK)) {
			if (Status!=STATUS_TIMEOUT) passive=0;
			return -1;
		};
		tString *log=CTRL->last();
		if (log == NULL) return -1;
		int PASSIVE_ADDR[6]={0,0,0,0,0,0};
		d4x_ftp_parse_pasv(log->body,PASSIVE_ADDR);
		/*
		if (index(log->body,'(')!=NULL)
			sscanf(index(log->body,'(')+1,"%i,%i,%i,%i,%i,%i",&PASSIVE_ADDR[0],&PASSIVE_ADDR[1],&PASSIVE_ADDR[2],&PASSIVE_ADDR[3],&PASSIVE_ADDR[4],&PASSIVE_ADDR[5]);
		*/
		LOG->log_printf(LOG_OK,_("try to connect to %i,%i,%i,%i,%i,%i"),PASSIVE_ADDR[0],PASSIVE_ADDR[1],PASSIVE_ADDR[2],PASSIVE_ADDR[3],PASSIVE_ADDR[4],PASSIVE_ADDR[5]);
		if (DataSocket->open_port(PASSIVE_ADDR)) {
			Status=STATUS_TRIVIAL;
			passive=0;
			return -1;
		};
	} else {
		unsigned int addr=CtrlSocket->get_addr();
		int ac=DataSocket->open_any(addr);
		if (ac) return ac;
		addr=DataSocket->get_addr();
		unsigned short int port=DataSocket->get_port();
		addr=DataSocket->get_addr();
		char data[MAX_LEN];
/*		unsigned char *a=(unsigned char*)(&addr);
		unsigned char *b=(unsigned char*)(&port);
		sprintf(data,"%u,%u,%u,%u,%u,%u",
			(unsigned int)(a[3]),(unsigned int)(a[2]),(unsigned int)(a[1]),(unsigned int)(a[0]),
			(unsigned int)(b[1]),(unsigned int)(b[0]));
*/
		sprintf(data,"%u,%u,%u,%u,%u,%u",
			(addr>>24)&0xff,(addr>>16)&0xff,(addr>>8)&0xff,(addr)&0xff,
			(port>>8)&0xff,(port)&0xff);
		send_command("PORT",data);
		if (analize_ctrl(1,&FTP_PORT_OK) || Status==STATUS_CMD_ERR) {
			passive=1;
			Status=STATUS_TRIVIAL;
			return -1;
		};
		Status=DSFlag=0;
	};
	return Status;
};

void tFtpClient::vdisconnect(){
//	printf("DISCONNECTED!\n");
	log_flag=0;
};

int tFtpClient::change_dir(const char *where) {
	if (where !=NULL && strlen(where)) {
		send_command("CWD",where);
		return analize_ctrl(1,&FTP_CWD_OK);
	};
	return RVALUE_OK;
}

void _ftp_filename_destroy_(void *a){
	char *b=(char *)a;
	delete[] b;
};

fsize_t tFtpClient::get_size(const std::string &filename,tStringList *list) {
	DBC_RETVAL_IF_FAIL(list!=NULL,RVALUE_OK);

	fsize_t rvalue=0;;
	if ((rvalue=rest(list->size()))) return(rvalue);
	if (!ReGet) list->done();
	switch (METHOD_TO_LIST){
	case 1:
		send_command("LIST",filename);
		break;
	default:
		send_command("LIST -la",filename);
	};
	if ((rvalue=analize_ctrl(sizeof(FTP_EXIST_DATA)/sizeof(char*),FTP_EXIST_DATA))){
		if ((Status==STATUS_UNSPEC_ERR || Status==STATUS_CMD_ERR) &&
		    METHOD_TO_LIST==0){
			METHOD_TO_LIST=1;
			return(get_size(filename,list));
		}else
			return(rvalue);
	};
	if ((rvalue=accepting())) return(rvalue);
	while (1) {
		int a=read_string(DataSocket,list,MAX_LEN);
		DSize=list->size();
		if (a<0) return(a);
		if (a==RVALUE_COMPLETED){
			DataSocket->down(); // Added by Terence Haddock
			break;
		};
		if (CFG.FTP_DIR_IN_LOG)
			LOG->log(LOG_FROM_SERVER,(list->last())->body);
	};
	if (((rvalue=analize_ctrl(1,&FTP_READ_OK)) ||
	     list->count()==0) && METHOD_TO_LIST==0){
		METHOD_TO_LIST=1;
		DataSocket->down();
		if ((rvalue=stand_data_connection()))
			return(rvalue);
		return(get_size(filename,list));
	};
	return(rvalue);
};

fsize_t tFtpClient::get_file_from(const char *what,fsize_t begin,fsize_t len) {
	DBC_RETVAL_IF_FAIL(what!=NULL,RVALUE_OK);
#ifdef DEBUG_ALL
	LOG->log_printf(LOG_OK,"tFtpClient::get_file_from(%s,%ll,%ll)",what,begin,len);
#endif //DEBUG ALL
	fsize_t rvalue=0;;
	FileLoaded=begin;
	send_command("TYPE","I");
	if ((rvalue=analize_ctrl(1,&FTP_PORT_OK))) return(rvalue);
	if ((rvalue=rest(begin))) return rvalue;
	if (!ReGet) {
#ifdef DEBUG_ALL
		LOG->log_printf(LOG_OK,"Warning! problems with resuming!",what,begin,len);
#endif //DEBUG ALL
		if (!RETRY_IF_NO_REGET) return(RVALUE_UNSPEC_ERR);
		begin=0;
		LOG->shift(0);
		FileLoaded=0;
		LOG->truncate(); //to avoid displaing wrong size
	};
	send_command("RETR",what);
	if ((rvalue=analize_ctrl(sizeof(FTP_EXIST_DATA)/sizeof(char*),FTP_EXIST_DATA))) return(rvalue);
	// Trying to determine file size ***************
	tString *log=CTRL->last();
	TEMP_SIZE=0;
	if (log) {
		char *str=rindex(log->body,'(');
		if (str) sscanf(str+1,"%li",&TEMP_SIZE);
	};
	TEMP_SIZE+=begin;
	if (OLD_SIZE && OLD_SIZE>TEMP_SIZE){
		LOG->log(LOG_WARNING,_("Probably file was changed on server!"));
		ReGet=0;
		if (!RETRY_IF_NO_REGET) return(RVALUE_UNSPEC_ERR);
		send_command("REST","0");
		LOG->shift(0);
		FileLoaded=0;
		LOG->truncate();
	};
	OLD_SIZE=TEMP_SIZE;
	/************************************************/
	if ((rvalue=accepting())) return(rvalue);
	if (Status) return RVALUE_TIMEOUT;
	DSize=0;
	int complete;
	fsize_t llen=len;
	LOG->log_printf(LOG_WARNING,"low level loading started from %ll load %ll bytes",begin,len);
	do {
		if ((complete=tClient::read_data(BLOCK_READ))<0) {
			LOG->log(LOG_WARNING,_("Data connection closed."));
			break;
		};
		if (len && FillSize>llen) FillSize=llen;
		FileLoaded+=FillSize;
		if (write_buffer()) {
			LOG->log(LOG_ERROR,_("Error have happened during writing buffer to disk!"));
			Status=STATUS_FATAL;
			break;
		};
		if (len){
			llen -=FillSize;
			if (llen==0){
				LOG->log(LOG_OK,_("Requested size was loaded"));
				DataSocket->flush(); /*read data from socket
						      to avoid "brocken pipe"
						      on linux;*/
				DataSocket->down();
				analize_ctrl(1,&FTP_READ_OK);
				Status=0;
				LOG->log_printf(LOG_WARNING,"loading end. loaded %ll bytes",DSize);
				return DSize;
			};
		};
		if (LOG->is_overlaped()){
			LOG->log(LOG_OK,_("Segment was loaded! Complete this thread."));
			DataSocket->down();
//			send_command("ABOR",NULL);
			Status=0;
			LOG->log_printf(LOG_WARNING,"loading end. loaded %ll bytes",DSize);
			return DSize;
		};
	} while (complete!=0);
	if (complete==0) LOG->log(LOG_WARNING,_("EOF recieved from server!"));
#ifdef DEBUG_ALL
	LOG->log_printf(LOG_OK,"Out of the cicle with DSize=%ll",DSize);
#endif //DEBUG ALL
	DataSocket->down(); // to prevent next ideas from guys of wu-ftpd's team
	LOG->log_printf(LOG_WARNING,"loading end. loaded %ll bytes",DSize);
	if (Status){
#ifdef DEBUG_ALL
		LOG->log_printf(LOG_OK,"exiting from tFtpDownload with %ll",DSize);
#endif //DEBUG ALL
		return DSize;
	};
	analize_ctrl(1,&FTP_READ_OK);
//	if (analize_ctrl(1,&FTP_READ_OK) &&  (len==0 || llen))
//		return(RVALUE_UNSPEC_ERR);
#ifdef DEBUG_ALL
	LOG->log_printf(LOG_OK,"exiting from tFtpDownload with %ll",DSize);
#endif //DEBUG ALL
	return DSize;
};

int tFtpClient::another_way_get_size() {
	return TEMP_SIZE;
};

void tFtpClient::set_retry(int a){
	RETRY_IF_NO_REGET=a;
};

void tFtpClient::set_passive(int a) {
	passive=a;
};

void tFtpClient::set_dont_set_quit(int a){
	DONT_SEND_QUIT=a;
};

void tFtpClient::down() {
	if (CtrlSocket) CtrlSocket->down();
	if (DataSocket) DataSocket->down();
	vdisconnect();
};


void tFtpClient::quit() {
	if (DONT_SEND_QUIT)
		return;
	if (CtrlSocket->connected() && log_flag){
		send_command("QUIT");
		analize_ctrl(1,&FTP_QUIT_OK);
		log_flag=0;
	};
};

void tFtpClient::done() {
	quit();
	down();
};

tFtpClient::~tFtpClient() {
	down();
	delete(CTRL);
	if (FIRST_REPLY) delete[] FIRST_REPLY;
};
