#include "sm.h"

d4xOldSocket::d4xOldSocket(){
	info=NULL;
	sock=NULL;
};

d4xOldSocket::d4xOldSocket(tAddr *a, tSocket *s){
	info=a;
	sock=s;
};

void d4xOldSocket::print(){
};

d4xOldSocket::~d4xOldSocket(){
	if (sock) delete(sock);
	if (info) delete(info);
};

/*********************************************************/

d4xSocketsHistory::d4xSocketsHistory():tQueue(){
	pthread_mutex_init(&my_lock,NULL);
	MaxNum=50; /* FIXME: it's bad to use such constants*/
};

d4xSocketsHistory::~d4xSocketsHistory(){
	pthread_mutex_destroy(&my_lock);
};


void d4xSocketsHistory::insert(d4xOldSocket *what){
	pthread_mutex_lock(&my_lock);
	what->birth=time(NULL);
	tQueue::insert(what);
	pthread_mutex_unlock(&my_lock);
};

void d4xSocketsHistory::del(d4xOldSocket *what){
	pthread_mutex_lock(&my_lock);
	tQueue::del(what);
	pthread_mutex_unlock(&my_lock);
};


tSocket *d4xSocketsHistory::find(tAddr *info){
	pthread_mutex_lock(&my_lock);
	d4xOldSocket *s=(d4xOldSocket *)First;
	while(s){
		if (equal(s->info->host.get(),info->host.get()) &&
		    s->info->port==info->port &&
		    s->info->proto==info->proto &&
		    equal(s->info->username.get(),info->username.get())){
			tQueue::del(s);
			tSocket *rval=s->sock;
			s->sock=NULL;
			delete(s);
			pthread_mutex_unlock(&my_lock);
			return(rval);
		}
		s=(d4xOldSocket *)(s->prev);
	};
	pthread_mutex_unlock(&my_lock);
	return(NULL);
};

void d4xSocketsHistory::kill_old(){
	time_t now=time(NULL);
	pthread_mutex_lock(&my_lock);
	while(First){
		d4xOldSocket *s=(d4xOldSocket *)First;
		time_t diff=s->birth-now;
		/* FIXME: constants again */
		if (diff<-50 || diff>50){
			tQueue::del(s);
			delete(s);
		}else
			break;
	};
	pthread_mutex_unlock(&my_lock);
};
