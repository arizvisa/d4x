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

#include <pthread.h>
#include "var.h"
#include "dbc.h"
#include "locstr.h"
#include "ntlocale.h"
#include "signal.h"

tGlobalVars GVARS;

int METER_LENGTH=50;
int BLOCK_READ=1500;
int LOCK_FILE_D=0;

tMainCfg CFG={
	{300,5,0,100,0,1,0,0,
	 0,0,0,0,0,1,1,1,0,0,0,0,1,0,0,
	 0,0},
	100,NULL,NULL,NULL,NULL,NULL,NULL,0,
	100,0,0,0,NULL,0,0, //Log
	5,0, //List
	1,600,0,0, //flags
	1,0,0,40,40,500,400,300,300,1,0,1,0,20,30,0,5,1,1,0,0,100,0,//interface
	0,1,NULL,NULL, //clipboard
	0xFFFFFF,0x555555,0xAAAAAA,0,0,
	/* Proxy */
	NULL,0,NULL,NULL,1,NULL,0,NULL,NULL,0,0,0,0,0,
	/* SOCKS */
	NULL,0,NULL,NULL,
	1,1,1,1,1,1,
	3,1024,10*1024,
	NULL,0,
	0x0FFFFFFF,
	0,0,1,
	1,0,15,
	1,0,(char*)NULL,(char*)NULL,(char*)NULL,(char*)NULL,(char*)NULL,(char*)NULL,
	0,(char*)NULL
};

char *DEFAULT_PROTO="ftp";
char *DEFAULT_USER="anonymous";
char *HOME_PAGE="http://www.krasu.ru/soft/chuchelo";

tHistory *ALL_HISTORIES[LAST_HISTORY];
tCookiesTree *COOKIES=NULL;
tQueue *D4X_THEME_DATA=(tQueue *)NULL;

tUserPassTree *PasswordsForHosts=NULL;
tDB *ALL_DOWNLOADS;
char *HOME_VARIABLE=NULL;
int GLOBAL_SLEEP_DELAY=2;
d4xDUpdate D4X_UPDATE;

void var_check_limits_int(int lower_value,int upper_value,int *value){
	if (*value>upper_value) *value=upper_value;
	if (*value<lower_value) *value=lower_value;
};

void var_check_limits_long(long int lower_value,long int upper_value,long int *value){
	if (*value>upper_value) *value=upper_value;
	if (*value<lower_value) *value=lower_value;
};

void var_check_all_limits(){
	var_check_limits_int(100,999,&CFG.MAX_LOG_LENGTH);
	var_check_limits_int(1,999,&CFG.DEFAULT_CFG.time_for_sleep);
	var_check_limits_int(0,999,&CFG.DEFAULT_CFG.number_of_attempts);
	var_check_limits_int(30,999,&CFG.DEFAULT_CFG.timeout);
	var_check_limits_int(100,9999,&CFG.MAX_MAIN_LOG_LENGTH);
	var_check_limits_int(0,5000,&CFG.DEFAULT_CFG.rollback);
	var_check_limits_int(0,999,&CFG.DEFAULT_CFG.ftp_recurse_depth);
	var_check_limits_int(0,999,&CFG.DEFAULT_CFG.http_recurse_depth);
	var_check_limits_int(100,99999,&CFG.SPEED_LIMIT_1);
	var_check_limits_int(100,99999,&CFG.SPEED_LIMIT_2);
	var_check_limits_int(1,999,&CFG.SAVE_LIST_INTERVAL);
	var_check_limits_int(1,999,&CFG.EXIT_COMPLETE_TIME);
	var_check_limits_long(0,99999,&CFG.MAIN_LOG_FILE_LIMIT);
	var_check_limits_int(1,100,&CFG.SEARCH_PING_TIMES);
	var_check_limits_int(1,30,&CFG.SEARCH_ENTRIES);
};

const char *CFG_FILE=".ntrc/config";
const char *CFG_DIR=".ntrc";

char *SPEED_LIMITATIONS_NAMES[]={
	"",
	N_("low"),
	N_("medium"),
	N_("unlimited")
};

