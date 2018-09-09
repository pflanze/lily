#include <lily.hpp>
#include <lilyConstruct.hpp>
#include <lilyParse.hpp>
#include <lilyDefaultEnvironment.hpp>

auto environment= lilyDefaultEnvironment();

void note(const char* s) {
	std::cout << "---- " << s << " ----\n";
}

void e(const char* codestring) {
	auto codeobject= lilyParse(std::string(codestring), true);
	try {
		auto result= eval(codeobject, environment);
		std::cout << result->typeName() << ": "; WRITELN(result);
	} catch (std::logic_error& e) {
		std::cout << "ERR: " << e.what() << std::endl;
	}
}

void _e(const char* codestring) {
	auto codeobject= lilyParse(std::string(codestring));
	auto result= eval(codeobject, environment);
	std::cout << result->typeName() << ": "; WRITELN(result);
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

	note("invalid evaluation rules");
	// parser giving ParseError, not handled thus passed to eval,
	// checking for the latter to produce an "ill-formed
	// expression" error
	e("(12 . 45 67)");
	// e("(lambda (x) 1)"); // procedure
	// e("(eval (lambda (x) 1))"); // ill-formed expression
	
	note("literals");
	e("1233");
	e(" \"1233\" ");
	e("#t");
	e("#!void");

	note("variable reference");
	e(" + "); // NativeProcedure
	e(" q "); // undefined variable

	note("calls");
	e("(+)"); // 0
	e("(+ 19)"); // 19
	e("(+ 10 20)"); // 30
	e("(- 10 20)"); // -10
	e("(- 10 20 -5)"); // -5
	e("(+ 10 3 4)"); // 17
	e("(+ (+ 10 3) 4)"); // 17
	e("(+ (* 10 3) 4)"); // 34
	e("(quote x)"); // x
	e("'y"); // y
	e("(quote (quote x))"); // 'x
	e("(cons 12 (cons 13 '()))"); // (12 13)
	e("(length (cons 12 (cons 13 '())))");
	e("(length '())");
	e("(length '(1 89 3))");
	e("(length 1 89 3)");

	note("list");
	e("(list)"); // ()
	e("(list 14)"); // (14)
	e("(list 12 13 14)"); // (12 13 14)
	e("(length \"hello\")"); // ERR
	e("(length (list 12 13 14))"); // 3
	e("(reverse (list 12 13 14))"); // (14 13 12)
	e("(cons (+ ((car (cons * +)) (+ 10 1) (- 3 2)) 4) 'hey)"); // (15 . hey)
	e("(cons (car 13) ())"); // tests argument eval order, too, not standardized
	e("(car 4 5)");
	e("(car)");
	e("(cdr (cons 3 6))");
	e("(cdr)");
	e("(cons 10 (list 11 12 '()))"); // (10 11 12 ())

	note("arithmetic");
	e("(-10)"); // err
	e("(- 10)"); // -10
	e("(- 10 . 33)"); // err
	e("(-)"); // Gambit: Wrong number of arguments passed to procedure
	e("(quotient 10 3)"); // 3
	e("(quotient -10 3)"); // -3
	e("(quotient 30)"); // 0 ? Gambit: Wrong number of arguments..
	e("(remainder 30)"); // 1 ? Gambit: Wrong number of arguments..
	e("(remainder -10 3)"); // -1
	e("(modulo -10 3)"); // 2
	e("(/ 10 3)"); // fractional
	e("(integer./ 10 3)"); // 3 -- ah, or now fractional, stupid?
	e("(double./ 10 3)"); // error
	e("(double./ 10. 3.)"); // 3.3333

	note("arithmetic r5rs tests");
	e("(modulo 13 4)");
	e("(remainder 13 4)");
	e("(modulo -13 4)");
	e("(remainder -13 4)");
	e("(modulo 13 -4)");
	e("(remainder 13 -4)");
	e("(modulo -13 -4)");
	e("(remainder -13 -4)");

	note("number tower");
	e("(+ 5 2/3)");
	e("(+ 100 1/2)");
	e("(+ 1/2 100)");
	e("(+ 5/4 2/3)");
	e(" (+ 1/6 1/3)");
	e("(+ 5/6 1/3)");
	e("(+ 1/4 0.5)");
	e("(+ 0.5 1/4)");
	e("(- 1/4 0.5)");
	e("(- 0.5 1/4)");
	e("(- 100)");
	e("(+ 100)");
	e("(- 3/4)");
	e("(+ 3/4)");
	e("(- 100.)");
	e("(+ 100.)");
	e("(* 100)");
	e("(/ 100)");
	e("(* 3/4)");
	e("(/ 3/4)");
	e("(* 100.)");
	e("(/ 100.)");
	e("(+ 2.25 0.5)"); // 2.75
	e("(- 2.25 0.5)"); // 1.75
	e("(* 2.25 0.5)"); // 1.125
	e("(/ 2.25 0.5)"); // 4.5

	note("exact->inexact");
	e("(exact->inexact 3)");
	e("(exact->inexact .3)");
	e("(exact->inexact 3/2)");

	note("invalid");
	e("()"); // empty call
	e("(+ 10 . 20)"); // improper list -- XX say 'argument list'
	e("(+ . 10)"); // improper list -- XX say 'argument list'
	e("(10 20)"); // not a function error
}
