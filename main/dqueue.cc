#include "dqueue.h"
#include "dbc.h"
#include "face/list.h"
#include "face/lod.h"

d4xDownloadQueue::d4xDownloadQueue(){
	queues[DL_ALONE]=NULL;
	for (int i=DL_ALONE+1;i<DL_TEMP;i++){
		queues[i]=new tDList(i);
		queues[i]->init(0);
	};
};

int d4xDownloadQueue::count(int q=DL_ALONE){
	if (q!=DL_ALONE){
		return(queues[q]->count());
	};
	int num=0;
	for (int i=DL_ALONE+1;i<DL_TEMP;i++)
		num+=queues[i]->count();
	return num;
};

void d4xDownloadQueue::set_defaults(){
	queues[DL_COMPLETE]->set_empty_func(main_menu_completed_empty,main_menu_completed_nonempty);
	queues[DL_STOP]->set_empty_func(main_menu_failed_empty,main_menu_failed_nonempty);
};

void d4xDownloadQueue::init_pixmaps(){
	queues[DL_COMPLETE]->init_pixmap(PIX_COMPLETE);
	queues[DL_RUN]->init_pixmap(PIX_RUN_PART);
	queues[DL_WAIT]->init_pixmap(PIX_WAIT);
	queues[DL_PAUSE]->init_pixmap(PIX_PAUSE);
	queues[DL_STOP]->init_pixmap(PIX_STOP);
	queues[DL_STOPWAIT]->init_pixmap(PIX_STOP_WAIT);
};

int d4xDownloadQueue::current_run(char *host,int port){
	DBC_RETVAL_IF_FAIL(host!=NULL,0);

	tDownload *temp=queues[DL_RUN]->last();
	int count=0;
	while(temp) {
		if (strcasecmp(host,temp->info->host.get())==0 &&
		    port==temp->info->port){
			count+=1;	
		if (temp->split){
				tDownload *temp1=temp->split->next_part;
				while (temp1){
					count+=1;
					temp1=temp1->split->next_part;
				};
			};
		};
		temp=queues[DL_RUN]->next();
	};
	return count;
};

void d4xDownloadQueue::replace_list(tDList *list,int q){
	if (queues[q]) delete(queues[q]);
	queues[q]=list;
};

tDownload *d4xDownloadQueue::first(int q){
	return(queues[q]->first());
};

tDownload *d4xDownloadQueue::last(int q){
	return(queues[q]->last());
};

void d4xDownloadQueue::forward(tDownload *what){
	if (what->myowner)
		what->myowner->forward(what);
};

void d4xDownloadQueue::backward(tDownload *what){
	if (what->myowner)
		what->myowner->backward(what);
};

void d4xDownloadQueue::insert_before(tDownload *what,tDownload *where){
	if (where->myowner){
		where->myowner->insert_before(what,where);
	}else{
		printf("Strange error in d4xDownloadQueue::insert_before\n");
	};
};

void d4xDownloadQueue::add(tDownload *what,int where=DL_WAIT){
	queues[where]->insert(what);
};

void d4xDownloadQueue::del(tDownload *what){
	if (what->myowner)
		what->myowner->del(what);
};

d4xDownloadQueue::~d4xDownloadQueue(){
	for (int i=DL_ALONE+1;i<DL_TEMP;i++)
		delete(queues[i]);	
};
	
