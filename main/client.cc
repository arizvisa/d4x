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


#include "socket.h"
#include "liststr.h"
#include "client.h"
#include "socks.h"
#include "var.h"
#include "savedvar.h"
#include "ntlocale.h"
#include <unistd.h>
#include <stdarg.h>
#include <sys/types.h>

tWriterLoger::tWriterLoger(){};

fsize_t tWriterLoger::shift(fsize_t len){
	return(shift(len,SEEK_SET));
};

tWriterLoger::~tWriterLoger(){};

int tWriterLoger::is_overlaped(){
	return(0);
};

void tWriterLoger::log_printf(int type,const char *fmt,...){
	DBC_RETURN_IF_FAIL(fmt!=NULL);
	char str[MAX_LEN+1];
	char *cur=str;
	va_list ap;
	va_start(ap,fmt);
	*cur=0;
	while (*fmt && cur-str<MAX_LEN){
		if (*fmt=='%'){
			fmt+=1;
			switch(*fmt){
			case 's':{
				char *s=va_arg(ap,char *);
				if (s)
					g_snprintf(cur,MAX_LEN-(cur-str),"%s",s);
				else
					g_snprintf(cur,MAX_LEN-(cur-str),"%s","NULL");
				break;
			};
			case 'i':{
				g_snprintf(cur,MAX_LEN-(cur-str),"%i",va_arg(ap,int));
				break;
			};
			case 'l':{
				fmt+=1;
				switch(*fmt){
				case 'i':
					g_snprintf(cur,MAX_LEN-(cur-str),"%li",va_arg(ap,long int));
					break;
				case 'l':
					g_snprintf(cur,MAX_LEN-(cur-str),"%lli",va_arg(ap,long long));
					break;
				default:
					*cur='l';
					cur[1]=*fmt;
					cur[2]=0;
				};
				break;
			};
			default:{
				*cur=*fmt;
				cur+=1;
				*cur=0;			       
			};
			};
			if (*fmt==0) break;
			while(*cur) cur+=1;
		}else{
			*cur=*fmt;
			cur+=1;
			*cur=0;
		};
		fmt+=1;
	};
	va_end(ap);
	log(type,str);
};

void tWriterLoger::truncate(){
};

char * tWriterLoger::cookie(const char *host, const char *path){
	return NULL;
};

void tWriterLoger::cookie_set(tCookie *cookie){
	//do nothing :-)
};


/**************************************************************/

void tSimplyCfg::copy_ints(tSimplyCfg *src){
	DBC_RETURN_IF_FAIL(src!=NULL);
	http_recursing=src->http_recursing;
	change_links=src->change_links;
	speed=src->speed;
	ftp_recurse_depth=src->ftp_recurse_depth;
	http_recurse_depth=src->http_recurse_depth;
	timeout = src->timeout;
	time_for_sleep = src->time_for_sleep;
	number_of_attempts = src->number_of_attempts;

	passive = src->passive;
	dont_send_quit = src->dont_send_quit;
	permisions = src->permisions;
	get_date = src->get_date;
	retry = src->retry;
	full_server_loading=src->full_server_loading;
	dont_leave_dir=src->dont_leave_dir;
	ftp_dirontop=src->ftp_dirontop;

	rollback = src->rollback;
	follow_link = src->follow_link;
	leave_server = src->leave_server;
	sleep_before_complete = src->sleep_before_complete;
	
	check_time = src->check_time;
	ihate_etag = src->ihate_etag;
	con_limit = src->con_limit;
};

/* ---------------------------------------- */

tCfg::tCfg() {
	speed=0;
	proxy_no_cache=proxy_type=0;
	follow_link=leave_server=0;
	dont_leave_dir=0;
	sleep_before_complete=0;
	socks_port=0;
	ftp_dirontop=0;
	isdefault=1;
	con_limit=0;
};

int tCfg::get_flags(){
	int rvalue=0;
	if (full_server_loading) rvalue|=1;
	if (retry) rvalue|=2;
	if (get_date) rvalue|=4;
	if (passive) rvalue|=8;
	if (permisions) rvalue|=16;
	return rvalue;
};

void tCfg::set_flags(int what){
	full_server_loading = ((what & 1)!=0);
	retry = ((what & 2) != 0);
	get_date = ((what & 4)!=0);
	passive = ((what & 8)!=0);
	permisions = ((what & 16)!=0);
};

