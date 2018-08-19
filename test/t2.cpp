#include <lily.hpp>
#include <lilyConstruct.hpp>
#include <lilyParse.hpp>

void pr(const char* s) {
	auto v= lilyParse(std::string(s));
	std::cout << v->typeName() << ": ";
	WRITELN(v);
}

int main () {
	pr(" \n \"Hi,\nand\\n \\\"you\" 123");
	pr("7");
	pr("1");
	pr("0");
	pr("+1");
	pr("-1");
	pr(" 12 ");
	pr(" 12 3");
	pr(" 12A 3");
	pr(" + 3");
	pr(" +3");
	pr(" -3");
	pr("-4 ");
	pr("9223372036854775808"); // 2^63
	pr("9223372036854775807"); // 2^63-1
	pr("-9223372036854775809");
	pr("-9223372036854775808");
}

