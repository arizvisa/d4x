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

extern const char *CFG_FILE;
extern const char *CFG_DIR;

struct tOption{
	char *name;
	int cmd;
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
	OPT_SPEED
};

#endif
