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


#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "var.h"
#include "locstr.h"
#include "history.h"
#include "config.h"
#include "face/list.h"
#include "face/fsface.h"
#include "face/buttons.h"
#include "ntlocale.h"
#include "srvclt.h"

void get_size_of_clist();

tOption downloader_parsed_args[]={
	{"--help",		OPT_HELP},
	{"-h",			OPT_HELP},
	{"--version",		OPT_VERSION},
	{"-v",			OPT_VERSION},
	{"--info", 		OPT_INFO},
	{"-i",	 		OPT_INFO},
	{"--speed",		OPT_SPEED},
	{"-s",			OPT_SPEED},
	{"-t1",			OPT_TRAFFIC_LOW},
	{"--traffic-low",	OPT_TRAFFIC_LOW},
	{"-t2",			OPT_TRAFFIC_MIDDLE},
	{"--traffic-medium",	OPT_TRAFFIC_MIDDLE},
	{"-t3",			OPT_TRAFFIC_HIGH},
	{"--traffic-high",	OPT_TRAFFIC_HIGH},
	{"-d",			OPT_SET_DIRECTORY},
	{"--directory",		OPT_SET_DIRECTORY},
	{"-c",			OPT_DEL_COMPLETED},
	{"--delete-completed",	OPT_DEL_COMPLETED},
	{"-m",			OPT_SET_MAX_THREADS},
	{"--max-running",	OPT_SET_MAX_THREADS},
	{"-r",			OPT_RERUN_FAILED},
	{"--rerun-failed",	OPT_RERUN_FAILED},
	{"-w",			OPT_WITHOUT_FACE},
	{"--without-face",	OPT_WITHOUT_FACE},
	{"--minimized",		OPT_RUN_MINIMIZED},
	{"--exit-time",		OPT_EXIT_TIME},
	{"--ls",		OPT_LS},
	{"--switch",		OPT_SWITCH},
	{"--del",		OPT_DEL},
	{"--stop",		OPT_STOP},
	{"--color",		OPT_COLOR},
	{"-geometry",		OPT_GEOMETRY},
	{"-a",			OPT_ADD_OPEN}
};

char *downloader_args_errors[]={
	(char *)NULL,
	(char *)NULL,
	(char *)NULL,
	(char *)NULL,
	(char *)NULL,
	(char *)NULL,
	(char *)NULL,	
	(char *)NULL,
	(char *)N_("You forgot to specify directory."),
	(char *)NULL,
	(char *)N_("You must specify number as second parameter for '-m' option"),
	(char *)NULL,
	(char *)NULL,
	(char *)NULL,
	(char *)N_("You must specify number as parameter for '--exit-time' option"),
	(char *)N_("Expect URL as parameter for '--ls' option"),
	(char *)NULL,
	(char *)N_("Expect URL as parameter for '--del' option"),
	(char *)N_("Expect string in format WIDTHxHEIGHT+X+Y as parameter for '-geometry'"),
	(char *)N_("Expect URL as parameter for '--stop' option"),
	(char *)NULL,
	(char *)N_("Expect URL as parameter for '-a' option")
};


