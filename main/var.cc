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

#include <pthread.h>
#include "var.h"

tGlobalVars GVARS;

int METER_LENGTH=50;
int BLOCK_READ=512;
int LOCK_FILE_D=0;

tMainCfg CFG={
    5,100,100,100,1,300,NULL,NULL,2,1,0,
    0,0,NULL,
    5,0,
    1,1,0,0,0,1,1,600,
    {0,0},0,1,0,0,40,40,500,400,300,1,150,50,0,1,0,20,30,
    NULL,0,NULL,NULL,1,NULL,0,NULL,NULL,0,0,0,0,
    1,1,1,1,1,
    3,2*1024,20*1024
};

char *DEFAULT_PROTO="ftp";
char *DEFAULT_PASS="-chuchelo@krasu.ru";
char *DEFAULT_USER="anonymous";
char *HOME_PAGE="http://www.krasu.ru/soft/chuchelo";

char *FTP_SERVER_OK="220";
char *FTP_USER_OK="331";
char *FTP_PASS_OK="230";
char *FTP_PASV_OK="227";
char *FTP_PORT_OK="200";
char *FTP_CWD_OK="250";
char *FTP_RETR_OK="150";
char *FTP_QUIT_OK="221";
char *FTP_READ_OK="226";
char *FTP_ABOR_OK="225";
char *FTP_REST_OK="350";

tHistory *UrlHistory=NULL;
tHistory *PathHistory=NULL;
tHistory *FileHistory=NULL;
tHistory *LogHistory=NULL;
tHistory *UserHistory=NULL;
tHistory *ProxyHistory=NULL;
tHistory *LoadSaveHistory=NULL;
tHostsLimits *LimitsForHosts=NULL;
tDB *ALL_DOWNLOADS;
char *HOME_VARIABLE=NULL;
