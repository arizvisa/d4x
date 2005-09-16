#include "path.h"

using namespace d4x;

void Path::normalize(){
	size_type p=npos;
	while((p=find("//",p))!=npos){
		replace(p,2,"/");
	};
	while((p=find("/./",p))!=npos){
		replace(p,3,"/");
	};
	while((p=find("/../"),p)!=npos){
		size_type r=rfind('/',p-1);
		if (r!=npos){
			replace(r,p+4-r,"/");
		}else{
			replace(0,p+4,"/");
		};
	};

	if (substr(0,2)=="./")
		replace(0,2,"/");
	if (substr(0,3)=="../")
		replace(0,3,"/");

	//replace tailing '/.'
	if ((p=rfind("/."))!=npos && p==length()-2)
	    replace(p,2,"/");
};

Path& Path::operator/=(const Path &_p){
	if (!_p.empty()){
		if (_p[0]!='/'){
			std::string::operator+=('/');
		};
		std::string::operator+=(_p);
		normalize();
	};
	return *this;
};

Path Path::operator/(const Path &_p) const{
	Path r(*this);
	r/=_p;
	return r;
};
