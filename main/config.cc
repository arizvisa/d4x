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

void get_size_of_clist();

const char *CFG_FILE=".ntrc/config";
const char *CFG_DIR=".ntrc";

void set_config(char *line) {
	char temp[MAX_LEN];
	int prom;
	if(*line!='\n' && *line!='#') {
		sscanf(line,"%s %i",temp,&prom);
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
			ListColumns[STATUS_COL].size=prom;
			return;
		};
		if (equal(temp,"dl_file_col")) {
			ListColumns[FILE_COL].size=prom;
			return;
		};
		if (equal(temp,"dl_file_type_col")) {
			ListColumns[FILE_TYPE_COL].size=prom;
			return;
		};
		if (equal(temp,"dl_full_size_col")) {
			ListColumns[FULL_SIZE_COL].size=prom;
			return;
		};
		if (equal(temp,"dl_downloaded_size_col")) {
			ListColumns[DOWNLOADED_SIZE_COL].size=prom;
			return;
		};
		if (equal(temp,"dl_percent_col")) {
			ListColumns[PERCENT_COL].size=prom;
			return;
		};
		if (equal(temp,"dl_speed_col")) {
			ListColumns[SPEED_COL].size=prom;
			return;
		};
		if (equal(temp,"dl_time_col")) {
			ListColumns[TIME_COL].size=prom;
			return;
		};
		if (equal(temp,"dl_elapsed_time_col")) {
			ListColumns[ELAPSED_TIME_COL].size=prom;
			return;
		};
		if (equal(temp,"dl_pause_col")) {
			ListColumns[PAUSE_COL].size=prom;
			return;
		};
		if (equal(temp,"dl_treat_col")) {
			ListColumns[TREAT_COL].size=prom;
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
	if (fd>0) {
		char temp[MAX_LEN];
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
	if (!UrlHistory)
		UrlHistory=new tHistory;
	if (!PathHistory)
		PathHistory=new tHistory;
	if (!FileHistory)
		FileHistory=new tHistory;
	if (!LogHistory)
		LogHistory=new tHistory;
	if (!UserHistory)
		UserHistory=new tHistory;
	if (!ProxyHistory)
		ProxyHistory=new tHistory;
	if (!LoadSaveHistory)
		LoadSaveHistory=new tHistory;
	load_strlist(UrlHistory, ".ntrc/history1",0);
	load_strlist(PathHistory,".ntrc/history2",1);
	load_strlist(LogHistory,".ntrc/history3",0);
	load_strlist(LoadSaveHistory,".ntrc/history4",0);
	load_strlist(UserHistory,".ntrc/history5",0);
	load_strlist(ProxyHistory,".ntrc/history6",0);
	load_strlist(FileHistory,".ntrc/history7",0);
};

static void save_integer_to_config(int fd,char *name,int num) {
	char data[MAX_LEN];
	sprintf(data,"%s %i\n\n",name,num);
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
	if (fd>0) {
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
		get_size_of_clist();
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
		list_of_downloads_get_sizes();
		save_integer_to_config(fd,"dl_status_col",ListColumns[STATUS_COL].size);
		save_integer_to_config(fd,"dl_file_col",ListColumns[FILE_COL].size);
		save_integer_to_config(fd,"dl_file_type_col",ListColumns[FILE_TYPE_COL].size);
		save_integer_to_config(fd,"dl_full_size_col",ListColumns[FULL_SIZE_COL].size);
		save_integer_to_config(fd,"dl_downloaded_size_col",ListColumns[DOWNLOADED_SIZE_COL].size);
		save_integer_to_config(fd,"dl_percent_col",ListColumns[PERCENT_COL].size);
		save_integer_to_config(fd,"dl_speed_col",ListColumns[SPEED_COL].size);
		save_integer_to_config(fd,"dl_time_col",ListColumns[TIME_COL].size);
		save_integer_to_config(fd,"dl_elapsed_time_col",ListColumns[ELAPSED_TIME_COL].size);
		save_integer_to_config(fd,"dl_pause_col",ListColumns[PAUSE_COL].size);
		save_integer_to_config(fd,"dl_treat_col",ListColumns[TREAT_COL].size);
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
		close(fd);
	} else {
		if (MainLog) {
			sprintf(data,"Can't write cfgfile to:%s",cfgpath);
			MainLog->add(data,LOG_ERROR);
		} else
			printf("Can't write cfgfile to:%s\n",cfgpath);
	};
	save_strlist(UrlHistory,".ntrc/history1");
	save_strlist(PathHistory,".ntrc/history2");
	save_strlist(LogHistory,".ntrc/history3");
	save_strlist(LoadSaveHistory,".ntrc/history4");
	save_strlist(UserHistory,".ntrc/history5");
	save_strlist(ProxyHistory,".ntrc/history6");
	save_strlist(FileHistory,".ntrc/history7");
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
	if (fd>0) {
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
	if (fd>0) {
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
	if (fd>0) {
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

int parse_command_line_preload(int argv,char **argc){
	int rvalue=0;
	for (int i=1;i<argv;i++){
		if (equal("-v",argc[i])||equal("--version",argc[i])){
			puts(VERSION_NAME);
			rvalue=1;
		};
	};
	return rvalue;
};

void parse_command_line_postload(int argv,char **argc){
	for (int i=1;i<argv;i++){
		if (equal("-t1",argc[i]) || equal("--trafic-low",argc[i]))
			CFG.SPEED_LIMIT=1;
		if (equal("-t2",argc[i]) || equal("--trafic-middle",argc[i]))
			CFG.SPEED_LIMIT=2;
		if (equal("-t3",argc[i]) || equal("--trafic-unlimited",argc[i]))
			CFG.SPEED_LIMIT=3;
	};
};
