#include <lily.hpp>
#include <lilyConstruct.hpp>
#include <lilyParse.hpp>
#include <lilyDefaultEnvironment.hpp>

auto environment= lilyDefaultEnvironment();

int main (int argc, const char** argv) {
	std::string line;
	while (true) {
		std::getline(std::cin, line);
		if (std::cin.eof())
			break;
		LilyObjectPtr expr= lilyParse(line, true); 
		// XX error handling.
		try {
			LilyObjectPtr res= eval(expr, environment);
			WRITELN(res);
		} catch (std::logic_error& e) {
			std::cout << "ERR: " << e.what() << std::endl;
		}
	}
}
