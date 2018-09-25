#ifndef _LILYUTIL_HPP
#define _LILYUTIL_HPP

#include <sstream>
#include <iostream>
#include <ostream>
#include <string>


std::string typeidToTypename(const char* typeidstr);


void stringlike_write(const std::string& str,
			 std::ostream& out,
			 char quoteChar);

namespace lilyUtil {
	std::string show(std::string str);
}

#define STR(e) ([&]() -> std::string {			\
			std::ostringstream _STR_o;	\
			_STR_o << e;			\
			return _STR_o.str();		\
		})()

#define XXX throw std::logic_error("XXX unfinished");

#define UNIMPLEMENTED throw std::logic_error("unimplemented");


// template <typename T>
// inline T identity(T v) { return v; }
// ah already in <functional> (since C++20)


#define WARN(e) (std::cerr << e << "\n")

#ifdef DEBUG
#define DEBUGWARN(e) WARN(e)
#else
#define DEBUGWARN(e)
#endif


void throwWithStrerror(const std::string &msg);


#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)


#define TRY(dodebug, calc, catchclauses)	\
	{					\
		if (dodebug) {			\
			calc;			\
		} else {			\
			try {			\
				calc;		\
			} catchclauses;		\
		}				\
	}


#define noreturn __attribute__ ((noreturn))


#endif /* _LILYUTIL_HPP */