tConfigVariable config_variables[]={
	{"max_log",		CV_TYPE_INT,	&(CFG.MAX_LOG_LENGTH)},
	{"max_main_log",	CV_TYPE_INT,	&(CFG.MAX_MAIN_LOG_LENGTH)},
	{"max_retries",		CV_TYPE_INT,	&(CFG.DEFAULT_CFG.number_of_attempts)},
	{"timeout",		CV_TYPE_INT,	&(CFG.DEFAULT_CFG.timeout)},
	{"break_timeout",	CV_TYPE_INT,	&(CFG.DEFAULT_CFG.time_for_sleep)},
	{"optimize",		CV_TYPE_BOOL,	&(CFG.RECURSIVE_OPTIMIZE)},
	{"ftp_passive_mode",	CV_TYPE_BOOL,	&(CFG.DEFAULT_CFG.passive)},
	{"retry_if_noreget",	CV_TYPE_BOOL,	&(CFG.DEFAULT_CFG.retry)},
	{"ftp_dirontop",	CV_TYPE_BOOL,	&(CFG.DEFAULT_CFG.ftp_dirontop)},
	{"sleeptime",		CV_TYPE_INT,	&(CFG.DEFAULT_CFG.time_for_sleep)},
	{"ihate_etag",		CV_TYPE_BOOL,	&(CFG.DEFAULT_CFG.ihate_etag)},
	{"savepath",		CV_TYPE_STRING,	&(CFG.GLOBAL_SAVE_PATH)},
	{"xposition",		CV_TYPE_INT,	&(CFG.WINDOW_X_POSITION)},
	{"yposition",		CV_TYPE_INT,	&(CFG.WINDOW_Y_POSITION)},
	{"windowwidth",		CV_TYPE_INT,	&(CFG.WINDOW_WIDTH)},
	{"windowheight",	CV_TYPE_INT,	&(CFG.WINDOW_HEIGHT)},
	{"clist_height",	CV_TYPE_INT,	&(CFG.WINDOW_CLIST_HEIGHT)},
	{"clist_width",		CV_TYPE_INT,	&(CFG.WINDOW_CLIST_WIDTH)},
	{"tree_width",		CV_TYPE_INT,	&(CFG.WINDOW_TREE_WIDTH)},
	{"ftp_proxy_host",	CV_TYPE_STRING,	&(CFG.FTP_PROXY_HOST)},
	{"ftp_proxy_user",	CV_TYPE_STRING,	&(CFG.FTP_PROXY_USER)},
	{"ftp_proxy_pass",	CV_TYPE_STRING,	&(CFG.FTP_PROXY_PASS)},
	{"ftp_proxy_type",	CV_TYPE_INT,	&(CFG.FTP_PROXY_TYPE)},
	{"http_proxy_host",	CV_TYPE_STRING,	&(CFG.HTTP_PROXY_HOST)},
	{"http_proxy_user",	CV_TYPE_STRING,	&(CFG.HTTP_PROXY_USER)},
	{"http_proxy_pass",	CV_TYPE_STRING,	&(CFG.HTTP_PROXY_PASS)},
	{"ftp_proxy_port",	CV_TYPE_INT,	&(CFG.FTP_PROXY_PORT)},
	{"http_proxy_port",	CV_TYPE_INT,	&(CFG.HTTP_PROXY_PORT)},
	{"proxy_no_cache",	CV_TYPE_BOOL,	&(CFG.PROXY_NO_CACHE)},
	{"need_pass_http_proxy",CV_TYPE_BOOL,	&(CFG.NEED_PASS_HTTP_PROXY)},
	{"need_pass_ftp_proxy",	CV_TYPE_BOOL,	&(CFG.NEED_PASS_FTP_PROXY)},
	{"use_proxy_for_ftp",	CV_TYPE_BOOL,	&(CFG.USE_PROXY_FOR_FTP)},
	{"use_proxy_for_http",	CV_TYPE_BOOL,	&(CFG.USE_PROXY_FOR_HTTP)},
	{"save_main_log",	CV_TYPE_BOOL,	&(CFG.SAVE_MAIN_LOG)},
	{"append_rewrite_log",	CV_TYPE_INT,	&(CFG.APPEND_REWRITE_LOG)},
	{"save_log_path",	CV_TYPE_STRING,	&(CFG.SAVE_LOG_PATH)},
	{"ftp_permisions",	CV_TYPE_INT,	&(CFG.DEFAULT_CFG.permisions)},
	{"save_list",		CV_TYPE_BOOL,	&(CFG.SAVE_LIST)},
	{"interval_save_list",	CV_TYPE_INT,	&(CFG.SAVE_LIST_INTERVAL)},
	{"use_mainwin_title",	CV_TYPE_BOOL,	&(CFG.USE_MAINWIN_TITLE)},
	{"use_mainwin_titleII",	CV_TYPE_BOOL,	&(CFG.USE_MAINWIN_TITLE2)},
	{"get_date_from_server",CV_TYPE_BOOL,	&(CFG.DEFAULT_CFG.get_date)},
	{"need_dialog_for_dnd",	CV_TYPE_BOOL,	&(CFG.NEED_DIALOG_FOR_DND)},
	{"window_lower",	CV_TYPE_BOOL,	&(CFG.WINDOW_LOWER)},
	{"confirm_exit",	CV_TYPE_BOOL,	&(CFG.CONFIRM_EXIT)},
	{"confirm_delete",	CV_TYPE_BOOL,	&(CFG.CONFIRM_DELETE)},
	{"confirm_delete_all",	CV_TYPE_BOOL,	&(CFG.CONFIRM_DELETE_ALL)},
	{"confirm_delete_completed",CV_TYPE_BOOL,&(CFG.CONFIRM_DELETE_COMPLETED)},
	{"confirm_delete_fataled",CV_TYPE_BOOL,	&(CFG.CONFIRM_DELETE_FATALED)},
	{"confirm_opening_many",CV_TYPE_BOOL,	&(CFG.CONFIRM_OPENING_MANY)},
	{"speed_limit",		CV_TYPE_INT,	&(CFG.SPEED_LIMIT)},
	{"speed_limit_one",	CV_TYPE_INT,	&(CFG.SPEED_LIMIT_1)},
	{"speed_limit_two",	CV_TYPE_INT,	&(CFG.SPEED_LIMIT_2)},
	{"graph_order",		CV_TYPE_BOOL,	&(CFG.GRAPH_ORDER)},
	{"ftp_recurse_depth",	CV_TYPE_INT,	&(CFG.DEFAULT_CFG.ftp_recurse_depth)},
	{"http_recurse_depth",	CV_TYPE_INT,	&(CFG.DEFAULT_CFG.http_recurse_depth)},
	{"default_name",	CV_TYPE_STRING,	&(CFG.DEFAULT_NAME)},
	{"scroll_mainwin_title",CV_TYPE_BOOL,	&(CFG.SCROLL_MAINWIN_TITLE)},
	{"default_permisions",	CV_TYPE_INT,	&(CFG.DEFAULT_PERMISIONS)},
	{"rollback",		CV_TYPE_INT,	&(CFG.DEFAULT_CFG.rollback)},
	{"dnd_trash",		CV_TYPE_BOOL,	&(CFG.DND_TRASH)},
	{"dnd_trash_x",		CV_TYPE_INT,	&(CFG.DND_TRASH_X)},
	{"dnd_trash_y",		CV_TYPE_INT,	&(CFG.DND_TRASH_Y)},
	{"exit_complete",	CV_TYPE_BOOL,	&(CFG.EXIT_COMPLETE)},
	{"exit_complete_time",	CV_TYPE_INT,	&(CFG.EXIT_COMPLETE_TIME)},
	{"main_log_detailed",	CV_TYPE_BOOL,	&(CFG.MAIN_LOG_DETAILED)},
	{"user_agent",		CV_TYPE_STRING,	&(CFG.USER_AGENT)},
	{"graph_back",		CV_TYPE_HEX,	&(CFG.GRAPH_BACK)},
	{"graph_fore1",		CV_TYPE_HEX,	&(CFG.GRAPH_FORE1)},
	{"graph_fore2",		CV_TYPE_HEX,	&(CFG.GRAPH_FORE2)},
	{"graph_pick",		CV_TYPE_HEX,	&(CFG.GRAPH_PICK)},
	{"graph_mode",		CV_TYPE_INT,	&(CFG.GRAPH_MODE)},
	{"exec_when_quit",	CV_TYPE_STRING,	&(CFG.EXEC_WHEN_QUIT)},
	{"remember_pass",	CV_TYPE_BOOL,	&(CFG.REMEMBER_PASS)},
	{"clipboard_monitor",	CV_TYPE_BOOL,	&(CFG.CLIPBOARD_MONITOR)},
	{"clipboard_skip_or_catch",	CV_TYPE_BOOL,	&(CFG.CLIPBOARD_SKIP_OR_CATCH)},
	{"skip_in_clipboard",	CV_TYPE_STRING,	&(CFG.SKIP_IN_CLIPBOARD)},
	{"catch_in_clipboard",	CV_TYPE_STRING,	&(CFG.CATCH_IN_CLIPBOARD)},
	{"buttons_flags",	CV_TYPE_HEX,	&(CFG.BUTTONS_FLAGS)},
	{"main_log_file_limit",	CV_TYPE_LONG,	&(CFG.MAIN_LOG_FILE_LIMIT)},
	{"fixed_log_font",	CV_TYPE_BOOL,	&(CFG.FIXED_LOG_FONT)},
	{"allow_force_run",	CV_TYPE_BOOL,	&(CFG.ALLOW_FORCE_RUN)},
	{"ftp_dir_in_log",	CV_TYPE_BOOL,	&(CFG.FTP_DIR_IN_LOG)},
	{"dont_send_quit",	CV_TYPE_BOOL,	&(CFG.DEFAULT_CFG.dont_send_quit)},
	{"follow_link",		CV_TYPE_BOOL,	&(CFG.DEFAULT_CFG.follow_link)},
	{"http_leave_server",	CV_TYPE_BOOL,	&(CFG.DEFAULT_CFG.leave_server)},
	{"http_leave_dir",	CV_TYPE_BOOL,	&(CFG.DEFAULT_CFG.dont_leave_dir)},
	{"sleep_before_complete",	CV_TYPE_BOOL,	&(CFG.DEFAULT_CFG.sleep_before_complete)},
	{"write_description",	CV_TYPE_BOOL,	&(CFG.WRITE_DESCRIPTION)},
	{"check_time",		CV_TYPE_BOOL,	&(CFG.DEFAULT_CFG.check_time)},
	{"pause_after_adding",	CV_TYPE_BOOL,	&(CFG.PAUSE_AFTER_ADDING)},
	{"search_ping_times",	CV_TYPE_INT,	&(CFG.SEARCH_PING_TIMES)},
	{"search_engines",	CV_TYPE_STRING,	&(CFG.SEARCH_ENGINES)},
	{"search_perserver",    CV_TYPE_INT,	&(CFG.SEARCH_PERSERVER)},
//	{"search_host",		CV_TYPE_INT,	&(CFG.SEARCH_HOST)},
	{"search_entries",	CV_TYPE_INT,	&(CFG.SEARCH_ENTRIES)},
	{"change_links",	CV_TYPE_BOOL,	&(CFG.DEFAULT_CFG.change_links)},
	{"anonymous_pass",	CV_TYPE_STRING,	&(CFG.ANONYMOUS_PASS)},
	{"socks_user",		CV_TYPE_STRING,	&(CFG.SOCKS_USER)},
	{"socks_pass",		CV_TYPE_STRING,	&(CFG.SOCKS_PASS)},
	{"socks_host",		CV_TYPE_STRING,	&(CFG.SOCKS_HOST)},
	{"socks_port",		CV_TYPE_INT,	&(CFG.SOCKS_PORT)},
	{"progress_mode",	CV_TYPE_INT,	&(CFG.PROGRESS_MODE)},
	{"enable_sounds",	CV_TYPE_BOOL,	&(CFG.ENABLE_SOUNDS)},
	{"esd_sound",		CV_TYPE_BOOL,	&(CFG.ESD_SOUND)},
	{"sound_complete",	CV_TYPE_STRING,	&(CFG.SOUND_COMPLETE)},
	{"sound_fail",		CV_TYPE_STRING,	&(CFG.SOUND_FAIL)},
	{"sound_add",		CV_TYPE_STRING,	&(CFG.SOUND_ADD)},
	{"sound_dnd_drop",	CV_TYPE_STRING,	&(CFG.SOUND_DND_DROP)},
	{"sound_queue_finish",	CV_TYPE_STRING,	&(CFG.SOUND_QUEUE_FINISH)},
	{"sound_startup",	CV_TYPE_STRING,	&(CFG.SOUND_STARTUP)},
	{"clist_shift",		CV_TYPE_FLOAT,	&(CFG.CLIST_SHIFT)},
	{"default_filter",	CV_TYPE_STRING,	&(CFG.DEFAULT_FILTER)},
	{"donotset_winpos",	CV_TYPE_BOOL,	&(CFG.DONOTSET_WINPOS)},
	{"hide_main_window",	CV_TYPE_BOOL,	&(CFG.HIDE_MAIN_WINDOW)},
	{"theme_file",		CV_TYPE_STRING,	&(CFG.THEME_FILE)},
	{"use_theme",		CV_TYPE_BOOL,	&(CFG.USE_THEME)},
	{"themes_dir",		CV_TYPE_STRING,	&(CFG.THEMES_DIR)},
	{"use_default_cfg",	CV_TYPE_BOOL,	&(CFG.USE_DEFAULT_CFG)},
	{"number_of_parts",	CV_TYPE_INT,	&(CFG.NUMBER_OF_PARTS)},
	{"graph_on_basket",	CV_TYPE_INT,	&(CFG.GRAPH_ON_BASKET)},
	{"speed_on_basket",	CV_TYPE_INT,	&(CFG.SHOW_SPEED_ON_BASKET)}
};

