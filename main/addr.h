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
#ifndef _MY_T_ADDR
#define _MY_T_ADDR
#include "locstr.h"
#include <string>
#include "path.h"

enum D_PROTOS{
	D_PROTO_UNKNOWN,
	D_PROTO_FTP,
	D_PROTO_HTTP,
	D_PROTO_HTTPS,
	D_PROTO_SEARCH,
	D_PROTO_SOCKS,
	D_PROTO_LAST
};

namespace d4x{
	struct ShortURL{
		std::string host,file,params;
		Path path;
		int proto,port;
		ShortURL():proto(D_PROTO_UNKNOWN),port(0){};
		operator std::string() const;
	};
	
	struct URL:public ShortURL{
		std::string user,pass;
		std::string tag;       // temporary field for HTML recursiont + filters
		bool mask;             // to specify '*.*' files etc.
		URL():mask(false){};
		URL(const URL&_u):ShortURL(_u),user(_u.user),pass(_u.user),tag(_u.tag),mask(_u.mask){};
		URL(const std::string &_s);
		URL &operator=(const URL &_u);
		bool operator==(const URL &_u) const;
		bool operator<(const URL &u) const;
		bool is_valid();
		operator std::string() const;
		void copy_host(const URL&_u);
		void clear();
	};
};

int get_proto_by_name(const char *str);
int get_port_by_proto(int proto);
const char *get_name_by_proto(int proto);
int global_url(char *url);

#endif
