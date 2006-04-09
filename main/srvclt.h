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

#ifndef MY_SERVER_CLIENT
#define MY_SERVER_CLIENT
#include "mutex.h"
#include <string>
#include <list>
#include <vector>
#include "queue.h"

struct tDownload;

namespace d4x{
	struct RemoteCommand{
		int type;
		std::vector<std::string> params;
	};
};

class tMsgServer{
	std::list<d4x::RemoteCommand> COMMANDS;
	char *file;
	d4x::Mutex lock;
	pthread_t thread_id;
	int fd,newfd; /* socket descriptors */
	void cmd_add(int len,int type);
	void cmd_ls(int len,int type);
	void cmd_ack();
	void cmd_return_int(int what);
	void write_dwn_status(tDownload *dwn,int full=0);
	void cmd_lstree();
	void cmd_lstree_sub(tQueue *q);
 public:
	tMsgServer();
	int init();
        void run();
	void run_thread();
	void stop_thread();
	d4x::RemoteCommand get_command();
	bool empty();
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
	int readdata(void *buf,int len);
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
	PACKET_LSTREE,
	PACKET_SWITCH_QUEUE,
	PACKET_REFERER,
	PACKET_OPENLIST,
	PACKET_UNKNOWN
};

enum {
	LST_QUEUE,
	LST_SUBQUEUE,
	LST_UPQUEUE,
	LST_DQUEUE
};

struct d4xDownloadQueue;
d4xDownloadQueue *d4x_get_queue_num(int N);

#endif
