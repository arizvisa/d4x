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
//-------------------------------------------------
tMain aa;

char *VERSION_NAME="WebDownloader for X 1.06";

int main(int argc,char **argv) {
	if (parse_command_line_preload(argc,argv)) return 0;
	HOME_VARIABLE=copy_string(getenv("HOME"));
	if (!CFG.GLOBAL_SAVE_PATH) {
		CFG.GLOBAL_SAVE_PATH=copy_string(HOME_VARIABLE);
		if (!CFG.GLOBAL_SAVE_PATH) {
			CFG.GLOBAL_SAVE_PATH=copy_string("/");
		};
	};
	CFG.DEFAULT_NAME=copy_string("index.html");
#ifdef ENABLE_NLS
	bindtextdomain("nt", LOCALE);
	textdomain("nt");
#endif
	if (!HOME_VARIABLE)
		puts(_("WARNING!!! Can't find HOME variable! So can't read config!"));
	char *LOCK_FILE=sum_strings(HOME_VARIABLE,"/.ntrc/lock");
	read_config();
	LOCK_FILE_D=open(LOCK_FILE,O_TRUNC | O_CREAT |O_RDWR,S_IRUSR | S_IWUSR);
	if (LOCK_FILE<0 || lockf(LOCK_FILE_D,F_TLOCK,0)) {
		printf(_("%s probably is already running\n"),VERSION_NAME);
		return 0;
	};
	delete(LOCK_FILE);
	aa.init();
	aa.run(argc,argv);
	return 0;
};
