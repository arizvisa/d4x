/*	WebDownloader for X-Window
 *	Copyright (C) 1999 Koshelev Maxim
 *	This Program is free but not GPL!!! You can't modify it
 *	without agreement with author. You can't distribute modified
 *	program but you can distribute unmodified program.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <pthread.h>
#include "var.h"

tGlobalVars GVARS;

int METER_LENGTH=50;
int BLOCK_READ=512;
int LOCK_FILE_D=0;

tMainCfg CFG={
    5,100,100,1,300,NULL,NULL,NULL,2,1,0,
    100,0,0,0,NULL,
    5,0,
    1,1,0,0,0,1,1,600,
    {0,0},0,1,0,0,40,40,500,400,300,1,150,50,0,1,0,20,30,0,5,
    0xFFFFFF,0x555555,0xAAAAAA,0,
    NULL,0,NULL,NULL,1,NULL,0,NULL,NULL,0,0,0,0,
    1,1,1,1,1,
    3,1024,10*1024,
    NULL,0
};

char *DEFAULT_PROTO="ftp";
char *DEFAULT_PASS="-chuchelo@krasu.ru";
char *DEFAULT_USER="anonymous";
char *HOME_PAGE="http://www.krasu.ru/soft/chuchelo";

tHistory *ALL_HISTORIES[LAST_HISTORY];

tHostsLimits *LimitsForHosts=NULL;
tDB *ALL_DOWNLOADS;
char *HOME_VARIABLE=NULL;
int GLOBAL_SLEEP_DELAY=2;
