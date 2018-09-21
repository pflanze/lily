#ifndef _LILYUTIL_HPP
#define _LILYUTIL_HPP

#include <sstream>
#include <iostream>
#include <ostream>
#include <string>


void stringlike_write(const std::string& str,
			 std::ostream& out,
			 char quoteChar);

#define STR(e) ([&]() -> std::string {			\
			std::ostringstream _STR_o;	\
			_STR_o << e;			\
			return _STR_o.str();		\
		})()


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
