#ifndef _D4X_PATH_CLASS_HEADER_
#define _D4X_PATH_CLASS_HEADER_

#include <string>

namespace d4x{

	/* why not boost::path?
	 */
	
	class Path:public std::string{
		void normalize(); //remove all /./ and /../
	public:
		Path():std::string(){};
		Path(const Path &_p):std::string(_p){};
		Path(const char*_p):std::string(_p){normalize();};
		Path(const std::string &_p):std::string(_p){normalize();};
		Path& operator =(const Path &_p){
			std::string::operator=(_p);
			return *this;
		};
		Path& operator /=(const Path &); //concatenate(append) path
		Path operator/(const Path &) const;
	};
};


#endif //_D4X_PATH_CLASS_HEADER_
