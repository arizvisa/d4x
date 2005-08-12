#include "sslsocket.h"
#include "dbc.h"
#include <iostream>

#ifdef HAVE_SSL

using namespace d4x;

SSLSocket::SSLSocket(){
	//FIXME: what about error in constructor? :-)
	ctx = SSL_CTX_new(SSLv23_client_method());
	Handler=SSL_new(ctx);
};

SSLSocket::~SSLSocket(){
	if (ctx) SSL_CTX_free(ctx);
	if (Handler) SSL_free(Handler);
};


int SSLSocket::open_port(char * host,guint16 port){
	if (tSocket::open_port(host,port)!=0)
		return(SOCKET_CANT_CONNECT);
	con_flag=0;
	SSL_set_fd(Handler,fd);
	if (SSL_connect(Handler)!=1)
		return(SOCKET_CANT_CONNECT);
	con_flag=1;
	return 0;
};

int SSLSocket::open_port(guint32 host,guint16 port){
	if (tSocket::open_port(host,port)!=0)
		return(SOCKET_CANT_CONNECT);
	con_flag=0;
	SSL_set_fd(Handler,fd);
	if (SSL_connect(Handler)!=1)
		return(SOCKET_CANT_CONNECT);
	con_flag=1;
	return 0;
};

int SSLSocket::send_string(char *what,int timeout){
	DBC_RETVAL_IF_FAIL(what!=NULL,-1);
	int a=strlen(what);
	int b=SSL_write(Handler,what,a);
	if (b<0) return -1;
	SBytes+=a-b;
	return a-b;
};

fsize_t SSLSocket::lowlevel_read(char *where,fsize_t len){
	return SSL_read(Handler,where,len);
};

void SSLSocket::down(){
	if (con_flag)
		SSL_shutdown(Handler);
	tSocket::down();
};

#endif //HAVE_SSL
