#ifndef _LILYUTIL_HPP
#define _LILYUTIL_HPP

#include <sstream>
#include <iostream>
#include <string>


void string_onelinePrint(std::string& str, std::ostream& out, char quoteChar);

std::string show(std::string str);

#define STR(e) ([&]() -> std::string {			\
			std::ostringstream _STR_o;	\
			_STR_o << e;			\
			return _STR_o.str();		\
		})()



#endif /* _LILYUTIL_HPP */
