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
#include "ntlocale.h"
#include "srvclt.h"

void get_size_of_clist();

const char *CFG_FILE=".ntrc/config";
const char *CFG_DIR=".ntrc";
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
	{"--traffic-middle",	OPT_TRAFFIC_MIDDLE},
	{"-t3",			OPT_TRAFFIC_HIGH},
	{"--traffic-high",	OPT_TRAFFIC_HIGH}
};
int downloader_parsed_args_num=sizeof(downloader_parsed_args)/sizeof(tOption);

void set_column_position(int type,int col){
	if (col>=0 && col<=NOTHING_COL){
		ListColumns[col].type=type;
		ListColumns[type].enum_index=col;
	};
};
void set_config(char *line) {
	if(*line!='\n' && *line!='#') {
		char temp[MAX_LEN];
		int prom=1;
		int rvalue=sscanf(line,"%s %i",temp,&prom);
		if (rvalue<1) return;
		if (equal(temp,"max_threads")) {
			if (prom<=50 && prom>0) CFG.MAX_THREADS=prom;
			return;
		};
		if (equal(temp,"max_log")) {
			if (prom>0 && prom<500) CFG.MAX_LOG_LENGTH=prom;
			return;
		};
		if (equal(temp,"max_main_log")) {
			if (prom>0 && prom<500) CFG.MAX_MAIN_LOG_LENGTH=prom;
			return;
		};
		if (equal(temp,"max_retries")) {
			if (prom>=0) CFG.MAX_RETRIES=prom;
			return;
		};
		if (equal(temp,"timeout")) {
			if (prom<500 && prom>0) CFG.TIME_OUT=prom;
			return;
		};
		if (equal(temp,"break_timeout")) {
			if (prom<=60 && prom>=0) CFG.RETRY_TIME_OUT=prom;
			return;
		};
		if (equal(temp,"optimize")) {
			if (prom==1 || prom==0) CFG.RECURSIVE_OPTIMIZE=prom;
			return;
		};
		if (equal(temp,"del_completed")) {
			if (prom==1 || prom==0) CFG.DELETE_COMPLETED=prom;
			return;
		};
		if (equal(temp,"del_fataled")) {
			if (prom==1 || prom==0) CFG.DELETE_FATAL=prom;
			return;
		};
		if (equal(temp,"nice_decs")) {
			CFG.NICE_DEC_DIGITALS.curent=prom;
			return;
		};
		if (equal(temp,"time_format")) {
			CFG.TIME_FORMAT=prom;
			return;
		};
		if (equal(temp,"ftp_passive_mode")) {
			CFG.FTP_PASSIVE_MODE=prom;
			return;
		};
		if (equal(temp,"retry_if_noreget")) {
			if (prom==1 || prom==0) CFG.RETRY_IF_NOREGET=prom;
			return;
		};
		if (equal(temp,"sleeptime")) {
			if (prom<60 && prom>0) CFG.RETRY_TIME_OUT=prom;
			return;
		};
		if (equal(temp,"savepath")) {
			CFG.GLOBAL_SAVE_PATH=copy_string(line+strlen("savepath")+1);
			return;
		};
		if (equal(temp,"xposition")) {
			CFG.WINDOW_X_POSITION=prom;
			return;
		};
		if (equal(temp,"yposition")) {
			CFG.WINDOW_Y_POSITION=prom;
			return;
		};
		if (equal(temp,"windowwidth")) {
			CFG.WINDOW_WIDTH=prom;
			return;
		};
		if (equal(temp,"windowheight")) {
			CFG.WINDOW_HEIGHT=prom;
			return;
		};
		if (equal(temp,"clist_height")) {
			CFG.WINDOW_CLIST_HEIGHT=prom;
			return;
		};
		if (equal(temp,"ftp_proxy_host")) {
			CFG.FTP_PROXY_HOST=copy_string(line+strlen("ftp_proxy_host")+1);
			return;
		};
		if (equal(temp,"ftp_proxy_user")) {
			CFG.FTP_PROXY_USER=copy_string(line+strlen("ftp_proxy_user")+1);
			return;
		};
		if (equal(temp,"ftp_proxy_pass")) {
			CFG.FTP_PROXY_PASS=copy_string(line+strlen("ftp_proxy_pass")+1);
			return;
		};
		if (equal(temp,"ftp_proxy_type")) {
			CFG.FTP_PROXY_TYPE=prom;
			return;
		};
		if (equal(temp,"http_proxy_host")) {
			CFG.HTTP_PROXY_HOST=copy_string(line+strlen("http_proxy_host")+1);
			return;
		};
		if (equal(temp,"http_proxy_user")) {
			CFG.HTTP_PROXY_USER=copy_string(line+strlen("http_proxy_user")+1);
			return;
		};
		if (equal(temp,"http_proxy_pass")) {
			CFG.HTTP_PROXY_PASS=copy_string(line+strlen("http_proxy_pass")+1);
			return;
		};
		if (equal(temp,"ftp_proxy_port")) {
			CFG.FTP_PROXY_PORT=prom;
			return;
		};
		if (equal(temp,"http_proxy_port")) {
			CFG.HTTP_PROXY_PORT=prom;
			return;
		};
		if (equal(temp,"need_pass_http_proxy")) {
			CFG.NEED_PASS_HTTP_PROXY=prom;
			return;
		};
		if (equal(temp,"need_pass_ftp_proxy")) {
			CFG.NEED_PASS_FTP_PROXY=prom;
			return;
		};
		if (equal(temp,"use_proxy_for_ftp")) {
			CFG.USE_PROXY_FOR_FTP=prom;
			return;
		};
		if (equal(temp,"use_proxy_for_http")) {
			CFG.USE_PROXY_FOR_HTTP=prom;
			return;
		};
		if (equal(temp,"save_main_log")) {
			CFG.SAVE_MAIN_LOG=prom;
			return;
		};
		if (equal(temp,"append_rewrite_log")) {
			CFG.APPEND_REWRITE_LOG=prom;
			return;
		};
		if (equal(temp,"save_log_path")) {
			CFG.SAVE_LOG_PATH=copy_string(line+strlen("save_log_path")+1);
			return;
		};
		if (equal(temp,"ftp_permisions")) {
			CFG.FTP_PERMISIONS=prom;
			return;
		};
		if (equal(temp,"save_list")) {
			CFG.SAVE_LIST=prom;
			return;
		};
		if (equal(temp,"interval_save_list")) {
			if (prom>0 && prom<1000)
				CFG.SAVE_LIST_INTERVAL=prom;
			return;
		};
		if (equal(temp,"use_mainwin_title")) {
			CFG.USE_MAINWIN_TITLE=prom;
			return;
		};
		if (equal(temp,"use_mainwin_titleII")) {
			CFG.USE_MAINWIN_TITLE2=prom;
			return;
		};
		if (equal(temp,"get_date_from_server")) {
			CFG.GET_DATE=prom;
			return;
		};
		if (equal(temp,"need_dialog_for_dnd")) {
			CFG.NEED_DIALOG_FOR_DND=prom;
			return;
		};
		/*
		 *	Parsing sizes of columns :
		 */
		if (equal(temp,"dl_status_col")) {
			ListColumns[ListColumns[STATUS_COL].enum_index].size=prom;
			return;
		};
		if (equal(temp,"dl_file_col")) {
			ListColumns[ListColumns[FILE_COL].enum_index].size=prom;
			return;
		};
		if (equal(temp,"dl_file_type_col")) {
			ListColumns[ListColumns[FILE_TYPE_COL].enum_index].size=prom;
			return;
		};
		if (equal(temp,"dl_full_size_col")) {
			ListColumns[ListColumns[FULL_SIZE_COL].enum_index].size=prom;
			return;
		};
		if (equal(temp,"dl_downloaded_size_col")) {
			ListColumns[ListColumns[DOWNLOADED_SIZE_COL].enum_index].size=prom;
			return;
		};
		if (equal(temp,"dl_remain_size_col")) {
			ListColumns[ListColumns[REMAIN_SIZE_COL].enum_index].size=prom;
			return;
		};
		if (equal(temp,"dl_percent_col")) {
			ListColumns[ListColumns[PERCENT_COL].enum_index].size=prom;
			return;
		};
		if (equal(temp,"dl_speed_col")) {
			ListColumns[ListColumns[SPEED_COL].enum_index].size=prom;
			return;
		};
		if (equal(temp,"dl_time_col")) {
			ListColumns[ListColumns[TIME_COL].enum_index].size=prom;
			return;
		};
		if (equal(temp,"dl_elapsed_time_col")) {
			ListColumns[ListColumns[ELAPSED_TIME_COL].enum_index].size=prom;
			return;
		};
		if (equal(temp,"dl_pause_col")) {
			ListColumns[ListColumns[PAUSE_COL].enum_index].size=prom;
			return;
		};
		if (equal(temp,"dl_treat_col")) {
			ListColumns[ListColumns[TREAT_COL].enum_index].size=prom;
			return;
		};
		if (equal(temp,"fl_host_col")) {
			CFG.FACE_LIMITS_SIZE1=prom;
			return;
		};
		if (equal(temp,"fl_limit_col")) {
			CFG.FACE_LIMITS_SIZE2=prom;
			return;
		};
		if (equal(temp,"window_lower")) {
			CFG.WINDOW_LOWER=prom;
			return;
		};
		if (equal(temp,"confirm_exit")) {
			CFG.CONFIRM_EXIT=prom;
			return;
		};
		if (equal(temp,"confirm_delete")) {
			CFG.CONFIRM_DELETE=prom;
			return;
		};
		if (equal(temp,"confirm_delete_all")) {
			CFG.CONFIRM_DELETE_ALL=prom;
			return;
		};
		if (equal(temp,"confirm_delete_completed")) {
			CFG.CONFIRM_DELETE_COMPLETED=prom;
			return;
		};
		if (equal(temp,"confirm_delete_fataled")) {
			CFG.CONFIRM_DELETE_FATALED=prom;
			return;
		};
		if (equal(temp,"speed_limit")) {
			CFG.SPEED_LIMIT=prom;
			return;
		};
		if (equal(temp,"speed_limit_one")) {
			CFG.SPEED_LIMIT_1=prom;
			return;
		};
		if (equal(temp,"speed_limit_two")) {
			CFG.SPEED_LIMIT_2=prom;
			return;
		};
		if (equal(temp,"graph_order")) {
			CFG.GRAPH_ORDER=prom;
			return;
		};
		if (equal(temp,"ftp_recurse_depth")) {
			CFG.FTP_RECURSE_DEPTH=prom;
			return;
		};
		if (equal(temp,"http_recurse_depth")) {
			CFG.HTTP_RECURSE_DEPTH=prom;
			return;
		};
		if (equal(temp,"default_name")) {
			CFG.DEFAULT_NAME=copy_string(line+strlen("default_name")+1);
			return;
		};
		if (equal(temp,"scroll_mainwin_title")) {
			CFG.SCROLL_MAINWIN_TITLE=prom;
			return;
		};
		if (equal(temp,"default_permisions")) {
			CFG.DEFAULT_PERMISIONS=prom;
			return;
		};
		if (equal(temp,"dl_status_col_pos")) {
			set_column_position(STATUS_COL,prom);
			return;
		};
		if (equal(temp,"dl_file_col_pos")) {
			set_column_position(FILE_COL,prom);
			return;
		};
		if (equal(temp,"dl_file_type_col_pos")) {
			set_column_position(FILE_TYPE_COL,prom);
			return;
		};
		if (equal(temp,"dl_full_size_col_pos")) {
			set_column_position(FULL_SIZE_COL,prom);
			return;
		};
		if (equal(temp,"dl_downloaded_size_col_pos")) {
			set_column_position(DOWNLOADED_SIZE_COL,prom);
			return;
		};
		if (equal(temp,"dl_remain_size_col_pos")) {
			set_column_position(REMAIN_SIZE_COL,prom);
			return;
		};
		if (equal(temp,"dl_percent_col_pos")) {
			set_column_position(PERCENT_COL,prom);
			return;
		};
		if (equal(temp,"dl_speed_col_pos")) {
			set_column_position(SPEED_COL,prom);
			return;
		};
		if (equal(temp,"dl_time_col_pos")) {
			set_column_position(TIME_COL,prom);
			return;
		};
		if (equal(temp,"dl_elapsed_time_col_pos")) {
			set_column_position(ELAPSED_TIME_COL,prom);
			return;
		};
		if (equal(temp,"dl_pause_col_pos")) {
			set_column_position(PAUSE_COL,prom);
			return;
		};
		if (equal(temp,"dl_treat_col_pos")) {
			set_column_position(TREAT_COL,prom);
			return;
		};
		if (equal(temp,"dl_url_col_pos")) {
			set_column_position(URL_COL,prom);
			return;
		};
		if (equal(temp,"dl_nothing_col_pos")) {
			set_column_position(NOTHING_COL,prom);
			return;
		};
		if (equal(temp,"rollback")) {
			if (prom>0) CFG.ROLLBACK=prom;
			return;
		};
		if (equal(temp,"dnd_trash")) {
			CFG.DND_TRASH=prom;
			return;
		};
		if (equal(temp,"dnd_trash_x")) {
			CFG.DND_TRASH_X=prom;
			return;
		};
		if (equal(temp,"dnd_trash_y")) {
			CFG.DND_TRASH_Y=prom;
			return;
		};
		if (equal(temp,"exit_complete")) {
			CFG.EXIT_COMPLETE=prom;
			return;
		};
		if (equal(temp,"exit_complete_time")) {
			CFG.EXIT_COMPLETE_TIME=prom;
			return;
		};
		if (equal(temp,"main_log_detailed")) {
			CFG.MAIN_LOG_DETAILED=prom;
			return;
		};
		if (equal(temp,"user_agent")) {
			CFG.USER_AGENT=copy_string(line+strlen("user_agent")+1);
			return;
		};
		if (equal(temp,"graph_back")) {
			sscanf(line+strlen("graph_back")+1,"%x",&prom);
			CFG.GRAPH_BACK=prom;
			return;
		};
		if (equal(temp,"graph_fore1")) {
			sscanf(line+strlen("graph_back1")+1,"%x",&prom);
			CFG.GRAPH_FORE1=prom;
			return;
		};
		if (equal(temp,"graph_fore2")) {
			sscanf(line+strlen("graph_back1")+1,"%x",&prom);
			CFG.GRAPH_FORE2=prom;
			return;
		};
		if (equal(temp,"graph_pick")) {
			sscanf(line+strlen("graph_pick")+1,"%x",&prom);
			CFG.GRAPH_PICK=prom;
			return;
		};
		if (equal(temp,"exec_when_quit")) {
			CFG.EXEC_WHEN_QUIT=copy_string(line+strlen("exec_when_quit")+1);
			return;
		};
		if (equal(temp,"remember_pass")) {
			CFG.REMEMBER_PASS=prom;
			return;
		};
	};
};

