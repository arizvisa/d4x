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
#include <string>
#include <boost/lexical_cast.hpp>

template<typename Integral>
std::string make_number_nice(Integral num,int NICE_DEC_DIGITALS){
	switch (NICE_DEC_DIGITALS) {
	case 1:
	case 3:{
		std::string amp(NICE_DEC_DIGITALS==1?" ":"'");
		std::string rval=boost::lexical_cast<std::string>(num);
		std::string::size_type len=rval.length();
		while(len>3) {
			len-=3;
			rval.insert(len,amp);
		};
		return rval;
	};
	case 2:	{
		Integral gigs,megs;
		megs=num/(1024*1024);
		gigs=megs/1024;
		if (gigs==0 && megs<1000){
			Integral kils=num/1024;
			if (megs==0 && kils<1000) {
				fsize_t bytes=((num-kils*1024)*10)/1024;
				if (kils==0 && bytes<1000)
					return boost::lexical_cast<std::string>(num);
				return boost::lexical_cast<std::string>(kils)+"."+
					boost::lexical_cast<std::string>(bytes)+"K";
			};
			Integral bytes=((num-megs*1024*1024)*10)/(1024*1024);
			return boost::lexical_cast<std::string>(megs)+"."+
				boost::lexical_cast<std::string>(bytes)+"M";
		};
		Integral bytes=((num-gigs*1024*1024*1024)*10)/(1024*1024*1024);
		return boost::lexical_cast<std::string>(gigs)+"."+
			boost::lexical_cast<std::string>(bytes)+"G";
	};
	};
	return boost::lexical_cast<std::string>(num);
};

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
char *compose_strings_array(int *len,const char *a,int la,const char *b);
int empty_string(char *a);
std::string convert_time(int what,int TIME_FORMAT);
void string_to_low(char *what,char delim);
void string_to_low(char *what);
int convert_from_hex(unsigned char what);
void convert_to_hex(unsigned char what,char *where);
char *parse_percents(char *what);
char *unparse_percents(char *what);

std::string hexed_string(const std::string &str);
std::string unhexed_string(const std::string &str);
std::string filename_extension(const std::string &name);

char *escape_char(const char *where,char what,char bywhat);
void del_crlf(char *what);
void str_non_print_replace(char *what, char symbol);
int convert_month(char *src);
int ctime_to_time(char *src);
int check_mask(const char *src,const char *mask);
int check_mask2(const char *src,const char *mask);
int check_mask2_uncase(const char *src,const char *mask);
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
int write_named_string(int fd,const char *name,const char *str);
int write_named_integer(int fd,const char *name,int num);
int write_named_time(int fd,const char *name,time_t when);
int write_named_fsize(int fd,const char *name,fsize_t size);
int int_to_strin_len(int num);
int sscanf_int(char *str,int *where);
char *parse_save_path(const char *str,const char *file);
char *index_mult(char *str,const char *what);
int file_copy(char *from,char *to,char *buf,int size);
char *str_replace(const char *str,const char *where,const char *what);
void make_proxy_host(const char *host,int port);

//**************************************************/
#endif