int downloader_parsed_args_num=sizeof(downloader_parsed_args)/sizeof(tOption);

void set_config(char *line){
	DBC_RETURN_IF_FAIL(line!=NULL);
	if(*line=='\n' || *line=='#') return;
	char *temp=new char[strlen(line)+1];
	char *next_word=extract_string(line,temp);
	int cv_list_len=sizeof(config_variables)/sizeof(struct tConfigVariable);
	for (int i=0;i<cv_list_len;i++){
		if (equal(config_variables[i].name,temp)){
			switch(config_variables[i].type){
			case CV_TYPE_FLOAT:{
				extract_string(next_word,temp);
				sscanf(temp,"%f",(float *)(config_variables[i].pointer));
				break;
			};
			case CV_TYPE_LONG:{
				extract_string(next_word,temp);
				sscanf(temp,"%li",(long int *)(config_variables[i].pointer));
				break;
			};
			case CV_TYPE_INT:{
				extract_string(next_word,temp);
				sscanf(temp,"%i",(int *)(config_variables[i].pointer));
				break;
			};
			case CV_TYPE_BOOL:{
				extract_string(next_word,temp);
				int tmp;
				if (sscanf(temp,"%i",&tmp)==1)
					*((int *)(config_variables[i].pointer))=tmp?1:0;
				break;
			};
			case CV_TYPE_HEX:{
				extract_string(next_word,temp);
				sscanf(temp,"%x",(int *)(config_variables[i].pointer));
				break;
			};
			case CV_TYPE_STRING:{
				char **tmp=(char **)(config_variables[i].pointer);
				*tmp=copy_string(skip_spaces(next_word));
				break;
			};
			};
			break;
		};
	};
	delete[] temp;
};