static int read_string(int fd,char *where,int max) {
	char *cur=where;
	int i=max;
	while(read(fd,cur,1) && i) {
		i-=1;
		if (*cur=='\n') break;
		cur+=1;
	};
	*cur=0;
	return max-i;
};

void read_config() {
	if (!HOME_VARIABLE)	return;
	char *cfgpath=compose_path(HOME_VARIABLE,CFG_FILE);
	int fd=open(cfgpath,O_RDONLY);
	if (fd>=0) {
		char temp[MAX_LEN];
		init_columns_info();
		while(read_string(fd,temp,MAX_LEN)) {
			set_config(temp);
		};
		close(fd);
	} else {
		save_config();
		printf(_("Can't open cfg file at '%s'\n"),cfgpath);
		printf(_("Use default cfg :))...\n"));
	};
	delete cfgpath;
	load_strlist(ALL_HISTORIES[URL_HISTORY], ".ntrc/history1",0);
	load_strlist(ALL_HISTORIES[PATH_HISTORY],".ntrc/history2",1);
	load_strlist(ALL_HISTORIES[LOG_HISTORY],".ntrc/history3",0);
	load_strlist(ALL_HISTORIES[LOAD_SAVE_HISTORY],".ntrc/history4",0);
	load_strlist(ALL_HISTORIES[USER_HISTORY],".ntrc/history5",0);
	load_strlist(ALL_HISTORIES[PROXY_HISTORY],".ntrc/history6",0);
	load_strlist(ALL_HISTORIES[FILE_HISTORY],".ntrc/history7",0);
	load_strlist(ALL_HISTORIES[USER_AGENT_HISTORY],".ntrc/history8",0);
	load_strlist(ALL_HISTORIES[EXEC_HISTORY],".ntrc/history9",0);
	if (CFG.REMEMBER_PASS) 
		load_strlist(ALL_HISTORIES[PASS_HISTORY],".ntrc/history10",0);
	ALL_HISTORIES[USER_AGENT_HISTORY]->add("%version");	
	ALL_HISTORIES[USER_AGENT_HISTORY]->add("Mozilla/4.05");	
	ALL_HISTORIES[USER_AGENT_HISTORY]->add("Mozilla/4.0 (compatible; MSIE 4.01; Windows 95)");	
};

