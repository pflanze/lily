#include <lily.hpp>
#include <lilyConstruct.hpp>
#include <lilyParse.hpp>
#include <lilyDefaultEnvironment.hpp>

using namespace lily;
using namespace lilyConstruct;

LilyListPtr environment= NIL;

int main (int argc, const char** argv) {
	lily::init();
	environment= lilyDefaultEnvironment();
	std::string line;
	while (true) {
		std::cout << "> " << std::flush;
		std::getline(std::cin, line);
		if (std::cin.eof())
			break;
		LilyObjectPtr expr= lilyParse(line, true); 
		IF_LETU_AS(err, LilyParseError, expr) {
			std::cout << "PARSE_ERR: " << err->what() << std::endl;
		} else {
			try {
				LilyObjectPtr res= eval(expr, environment);
				WRITELN(res);
			} catch (std::logic_error& e) {
				std::cout << "ERR: " << e.what() << std::endl;
			} catch (LilyErrorWithWhat& e) {
				std::cout << "ERR: " << e.what() << std::endl;
			}
		}
	}
	if (line.length()) {
		std::cout << "PRE_PARSE_ERR: unfinished line" << std::endl;
		return 1;
	} else {
		return 0;
	}
}