void read_config() {
	if (!HOME_VARIABLE)	return;
	char *cfgpath=compose_path(HOME_VARIABLE,CFG_FILE);
	int fd=open(cfgpath,O_RDONLY);
	if (fd>=0) {
		char temp[MAX_LEN];
		while(f_rstr(fd,temp,MAX_LEN)) {
			set_config(temp);
		};
		close(fd);
	} else {
		save_config();
		g_print(_("Can't open cfg file at '%s'\n"),cfgpath);
		g_print(_("Use default cfg :))...\n"));
	};
	delete[] cfgpath;
	load_strlist(ALL_HISTORIES[URL_HISTORY], ".ntrc_2/history1",0);
	load_strlist(ALL_HISTORIES[PATH_HISTORY],".ntrc_2/history2",1);
	load_strlist(ALL_HISTORIES[LOG_HISTORY],".ntrc_2/history3",0);
	load_strlist(ALL_HISTORIES[LOAD_SAVE_HISTORY],".ntrc_2/history4",1);
	load_strlist(ALL_HISTORIES[USER_HISTORY],".ntrc_2/history5",0);
	load_strlist(ALL_HISTORIES[PROXY_HISTORY],".ntrc_2/history6",0);
	load_strlist(ALL_HISTORIES[FILE_HISTORY],".ntrc_2/history7",0);
	load_strlist(ALL_HISTORIES[USER_AGENT_HISTORY],".ntrc_2/history8",0);
	load_strlist(ALL_HISTORIES[EXEC_HISTORY],".ntrc_2/history9",0);
	load_strlist(ALL_HISTORIES[SKIP_HISTORY],".ntrc_2/history11",0);
	load_strlist(ALL_HISTORIES[SAVE_HISTORY],".ntrc_2/history12",1);
	load_strlist(ALL_HISTORIES[LOG_SAVE_HISTORY],".ntrc_2/history13",1);
	load_strlist(ALL_HISTORIES[DESC_HISTORY],".ntrc_2/history14",0);
	load_strlist(ALL_HISTORIES[REFERER_HISTORY],".ntrc_2/history15",0);
	load_strlist(ALL_HISTORIES[COOKIE_HISTORY],".ntrc_2/history16",0);
	load_strlist(ALL_HISTORIES[SOUNDS_HISTORY],".ntrc_2/history17",1);
	load_strlist(ALL_HISTORIES[THEMES_HISTORY],".ntrc_2/history18",1);
	if (CFG.REMEMBER_PASS)
		load_strlist(ALL_HISTORIES[PASS_HISTORY],".ntrc_2/history10",0);
	ALL_HISTORIES[USER_AGENT_HISTORY]->add("%version");
	ALL_HISTORIES[USER_AGENT_HISTORY]->add("Mozilla/4.05");
	ALL_HISTORIES[USER_AGENT_HISTORY]->add("Mozilla/4.0 (compatible; MSIE 4.01; Windows 95)");
};
	
