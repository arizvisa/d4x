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

#include "eff.h"
#include <string.h>

tUrlParser::tUrlParser(const char *filename){
	fd=open(filename,O_RDONLY);
	*buf=0;
};

int tUrlParser::sequence(unsigned char *where, char *str){
	while(read(fd,where,1)>0){
		if (*str){
			if (*str!=*where){
				*buf=*where;
				return 0;
			};
		}else
			break;
		where+=1;
		str+=1;
		if (*str==0) break;
	};
	return 1;
};

int tUrlParser::read_url(unsigned char *where, tStringList *list){
	char *bad_chars="[]()\"'`*";
	while(read(fd,where,1)>0){
		if (*where<=' ' || *where>127 || index(bad_chars,*where)){
			*where=0;
			list->add((char *)buf);
			return 0;
		};
		if (where>=buf+MAX_LEN) return 0;
		where+=1;
	};
	return 1;
};

tStringList *tUrlParser::parse(){
	tStringList  *list=new tStringList;
	list->init(0);
	if (read(fd,buf,1)<=0) return(list);
	while (1){
		switch(*buf){
		case 'f':{
			if (sequence(buf+1,"tp://")){
				if (read_url(buf+6,list))
					return (list);
			};
			break;
		};
		case 'h':{
			if (sequence(buf+1,"ttp://")){
				if (read_url(buf+7,list))
					return (list);
			};
			break;
		};
		default:
			if (read(fd,buf,1)<=0) return(list);
		};
	};
	return(list);
};

tUrlParser::~tUrlParser(){
	if (fd>=0) close(fd);
};