static void save_integer_to_config(int fd,char *name,int num) {
	char data[MAX_LEN];
	sprintf(data,"%s %i\n\n",name,num);
	write(fd,data,strlen(data));
};

static void save_hex_integer_to_config(int fd,char *name,int num) {
	char data[MAX_LEN];
	sprintf(data,"%s 0x%06x\n\n",name,num);
	write(fd,data,strlen(data));
};

static void save_string_to_config(int fd,char *name,char *str) {
	if (!str) return;
	char data[MAX_LEN];
	sprintf(data,"%s %s\n\n",name,str);
	write(fd,data,strlen(data));
};

void save_config() {
	if (!HOME_VARIABLE)	return;
	char *cfgpath=compose_path(HOME_VARIABLE,CFG_FILE);
	int fd=open(cfgpath,O_TRUNC | O_CREAT |O_RDWR,S_IRUSR | S_IWUSR);
	if (fd<0) {
		char *cfg_dir=compose_path(HOME_VARIABLE,CFG_DIR);
		mkdir(cfg_dir,S_IRWXU| S_IXGRP|S_IRGRP);
		fd=open(cfgpath,O_TRUNC | O_CREAT |O_RDWR,S_IRUSR | S_IWUSR);
		delete cfg_dir;
	};
	char data[MAX_LEN];
	if (fd>=0) {
		save_integer_to_config(fd,"max_threads",CFG.MAX_THREADS);
		save_integer_to_config(fd,"max_log",CFG.MAX_LOG_LENGTH);
		save_integer_to_config(fd,"max_main_log",CFG.MAX_MAIN_LOG_LENGTH);
		save_integer_to_config(fd,"max_retries",CFG.MAX_RETRIES);
		save_integer_to_config(fd,"timeout",CFG.TIME_OUT);
		save_integer_to_config(fd,"break_timeout",CFG.RETRY_TIME_OUT);
		save_integer_to_config(fd,"sleeptime",CFG.RETRY_TIME_OUT);
		save_integer_to_config(fd,"optimize",CFG.RECURSIVE_OPTIMIZE);
		save_integer_to_config(fd,"retry_if_noreget",CFG.RETRY_IF_NOREGET);
		save_integer_to_config(fd,"del_completed",CFG.DELETE_COMPLETED);
		save_integer_to_config(fd,"del_fataled",CFG.DELETE_FATAL);
		save_integer_to_config(fd,"nice_decs",CFG.NICE_DEC_DIGITALS.curent);
		save_integer_to_config(fd,"time_format",CFG.TIME_FORMAT);
		save_integer_to_config(fd,"ftp_passive_mode",CFG.FTP_PASSIVE_MODE);
		save_string_to_config(fd,"savepath",CFG.GLOBAL_SAVE_PATH);
		save_integer_to_config(fd,"xposition",CFG.WINDOW_X_POSITION);
		save_integer_to_config(fd,"yposition",CFG.WINDOW_Y_POSITION);
		save_integer_to_config(fd,"windowwidth",CFG.WINDOW_WIDTH);
		save_integer_to_config(fd,"windowheight",CFG.WINDOW_HEIGHT);
		list_of_downloads_get_height();
		save_integer_to_config(fd,"clist_height",CFG.WINDOW_CLIST_HEIGHT);
		save_string_to_config(fd,"ftp_proxy_host",CFG.FTP_PROXY_HOST);
		save_string_to_config(fd,"ftp_proxy_user",CFG.FTP_PROXY_USER);
		save_string_to_config(fd,"ftp_proxy_pass",CFG.FTP_PROXY_PASS);
		save_string_to_config(fd,"http_proxy_host",CFG.HTTP_PROXY_HOST);
		save_string_to_config(fd,"http_proxy_user",CFG.HTTP_PROXY_USER);
		save_string_to_config(fd,"http_proxy_pass",CFG.HTTP_PROXY_PASS);
		save_integer_to_config(fd,"ftp_proxy_port",CFG.FTP_PROXY_PORT);
		save_integer_to_config(fd,"ftp_proxy_type",CFG.FTP_PROXY_TYPE);
		save_integer_to_config(fd,"http_proxy_port",CFG.HTTP_PROXY_PORT);
		save_integer_to_config(fd,"need_pass_ftp_proxy",CFG.NEED_PASS_FTP_PROXY);
		save_integer_to_config(fd,"need_pass_http_proxy",CFG.NEED_PASS_HTTP_PROXY);
		save_integer_to_config(fd,"use_proxy_for_ftp",CFG.USE_PROXY_FOR_FTP);
		save_integer_to_config(fd,"use_proxy_for_http",CFG.USE_PROXY_FOR_HTTP);
		save_integer_to_config(fd,"save_main_log",CFG.SAVE_MAIN_LOG);
		save_integer_to_config(fd,"append_rewrite_log",CFG.APPEND_REWRITE_LOG);
		save_string_to_config(fd,"save_log_path",CFG.SAVE_LOG_PATH);
		save_integer_to_config(fd,"ftp_permisions",CFG.FTP_PERMISIONS);
		save_integer_to_config(fd,"save_list",CFG.SAVE_LIST);
		save_integer_to_config(fd,"interval_save_list",CFG.SAVE_LIST_INTERVAL);
		save_integer_to_config(fd,"use_mainwin_title",CFG.USE_MAINWIN_TITLE);
		save_integer_to_config(fd,"use_mainwin_titleII",CFG.USE_MAINWIN_TITLE2);
		save_integer_to_config(fd,"get_date_from_server",CFG.GET_DATE);
		save_integer_to_config(fd,"need_dialog_for_dnd",CFG.NEED_DIALOG_FOR_DND);
		/* saving list columns
		 */
		save_integer_to_config(fd,"dl_status_col_pos",ListColumns[STATUS_COL].enum_index);
		save_integer_to_config(fd,"dl_file_col_pos",ListColumns[FILE_COL].enum_index);
		save_integer_to_config(fd,"dl_file_type_col_pos",ListColumns[FILE_TYPE_COL].enum_index);
		save_integer_to_config(fd,"dl_full_size_col_pos",ListColumns[FULL_SIZE_COL].enum_index);
		save_integer_to_config(fd,"dl_downloaded_size_col_pos",ListColumns[DOWNLOADED_SIZE_COL].enum_index);
		save_integer_to_config(fd,"dl_remain_size_col_pos",ListColumns[REMAIN_SIZE_COL].enum_index);
		save_integer_to_config(fd,"dl_percent_col_pos",ListColumns[PERCENT_COL].enum_index);
		save_integer_to_config(fd,"dl_speed_col_pos",ListColumns[SPEED_COL].enum_index);
		save_integer_to_config(fd,"dl_time_col_pos",ListColumns[TIME_COL].enum_index);
		save_integer_to_config(fd,"dl_elapsed_time_col_pos",ListColumns[ELAPSED_TIME_COL].enum_index);
		save_integer_to_config(fd,"dl_pause_col_pos",ListColumns[PAUSE_COL].enum_index);
		save_integer_to_config(fd,"dl_treat_col_pos",ListColumns[TREAT_COL].enum_index);
		save_integer_to_config(fd,"dl_url_col_pos",ListColumns[URL_COL].enum_index);
		save_integer_to_config(fd,"dl_nothing_col_pos",ListColumns[NOTHING_COL].enum_index);
		list_of_downloads_get_sizes();
		save_integer_to_config(fd,"dl_status_col",ListColumns[ListColumns[STATUS_COL].enum_index].size);
		save_integer_to_config(fd,"dl_file_col",ListColumns[ListColumns[FILE_COL].enum_index].size);
		save_integer_to_config(fd,"dl_file_type_col",ListColumns[ListColumns[FILE_TYPE_COL].enum_index].size);
		save_integer_to_config(fd,"dl_full_size_col",ListColumns[ListColumns[FULL_SIZE_COL].enum_index].size);
		save_integer_to_config(fd,"dl_downloaded_size_col",ListColumns[ListColumns[DOWNLOADED_SIZE_COL].enum_index].size);
		save_integer_to_config(fd,"dl_remain_size_col",ListColumns[ListColumns[REMAIN_SIZE_COL].enum_index].size);
		save_integer_to_config(fd,"dl_percent_col",ListColumns[ListColumns[PERCENT_COL].enum_index].size);
		save_integer_to_config(fd,"dl_speed_col",ListColumns[ListColumns[SPEED_COL].enum_index].size);
		save_integer_to_config(fd,"dl_time_col",ListColumns[ListColumns[TIME_COL].enum_index].size);
		save_integer_to_config(fd,"dl_elapsed_time_col",ListColumns[ListColumns[ELAPSED_TIME_COL].enum_index].size);
		save_integer_to_config(fd,"dl_pause_col",ListColumns[ListColumns[PAUSE_COL].enum_index].size);
		save_integer_to_config(fd,"dl_treat_col",ListColumns[ListColumns[TREAT_COL].enum_index].size);
		if (FaceForLimits) {
			FaceForLimits->get_sizes();
		};
		save_integer_to_config(fd,"fl_host_col", CFG.FACE_LIMITS_SIZE1);
		save_integer_to_config(fd,"fl_limit_col",CFG.FACE_LIMITS_SIZE2);
		save_integer_to_config(fd,"window_lower",CFG.WINDOW_LOWER);
		save_integer_to_config(fd,"confirm_delete",CFG.CONFIRM_DELETE);
		save_integer_to_config(fd,"confirm_delete_all",CFG.CONFIRM_DELETE_ALL);
		save_integer_to_config(fd,"confirm_delete_completed",CFG.CONFIRM_DELETE_COMPLETED);
		save_integer_to_config(fd,"confirm_delete_fataled",CFG.CONFIRM_DELETE_FATALED);
		save_integer_to_config(fd,"confirm_exit",CFG.CONFIRM_EXIT);
		save_integer_to_config(fd,"speed_limit",CFG.SPEED_LIMIT);
		save_integer_to_config(fd,"speed_limit_one",CFG.SPEED_LIMIT_1);
		save_integer_to_config(fd,"speed_limit_two",CFG.SPEED_LIMIT_2);
		save_integer_to_config(fd,"graph_order",CFG.GRAPH_ORDER);
		save_integer_to_config(fd,"ftp_recurse_depth",CFG.FTP_RECURSE_DEPTH);
		save_integer_to_config(fd,"http_recurse_depth",CFG.HTTP_RECURSE_DEPTH);
		save_string_to_config(fd,"default_name",CFG.DEFAULT_NAME);
		save_integer_to_config(fd,"scroll_mainwin_title",CFG.SCROLL_MAINWIN_TITLE);
		save_integer_to_config(fd,"default_permisions",CFG.DEFAULT_PERMISIONS);
		save_integer_to_config(fd,"rollback",CFG.ROLLBACK);
		save_integer_to_config(fd,"dnd_trash",CFG.DND_TRASH);
		save_integer_to_config(fd,"dnd_trash_x",CFG.DND_TRASH_X);
		save_integer_to_config(fd,"dnd_trash_y",CFG.DND_TRASH_Y);
		save_integer_to_config(fd,"exit_complete",CFG.EXIT_COMPLETE);
		save_integer_to_config(fd,"exit_complete_time",CFG.EXIT_COMPLETE_TIME);
		save_integer_to_config(fd,"main_log_detailed",CFG.MAIN_LOG_DETAILED);
		save_string_to_config(fd,"user_agent",CFG.USER_AGENT);
		save_hex_integer_to_config(fd,"graph_back",CFG.GRAPH_BACK);
		save_hex_integer_to_config(fd,"graph_fore1",CFG.GRAPH_FORE1);
		save_hex_integer_to_config(fd,"graph_fore2",CFG.GRAPH_FORE2);
		save_hex_integer_to_config(fd,"graph_pick",CFG.GRAPH_PICK);
		save_string_to_config(fd,"exec_when_quit",CFG.EXEC_WHEN_QUIT);
		save_integer_to_config(fd,"remember_pass",CFG.REMEMBER_PASS);
		close(fd);
	} else {
		if (MainLog) {
			sprintf(data,"Can't write cfgfile to:%s",cfgpath);
			MainLog->add(data,LOG_ERROR);
		} else
			printf("Can't write cfgfile to:%s\n",cfgpath);
	};
	save_strlist(ALL_HISTORIES[URL_HISTORY], ".ntrc/history1");
	save_strlist(ALL_HISTORIES[PATH_HISTORY],".ntrc/history2");
	save_strlist(ALL_HISTORIES[LOG_HISTORY],".ntrc/history3");
	save_strlist(ALL_HISTORIES[LOAD_SAVE_HISTORY],".ntrc/history4");
	save_strlist(ALL_HISTORIES[USER_HISTORY],".ntrc/history5");
	save_strlist(ALL_HISTORIES[PROXY_HISTORY],".ntrc/history6");
	save_strlist(ALL_HISTORIES[FILE_HISTORY],".ntrc/history7");
	save_strlist(ALL_HISTORIES[USER_AGENT_HISTORY],".ntrc/history8");
	save_strlist(ALL_HISTORIES[EXEC_HISTORY],".ntrc/history9");
	save_strlist(ALL_HISTORIES[PASS_HISTORY],".ntrc/history10");
	delete cfgpath;
};