void tCfg::copy_proxy(tCfg *src){
	socks_port = src->socks_port;
	socks_host.set(src->socks_host.get());
	socks_user.set(src->socks_user.get());
	socks_pass.set(src->socks_pass.get());
	proxy_type = src->proxy_type;
	proxy_no_cache = src->proxy_no_cache;
	hproxy_port = src->hproxy_port;
	hproxy_host.set(src->hproxy_host.get());
	hproxy_user.set(src->hproxy_user.get());
	hproxy_pass.set(src->hproxy_pass.get());
	fproxy_port = src->fproxy_port;
	fproxy_host.set(src->fproxy_host.get());
	fproxy_user.set(src->fproxy_user.get());
	fproxy_pass.set(src->fproxy_pass.get());
};

void tCfg::copy(tCfg *src) {
	copy_ints(src);
	copy_proxy(src);
	user_agent.set(src->user_agent.get());
	cookie.set(src->cookie.get());
	Filter.set(src->Filter.get());
	isdefault=src->isdefault;
};

void tCfg::reset_proxy() {
	socks_host.set(NULL);
	hproxy_user.set(NULL);
	hproxy_pass.set(NULL);
	hproxy_host.set(NULL);
	fproxy_user.set(NULL);
	fproxy_pass.set(NULL);
	fproxy_host.set(NULL);
};

void tCfg::save_to_config(int fd){
	f_wstr_lf(fd,"Cfg:");
	if (hproxy_host.get()){
		write_named_string(fd,"Hproxy:",hproxy_host.get());
		write_named_integer(fd,"Hproxy_port:",hproxy_port);
		if(hproxy_pass.get()!=NULL && hproxy_user.get()!=NULL){
			write_named_string(fd,"Hproxy_pass:",hproxy_pass.get());
			write_named_string(fd,"Hproxy_user:",hproxy_user.get());
		};
	};
	if (fproxy_host.get()){
		write_named_string(fd,"Fproxy:",fproxy_host.get());
		write_named_integer(fd,"Fproxy_port:",fproxy_port);
		if(fproxy_pass.get()!=NULL && fproxy_user.get()!=NULL){
			write_named_string(fd,"Fproxy_pass:",fproxy_pass.get());
			write_named_string(fd,"Fproxy_user:",fproxy_user.get());
		};
		write_named_integer(fd,"proxy_type:",proxy_type);
	};
	if (socks_host.get()){
		write_named_string(fd,"socks:",socks_host.get());
		write_named_integer(fd,"socks_port:",socks_port);
		if (socks_user.get() && socks_pass.get()){
			write_named_string(fd,"socks_user:",socks_user.get());
			write_named_string(fd,"socks_pass:",socks_pass.get());
		};
	};
	write_named_integer(fd,"proxy_no_cache:",proxy_no_cache);
	if (user_agent.get() && *(user_agent.get()))
		write_named_string(fd,"User_agent:",user_agent.get());
		
	write_named_integer(fd,"Timeout:",timeout);
	write_named_integer(fd,"time_for_sleep:",time_for_sleep);
	write_named_integer(fd,"number_of_attempts:",number_of_attempts);
	write_named_integer(fd,"sleep_before_complete:",sleep_before_complete);
	write_named_integer(fd,"ftp_recurse_depth:",ftp_recurse_depth);
	write_named_integer(fd,"http_recurse_depth:",http_recurse_depth);
	write_named_integer(fd,"rollback:",rollback);
	write_named_integer(fd,"speed:",speed);
	write_named_integer(fd,"passive:",passive);
	write_named_integer(fd,"dont_send_quit:",dont_send_quit);
	write_named_integer(fd,"retry:",retry);
	write_named_integer(fd,"permisions:",permisions);
	write_named_integer(fd,"get_date:",get_date);
	write_named_integer(fd,"http_recursing:",http_recursing);
	write_named_integer(fd,"follow_link:",follow_link);
	write_named_integer(fd,"leave_server:",leave_server);
	write_named_integer(fd,"dont_leave_dir:",dont_leave_dir);
	write_named_integer(fd,"check_time:",check_time);
	write_named_integer(fd,"change_links:",change_links);
	write_named_integer(fd,"ftp_dirontop:",ftp_dirontop);
	write_named_integer(fd,"ihate_etag:",ihate_etag);
	if (con_limit)
		write_named_integer(fd,"con_limit:",con_limit);
	if (save_path.get() && *(save_path.get()))
		write_named_string(fd,"save_path:",save_path.get());
	if (referer.get())
		write_named_string(fd,"referer:",referer.get());
	if (cookie.get())
		write_named_string(fd,"cookie:",cookie.get());
	if (log_save_path.get())
		write_named_string(fd,"log_save_path:",log_save_path.get());
	if (Filter.get())
		write_named_string(fd,"Filter:",Filter.get());
	f_wstr_lf(fd,"EndCfg:");
};

