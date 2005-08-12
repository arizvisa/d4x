#ifndef _D4X_SSLSOCKET_HEADER_
#define _D4X_SSLSOCKET_HEADER_

#ifdef HAVE_SSL

#include "socket.h"
#include <openssl/ssl.h>

namespace d4x{
	class SSLSocket:public tSocket{
		SSL_CTX *ctx;
		SSL *Handler;
		fsize_t lowlevel_read(char *where,fsize_t len);
	public:
		SSLSocket();
		int open_port(char * host,guint16 port);
		int open_port(guint32 host,guint16 port);
		int send_string(char *what,int timeout);
		void down();
		~SSLSocket(); 
	};
};

#endif //HAVE_SSL

#endif //_D4X_SSLSOCKET_HEADER_
