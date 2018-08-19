#include <lily.hpp>
#include <lilyConstruct.hpp>
#include <lilyParse.hpp>

void pr(const char* s) {
	WRITELN(lilyParse(std::string(s)));
}

int main () {
	pr(" \n \"Hi,\nand\\n \\\"you\" 123");
	pr(" 12 3");
	// pr(" + 3");
	// pr(" +3");
	// pr(" -3");
	// pr("-4 ");
	// pr("134");
	// pr("-12345667790213458123190");
	// pr("12345667790213458123190");
}

