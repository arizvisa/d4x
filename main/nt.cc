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

#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include "main.h"
#include "config.h"
#include "face/list.h"
#include "var.h"
#include "locstr.h"
#include "ntlocale.h"
#include "ftpd.h"
//-------------------------------------------------
tMain aa;

char *VERSION_NAME="WebDownloader for X 1.13";
char *LOCK_FILE;

static void init_string_variables(){
	if (CFG.DEFAULT_NAME==NULL)
		CFG.DEFAULT_NAME=copy_string("index.html");
	if (CFG.USER_AGENT==NULL)
		CFG.USER_AGENT=copy_string("%version");
	if (CFG.EXEC_WHEN_QUIT==NULL)
		CFG.EXEC_WHEN_QUIT=copy_string("");
	if (!CFG.GLOBAL_SAVE_PATH) {
		CFG.GLOBAL_SAVE_PATH=copy_string(HOME_VARIABLE);
		if (!CFG.GLOBAL_SAVE_PATH) {
			CFG.GLOBAL_SAVE_PATH=copy_string("/");
		};
	};
	if (CFG.SKIP_IN_CLIPBOARD==NULL)
		CFG.SKIP_IN_CLIPBOARD=copy_string("html htm php3 gif jpg");
	CFG.LOCAL_SAVE_PATH=copy_string(CFG.GLOBAL_SAVE_PATH);
};

int main(int argc,char **argv) {
#ifdef ENABLE_NLS
	bindtextdomain("nt", LOCALE);
	textdomain("nt");
#endif
	if (parse_command_line_preload(argc,argv)) return 0;
	HOME_VARIABLE=copy_string(getenv("HOME"));
	if (!HOME_VARIABLE)
		puts(_("WARNING!!! Can't find HOME variable! So can't read config!"));
	LOCK_FILE=sum_strings(g_get_tmp_dir(),
			      "/downloader_for_x_lock_",
			      g_get_user_name(),NULL);
/* init histories
 */
	for (int i=0;i<LAST_HISTORY;i++)
		ALL_HISTORIES[i]=new tHistory;
	read_config();
	init_string_variables();
	LOCK_FILE_D=open(LOCK_FILE,O_TRUNC | O_CREAT |O_RDWR,S_IRUSR | S_IWUSR);
	if (LOCK_FILE<0 || lockf(LOCK_FILE_D,F_TLOCK,0)) {
		if (parse_command_line_already_run(argc,argv))
			printf(_("%s probably is already running\n"),VERSION_NAME);
		return 0;
	};
	aa.init();
	aa.run(argc,argv);
	aa.run_after_quit();
	return 0;
};