void var_free(tMainCfg *dst){
	if (dst->EXEC_WHEN_QUIT) delete[] dst->EXEC_WHEN_QUIT;
	if (dst->HTTP_PROXY_PASS) delete[] dst->HTTP_PROXY_PASS;
	if (dst->HTTP_PROXY_USER) delete[] dst->HTTP_PROXY_USER;
	if (dst->HTTP_PROXY_HOST) delete[] dst->HTTP_PROXY_HOST;
	if (dst->FTP_PROXY_PASS) delete[] dst->FTP_PROXY_PASS;
	if (dst->FTP_PROXY_USER) delete[] dst->FTP_PROXY_USER;
	if (dst->FTP_PROXY_HOST) delete[] dst->FTP_PROXY_HOST;
	if (dst->SKIP_IN_CLIPBOARD) delete[] dst->SKIP_IN_CLIPBOARD;
	if (dst->CATCH_IN_CLIPBOARD) delete[] dst->CATCH_IN_CLIPBOARD;
	if (dst->SAVE_LOG_PATH) delete[] dst->SAVE_LOG_PATH;
	if (dst->GLOBAL_SAVE_PATH) delete[] dst->GLOBAL_SAVE_PATH;
	if (dst->LOCAL_SAVE_PATH) delete[] dst->LOCAL_SAVE_PATH;
	if (dst->DEFAULT_NAME) delete[] dst->DEFAULT_NAME;
	if (dst->USER_AGENT) delete[] dst->USER_AGENT;
	if (dst->ANONYMOUS_PASS) delete[] dst->ANONYMOUS_PASS;
	if (dst->SOCKS_PASS) delete[] dst->SOCKS_PASS;
	if (dst->SOCKS_USER) delete[] dst->SOCKS_USER;
	if (dst->SOCKS_HOST) delete[] dst->SOCKS_HOST;
	if (dst->SOUND_ADD) delete[] dst->SOUND_ADD;
	if (dst->SOUND_COMPLETE) delete[] dst->SOUND_COMPLETE;
	if (dst->SOUND_FAIL) delete[] dst->SOUND_FAIL;
	if (dst->SOUND_DND_DROP) delete[] dst->SOUND_DND_DROP;
	if (dst->SOUND_QUEUE_FINISH) delete[] dst->SOUND_QUEUE_FINISH;
	if (dst->SOUND_STARTUP) delete[] dst->SOUND_STARTUP;
	if (dst->DEFAULT_FILTER) delete[] dst->DEFAULT_FILTER;
	if (dst->THEME_FILE) delete[] dst->THEME_FILE;
};