static void save_integer_to_config(int fd,char *name,int num) {
	DBC_RETURN_IF_FAIL(name!=NULL);
	char data[MAX_LEN];
	sprintf(data,"%s %i\n\n",name,num);
	f_wstr(fd,data);
};

static void save_long_to_config(int fd,char *name,long int num) {
	DBC_RETURN_IF_FAIL(name!=NULL);
	char data[MAX_LEN];
	sprintf(data,"%s %li\n\n",name,num);
	f_wstr(fd,data);
};

static void save_float_to_config(int fd,char *name,float num) {
	DBC_RETURN_IF_FAIL(name!=NULL);
	char data[MAX_LEN];
	sprintf(data,"%s %f\n\n",name,num);
	f_wstr(fd,data);
};
	
static void save_hex_integer_to_config(int fd,char *name,int num) {
	DBC_RETURN_IF_FAIL(name!=NULL);
	char data[MAX_LEN];
	sprintf(data,"%s 0x%06x\n\n",name,num);
	f_wstr(fd,data);
};

static void save_string_to_config(int fd,char *name,char *str) {
	DBC_RETURN_IF_FAIL(name!=NULL);
	if (!str) return;
	char data[MAX_LEN];
	sprintf(data,"%s %s\n\n",name,str);
	f_wstr(fd,data);
};

char *d4x_cfg_search_engines(){
	int count=D4X_SEARCH_ENGINES.count();
	d4xSearchEngine *first=D4X_SEARCH_ENGINES.first();
	char *rval=new char[count+1];
	char *cur=rval;
	while(first){
		if (first->used)
			*cur='1';
		else
			*cur='0';
		cur++;
		first=D4X_SEARCH_ENGINES.prev();
	};
	*cur=0;
	return rval;
};

void save_config() {
	if (!HOME_VARIABLE)	return;
	char *cfgpath=compose_path(HOME_VARIABLE,CFG_FILE);
	int fd=open(cfgpath,O_TRUNC | O_CREAT |O_RDWR,S_IRUSR | S_IWUSR);
	if (fd<0) {
		char *cfg_dir=compose_path(HOME_VARIABLE,CFG_DIR);
		mkdir(cfg_dir,S_IRWXU| S_IXGRP|S_IRGRP);
		fd=open(cfgpath,O_TRUNC | O_CREAT |O_RDWR,S_IRUSR | S_IWUSR);
		delete[] cfg_dir;
	};
	if (CFG.SEARCH_ENGINES) delete[] CFG.SEARCH_ENGINES;
	CFG.SEARCH_ENGINES=d4x_cfg_search_engines();
	char data[MAX_LEN];
	if (fd>=0) {
		lod_get_height();
		fs_list_get_size();
		int cv_list_len=sizeof(config_variables)/sizeof(struct tConfigVariable);
		for (int i=0;i<cv_list_len;i++){
			switch(config_variables[i].type){
			case CV_TYPE_FLOAT:{
				float *cv_tmp=(float *)(config_variables[i].pointer);
				save_float_to_config(fd,config_variables[i].name,*cv_tmp);
				break;
			};
			case CV_TYPE_LONG:{
				long int *cv_tmp=(long int *)(config_variables[i].pointer);
				save_long_to_config(fd,config_variables[i].name,*cv_tmp);
				break;
			};
			case CV_TYPE_INT:{
				int *cv_tmp=(int *)(config_variables[i].pointer);
				save_integer_to_config(fd,config_variables[i].name,*cv_tmp);
				break;
			};
			case CV_TYPE_BOOL:{
				int *cv_tmp=(int *)(config_variables[i].pointer);
				save_integer_to_config(fd,config_variables[i].name,*cv_tmp);
				break;
			};
			case CV_TYPE_HEX:{
				int *cv_tmp=(int *)(config_variables[i].pointer);
				save_hex_integer_to_config(fd,config_variables[i].name,*cv_tmp);
				break;
			};
			case CV_TYPE_STRING:{
				char **cv_tmp=(char **)(config_variables[i].pointer);
				save_string_to_config(fd,config_variables[i].name,*cv_tmp);
				break;
			};
			};
		};
		close(fd);
	} else {
		if (MainLog) {
			sprintf(data,"Can't write cfgfile to:%s",cfgpath);
			MainLog->add(data,LOG_ERROR);
		} else
			g_print("Can't write cfgfile to:%s\n",cfgpath);
	};
	save_strlist(ALL_HISTORIES[URL_HISTORY], ".ntrc_2/history1");
	save_strlist(ALL_HISTORIES[PATH_HISTORY],".ntrc_2/history2");
	save_strlist(ALL_HISTORIES[LOG_HISTORY],".ntrc_2/history3");
	save_strlist(ALL_HISTORIES[LOAD_SAVE_HISTORY],".ntrc_2/history4");
	save_strlist(ALL_HISTORIES[USER_HISTORY],".ntrc_2/history5");
	save_strlist(ALL_HISTORIES[PROXY_HISTORY],".ntrc_2/history6");
	save_strlist(ALL_HISTORIES[FILE_HISTORY],".ntrc_2/history7");
	save_strlist(ALL_HISTORIES[USER_AGENT_HISTORY],".ntrc_2/history8");
	save_strlist(ALL_HISTORIES[EXEC_HISTORY],".ntrc_2/history9");
	save_strlist(ALL_HISTORIES[PASS_HISTORY],".ntrc_2/history10");
	save_strlist(ALL_HISTORIES[SKIP_HISTORY],".ntrc_2/history11");
	save_strlist(ALL_HISTORIES[SAVE_HISTORY],".ntrc_2/history12");
	save_strlist(ALL_HISTORIES[LOG_SAVE_HISTORY],".ntrc_2/history13");
	save_strlist(ALL_HISTORIES[DESC_HISTORY],".ntrc_2/history14");
	save_strlist(ALL_HISTORIES[REFERER_HISTORY],".ntrc_2/history15");
	save_strlist(ALL_HISTORIES[COOKIE_HISTORY],".ntrc_2/history16");
	save_strlist(ALL_HISTORIES[SOUNDS_HISTORY],".ntrc_2/history17");
	save_strlist(ALL_HISTORIES[THEMES_HISTORY],".ntrc_2/history18");
	delete[] cfgpath;
};

