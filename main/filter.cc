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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "dbc.h"
#include "savedvar.h"
#include "var.h"
#include "filter.h"
#include "face/filtrgui.h"
#include "face/mywidget.h"
#include "face/edit.h"
#include "signal.h"
#include <algorithm>
#include <iterator>
#include <fstream>

using namespace d4x;

bool Filter::Rule::match(const URL &addr) const{
	if (proto && addr.proto!=proto)
		return !include;
	if (!check_mask2_uncase(addr.file.c_str(),file.c_str()))
		return !include;
	
	if (!check_mask2(addr.host.c_str(),host.c_str()))
		return !include;
	if (!check_mask2(addr.path.c_str(),path.c_str()))
		return !include;
	if (equal_uncase(addr.tag.c_str(),tag.c_str())==0)
		return !include;
	if (!check_mask2(addr.params.c_str(),params.c_str()))
		return !include;
	return include;
};

struct FilterMatcher{
	URL addr;
	FilterMatcher(const URL &a):addr(a){};
	bool operator()(const Filter::Rule &rule){
		return rule.match(addr);
	};
};

bool Filter::match(const URL &addr) const{
	std::list<Rule>::const_iterator it=std::find_if(Rules.begin(),Rules.end(),FilterMatcher(addr));
	if (it!=Rules.end())
		return true;
	return include;
};

Filter::iterator Filter::append(const Rule &r){
	Rules.push_back(r);
	iterator it=Rules.begin();
	std::advance(it,Rules.size()-1);
	return it;
};

bool Filter::empty(){
	return name.empty();
};


void Filter::replace(iterator &it,const Rule &r){
	*it=r;
};


void Filter::remove(iterator &it){
	Rules.erase(it);
};

#include <iostream>
namespace d4x{

	std::ostream &operator<<(std::ostream &a,const Filter::Rule &rule){
		a<<"d4xRule:"<<std::endl;
		a<<"inc:\n"<<rule.include<<std::endl;
		a<<"proto:\n"<<rule.proto<<std::endl;
		a<<"host:\n"<<rule.host<<std::endl;
		a<<"path:\n"<<rule.path<<std::endl;
		a<<"tag:\n"<<rule.tag<<std::endl;
		a<<"file:\n"<<rule.file<<std::endl;
		a<<"params:\n"<<rule.params<<std::endl;
		a<<"d4xRule_end"<<std::endl;
		return a;
	};

	std::istream &operator>>(std::istream &a,Filter::Rule &rule){
		std::string tmp;
		std::getline(a,tmp);
		while(tmp!="d4xRule_end"){
			if (a.eof()) break;
			if (tmp=="inc:"){
				a>>rule.include;
				std::getline(a,tmp);
			}else if(tmp=="proto:"){
				a>>rule.proto;
				std::getline(a,tmp);
			}else if(tmp=="host:"){
				std::getline(a,rule.host);
			}else if(tmp=="path:"){
				std::getline(a,rule.path);
			}else if(tmp=="tag:"){
				std::getline(a,rule.tag);
			}else if(tmp=="file:"){
				std::getline(a,rule.file);
			}else if(tmp=="params:"){
				std::getline(a,rule.params);
			}
			std::getline(a,tmp);
		};
		return a;
	};


	std::ostream &operator<<(std::ostream &a,const Filter &filter){
		a<<"d4xFilter:"<<std::endl;
		a<<"name:\n"<<filter.name<<std::endl;
		a<<"inc:\n"<<filter.include<<std::endl;
		std::copy(filter.Rules.begin(),filter.Rules.end(),std::ostream_iterator<Filter::Rule>(a,""));
		a<<"d4xFilter_end"<<std::endl;
		return a;
	};

	std::istream &operator>>(std::istream &a,Filter &filter){
		std::string tmp;
		std::getline(a,tmp);
		if (tmp.empty()) return a;
		while(tmp!="d4xFilter_end"){
			if (tmp=="inc:"){
				a>>filter.include;
				std::getline(a,tmp);
			}else if(tmp=="name:"){
				std::getline(a,filter.name);
			}else if(tmp=="d4xRule:"){
				Filter::Rule rule;
				a>>rule;
				filter.append(rule);
			};
			std::getline(a,tmp);
		};
		return a;
	};

};

/******************************************************************/

void FiltersDB::insert(const Filter &f){
	MutexLocker l(mtx);
	Filters[f.name]=f;
};

void FiltersDB::remove(const std::string &name){
	MutexLocker l(mtx);
	Filters.erase(name);
};


Filter FiltersDB::find(const std::string &name){
	MutexLocker l(mtx);
	std::map<std::string,Filter>::iterator it=Filters.find(name);
	Filter rval;
	if (it!=Filters.end()){
		rval=it->second;
	};
	return rval;
};

bool FiltersDB::empty(){
    MutexLocker l(mtx);
    return Filters.empty();
};


d4x::FiltersDB FILTERS_DB;

#include <iostream>
namespace d4x{
	std::ostream &operator<<(std::ostream &a,const std::pair<std::string,Filter> &p){
		a<<p.second;
		return a;
	};
	
	std::ostream &operator<<(std::ostream &a,const FiltersDB &DB){
		std::copy(DB.Filters.begin(),DB.Filters.end(),std::ostream_iterator<std::pair<std::string,Filter> >(a,""));
		return a;
	};
	
	void filters_store_rc(){
		if (!HOME_VARIABLE)
			return;
		d4x::Path path(HOME_VARIABLE);
		path/=CFG_DIR;
		path/="Filters.2";
		std::ofstream of(path.c_str());
		if (!of) return;
		of<<FILTERS_DB;
	};
	
	std::istream &operator>>(std::istream &a,FiltersDB &DB){
		while(!a.eof()){
			Filter filt;
			a>>filt;
			if(!filt.empty()){
				DB.insert(filt);
			};
		};
		return a;
	};
	
	void filters_load_rc(){
		if (!HOME_VARIABLE)
			return;
		d4x::Path path(HOME_VARIABLE);
		path/=CFG_DIR;
		path/="Filters.2";
		std::ifstream ifs(path.c_str());
		if (!ifs) return;
		ifs>>FILTERS_DB;
	};
};


