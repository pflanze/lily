#include <lily.hpp>
#include <lilyConstruct.hpp>
#include <lilyParse.hpp>
#include <lilyDefaultEnvironment.hpp>

auto environment= lilyDefaultEnvironment();

void note(const char* s) {
	std::cout << "---- " << s << " ----\n";
}

void e(const char* codestring) {
	auto codeobject= lilyParse(std::string(codestring));
	auto result= codeobject->eval(environment);
	std::cout << result->typeName() << ": "; WRITELN(result);
}


int main () {
	note("addition");
	e("(+)"); // 0
	e("(+ 10 20)"); // 30
	e("(+ 10 . 20)"); // error
}
