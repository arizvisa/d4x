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

#ifndef MY_SERVER_CLIENT
#define MY_SERVER_CLIENT
#include "liststr.h"
#include <pthread.h>

struct tDownload;

class tMsgServer{
	tStringList *list;
	char *file;
	pthread_mutex_t lock;
	pthread_t thread_id;
	int fd,newfd; /* socket descriptors */
	void cmd_add(int len,int type);
	void cmd_ls(int len,int type);
	void cmd_ack();
	void cmd_return_int(int what);
	void write_dwn_status(tDownload *dwn,int full=0);
 public:
	tMsgServer();
	int init();
        void run();
	void run_thread();
	void stop_thread();
	tString *get_string();
	~tMsgServer();
};

struct tPacketStatus{
	int Status;
	int Size;
	int Download;
	int Speed;
	int Time;
	int Attempt;
	int MaxAttempt;
	char *url;
	tPacketStatus(){url=(char*)NULL;};
	~tPacketStatus(){if (url) delete[] url;};
};

class tMsgClient{
    int fd;
    char *buf;
    int bufsize;
    public:
		tMsgClient();
		int init();
		int send_command(int cmd,char *data,int len);
		int send_command_short(int cmd,char *data,int len);
		int get_answer_int();
		int get_answer_status(tPacketStatus *status);
		void done();
		~tMsgClient();
};

struct tPacket{
	int type;
	int len;
};

enum {
	PACKET_NOP=0,	//nothing to do
	PACKET_ACK,	//just return ACK
	PACKET_ADD,
	PACKET_ASK_SPEED,
	PACKET_ASK_RUN,		// amount of downloads in RUN state
	PACKET_ASK_STOP,	// amount of downloads in FAIL state
	PACKET_ASK_PAUSE,	// amount of paused downloads 
	PACKET_ASK_COMPLETE,	// amount of completed downloads
	PACKET_ASK_READED_BYTES,
	PACKET_SET_SPEED_LIMIT,
	PACKET_SET_SAVE_PATH,
	PACKET_DEL_COMPLETED,
	PACKET_SET_MAX_THREADS,
	PACKET_ICONIFY,
	PACKET_POPUP,
	PACKET_MSG,
	PACKET_ADD_OPEN,
	PACKET_ASK_FULLAMOUNT,	// amount of all downloads
	PACKET_RERUN_FAILED,
	PACKET_EXIT_TIME,
	PACKET_LS,
	PACKET_DEL,
	PACKET_STOP,
	PACKET_UNKNOWN
};

#endif