void save_strlist(tStringList *what,char *where) {
	if (!what || !where) return;
	if (!HOME_VARIABLE) return;
	char *path=compose_path(HOME_VARIABLE,where);
	int fd=open(path,O_TRUNC | O_CREAT |O_RDWR,S_IRUSR | S_IWUSR);
	if (fd>0) {
		tString *tmp=what->first();
		while(tmp) {
			write(fd,tmp->body,strlen(tmp->body));
			write(fd,"\n",strlen("\n"));
			tmp=what->prev();
		};
		close(fd);
	};
	delete(path);
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
	delete(path);
};

void read_limits() {
	if (!HOME_VARIABLE) return;
	char *path=compose_path(HOME_VARIABLE,".ntrc/limits");
	int fd=open(path,O_RDONLY);
	if (fd>=0) {
		char temp[MAX_LEN];
		while (read_string(fd,temp,MAX_LEN)) {
			char temp1[MAX_LEN];
			char temp2[MAX_LEN];
			if (read_string(fd,temp1,MAX_LEN) && read_string(fd,temp2,MAX_LEN)) {
				int port,upper;
				sscanf(temp1,"%i",&port);
				sscanf(temp2,"%i",&upper);
				LimitsForHosts->add(temp,port,0,upper);
			} else {
				printf("bad read\n");
			};
		};
		close(fd);
	} else {
		printf(_("Can't open file of limits!!!\n"));
	};
};

