#include <lily.hpp>
#include <lilyConstruct.hpp>
#include <lilyParse.hpp>

int main () {
	auto s= std::string(" \"Hi\" 123");
	LilyObjectPtr o1= lilyParse(s);
	WRITELN(o1);
}

