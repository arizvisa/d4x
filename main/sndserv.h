#ifndef _D4X_SOUND_HEADER_
#define _D4X_SOUND_HEADER_

#include "queue.h"
#include <pthread.h>

enum SOUND_EVENTS{
	SND_STARTUP,
	SND_COMPLETE,
	SND_FAIL,
	SND_QUEUE_FINISH,
	SND_ADD,
	SND_DND_DROP,
	SND_LAST
};

struct d4xSndEvent:public tNode{
	int event;
	time_t birth;
	void print(){};
};

class d4xSndServer{
	tQueue *queue;
	pthread_mutex_t my_mutex;
	pthread_t thread_id;
	char *snd_table[SND_LAST];
	void play_sound(int event);
	int stop_now;
	pthread_cond_t cond;
 public:
	d4xSndServer();
	void add_event(int event);
	void run();
	void run_thread();
	void stop_thread();
	void set_sound_file(int event,char *path);
	char *get_sound_file(int event);
	void init_sounds();
	void reinit_sounds();
	~d4xSndServer();
};

extern d4xSndServer *SOUND_SERVER;
#endif
