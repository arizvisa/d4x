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
#ifndef T_VARIABLE
#define T_VARIABLE


#define STATUS_TRIVIAL -1
#define STATUS_FATAL -3
#define STATUS_TIMEOUT -2
#define STATUS_CMD_ERR -4
#define STATUS_BAD_ANSWER -5
#define MAX_LEN 4048
enum{
	DOWNLOAD_STOP,
	DOWNLOAD_GO,
	DOWNLOAD_WAIT,
	DOWNLOAD_SIZE_WAIT,
	DOWNLOAD_FATAL,
	DOWNLOAD_COMPLETE,
	READY_TO_RUN,
	DOWNLOAD_REAL_STOP
};
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include "history.h"
#include "mainlog.h"
#include "dlist.h"
#include "sortstr.h"
#include "db.h"

struct tMainCfg{
	int RETRY_TIME_OUT;
	int MAX_RETRIES;
	int MAX_LOG_LENGTH;
	int MAX_MAIN_LOG_LENGTH;
	int MAX_THREADS;
	int TIME_OUT;
	char *GLOBAL_SAVE_PATH;
	char *DEFAULT_NAME;
	int FTP_RECURSE_DEPTH;
	int HTTP_RECURSE_DEPTH;
	int ROLLBACK;
/* Log
 */
	int SAVE_MAIN_LOG;
	int APPEND_REWRITE_LOG;
	char *SAVE_LOG_PATH;
/* List
 */
	int SAVE_LIST_INTERVAL;
	int SAVE_LIST;
/* Flags
 */
	int RETRY_IF_NOREGET;
	int RECURSIVE_OPTIMIZE;
	int DELETE_FATAL;
	int DELETE_COMPLETED;
	int FTP_PASSIVE_MODE;
	int FTP_PERMISIONS;
	int GET_DATE;
	int DEFAULT_PERMISIONS;
/* Interface
 */
	tTriger NICE_DEC_DIGITALS;
	int TIME_FORMAT;
	int USE_MAINWIN_TITLE;
	int USE_MAINWIN_TITLE2;
	int SCROLL_MAINWIN_TITLE;
	int WINDOW_X_POSITION;
	int WINDOW_Y_POSITION;
	int WINDOW_WIDTH;
	int WINDOW_HEIGHT;
	int WINDOW_CLIST_HEIGHT;
	int NEED_DIALOG_FOR_DND;
	int FACE_LIMITS_SIZE1;
	int FACE_LIMITS_SIZE2;
	int WINDOW_LOWER;
	int GRAPH_ORDER;
	int DND_TRASH,DND_TRASH_X,DND_TRASH_Y;
/* Proxies....
 */
	char *FTP_PROXY_HOST;
	int FTP_PROXY_PORT;
	char *FTP_PROXY_USER;
	char *FTP_PROXY_PASS;
	int FTP_PROXY_TYPE;
	char *HTTP_PROXY_HOST;
	int HTTP_PROXY_PORT;
	char *HTTP_PROXY_USER;
	char *HTTP_PROXY_PASS;
	int USE_PROXY_FOR_FTP;
	int USE_PROXY_FOR_HTTP;
	int NEED_PASS_HTTP_PROXY;
	int NEED_PASS_FTP_PROXY;
/*  Confirmation
 */
	int CONFIRM_DELETE;
	int CONFIRM_EXIT;
	int CONFIRM_DELETE_ALL;
	int CONFIRM_DELETE_COMPLETED;
	int CONFIRM_DELETE_FATALED;
/* Speeds
 */
 	int SPEED_LIMIT;
 	int SPEED_LIMIT_1;
 	int SPEED_LIMIT_2;
};

extern tMLog *MainLog;

extern int METER_LENGTH;
extern tMainCfg CFG;

struct tGlobalVars{
	pthread_mutex_t READED_BYTES_MUTEX;
	int READED_BYTES;
};

extern tGlobalVars GVARS;

extern tHistory *UrlHistory;
extern tHistory *PathHistory;
extern tHistory *FileHistory;
extern tHistory *LogHistory;
extern tHistory *UserHistory;
extern tHistory *ProxyHistory;
extern tHistory *LoadSaveHistory;

extern tHostsLimits *LimitsForHosts;

extern key_t LogsMsgQueue;

extern char *FTP_SERVER_OK;
extern char *FTP_USER_OK;
extern char *FTP_PASS_OK;
extern char *FTP_PASV_OK;
extern char *FTP_PORT_OK;
extern char *FTP_CWD_OK;
extern char *FTP_RETR_OK;
extern char *FTP_QUIT_OK;
extern char *FTP_READ_OK;
extern char *FTP_ABOR_OK;
extern char *FTP_REST_OK;

extern char *VERSION_NAME;
extern char *DEFAULT_PASS;
extern char *DEFAULT_USER;
extern char *HOME_PAGE;


extern char *DEFAULT_PROTO;
extern int BLOCK_READ;

extern char *HOME_VARIABLE;
extern char *LOCK_FILE;
extern int LOCK_FILE_D;
extern tDB *ALL_DOWNLOADS;
#endif