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
#ifndef D4X_FILTERS_IMPLEMENTATION_HEADER_20051127
#define D4X_FILTERS_IMPLEMENTATION_HEADER_20051127

#include "locstr.h"
#include "queue.h"
#include "addr.h"
#include "sort.h"
#include "mutex.h"
#include <list>
#include <map>
#include <ostream>
#include <istream>

namespace d4x{
	class Filter{
	public:
		struct Rule{
			std::string path,file,host,params,tag;
			int proto;
			bool include;
			bool match(const URL &addr) const;
		};
		
		bool include;
		std::string name;
		typedef std::list<Rule>::iterator iterator;
		typedef std::list<Rule>::const_iterator const_iterator;

		bool match(const URL &addr) const;
		iterator append(const Rule &r);
		void replace(iterator &it,const Rule &r);
		void remove(iterator &it);
		bool empty();
		
		template<typename Functor>
		void each_iter(Functor F){
			iterator it=Rules.begin(),end=Rules.end();
			while(it!=end){
				F(it);
				it++;
			};
		};
	private:
		friend std::ostream &operator<<(std::ostream &,const Filter &);
		std::list<Rule> Rules;
	};

	std::ostream &operator<<(std::ostream &a,const Filter::Rule &rule);
	std::istream &operator>>(std::istream &a,Filter::Rule &rule);
		
	std::ostream &operator<<(std::ostream &a,const Filter &filter);
	std::istream &operator>>(std::istream &a,Filter &filter);
	
	class FiltersDB{
		Mutex mtx;
		std::map<std::string,Filter> Filters;
	public:
		void insert(const Filter&);
		void remove(const std::string &name);
		Filter find(const std::string &name); //if notfound return unnamed filter
		template<typename Functor>
		void each(Functor &F) const{
			std::map<std::string,Filter>::const_iterator it=Filters.begin(),end=Filters.end();
			while(it!=end){
				F(*it);
				++it;
			};
		};
		template<typename Functor>
		void each_name(Functor F) const{
			std::map<std::string,Filter>::const_iterator it=Filters.begin(),end=Filters.end();
			while(it!=end){
				F(it->first);
				++it;
			};
		};
		
		bool empty();
		friend std::ostream &operator<<(std::ostream &,const FiltersDB &);
	};
	
	std::ostream &operator<<(std::ostream &a,const FiltersDB &DB);
	std::istream &operator>>(std::istream &a,FiltersDB &DB);
	void filters_store_rc();
	void filters_load_rc();
};

extern d4x::FiltersDB FILTERS_DB;
#endif //D4X_FILTERS_IMPLEMENTATION_HEADER_