void save_limits() {
	if (!HOME_VARIABLE) return;
	char *path=compose_path(HOME_VARIABLE,".ntrc/limits");
	int fd=open(path,O_TRUNC | O_CREAT |O_RDWR,S_IRUSR | S_IWUSR);
	if (fd>=0) {
		tSortString *tmp=LimitsForHosts->last();
		while (tmp) {
			char data[MAX_LEN];
			write(fd,tmp->body,strlen(tmp->body));
			write(fd,"\n",strlen("\n"));
			sprintf(data,"%i",tmp->key);
			write(fd,data,strlen(data));
			write(fd,"\n",strlen("\n"));
			sprintf(data,"%i",tmp->upper);
			write(fd,data,strlen(data));
			write(fd,"\n",strlen("\n"));
			tmp=LimitsForHosts->next();
		};
		close(fd);
	} else {
		printf(_("Can't write limits to file!!!\n"));
	};
};

int downloader_args_type(char *str){
	for (int i=0;i<downloader_parsed_args_num;i++){
		if (equal(str,downloader_parsed_args[i].name))
			return downloader_parsed_args[i].cmd;
	};
	return OPT_UNKNOWN;
};

int parse_command_line_preload(int argv,char **argc){
	int rvalue=0;
	for (int i=1;i<argv;i++){
		int option=downloader_args_type(argc[i]);
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
			printf(_("unknown option:"));
			printf(" %s\n",argc[i]);
			break;
		};
		};
	};
	return rvalue;
};

