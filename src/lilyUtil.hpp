#include <sstream>


// XXX properly compile
static void
string_onelinePrint(std::string& str, std::ostream& out, char quoteChar) {
	for (char c : str) {
		if (c==quoteChar) {
			out << '\\' << c;
		} else if (c== '\n') {
			out << "\\n";
		} else if (c== '\\') {
			out << "\\\\";
		} else {
			out << c;
			// XX utf8
		}
	}
}


// would be useful to immplement show(LilyObjectPtr):

// static
// std::string
// string_onelineString(std::string str) {
// 	std::ostringstream out;
// 	out << '"';
// 	string_onelinePrint(str, out, '"');
// 	out << '"';
// 	return out.str();
// }

