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
#include "savedvar.h"
#include "locstr.h"
#include "dlist.h"
#include "filter.h"

/* -1 on error */

int sv_parse_file(int fd,tSavedVar *var,char *buf,int bufsize){
	switch(var->type){
	case SV_TYPE_INT:{
		if (f_rstr(fd,buf,bufsize)<0) return -1;
		sscanf(buf,"%d",(int *)(var->where));
		break;
	};
	case SV_TYPE_PSTR:{
		if (f_rstr(fd,buf,bufsize)<0) return -1;
		((tPStr *)(var->where))->set(buf);
		break;
	};
	case SV_TYPE_URL:{
		tAddr **info=(tAddr **)(var->where);
		if (f_rstr(fd,buf,bufsize)<0) return -1;
		if (*info) delete(*info);
		*info=new tAddr(buf);
		break;
	};
	case SV_TYPE_TIME:{
		if (f_rstr(fd,buf,bufsize)<0) return -1;
		sscanf(buf,"%ld",(time_t *)(var->where));
		break;
	};
	case SV_TYPE_CFG:{
		tCfg *config=(tCfg *)(var->where);
		if (config->load_from_config(fd)<0) return -1;
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
	case SV_TYPE_RULE:{
		d4xFilter *filter=(d4xFilter *)(var->where);
		d4xRule *rule=new d4xRule;
		if (rule->load(fd)==0 && filter!=NULL)
			filter->insert(rule);
		else{
			delete(rule);
			return(-1);
		};
		break;
	};
	case SV_TYPE_FILTER:{
		d4xFiltersTree *tree=(d4xFiltersTree *)(var->where);
		d4xFNode *node=new d4xFNode;
		node->filter=new d4xFilter;
		node->filter->ref();
		if (node->filter->load(fd)==0 && tree!=NULL)
			tree->add(node);
		else{
			delete(node);
			return(-1);
		};
		break;
	};
	default: return(-1);
	};
	return(0);
};
