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


#include "base64.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>

char Table64[64]={  'A','B','C','D','E','F','G','H',
		    'I','J','K','L','M','N','O','P',
		    'Q','R','S','T','U','V','W','X',
		    'Y','Z','a','b','c','d','e','f',
		    'g','h','i','j','k','l','m','n',
		    'o','p','q','r','s','t','u','v',
		    'w','x','y','z','0','1','2','3',
		    '4','5','6','7','8','9','+','/'
};

void three_to_four(const unsigned char *what,unsigned char *where) {
	*where=(*what >> 2) & 63;
	*(where+1)=((*what << 4) | (*(what+1) >> 4)) & 63;
	*(where+2)=((*(what+1) << 2) | (*(what+2) >> 6)) & 63;
	*(where+3)=*(what+2) & 63;
	// Unroll this loop to produce faster code:
	//   for (int i=0;i<4;where[i++]= Table64[where[i]]);
	where[0]= Table64[where[0]];
	where[1]= Table64[where[1]];
	where[2]= Table64[where[2]];
	where[3]= Table64[where[3]];
};

char *string_to_base64(const char *what) {
	int len=strlen(what),len2=0;
	char *rvalue;
	len2=(len/3 + int((len%3)!=0))*4 +1;
	rvalue=new char[len2];
	unsigned char *tmp=(unsigned char *)rvalue;
	unsigned char four[4];
	while (len>=3) {
		three_to_four((unsigned char *)what,four);
		//*tmp=*four; *++tmp=four[1]; *++tmp=four[2]; *++tmp=four[3]; ++tmp;
		*((uint32_t *)tmp)=*((uint32_t*)four);
		tmp+=4;
		len-=3;
		what+=3;
	};
	if (len) {
 		unsigned char three[3]={0,0,0};
		int i;
		for (i=0;i<len;three[i++]=*((unsigned char*)what++));
		three_to_four(three,four);
		for (i+=1;i<4;four[i++]='=');
		// *tmp=*four; *++tmp=four[1]; *++tmp=four[2]; *++tmp=four[3]; ++tmp;
		*((uint32_t *)tmp)=*((uint32_t*)four);
		tmp+=4;
	};
	*tmp=0;
	return rvalue;
};
