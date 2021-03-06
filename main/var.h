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

#ifndef T_VARIABLE
#define T_VARIABLE


#define STATUS_TRIVIAL -1
#define STATUS_FATAL -3
#define STATUS_TIMEOUT -2
#define STATUS_CMD_ERR -4
#define STATUS_UNSPEC_ERR -5
#define STATUS_BAD_ANSWER -6
#define STATUS_NOT_FOUND -7
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
#include <sys/types.h>
#include <glib.h>
#include "history.h"
#include "mainlog.h"
#include "dlist.h"
#include "db.h"
#include "cookie.h"
#include "dbc.h"
#include "sm.h"
#include "dqueue.h"
#include "mutex.h"

enum DOUBLE_CLICK_ACTIONS{
	DBCLA_OPENLOG,
	DBCLA_EDIT,
	DBCLA_OPENFILE,
	DBCLA_LAST
};
struct tMainCfg{
	tSimplyCfg DEFAULT_CFG;
	int MAX_LOG_LENGTH;
	char *GLOBAL_SAVE_PATH;
	char *LOCAL_SAVE_PATH;
	char *DEFAULT_NAME;
	char *USER_AGENT;
	char *ANONYMOUS_PASS;
	char *DEFAULT_FILTER;
	int ALLOW_FORCE_RUN;
	int NUMBER_OF_PARTS;
/* Log
 */
	int MAX_MAIN_LOG_LENGTH;
	int MAIN_LOG_DETAILED;
	int SAVE_MAIN_LOG;
	int APPEND_REWRITE_LOG;
	char *SAVE_LOG_PATH;
	long long MAIN_LOG_FILE_LIMIT;
	int WRITE_DESCRIPTION;
/* List
 */
	int SAVE_LIST_INTERVAL;
	int SAVE_LIST;
/* Flags
 */
	int RECURSIVE_OPTIMIZE;
	int DEFAULT_PERMISIONS;
	int FTP_DIR_IN_LOG;
	int PAUSE_AFTER_ADDING;
/* Interface
 */
	int USE_MAINWIN_TITLE;
	int USE_MAINWIN_TITLE2;
	int SCROLL_MAINWIN_TITLE;
	int WINDOW_X_POSITION;
	int WINDOW_Y_POSITION;
	int WINDOW_WIDTH;
	int WINDOW_HEIGHT;
	int WINDOW_CLIST_HEIGHT;
	int WINDOW_CLIST_WIDTH;
	int NEED_DIALOG_FOR_DND;
	int WINDOW_LOWER;
	int GRAPH_ORDER;
	int DND_TRASH,DND_TRASH_X,DND_TRASH_Y;
	int EXIT_COMPLETE,EXIT_COMPLETE_TIME;
	int FIXED_LOG_FONT;
	int PROGRESS_MODE;
	float CLIST_SHIFT;
	int DONOTSET_WINPOS;
	int WINDOW_TREE_WIDTH;
	int HIDE_MAIN_WINDOW;
	int GRAPH_ON_BASKET;
	int SHOW_SPEED_ON_BASKET;
	int DBLCLK_ACT;
/* Clipboard
 */
	int CLIPBOARD_MONITOR;
	int CLIPBOARD_SKIP_OR_CATCH;
	char *SKIP_IN_CLIPBOARD;
	char *CATCH_IN_CLIPBOARD;
/* Graph colors....
 */
	int GRAPH_BACK;
	int GRAPH_FORE1;
	int GRAPH_FORE2;
	int GRAPH_PICK;
	int GRAPH_MODE;
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
	int PROXY_NO_CACHE;
/* SOCKS */
	char *SOCKS_HOST;
	int SOCKS_PORT;
	char *SOCKS_USER;
	char *SOCKS_PASS;
/*  Confirmation
 */
	int CONFIRM_DELETE;
	int CONFIRM_EXIT;
	int CONFIRM_DELETE_ALL;
	int CONFIRM_DELETE_COMPLETED;
	int CONFIRM_DELETE_FATALED;
	int CONFIRM_OPENING_MANY;
/* Speeds
 */
 	int SPEED_LIMIT;
 	int SPEED_LIMIT_1;
 	int SPEED_LIMIT_2;
/* Session
 */
	char *EXEC_WHEN_QUIT;
	int REMEMBER_PASS;
/* Buttons
 */
	int BUTTONS_FLAGS;
/* SPECIAL THINGS
 */
	int WITHOUT_FACE;
	int COLORIFIED_OUTPUT;
	int DND_NEED_POPUP;
	int USE_DEFAULT_CFG;
	int OFFLINE_MODE;
/* FTP SEARCH
*/
	int SEARCH_PING_TIMES;
	int SEARCH_ENTRIES;
	int SEARCH_PERSERVER;
	char *SEARCH_ENGINES;
/* SOUNDS
 */
	int ENABLE_SOUNDS;
	int ESD_SOUND;
	char *SOUND_STARTUP;
	char *SOUND_COMPLETE;
	char *SOUND_FAIL;
	char *SOUND_DND_DROP;
	char *SOUND_ADD;
	char *SOUND_QUEUE_FINISH;
/* THEMES
 */
	int USE_THEME;
	char *THEME_FILE;
	char *THEMES_DIR;
};

extern tMLog *MainLog;
extern tCookiesTree *COOKIES;


extern int METER_LENGTH;
extern int GRAPH_METER_LENGTH;
extern tMainCfg CFG;

struct tGlobalVars{
	d4x::Mutex MUTEX;
	unsigned long long READED_BYTES;
	d4x::SocketsHistory *SOCKETS;
};

extern tGlobalVars GVARS;

enum HISTORIES_ENUM{
	URL_HISTORY=0,
	PATH_HISTORY,
	FILE_HISTORY,
	LOG_HISTORY,
	USER_HISTORY,
	PROXY_HISTORY,
	LOAD_SAVE_HISTORY,
	USER_AGENT_HISTORY,
	EXEC_HISTORY,
	PASS_HISTORY,
	SKIP_HISTORY,
	SAVE_HISTORY,
	LOG_SAVE_HISTORY,
	DESC_HISTORY,
	REFERER_HISTORY,
	COOKIE_HISTORY,
	SOUNDS_HISTORY,
	THEMES_HISTORY,
	LAST_HISTORY
};

extern tHistory *ALL_HISTORIES[LAST_HISTORY];

extern char *VERSION_NAME;
extern char *DEFAULT_PASS;
extern char *DEFAULT_USER;
extern char *HOME_PAGE;
extern char *LOCALE_CODEPAGE;


extern char *DEFAULT_PROTO;
extern int BLOCK_READ;

extern char *HOME_VARIABLE;
extern char *LOCK_FILE;
extern int LOCK_FILE_D;
/*time for sleep in secs from previous update of
  interface to next one
 */
extern int GLOBAL_SLEEP_DELAY; 

extern tDB *ALL_DOWNLOADS;
void var_check_all_limits();
void var_copy_cfg(tMainCfg *dst,tMainCfg *src);
void var_free(tMainCfg *dst);

extern const char *CFG_FILE;
extern const char *CFG_DIR;

extern char *SPEED_LIMITATIONS_NAMES[];
extern d4xDUpdate D4X_UPDATE;

extern tQueue *D4X_THEME_DATA;
#endif