int tCfg::load_from_config(int fd){
	tSavedVar table_of_fields[]={
		{"User_agent:", SV_TYPE_PSTR,	&user_agent}, 
		{"Hproxy:",	SV_TYPE_PSTR,	&hproxy_host},
		{"Hproxy_pass:",	SV_TYPE_PSTR,	&hproxy_pass},
		{"Hproxy_user:",	SV_TYPE_PSTR,	&hproxy_user},
		{"Hproxy_port:",	SV_TYPE_INT,	&hproxy_port},
		{"proxy_type:",	SV_TYPE_INT,	&proxy_type},
		{"Fproxy:",	SV_TYPE_PSTR,	&fproxy_host},
		{"Fproxy_pass:",	SV_TYPE_PSTR,	&fproxy_pass},
		{"Fproxy_user:",	SV_TYPE_PSTR,	&fproxy_user},
		{"Fproxy_port:",	SV_TYPE_INT,	&fproxy_port},
		{"socks:",	SV_TYPE_PSTR,	&socks_host},
		{"socks_pass:",	SV_TYPE_PSTR,	&socks_pass},
		{"socks_user:",	SV_TYPE_PSTR,	&socks_user},
		{"socks_port:",	SV_TYPE_INT,	&socks_port},
		{"timeout:",	SV_TYPE_INT,	&timeout},
		{"time_for_sleep:",SV_TYPE_INT,	&time_for_sleep},
		{"sleep_before_complete:",SV_TYPE_INT,	&sleep_before_complete},
		{"number_of_attempts:",SV_TYPE_INT,&number_of_attempts},
		{"ftp_recurse_depth:",SV_TYPE_INT,&ftp_recurse_depth},
		{"http_recurse_depth:",SV_TYPE_INT,&http_recurse_depth},
		{"rollback:",	SV_TYPE_LINT,	&rollback},
		{"speed:",	SV_TYPE_INT,	&speed},
		{"passive:",	SV_TYPE_INT,	&passive},
		{"dont_send_quit:",	SV_TYPE_INT,	&dont_send_quit},
		{"retry:",	SV_TYPE_INT,	&retry},
		{"permisions:",	SV_TYPE_INT,	&permisions},
		{"get_date:",	SV_TYPE_INT,	&get_date},
		{"http_recursing:",SV_TYPE_INT,	&http_recursing},
		{"follow_link:",SV_TYPE_INT,	&follow_link},
		{"leave_server:",SV_TYPE_INT,	&leave_server},
		{"ihate_etag:",	SV_TYPE_INT,	&ihate_etag},
		{"save_path:",	SV_TYPE_PSTR,	&save_path},
		{"dont_leave_dir:",SV_TYPE_INT,	&dont_leave_dir},
		{"referer:",	SV_TYPE_PSTR,	&referer},
		{"cookie:",	SV_TYPE_PSTR,	&cookie},
		{"proxy_no_cache:",SV_TYPE_INT,	&proxy_no_cache},
		{"EndCfg:",	SV_TYPE_END,	NULL},
		{"check_time:",	SV_TYPE_INT,	&check_time},
		{"change_links:",SV_TYPE_INT,	&change_links},
		{"con_limit:",	SV_TYPE_INT,	&con_limit},
		{"ftp_dirontop:",SV_TYPE_INT,&ftp_dirontop},
		{"log_save_path:",SV_TYPE_PSTR,	&log_save_path},
		{"Filter:",	SV_TYPE_PSTR,	&(Filter)}
	};
	char buf[MAX_LEN];
	while(f_rstr(fd,buf,MAX_LEN)>0){
		unsigned int i;
		for (i=0;i<sizeof(table_of_fields)/sizeof(tSavedVar);i++){
			if (equal_uncase(buf,table_of_fields[i].name)){
				if (table_of_fields[i].type==SV_TYPE_END){
					return(0);
				}else{
					if (sv_parse_file(fd,&(table_of_fields[i]),buf,MAX_LEN))
						return(-1);
				};
			};
		};
	};
	return -1;
};

tCfg::~tCfg() {
};


/* tClient 
 */

tClient::tClient(){
	hostname=username=userword=buffer=NULL;
	FileLoaded=0;
	CtrlSocket=new tSocket;
};

