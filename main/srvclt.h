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

#ifndef MY_SERVER_CLIENT
#define MY_SERVER_CLIENT
#include "liststr.h"
#include <pthread.h>

class tMsgServer{
	tStringList *list;
	char *file;
	pthread_mutex_t lock;
	int fd,newfd; /* socket descriptors */
	void cmd_add(int len,int type);
	void cmd_ack();
	void cmd_return_int(int what);
 public:
	tMsgServer();
	void init();
        void run();
	tString *get_string();
	~tMsgServer();
};

class tMsgClient{
    int fd;
    int answer_int;
    public:
		tMsgClient();
		int init();
		int send_command(int cmd,char *data,int len);
		int get_answer_int();
		void done();
		~tMsgClient();
};

struct tPacket{
	int type;
	int len;
};

enum {
	PACKET_NOP=0,
	PACKET_ACK,
	PACKET_ADD,
	PACKET_ASK_SPEED,
	PACKET_ASK_RUN,
	PACKET_ASK_STOP,
	PACKET_ASK_PAUSE,
	PACKET_ASK_COMPLETE,
	PACKET_ASK_READED_BYTES,
	PACKET_SET_SPEED_LIMIT,
	PACKET_SET_SAVE_PATH,
	PACKET_DEL_COMPLETED,
	PACKET_SET_MAX_THREADS,
	PACKET_ICONIFY,
	PACKET_POPUP,
	PACKET_MSG,
	PACKET_ADD_OPEN,
	PACKET_UNKNOWN
};

void *server_thread_run(void *what);

#endif
