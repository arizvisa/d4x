#ifndef _D4X_DOWNLOAD_QUEUE_HEADER_
#define _D4X_DOWNLOAD_QUEUE_HEADER_

#include "dlist.h"

class d4xDownloadQueue{
	tDList *queues[DL_TEMP];
public:
	d4xDownloadQueue();
	void set_defaults();
	void init_pixmaps();
	int count(int q=DL_ALONE);
	int current_run(char *host,int port);
	tDownload *first(int q);
	tDownload *last(int q);
	void forward(tDownload *what);
	void backward(tDownload *what);
	void insert_before(tDownload *what,tDownload *where);
	void replace_list(tDList *list,int q);
	void add(tDownload *what,int where=DL_WAIT);
	void del(tDownload *what);
	~d4xDownloadQueue();
};

#endif // define _D4X_DOWNLOAD_QUEUE_HEADER_
