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

#include "base64.h"
#include <string.h>
#include <stdio.h>

char Table64[64]={  'A','B','C','D','E','F','G','H',
		    'I','J','K','L','M','N','O','P',
		    'Q','R','S','T','U','V','W','X',
		    'Y','Z','a','b','c','d','e','f',
		    'g','h','i','j','k','l','m','n',
		    'o','p','q','r','s','t','u','v',
		    'w','x','y','z','0','1','2','3',
		    '4','5','6','7','8','9','+','/'
};

void three_to_four(unsigned char *what,unsigned char *where) {
	*where=(*what >> 2) & 63;
	*(where+1)=((*what << 4) | (*(what+1) >> 4)) & 63;
	*(where+2)=((*(what+1) << 2) | (*(what+2) >> 6)) & 63;
	*(where+3)=*(what+2) & 63;
	for (int i=0;i<4;i++) {
		where[i]= Table64[where[i]];
	};
};

char *string_to_base64(char *what) {
	int len=strlen(what),len2=0;
	char *rvalue;
	if (len%3) len2=(len/3 +1)*4 +1;
	else len2=(len/3)*4 +1;
	rvalue=new char[len2];
	unsigned char *tmp=(unsigned char *)rvalue;
	unsigned char four[4];
	while (len>=3) {
		three_to_four((unsigned char *)what,four);
		for (int i=0;i<4;i++)
			*(tmp++)=four[i];
		len-=3;
		what+=3;
	};
	if (len) {
		unsigned char three[3]={0,0,0};
		for (int i=0;i<len;i++) {
			three[i]=*((unsigned char*)what);
			what++;
		};
		three_to_four(three,four);
		for (int i=3-(len==1);i<4;i++)
			four[i]='=';
		for (int i=0;i<4;i++)
			*(tmp++)=four[i];
	};
	*tmp=0;
	return rvalue;
};
