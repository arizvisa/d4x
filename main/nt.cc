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
#include <sys/resource.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <ctype.h>
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
tMain _aa_;

char *VERSION_NAME="WebDownloader for X " VERSION;
char *LOCK_FILE;
char *LOCALE_CODEPAGE;

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
		if (HOME_VARIABLE)
			CFG.GLOBAL_SAVE_PATH=sum_strings(HOME_VARIABLE,"/MyDownloads",NULL);
		else
			CFG.GLOBAL_SAVE_PATH=copy_string("/");
	};
	if (CFG.SKIP_IN_CLIPBOARD==NULL)
		CFG.SKIP_IN_CLIPBOARD=copy_string("html htm php3 gif jpg png");
	if (CFG.CATCH_IN_CLIPBOARD==NULL)
		CFG.CATCH_IN_CLIPBOARD=copy_string("zip .gz rar arj exe rpm .bz2 deb tgz mp3");
	CFG.LOCAL_SAVE_PATH=copy_string(CFG.GLOBAL_SAVE_PATH);
	if (!CFG.SEARCH_ENGINES)
		CFG.SEARCH_ENGINES=copy_string("1");
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
	if (!CFG.THEMES_DIR)
		CFG.THEMES_DIR=sum_strings(D4X_SHARE_PATH,"/themes/",NULL);
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

void test_segments(){
	tSegmentator segments;
	printf("0-1500:%i\n",segments.insert(0,1500));
	segments.print();
	printf("0-1500:%i\n",segments.insert(0,1500));
	segments.print();
	printf("1500-1600:%i\n",segments.insert(1500,1600));
	segments.print();
	printf("1100-1200:%i\n",segments.insert(1100,1200));
	segments.print();
};

int main(int argc,char **argv) {
	struct rlimit rl;
	getrlimit(RLIMIT_FSIZE,&rl);
	if (rl.rlim_cur<rl.rlim_max){
		rl.rlim_cur=rl.rlim_max;
		setrlimit(RLIMIT_FSIZE,&rl);
	};
#ifdef ENABLE_NLS
	bindtextdomain("d4x", LOCALEDIR);
	bind_textdomain_codeset (PACKAGE, "UTF-8");
	textdomain("d4x");
	char *a=getenv("LANG");
	a=a?index(a,'.'):NULL;
	if (a){
		LOCALE_CODEPAGE=copy_string(a+1);
		a=LOCALE_CODEPAGE;
		while (*a){ *a=toupper(*a);a++;};
	}else
		LOCALE_CODEPAGE="UTF-8";
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
	struct stat stat_buf;
	if (stat(LOCK_FILE,&stat_buf)==0 && S_ISLNK(stat_buf.st_mode))
		unlink(LOCK_FILE);
	LOCK_FILE_D=open(LOCK_FILE,O_TRUNC | O_CREAT |O_RDWR,S_IRUSR | S_IWUSR);
	if (LOCK_FILE<0 || lockf(LOCK_FILE_D,F_TLOCK,0)) {
		if (parse_command_line_already_run(argc,argv))
			g_print(_("%s probably is already running\n"),VERSION_NAME);
		if (argc==1)
			send_popup();
		return 0;
	};
	if (_aa_.init()) return(1);
	_aa_.run(argc,argv);
	_aa_.run_after_quit();
	var_free(&CFG);
	return 0;
};
