#include "lilyUtil.hpp"


void string_onelinePrint(std::string& str, std::ostream& out, char quoteChar) {
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
}


std::string show(std::string str) {
	std::ostringstream out;
	out << '"';
	string_onelinePrint(str, out, '"');
	out << '"';
	return out.str();
}


