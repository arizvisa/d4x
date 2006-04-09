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
#ifndef _D4X_SOUND_HEADER_
#define _D4X_SOUND_HEADER_

#include "queue.h"
#include "mutex.h"
#include <pthread.h>
#include <list>

enum SOUND_EVENTS{
	SND_STARTUP,
	SND_COMPLETE,
	SND_FAIL,
	SND_QUEUE_FINISH,
	SND_ADD,
	SND_DND_DROP,
	SND_LAST
};

namespace d4x{

	struct SndEvent{
		int event;
		time_t birth;
		SndEvent(int _event):event(_event),birth(time(NULL)){};
		SndEvent(const SndEvent &ev):event(ev.event),birth(ev.birth){};
	};

};

class d4xSndServer{
	std::list<d4x::SndEvent> queue;
	d4x::Mutex my_mutex,exit_lock;
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
