/*	WebDownloader for X-Window
 *	Copyright (C) 1999-2001 Koshelev Maxim
 *	This Program is free but not GPL!!! You can't modify it
 *	without agreement with author. You can't distribute modified
 *	program but you can distribute unmodified program.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */
#ifndef MY_CONFIG
#define MY_CONFIG
#include "liststr.h"
void set_config(char *line);

void read_config();
void save_config();
void save_strlist(tStringList *what,char *where);
void load_strlist(tStringList *where,char *what,int normalize);
void save_limits();
void read_limits();
int parse_command_line_preload(int argv,char **argc);
void parse_command_line_postload(int argv,char **argc);
int parse_command_line_already_run(int argv,char **argc);
void help_print();

struct tOption{
	char *name;
	int cmd;
};
struct tConfigVariable{
	char *name;
	int type;
	void *pointer;
};

enum CONFIG_VARIABLE_TYPES{
	CV_TYPE_UNKNOW=0,
	CV_TYPE_STRING,
	CV_TYPE_INT,
	CV_TYPE_HEX,
	CV_TYPE_GINT,
	CV_TYPE_BOOL,
	CV_TYPE_LONG,
	CV_TYPE_FLOAT
};

extern tOption downloader_parsed_args[];

enum OPTION_ENUM{
	OPT_UNKNOWN=0,
	OPT_VERSION,
	OPT_HELP,
	OPT_TRAFFIC_LOW,
	OPT_TRAFFIC_MIDDLE,
	OPT_TRAFFIC_HIGH,
	OPT_INFO,
	OPT_SPEED,
	OPT_SET_DIRECTORY,
	OPT_DEL_COMPLETED,
	OPT_SET_MAX_THREADS,
	OPT_RERUN_FAILED,
	OPT_WITHOUT_FACE,
	OPT_RUN_MINIMIZED,
	OPT_EXIT_TIME,
	OPT_LS,
	OPT_COLOR,
	OPT_DEL
};

#endif
