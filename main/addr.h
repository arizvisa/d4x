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

struct tAddr{
	private:
	char *protocol,*host,*username,*pass,*path,*file;
	public:
	int port;
	int mask;
	tAddr();
	tAddr(char *str);
	void print();
	void set_proto(char *what);
	void set_host(char *what);
	void set_username(char *what);
	void set_pass(char *what);
	void set_path(char *what);
	void set_file(char *what);
	void compose_path(char *aa,char *bb);
	char *get_proto(){return protocol;};
	char *get_host(){return host;};
	char *get_pass(){return pass;};
	char *get_username(){return username;};
	char *get_path(){return path;};
	char *get_file(){return file;};
	void file_del_sq();
	void make_url(char *where);
	char *url();
	void copy_host(tAddr *what);
	void save_to_config(int fd);
	~tAddr();
};

#endif