void save_strlist(tStringList *what,char *where) {
	if (!what || !where) return;
	if (!HOME_VARIABLE) return;
	char *path=compose_path(HOME_VARIABLE,where);
	int fd=open(path,O_TRUNC | O_CREAT |O_RDWR,S_IRUSR | S_IWUSR);
	if (fd>0) {
		tString *tmp=what->first();
		while(tmp) {
			f_wstr_lf(fd,tmp->body);
			tmp=what->prev();
		};
		close(fd);
	};
	delete[] path;
};

void load_strlist(tStringList *where,char *what,int normalize) {
	if (!what || !where) return;
	if (!HOME_VARIABLE) return;
	char *path=compose_path(HOME_VARIABLE,what);
	int fd=open(path,O_RDONLY);
	where->done();
	if (fd>=0) {
		char temp[MAX_LEN];
		char *cur=temp;
		int len=0;
		while(read(fd,cur,1)) {
			while(*cur!='\n' && len<(MAX_LEN-1)) {
				cur++;
				len++;
				if (read(fd,cur,1)==0) break;
			};
			*cur=0;
			if (strlen(temp)) {
				if (normalize)
					normalize_path(temp);
				where->add(temp);
			};
			cur=temp;
		};
		close(fd);
	};
	delete[] path;
};

int downloader_args_type(char *str){
	DBC_RETVAL_IF_FAIL(str!=NULL,OPT_UNKNOWN);
	for (int i=0;i<downloader_parsed_args_num;i++){
		if (equal(str,downloader_parsed_args[i].name))
			return downloader_parsed_args[i].cmd;
	};
	return OPT_UNKNOWN;
};

int parse_command_line_preload(int argc,char **argv){
	int rvalue=0;
	for (int i=1;i<argc;i++){
		int option=downloader_args_type(argv[i]);
		switch(option){
		case OPT_VERSION:{
			puts(VERSION_NAME);
			rvalue=1;
			break;
		};
		case OPT_HELP:{
			help_print();
			rvalue=1;
			break;
		};
		case OPT_UNKNOWN:{
			if (argv[i] && *argv[i]=='-') {
				g_print(_("unknown option:"));
				g_print(" %s\n",argv[i]);
			}
			break;
		};
		case OPT_WITHOUT_FACE:{
			CFG.WITHOUT_FACE=1;
			break;
		};
		case OPT_COLOR:{
			CFG.COLORIFIED_OUTPUT=1;
			break;
		};
		};
	};
	return rvalue;
};

static void _remote_set_directory_(tMsgClient *clt,char *param){
	char temp[MAX_LEN];
	*temp=0;
	if (*param!='/' && getcwd(temp,MAX_LEN)!=NULL){
		char *path=sum_strings(temp,"/",param,NULL);
		normalize_path(path);
		clt->send_command(PACKET_SET_SAVE_PATH,path,strlen(path));
		delete[] path;
	}else{
		clt->send_command(PACKET_SET_SAVE_PATH,param,strlen(param));
	};
};

static void _do_ls_(char *url,tMsgClient *clt){
	tPacketStatus status;
	clt->send_command_short(PACKET_LS,url,strlen(url));
	while(clt->get_answer_status(&status)){
		switch(status.Status){
		case DL_RUN:
			printf(">");
			break;
		case DL_STOP:
			printf("X");
			break;
		case DL_WAIT:
			printf("o");
			break;
		case DL_PAUSE:
			printf("=");
			break;
		case DL_COMPLETE:
			printf("+");
			break;
		case DL_STOPWAIT:
			printf("-");
			break;
		default:
			printf("?");
			break;
		};
		if (status.url){
			printf("%s",status.url);
			delete[] status.url;
			status.url=NULL;
		};
		printf(" %i/%i bytes %i B/s %i/%i attempts\n",
		       status.Download,status.Size,
		       status.Speed,
		       status.Attempt,status.MaxAttempt);
	};
};

