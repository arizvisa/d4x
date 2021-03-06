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

#include "savedvar.h"
#include "locstr.h"
#include "dlist.h"
#include "filter.h"
#include "dqueue.h"
#include "var.h"

/* -1 on error */

int sv_parse_file(int fd,tSavedVar *var,char *buf,int bufsize){
	switch(var->type){
	case SV_TYPE_INT:{
		if (f_rstr(fd,buf,bufsize)<0) return -1;
		sscanf(buf,"%d",(int *)(var->where));
		break;
	};
	case SV_TYPE_LINT:{
		if (f_rstr(fd,buf,bufsize)<0) return -1;
		sscanf(buf,"%Li",(long long int*)(var->where));
		break;
	};
	case SV_TYPE_FSIZE_TRIGER:{
		if (f_rstr(fd,buf,bufsize)<0) return -1;
		*((d4x::Triger<fsize_t> *)(var->where))=boost::lexical_cast<fsize_t>(std::string(buf));
		break;
	};
	case SV_TYPE_STDSTR:{
		if (f_rstr(fd,buf,bufsize)<0) return -1;
		*((std::string *)(var->where))=buf;
		break;
	};
	case SV_TYPE_PSTR:{
		if (f_rstr(fd,buf,bufsize)<0) return -1;
		((tPStr *)(var->where))->set(buf);
		break;
	};
	case SV_TYPE_URL:{
		d4x::URL *info=(d4x::URL *)(var->where);
		if (f_rstr(fd,buf,bufsize)<0) return -1;
		*info=std::string(buf);
		break;
	};
	case SV_TYPE_TIME:{
		if (f_rstr(fd,buf,bufsize)<0) return -1;
		sscanf(buf,"%ld",(time_t *)(var->where));
		break;
	};
	case SV_TYPE_CFG:{
		tCfg **config=(tCfg **)(var->where);
		if (*config==NULL) *config=new tCfg;
		(*config)->isdefault=0;
		if ((*config)->load_from_config(fd)<0) return -1;
		break;
	};
	case SV_TYPE_SPLIT:{
		if (f_rstr(fd,buf,bufsize)<0) return -1;
		int tmp;
		tSplitInfo **split=(tSplitInfo **)(var->where);
		sscanf(buf,"%d",&tmp);
		if (tmp>10) tmp=10;
		if (tmp>1){
			if (*split==NULL)
				*split=new tSplitInfo;
			(*split)->NumOfParts=tmp;
		};
		break;
	};
	case SV_TYPE_DOWNLOAD:{
		tDownload **dwn=(tDownload **)(var->where);
		if (*dwn==NULL) *dwn=new tDownload;
		return((*dwn)->load_from_config(fd));
		break;
	};
	case SV_TYPE_QDOWNLOAD:{
		d4xDownloadQueue *q=(d4xDownloadQueue *)(var->where);
		tDownload *dwn=new tDownload;
		if (dwn->load_from_config(fd)==0){
			if (CFG.WITHOUT_FACE==0 && q->qv.ListOfDownloads==NULL){
				q->qv.init();
				q->init_pixmaps();
			};
			int s=dwn->status;
			if (s==DL_LIMIT) s=DL_WAIT;
			if (s>DL_COMPLETE) s=DL_STOP;
			if (s<DL_RUN) s=DL_STOP;
			if (ALL_DOWNLOADS->find(dwn)==NULL){
				ALL_DOWNLOADS->insert(dwn);
				q->add(dwn,s);
				q->qv.add(dwn);
			}else{
				delete(dwn);
			};
			return(0);
		};
		delete(dwn);
		return(-1);
		break;
	};
	case SV_TYPE_QUEUE:{
		tQueue *papa=(tQueue *)(var->where);
		d4xDownloadQueue *q=new d4xDownloadQueue;
		q->load_from_config(fd);
		if (q->name.get()){
			papa->insert(q);
			return(0);
		};
		delete(q);
		return(-1);
		break;
	};
	case SV_TYPE_QV:{
		d4xQueueView *qv=(d4xQueueView *)(var->where);
		return(qv->load_from_config(fd));
		break;
	};
	case SV_TYPE_ALT:{
		d4xAltList **alts=(d4xAltList **)(var->where);
		if (*alts==NULL) *alts=new d4xAltList;
		return((*alts)->load_from_config(fd));
		break;
	};
	default: return(-1);
	};
	return(0);
};
