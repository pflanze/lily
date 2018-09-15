#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdexcept>

#include "lilyUtil.hpp"


void stringlike_write(const std::string& str,
		      std::ostream& out,
		      char quoteChar) {
	out << quoteChar;
	for (char c : str) {
		if (c==quoteChar) {
			out << '\\' << c;
		} else {
			const char* str= NULL;
			switch (c) {
			case '\n': str = "\\n"; break;
			case '\r': str = "\\r"; break;
			case '\f': str = "\\f"; break;
			case '\t': str = "\\t"; break;
			case '\\': str = "\\\\"; break;
			default:
				out << c;
				// XX utf8
				continue;
			}
			out << str;
		}
	}
	out << quoteChar;
}


std::string show(std::string str) {
	std::ostringstream out;
	stringlike_write(str, out, '"');
	return out.str();
}


void throwWithStrerror(const std::string &msg) {
#define SIZ 256
	char buf[SIZ];
	// https://en.cppreference.com/w/c/string/byte/strerror
	// describes strerror_s but not available
	//strerror_s(buf, SIZ, errno);
	const char* err= strerror_r(errno, buf, SIZ);
	
	throw std::runtime_error(STR(msg << ": " << err));
#undef SIZ
}

