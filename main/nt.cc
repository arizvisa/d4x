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


#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "main.h"
#include "config.h"
#include "face/list.h"
#include "var.h"
#include "locstr.h"
#include "ntlocale.h"
#include "ftpd.h"
#include "segments.h"
#include "srvclt.h"
//-------------------------------------------------
tMain aa;

char *VERSION_NAME="WebDownloader for X "
			VERSION;
char *LOCK_FILE;

static void init_string_variables(){
	if (CFG.DEFAULT_NAME==NULL)
		CFG.DEFAULT_NAME=copy_string("index.html");
	if (CFG.USER_AGENT==NULL)
		CFG.USER_AGENT=copy_string("%version");
	if (CFG.ANONYMOUS_PASS==NULL)
		CFG.ANONYMOUS_PASS=copy_string("-mdem@chat.ru");
	if (CFG.EXEC_WHEN_QUIT==NULL)
		CFG.EXEC_WHEN_QUIT=copy_string("");
	if (!CFG.GLOBAL_SAVE_PATH) {
		CFG.GLOBAL_SAVE_PATH=copy_string(HOME_VARIABLE);
		if (!CFG.GLOBAL_SAVE_PATH) {
			CFG.GLOBAL_SAVE_PATH=copy_string("/");
		};
	};
	if (CFG.SKIP_IN_CLIPBOARD==NULL)
		CFG.SKIP_IN_CLIPBOARD=copy_string("html htm php3 gif jpg png");
	if (CFG.CATCH_IN_CLIPBOARD==NULL)
		CFG.CATCH_IN_CLIPBOARD=copy_string("zip .gz rar arj exe rpm .bz2 deb tgz mp3");
	CFG.LOCAL_SAVE_PATH=copy_string(CFG.GLOBAL_SAVE_PATH);
	if (!CFG.SOUND_COMPLETE)
		CFG.SOUND_COMPLETE=sum_strings(D4X_SHARE_PATH,"/sounds/complete.wav",NULL);
	if (!CFG.SOUND_ADD)
		CFG.SOUND_ADD=sum_strings(D4X_SHARE_PATH,"/sounds/add.wav",NULL);
	if (!CFG.SOUND_FAIL)
		CFG.SOUND_FAIL=sum_strings(D4X_SHARE_PATH,"/sounds/fail.wav",NULL);
	if (!CFG.SOUND_DND_DROP)
		CFG.SOUND_DND_DROP=sum_strings(D4X_SHARE_PATH,"/sounds/dnd.wav",NULL);
	if (!CFG.SOUND_QUEUE_FINISH)
		CFG.SOUND_QUEUE_FINISH=sum_strings(D4X_SHARE_PATH,"/sounds/finish.wav",NULL);
	if (!CFG.SOUND_STARTUP)
		CFG.SOUND_STARTUP=sum_strings(D4X_SHARE_PATH,"/sounds/startup.wav",NULL);
};

static void send_popup(){
	tMsgClient *clt=new tMsgClient;
	clt->send_command(PACKET_POPUP,NULL,0);
};

#ifdef DEBUG_ALL

static char *prog_name;

/* aught! got sigsegv :(
 *  try to attach gdb to failed process to look around
 */
void segv_handler(int signum) {
	char pid_str[128];
	volatile int tmp;
	
	fprintf(stderr, "pid %d got sinal SIGSEGV\n", getpid());
	if ((tmp=fork()) < 0) {
		perror("fork"); exit(EXIT_FAILURE);
	}
	if (tmp) {
		while (tmp) { /* do nothing forever :) */ }
	} else {
		sprintf(pid_str, "%d", getppid());
		if (CFG.WITHOUT_FACE) {
			execlp("gdb", "gdb", prog_name, pid_str, NULL);
		} else {
			execlp("xterm", "xterm", "-e", "gdb", prog_name, pid_str, NULL);
		}
		perror("execlp");
		exit(EXIT_FAILURE);
	}
}
#endif

#include "xml.h"

int main(int argc,char **argv) {
//	free(malloc(10)); //hack for electricFence which does not work with threads :(
#ifdef ENABLE_NLS
	bindtextdomain("d4x", LOCALEDIR);
	textdomain("d4x");
	setlocale(LC_ALL,"");
#endif

#ifdef DEBUG_ALL
	prog_name = argv[0];
	signal(SIGSEGV, segv_handler);
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
		if (argc==1)
			send_popup();
		return 0;
	};
	if (aa.init()) return(1);
	aa.run(argc,argv);
	aa.run_after_quit();
	var_free(&CFG);
	return 0;
};
