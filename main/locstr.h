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
#ifndef LOC_STRING
#define LOC_STRING

char *copy_string(const char *dest);
char *copy_string(const char *dest,int len);
int equal(const char *a,const char *b);
int equal_first(const char *a,const char *b);
int equal_first_uncase(const char *a,const char *b);
int begin_string(const char *str,const char *begin);
int begin_string_uncase(const char *str,const char *begin);
char *sum_strings(const char *a,const char *b);
char *sum_strings(const char *a,const char *b,const char *c);
int empty_string(char *a);
void convert_time(int what,char *where);
void string_to_low(char *what,char delim);
void string_to_low(char *what);
int convert_from_hex(char what);
char *parse_percents(char *what);
char *unparse_percents(char *what);
void del_crlf(char *what);
void make_number_nice(char *where,int num);
int convert_month(char *src);
int ctime_to_time(char *src);
int check_mask(char *src,char *mask);
void normalize_path(char *src);
char *compose_path(const char *left,const char *right);
int global_url(char *url);
void scroll_string_left(char *str,unsigned int shift);
char *extract_from_prefixed_string(char *str,char *begin);
char *extract_string(char *src,char *dst);
char *extract_string(char *src,char *dst,int num);
int get_permisions_from_int(int a);
char *subtract_path(const char *a,const char *b);
int reallocate_string(char **what, int len);
int is_string(char *what);

//**************************************************/
#endif