int parse_command_line_already_run(int argv,char **argc){
	int rvalue=1;
	if (argv>1){
		tMsgClient *clt=new tMsgClient;
		for (int i=1;i<argv;i++){
			if (*(argc[i])!='-'){
				rvalue=0;
				if (clt->send_command(PACKET_ADD,argc[i],strlen(argc[i])+1)) break;
			}else{
				int option=downloader_args_type(argc[i]);
				switch(option){
				case OPT_INFO:{
					rvalue=0;
					int total=0;
					clt->send_command(PACKET_ASK_RUN,NULL,0);
					printf(_("Run downloads: %d\n"),clt->get_answer_int());
					total+=clt->get_answer_int();
					clt->send_command(PACKET_ASK_PAUSE,NULL,0);
					printf(_("Paused downloads: %d\n"),clt->get_answer_int());
					total+=clt->get_answer_int();
					clt->send_command(PACKET_ASK_STOP,NULL,0);
					printf(_("Failed downloads: %d\n"),clt->get_answer_int());
					total+=clt->get_answer_int();
					clt->send_command(PACKET_ASK_COMPLETE,NULL,0);
					printf(_("Completed downloads: %d\n"),clt->get_answer_int());
					total+=clt->get_answer_int();
					printf("-------------------------------\n");
					printf(_("Total: %d\n"),total);
					clt->send_command(PACKET_ASK_SPEED,NULL,0);
					printf(_("Curent speed: %d\n"),clt->get_answer_int());
					break;
				};
				case OPT_SPEED:{
					clt->send_command(PACKET_ASK_SPEED,NULL,0);
					rvalue=0;
					printf(_("Curent speed: %d\n"),clt->get_answer_int());
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
				};
			};
		};		
		delete clt;
	};
	return rvalue;
};

void parse_command_line_postload(int argv,char **argc){
	for (int i=1;i<argv;i++){		
		int option=downloader_args_type(argc[i]);
		switch (option){
		case OPT_TRAFFIC_LOW:{
			CFG.SPEED_LIMIT=1;
			break;
		};
		case OPT_TRAFFIC_MIDDLE:{
			CFG.SPEED_LIMIT=2;
			break;
		};
		case OPT_TRAFFIC_HIGH:{
			CFG.SPEED_LIMIT=3;
			break;
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
	printf(_("Usage: nt [OPTION] ... [URL]"));printf("\n\n");
	help_print_args(OPT_HELP);printf(_("print this page and exit"));printf("\n");
	help_print_args(OPT_VERSION);printf(_("show version information and exit"));printf("\n");
	help_print_args(OPT_INFO);printf(_("show information if already run"));printf("\n");
	help_print_args(OPT_SPEED);printf(_("show curent speed if already run"));printf("\n");
	help_print_args(OPT_TRAFFIC_LOW);printf(_("set lower speed limitation"));printf("\n");
	help_print_args(OPT_TRAFFIC_MIDDLE);printf(_("set middle speed limitation"));printf("\n");
	help_print_args(OPT_TRAFFIC_HIGH);printf(_("set unlimited speed"));printf("\n");
};