tClient::tClient(tCfg *cfg,tSocket *ctrl){
	hostname=username=userword=buffer=NULL;
	FileLoaded=0;
	if (ctrl)
		CtrlSocket=ctrl;
	else{
		if (cfg->socks_host.get() && cfg->socks_port){
			CtrlSocket=new tSocksSocket(cfg->socks_host.get(),
						    cfg->socks_port,
						    cfg->socks_user.get(),
						    cfg->socks_pass.get());
		}else
			CtrlSocket=new tSocket;
	};
};

tClient::~tClient(){
	if (buffer) delete[] buffer;
	if (CtrlSocket) delete(CtrlSocket);
};

void tClient::init(char *host,tWriterLoger *log,int prt,int time_out) {
	DBC_RETURN_IF_FAIL(host!=NULL);
	DBC_RETURN_IF_FAIL(log!=NULL);

	Status=0;
	LOG=log;
	port=prt;
	hostname=host;
	FileLoaded=0;
	timeout=time_out;
};

int tClient::read_data(fsize_t len) {
	FillSize=read_data(buffer,len);
	if (FillSize<0) return RVALUE_TIMEOUT;
	DSize+=FillSize;
	return FillSize;
};

int tClient::test_reget() {
	return ReGet;
};


int tClient::get_readed() {
	return FileLoaded;
};

int tClient::read_string(tSocket *sock,tStringList *list,int maxlen) {
	DBC_RETVAL_IF_FAIL(sock!=NULL,0);
	DBC_RETVAL_IF_FAIL(list!=NULL,0);

	int rvalue;
	char temp[maxlen+1];
	char *cur=temp;
	do {
		*cur=0;
		int err=sock->rec_string(cur,1,timeout);
		if ((rvalue=socket_err_handler(err))) return(rvalue);
		if (err==0 && temp==cur) return RVALUE_COMPLETED;
	} while(cur-temp<maxlen && *(cur++)!='\n');
	*cur=0;
	list->add(temp);
	return RVALUE_OK;
};

char *tClient::read_string(tSocket *sock,int maxlen) {
	DBC_RETVAL_IF_FAIL(sock!=NULL,0);
	char temp[maxlen+1];
	char *cur=temp;
	do {
		*cur=0;
		int err=sock->rec_string(cur,1,timeout);
		if (socket_err_handler(err)) return(NULL);
		if (err==0 && temp==cur) return NULL;
	} while(cur-temp<maxlen && *(cur++)!='\n');
	*cur=0;
	return copy_string(temp);
};

int tClient::socket_err_handler(int err) {
	if (err==STATUS_TIMEOUT) {
		Status=STATUS_TIMEOUT;
		LOG->log(LOG_ERROR,_("Timeout while reading from socket!"));
		return RVALUE_TIMEOUT;
	};
	if (err<0) {
		Status=STATUS_TRIVIAL;
		LOG->log(LOG_ERROR,_("Error while reading from socket!"));
		return RVALUE_TIMEOUT;
	};
	return RVALUE_OK;
};

int tClient::reinit() {
	Status=0;
	int err=-1;
	LOG->log(LOG_OK,_("Trying to connect..."));
	if (hostname && (err=CtrlSocket->open_port(hostname,port))==0) {
		LOG->log(LOG_WARNING,_("Socket was opened!"));
		return RVALUE_OK;
	};
	switch (err) {
	case SOCKET_UNKNOWN_HOST:{
		LOG->log(LOG_ERROR,_("Host not found!"));
		Status=STATUS_FATAL;
		break;
	};
	case SOCKET_CANT_ALLOCATE:{
		LOG->log(LOG_ERROR,_("Can't allocate socket"));
		Status=STATUS_FATAL;
		break;
	};
	case SOCKET_CANT_CONNECT:{
		LOG->log(LOG_ERROR,_("Can't connect"));
		Status=STATUS_TRIVIAL;
		break;
	};
	};
	return RVALUE_TIMEOUT;
};

int tClient::write_buffer() {
	return (FillSize-LOG->write(buffer,FillSize));
};


int tClient::get_status() {
	return Status;
};

tSocket *tClient::export_ctrl_socket(){
	tSocket *tmp=CtrlSocket;
	CtrlSocket=NULL;
	return(tmp);
};

void tClient::import_ctrl_socket(tSocket *s){
	if (CtrlSocket) delete(CtrlSocket);
	CtrlSocket=s;
};

//**************************************************/