static void _do_lstree_(tMsgClient *clt){
	clt->send_command_short(PACKET_LSTREE,NULL,0);
	char b;
	int depth=-1;
	g_print(_("List of queues:\n"));
	g_print("-----------------------------\n");
	int N=1;
	while(clt->readdata(&b,sizeof(b))==sizeof(b)){
		int len=0;
		int def_queue=0;
		switch(b){
		case LST_SUBQUEUE:
			depth+=1;
			break;
		case LST_DQUEUE:
			def_queue=1;
		case LST_QUEUE:{
			clt->readdata(&len,sizeof(len));
			printf("[%i] ",N);
			for (int i=0;i<depth;i++) printf("  ");
			for (;len>0;len--){
				clt->readdata(&b,sizeof(b));
				printf("%c",b);
			};
			int num=0,run=0,maxrun=0,complete=0;
			clt->readdata(&num,sizeof(num));
			clt->readdata(&run,sizeof(run));
			clt->readdata(&complete,sizeof(complete));
			clt->readdata(&maxrun,sizeof(maxrun));
			printf("\t %i %i %i/%i",num,complete,run,maxrun);
			if (def_queue) printf("\t<--");
			printf("\n");
			N++;
			break;
		};
		case LST_UPQUEUE:{
			depth-=1;
			break;
		};
		};
	};
};

int parse_command_line_already_run(int argv,char **argc){
	int rvalue=1;
	if (argv>1){
		tMsgClient *clt=new tMsgClient;
		for (int i=1;i<argv;i++){
			int opt_error=0;
			int option=OPT_UNKNOWN;
			if (*(argc[i])!='-'){
				rvalue=0;
				if (clt->send_command(PACKET_ADD,argc[i],strlen(argc[i])+1)) break;
			}else{
				option=downloader_args_type(argc[i]);
				switch(option){
				case OPT_INFO:{
					rvalue=0;
					clt->send_command(PACKET_ASK_RUN,NULL,0);
					g_print(_("Run downloads: %d\n"),clt->get_answer_int());
					clt->send_command(PACKET_ASK_PAUSE,NULL,0);
					g_print(_("Paused downloads: %d\n"),clt->get_answer_int());
					clt->send_command(PACKET_ASK_STOP,NULL,0);
					g_print(_("Failed downloads: %d\n"),clt->get_answer_int());
					clt->send_command(PACKET_ASK_COMPLETE,NULL,0);
					g_print(_("Completed downloads: %d\n"),clt->get_answer_int());
					g_print("-------------------------------\n");
					clt->send_command(PACKET_ASK_FULLAMOUNT,NULL,0);
					g_print(_("Total: %d\n"),clt->get_answer_int());
					clt->send_command(PACKET_ASK_READED_BYTES,NULL,0);
					g_print(_("Total bytes loaded: %d\n"),clt->get_answer_int());
					clt->send_command(PACKET_ASK_SPEED,NULL,0);
					g_print(_("Current speed: %d\n"),clt->get_answer_int());
					break;
				};
				case OPT_SPEED:{
					clt->send_command(PACKET_ASK_SPEED,NULL,0);
					rvalue=0;
					g_print(_("Curent speed: %d\n"),clt->get_answer_int());
					break;
				};
				case OPT_TRAFFIC_LOW:{
					clt->send_command(PACKET_SET_SPEED_LIMIT,"1",2);
					rvalue=0;
					break;
				};
				case OPT_TRAFFIC_MIDDLE:{
					clt->send_command(PACKET_SET_SPEED_LIMIT,"2",2);
					rvalue=0;
					break;
				};
				case OPT_TRAFFIC_HIGH:{
					clt->send_command(PACKET_SET_SPEED_LIMIT,"3",2);
					rvalue=0;
					break;
				};
				case OPT_SET_DIRECTORY:{
					rvalue=0;
					if (argv>i+1){
						i+=1;
						_remote_set_directory_(clt,argc[i]);
					}else
						opt_error=1;
					break;
				};
				case OPT_DEL_COMPLETED:{
					clt->send_command(PACKET_DEL_COMPLETED,NULL,0);
					rvalue=0;
					break;
				};
				case OPT_SET_MAX_THREADS:{
					rvalue=0;
					if (argv>i+1){
						i+=1;
						clt->send_command(PACKET_SET_MAX_THREADS,argc[i],strlen(argc[i]));
					}else
						opt_error=1;
					break;
				};
				case OPT_RERUN_FAILED:{
					rvalue=0;
					clt->send_command(PACKET_RERUN_FAILED,NULL,0);
					break;
				};
				case OPT_EXIT_TIME:
					rvalue=0;
					if (argv>i+1){
						i+=1;
						clt->send_command(PACKET_EXIT_TIME,argc[i],strlen(argc[i]));
					}else
						opt_error=1;
					break;
				case OPT_STOP:{
					rvalue=0;
					if (argv>i+1){
						i+=1;
						clt->send_command(PACKET_STOP,argc[i],strlen(argc[i]));
					}else{
						opt_error=1;
					};
					break;
				};
				case OPT_DEL:{
					rvalue=0;
					if (argv>i+1){
						i+=1;
						clt->send_command(PACKET_DEL,argc[i],strlen(argc[i]));
					}else{
						opt_error=1;
					};
					break;
				};
				case OPT_LS:{
					rvalue=0;
					char *which=NULL;
					if (argv>i+1){
						i+=1;
						which=argc[i];
					};
					if (which)
						_do_ls_(which,clt);
					else
						_do_lstree_(clt);
					break;
				};
				case OPT_SWITCH:{
					rvalue=0;
					if (argv>i+1){
						i+=1;
						clt->send_command(PACKET_SWITCH_QUEUE,argc[i],strlen(argc[i]));
					}else{
						opt_error=1;
					};
					break;
				};
				case OPT_ADD_OPEN:{
					rvalue=0;
					if (argv>i+1){
						i+=1;
						clt->send_command(PACKET_ADD_OPEN,argc[i],strlen(argc[i]));
					}else{
						opt_error=1;
					};
					break;
				};
				};
			};
			if (opt_error && downloader_args_errors[option])
				g_print("%s\n",_(downloader_args_errors[option]));
		};		
		delete clt;
	};
	return rvalue;
};