void var_copy_cfg(tMainCfg *dst,tMainCfg *src){
	dst->DEFAULT_CFG.copy_ints(&(src->DEFAULT_CFG));
	dst->MAX_LOG_LENGTH=src->MAX_LOG_LENGTH;
	dst->ALLOW_FORCE_RUN=src->ALLOW_FORCE_RUN;
	dst->MAX_MAIN_LOG_LENGTH=src->MAX_MAIN_LOG_LENGTH;
	dst->MAIN_LOG_DETAILED=src->MAIN_LOG_DETAILED;
	dst->SAVE_MAIN_LOG=src->SAVE_MAIN_LOG;
	dst->APPEND_REWRITE_LOG=src->APPEND_REWRITE_LOG;
	dst->MAIN_LOG_FILE_LIMIT=src->MAIN_LOG_FILE_LIMIT;
	dst->WRITE_DESCRIPTION=src->WRITE_DESCRIPTION;
	dst->SAVE_LIST_INTERVAL=src->SAVE_LIST_INTERVAL;
	dst->SAVE_LIST=src->SAVE_LIST;
	dst->RECURSIVE_OPTIMIZE=src->RECURSIVE_OPTIMIZE;
	dst->DEFAULT_PERMISIONS=src->DEFAULT_PERMISIONS;
	dst->FTP_DIR_IN_LOG=src->FTP_DIR_IN_LOG;
	dst->PAUSE_AFTER_ADDING=src->PAUSE_AFTER_ADDING;
	dst->USE_MAINWIN_TITLE=src->USE_MAINWIN_TITLE;
	dst->USE_MAINWIN_TITLE2=src->USE_MAINWIN_TITLE2;
	dst->SCROLL_MAINWIN_TITLE=src->SCROLL_MAINWIN_TITLE;
	dst->WINDOW_X_POSITION=src->WINDOW_X_POSITION;
	dst->WINDOW_Y_POSITION=src->WINDOW_Y_POSITION;
	dst->WINDOW_WIDTH=src->WINDOW_WIDTH;
	dst->WINDOW_HEIGHT=src->WINDOW_HEIGHT;
	dst->WINDOW_CLIST_HEIGHT=src->WINDOW_CLIST_HEIGHT;
	dst->WINDOW_CLIST_WIDTH=src->WINDOW_CLIST_WIDTH;
	dst->NEED_DIALOG_FOR_DND=src->NEED_DIALOG_FOR_DND;
	dst->WINDOW_LOWER=src->WINDOW_LOWER;
	dst->GRAPH_ORDER=src->GRAPH_ORDER;
	dst->DND_TRASH=src->DND_TRASH;
	dst->DND_TRASH_X=src->DND_TRASH_X;
	dst->DND_TRASH_Y=src->DND_TRASH_Y;
	dst->EXIT_COMPLETE=src->EXIT_COMPLETE;
	dst->EXIT_COMPLETE_TIME=src->EXIT_COMPLETE_TIME;
	dst->FIXED_LOG_FONT=src->FIXED_LOG_FONT;
	dst->CLIPBOARD_MONITOR=src->CLIPBOARD_MONITOR;
	dst->CLIPBOARD_SKIP_OR_CATCH=src->CLIPBOARD_SKIP_OR_CATCH;
	dst->GRAPH_BACK=src->GRAPH_BACK;
	dst->GRAPH_FORE1=src->GRAPH_FORE1;
	dst->GRAPH_FORE2=src->GRAPH_FORE2;
	dst->GRAPH_PICK=src->GRAPH_PICK;
	dst->GRAPH_MODE=src->GRAPH_MODE;
	dst->FTP_PROXY_PORT=src->FTP_PROXY_PORT;
	dst->FTP_PROXY_TYPE=src->FTP_PROXY_TYPE;
	dst->HTTP_PROXY_PORT=src->HTTP_PROXY_PORT;
	dst->USE_PROXY_FOR_FTP=src->USE_PROXY_FOR_FTP;
	dst->USE_PROXY_FOR_HTTP=src->USE_PROXY_FOR_HTTP;
	dst->NEED_PASS_HTTP_PROXY=src->NEED_PASS_HTTP_PROXY;
	dst->NEED_PASS_FTP_PROXY=src->NEED_PASS_FTP_PROXY;
	dst->PROXY_NO_CACHE=src->PROXY_NO_CACHE;
	dst->CONFIRM_DELETE=src->CONFIRM_DELETE;
	dst->CONFIRM_EXIT=src->CONFIRM_EXIT;
	dst->CONFIRM_DELETE_ALL=src->CONFIRM_DELETE_ALL;
	dst->CONFIRM_DELETE_COMPLETED=src->CONFIRM_DELETE_COMPLETED;
	dst->CONFIRM_DELETE_FATALED=src->CONFIRM_DELETE_FATALED;
	dst->CONFIRM_OPENING_MANY=src->CONFIRM_OPENING_MANY;
	dst->SPEED_LIMIT=src->SPEED_LIMIT;
	dst->SPEED_LIMIT_1=src->SPEED_LIMIT_1;
	dst->SPEED_LIMIT_2=src->SPEED_LIMIT_2;
	dst->REMEMBER_PASS=src->REMEMBER_PASS;
	dst->WITHOUT_FACE=src->WITHOUT_FACE;
	dst->DND_NEED_POPUP=src->DND_NEED_POPUP;
	dst->SEARCH_PING_TIMES=src->SEARCH_PING_TIMES;
	dst->SEARCH_HOST=src->SEARCH_HOST;
	dst->SEARCH_ENTRIES=src->SEARCH_ENTRIES;
	dst->SOCKS_PORT=src->SOCKS_PORT;
	dst->BUTTONS_FLAGS=src->BUTTONS_FLAGS;
	dst->PROGRESS_MODE=src->PROGRESS_MODE;
	dst->ENABLE_SOUNDS=src->ENABLE_SOUNDS;
	dst->ESD_SOUND=src->ESD_SOUND;
	dst->DONOTSET_WINPOS=src->DONOTSET_WINPOS;
	dst->USE_THEME=src->USE_THEME;
	/* strings */
	var_free(dst);
	dst->EXEC_WHEN_QUIT=copy_string(src->EXEC_WHEN_QUIT);
	dst->HTTP_PROXY_PASS=copy_string(src->HTTP_PROXY_PASS);
	dst->HTTP_PROXY_USER=copy_string(src->HTTP_PROXY_USER);
	dst->HTTP_PROXY_HOST=copy_string(src->HTTP_PROXY_HOST);
	dst->FTP_PROXY_PASS=copy_string(src->FTP_PROXY_PASS);
	dst->FTP_PROXY_USER=copy_string(src->FTP_PROXY_USER);
	dst->FTP_PROXY_HOST=copy_string(src->FTP_PROXY_HOST);
	dst->SKIP_IN_CLIPBOARD=copy_string(src->SKIP_IN_CLIPBOARD);
	dst->CATCH_IN_CLIPBOARD=copy_string(src->CATCH_IN_CLIPBOARD);
	dst->SAVE_LOG_PATH=copy_string(src->SAVE_LOG_PATH);
	dst->GLOBAL_SAVE_PATH=copy_string(src->GLOBAL_SAVE_PATH);
	dst->LOCAL_SAVE_PATH=copy_string(src->LOCAL_SAVE_PATH);
	dst->DEFAULT_NAME=copy_string(src->DEFAULT_NAME);
	dst->USER_AGENT=copy_string(src->USER_AGENT);
	dst->ANONYMOUS_PASS=copy_string(src->ANONYMOUS_PASS);
	dst->SOCKS_PASS=copy_string(src->SOCKS_PASS);
	dst->SOCKS_HOST=copy_string(src->SOCKS_HOST);
	dst->SOCKS_USER=copy_string(src->SOCKS_USER);
	dst->SOUND_FAIL=copy_string(src->SOUND_FAIL);
	dst->SOUND_COMPLETE=copy_string(src->SOUND_COMPLETE);
	dst->SOUND_STARTUP=copy_string(src->SOUND_STARTUP);
	dst->SOUND_ADD=copy_string(src->SOUND_ADD);
	dst->SOUND_DND_DROP=copy_string(src->SOUND_DND_DROP);
	dst->SOUND_QUEUE_FINISH=copy_string(src->SOUND_QUEUE_FINISH);
	dst->DEFAULT_FILTER=copy_string(src->DEFAULT_FILTER);
	dst->THEME_FILE=copy_string(src->THEME_FILE);
};
