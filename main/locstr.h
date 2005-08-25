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
#ifndef LOC_STRING
#define LOC_STRING

#include <time.h>
#include "speed.h"

class tPStr{
	char *a;
 public:
	tPStr(const char *a);
	tPStr();
	void set(const char *a);
	char *get()const{return a;};
	bool notempty();
	~tPStr();
};

#define REMOVE_SC_FROM_HOST(__host) if (__host){ \
   char *__sc=index(__host,':');\
   if (__sc) *__sc=0; \
  };

//#define copy_string(var) (var?strncpy(new char[strlen(var)+1],var,strlen(var)+1):NULL)

char *copy_string(const char *dest);
char *copy_string2(const char *dest,int len);
int equal(const char *a,const char *b);
int equal_uncase(const char *a,const char *b);
int equal_first(const char *a,const char *b);
int equal_first_uncase(const char *a,const char *b);
int begin_string(const char *str,const char *begin);
int begin_string_uncase(const char *str,const char *begin);
char *sum_strings(const char *a,...);
int empty_string(char *a);
void convert_time(int what,char *where,int TIME_FORMAT);
void string_to_low(char *what,char delim);
void string_to_low(char *what);
int convert_from_hex(unsigned char what);
void convert_to_hex(unsigned char what,char *where);
char *parse_percents(char *what);
char *unparse_percents(char *what);
char *escape_char(const char *where,char what,char bywhat);
void del_crlf(char *what);
void str_non_print_replace(char *what, char symbol);
void make_number_nice(char *where,fsize_t num,int NICE_DEC_DIGITALS);
void make_number_nicel(char *where,unsigned long num,int NICE_DEC_DIGITALS);
int convert_month(char *src);
int ctime_to_time(char *src);
int check_mask(char *src,char *mask);
int check_mask2(char *src,char *mask);
int check_mask2_uncase(char *src,char *mask);
void normalize_path(char *src);
char *normalize_path_full(char *src);
char *compose_path(const char *left,const char *right);
void scroll_string_left(char *str,unsigned int shift);
char *extract_from_prefixed_string(char *str,char *begin);
char *extract_string(char *src,char *dst);
char *extract_string(char *src,char *dst,int num);
char *skip_strings(char *src,int num);
int get_permisions_from_int(int a);
char *subtract_path(const char *a,const char *b);
int reallocate_string(char **what, int len);
int is_string(char *what);
int string_ended(const char *ended, const char *what);
int string_ended_uncase(const char *ended, const char *what);
char *skip_spaces(char *src);
int f_wchar(int fd,char c);
int f_rstr(int fd,char *where,int max);
int f_wstr(int fd,const char *str);
int f_wstr_lf(int fd,const char *str);
int write_named_string(int fd,char *name,char *str);
int write_named_integer(int fd,char *name,int num);
int write_named_time(int fd,char *name,time_t when);
int write_named_fsize(int fd,char *name,fsize_t size);
int int_to_strin_len(int num);
int sscanf_int(char *str,int *where);
char *parse_save_path(const char *str,char *file);
char *index_mult(char *str,const char *what);
int file_copy(char *from,char *to,char *buf,int size);
char *str_replace(const char *str,const char *where,const char *what);
void make_proxy_host(const char *host,int port);

//**************************************************/
#endif
