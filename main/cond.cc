#include "cond.h"
#include "locstr.h"
#include "var.h"
#include <unistd.h>
#include <stdio.h>

char *cond_names[]={
	"Time"
};

tAbstractCondition::tAbstractCondition(){
	parent=NULL;
};

void tAbstractCondition::set(tDownload *where){
	if (parent) parent->conditions->del(this);
	parent=where;
	parent->conditions->insert(this);
};

void tAbstractCondition::save_begin(int fd){
	write(fd,"Condition:\n",strlen("Condition:\n"));
	write_named_string(fd,"Type:",cond_names[type()]);
};

void tAbstractCondition::save_end(int fd){
	write(fd,"ConditionEnd:\n",strlen("ConditionEnd:\n"));
};

void tAbstractCondition::save(int fd){
	save_begin(fd);
	save_data(fd);
	save_end(fd);
};

tAbstractCondition::~tAbstractCondition(){
	if (parent) parent->conditions->del(this);
	parent=NULL;
};

/* time condition is replacer for time filed in tDownload
 */

tTimeCond::tTimeCond(){
	parent=NULL;
	date=time_t(0);
};

void tTimeCond::set(tDownload *where){
	tAbstractCondition *cond=(tAbstractCondition *)where->conditions->last();
	while(cond){
		if (cond->type()==COND_TIME){
			delete(cond);
			break;
		};
		cond=(tAbstractCondition *)where->conditions->next();
	};
	tAbstractCondition::set(where);
};

void tTimeCond::save_data(int fd){
	write_named_time(fd,"Time:",date);
};

int tTimeCond::type(){
	return COND_TIME;
};

int tTimeCond::load(int fd){
	char buf[MAX_LEN];
	while(f_rstr(fd,buf,MAX_LEN)>0){
		if (equal_uncase(buf,"ConditionEnd:")) break;
		if (equal_uncase(buf,"Time:")){
			if (f_rstr(fd,buf,MAX_LEN)<0) return -1;
			sscanf(buf,"%ld",&date);
		};
	};
	return 0;
};

void tTimeCond::set_time(time_t when){
	date=when;
};

int tTimeCond::check(){
	time_t temp;
	time(&temp);
	if (temp<date)
		return 1;
	return 0;
};
