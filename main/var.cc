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

#include <pthread.h>
#include "var.h"

tGlobalVars GVARS;

int METER_LENGTH=50;
int BLOCK_READ=512;
int LOCK_FILE_D=0;

tMainCfg CFG={
    5,100,100,1,300,NULL,NULL,NULL,NULL,2,1,0,
    100,0,0,0,NULL,
    5,0,
    1,1,0,0,0,1,1,600,
    {0,0},0,1,0,0,40,40,500,400,300,1,150,50,0,1,0,20,30,0,5,1,NULL,
    0xFFFFFF,0x555555,0xAAAAAA,0,
    NULL,0,NULL,NULL,1,NULL,0,NULL,NULL,0,0,0,0,
    1,1,1,1,1,
    3,1024,10*1024,
    NULL,0,
    1,1,1,1
};

char *DEFAULT_PROTO="ftp";
char *DEFAULT_PASS="-chuchelo@krasu.ru";
char *DEFAULT_USER="anonymous";
char *HOME_PAGE="http://www.krasu.ru/soft/chuchelo";

tHistory *ALL_HISTORIES[LAST_HISTORY];
tCookiesTree *COOKIES=NULL;

tHostsLimits *LimitsForHosts=NULL;
tUserPassTree *PasswordsForHosts=NULL;
tDB *ALL_DOWNLOADS;
char *HOME_VARIABLE=NULL;
int GLOBAL_SLEEP_DELAY=2;

void var_check_limits_int(int lower_value,int upper_value,int *value){
	if (*value>upper_value) *value=upper_value;
	if (*value<lower_value) *value=lower_value;
};

void var_check_all_limits(){
	var_check_limits_int(1,50,&CFG.MAX_THREADS);
	var_check_limits_int(100,999,&CFG.MAX_LOG_LENGTH);
	var_check_limits_int(1,999,&CFG.RETRY_TIME_OUT);
	var_check_limits_int(0,999,&CFG.MAX_RETRIES);
	var_check_limits_int(30,999,&CFG.TIME_OUT);
	var_check_limits_int(100,999,&CFG.MAX_MAIN_LOG_LENGTH);
	var_check_limits_int(0,5000,&CFG.ROLLBACK);
	var_check_limits_int(0,999,&CFG.FTP_RECURSE_DEPTH);
	var_check_limits_int(0,999,&CFG.HTTP_RECURSE_DEPTH);
	var_check_limits_int(100,99999,&CFG.SPEED_LIMIT_1);
	var_check_limits_int(100,99999,&CFG.SPEED_LIMIT_2);
	var_check_limits_int(1,999,&CFG.SAVE_LIST_INTERVAL);
	var_check_limits_int(1,999,&CFG.EXIT_COMPLETE_TIME);
};

const char *CFG_FILE=".ntrc/config";
const char *CFG_DIR=".ntrc";
