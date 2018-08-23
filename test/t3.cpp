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
	try {
		auto result= eval(codeobject, environment);
		std::cout << result->typeName() << ": "; WRITELN(result);
	} catch (std::logic_error& e) {
		std::cout << "ERR: " << e.what() << std::endl;
	}
}

int main () {
	note("string and symbol pointer comparison");
	// Yes, == on the pointer is a pointer comparison, and
	// interning works
	{
		const char* p= "Hello";
		auto s1= STRING(p);
		auto s2= STRING(p);
		auto s3= STRING(std::string("Hel") + std::string("lo"));
		WRITELN(LIST(s1,s1,BOOLEAN(s1==s1)));
		WRITELN(LIST(s1,s2,BOOLEAN(s1==s2)));
		WRITELN(LIST(s1,s3,BOOLEAN(s1==s3)));
		// WRITELN(LIST(s1,s3,BOOLEAN(*s1==*s3)));
		// no match for ‘operator==’ (operand types are ‘LilyObject’ and ‘LilyObject’)
		//  of course.
	}
	{
		auto s1= SYMBOL("Hello");
		auto s2= SYMBOL("Hello");
		auto s3= SYMBOL(std::string("Hel") + std::string("lo"));
		auto s4= SYMBOL("Hello2");
		WRITELN(LIST(s1,s1,BOOLEAN(s1==s1)));
		WRITELN(LIST(s1,s2,BOOLEAN(s1==s2)));
		WRITELN(LIST(s1,s3,BOOLEAN(s1==s3)));
		// WRITELN(LIST(s1,s3,BOOLEAN(*s1==*s3)));
		WRITELN(LIST(s1,s4,BOOLEAN(s1==s4)));
	}

	note("literals");
	e("1233");
	e(" \"1233\" ");
	e("#t");
	e("#!void");

	note("variable reference");
	e(" + "); // Primitive
	e(" q "); // undefined variable

	note("calls");
	e("(+)"); // 0
	e("(+ 10 20)"); // 30
	e("(+ 10 3 4)"); // 17
	e("(+ (+ 10 3) 4)"); // 17
	e("(+ (* 10 3) 4)"); // 34
	e("(/ 10 3)"); // 3 XX ?  quotient ?

	note("invalid");
	e("()"); // empty call
	e("(+ 10 . 20)"); // improper list -- XX say 'argument list'
	e("(10 20)"); // not a function error
}