void init_add_dnd_window(char *url,char *desc);

void parse_command_line_postload(int argv,char **argc){
	for (int i=1;i<argv;i++){
		if (*(argc[i])!='-'){
			_aa_.add_downloading(argc[i]);
		};
		int option=downloader_args_type(argc[i]);
		switch (option){
		case OPT_ADD_OPEN:{
			if (argv>i+1){
				i++;
				init_add_dnd_window(argc[i],_("Added from command line"));
			};
			break;
		};
		case OPT_TRAFFIC_LOW:{
			CFG.SPEED_LIMIT=1;
			set_speed_buttons();
			break;
		};
		case OPT_TRAFFIC_MIDDLE:{
			CFG.SPEED_LIMIT=2;
			set_speed_buttons();
			break;
		};
		case OPT_TRAFFIC_HIGH:{
			CFG.SPEED_LIMIT=3;
			set_speed_buttons();
			break;
		};
		case OPT_EXIT_TIME:
			int tmp;
			if (argv<=i+1 || sscanf(argc[i+1],"%d",&tmp)<1 || tmp<0)
				g_print("%s\n",_(downloader_args_errors[OPT_EXIT_TIME]));
			else{
				if (tmp>=1){
					CFG.EXIT_COMPLETE=1;
					CFG.EXIT_COMPLETE_TIME=tmp;
				}else
					CFG.EXIT_COMPLETE=0;
				i+=1;
			};
			break;
		case OPT_RUN_MINIMIZED:
			main_window_iconify();
			break;
		case OPT_SET_DIRECTORY:{
			if (argv>i+1){
				if (CFG.GLOBAL_SAVE_PATH) delete[] CFG.GLOBAL_SAVE_PATH;
				i+=1;
				CFG.GLOBAL_SAVE_PATH=copy_string(argc[i]);
			}else
				g_print("%s\n",_(downloader_args_errors[OPT_SET_DIRECTORY]));
			break;
		};
		case OPT_GEOMETRY:{
			if (argv>i+1){
				i+=1;
				int x=0,y=0,w=0,h=0;
				if (index(argc[i],'x'))
					sscanf(argc[i],"%ix%i+%i+%i",&w,&h,&x,&y);
				else
					sscanf(argc[i],"+%i+%i",&x,&y);
				if (w>0 && h>0){
					main_window_resize(h,w);
//					printf("h=%i w=%i\n",h,w);
				};
				if (x>0 && y>0){
					main_window_move(x,y);
//					printf("x=%i y=%i\n",x,y);
				};
			}else
				g_print("%s\n",_(downloader_args_errors[OPT_GEOMETRY]));
		};
		};
	};
};

void help_print_args(int type){
	int num=0;
	printf("\t");
	for (int i=0;i<downloader_parsed_args_num;i++){
		if (downloader_parsed_args[i].cmd==type){
			if (num) printf(",");
			printf(downloader_parsed_args[i].name);
			num+=1;
		};
	};
	printf("\t");
};

void help_print(){
	g_print(_("Usage: nt [OPTION] ... [URL]"));printf("\n\n");
	help_print_args(OPT_HELP);g_print(_("print this page and exit"));printf("\n");
	help_print_args(OPT_VERSION);g_print(_("show version information and exit"));printf("\n");
	help_print_args(OPT_INFO);g_print(_("show information if already run"));printf("\n");
	help_print_args(OPT_SPEED);g_print(_("show current speed if already run"));printf("\n");
	help_print_args(OPT_RUN_MINIMIZED);g_print(_("run in minimized mode"));printf("\n");
	help_print_args(OPT_EXIT_TIME);g_print(_("set timeout for exiting if nothing to do"));printf("\n");
	help_print_args(OPT_SET_DIRECTORY);g_print(_("set directory for saving files"));printf("\n");
	help_print_args(OPT_TRAFFIC_LOW);g_print(_("set lower speed limitation"));printf("\n");
	help_print_args(OPT_TRAFFIC_MIDDLE);g_print(_("set medium speed limitation"));printf("\n");
	help_print_args(OPT_TRAFFIC_HIGH);g_print(_("set unlimited speed"));printf("\n");
	help_print_args(OPT_DEL_COMPLETED);g_print(_("delete completed if already run"));printf("\n");
	help_print_args(OPT_SET_MAX_THREADS);g_print(_("set maximum active downloads"));printf("\n");
	help_print_args(OPT_RERUN_FAILED);g_print(_("restart all failed downloads"));printf("\n");
	help_print_args(OPT_WITHOUT_FACE);g_print(_("run program without X interface"));printf("\n");
	help_print_args(OPT_LS);g_print(_("display info about URL in queue of downloads"));printf("\n");
	help_print_args(OPT_SWITCH);g_print(_("change current default queue"));printf("\n");
	help_print_args(OPT_DEL);g_print(_("remove a download from queue"));printf("\n");
	help_print_args(OPT_STOP);g_print(_("stop a download"));printf("\n");
	help_print_args(OPT_COLOR);g_print(_("using colors if run without interface"));printf("\n");
	help_print_args(OPT_GEOMETRY);g_print(_("specifies preferred size and position of main window"));printf("\n");
	help_print_args(OPT_ADD_OPEN);g_print(_("open \"Add new download\" dialog for the URL"));printf("\n");
	printf("\n");
};
