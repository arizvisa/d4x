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
#ifndef _MY_T_ADDR
#define _MY_T_ADDR
#include "locstr.h"

struct tAddr{
	tPStr host,username,pass,path,file,params;
	int proto,port;
	int mask;
	tAddr();
	tAddr(char *str);
	void print();
	void compose_path(char *aa,char *bb);
	void file_del_sq();
	void make_url(char *where);
	char *url();
	void copy_host(tAddr *what);
	void save_to_config(int fd);
	int is_valid();
	~tAddr();
};
enum D_PROTOS{
	D_PROTO_UNKNOWN,
	D_PROTO_FTP,
	D_PROTO_HTTP,
	D_PROTO_LAST
};

int get_proto_by_name(char *str);
int get_port_by_proto(int proto);
char *get_name_by_proto(int proto);
#endif
