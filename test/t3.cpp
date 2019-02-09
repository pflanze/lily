#include <lily.hpp>
#include <lilyConstruct.hpp>
#include <lilyParse.hpp>
#include <lilyDefaultEnvironment.hpp>

using namespace lily;
using namespace lilyConstruct;

LilyListPtr environment;

void note(const char* s) {
	std::cout << "\n---- " << s << " ----\n";
}

void run(const char* codestring, bool catchExceptions) {
	auto codeobject= lilyParse(std::string(codestring),
				   true,
				   !catchExceptions);
	std::cout << "> " << lily::show(codeobject) << std::endl;
	auto action= [&]() {
		auto result= eval(codeobject, environment);
		std::cout << result->typeName() << ": "; WRITELN(result);
	};
	if (catchExceptions) {
		try {
			action();
		} catch (std::logic_error& e) {
			std::cout << "ERR: " << e.what() << std::endl;
		} catch (LilyErrorWithWhat& e) {
			std::cout << "ERR: " << e.what() << std::endl;
		}
	} else {
		action();
	}
}

void e(const char* codestring) { run(codestring, true); }
void _e(const char* codestring) { run(codestring, false); }

int main () {
	lily::init();
	environment= lilyDefaultEnvironment();

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
	e("(+ 10 'a)");
	e("(+ 10 \"a\")");
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
	e("(* 1/4 2/3)"); // 1/6
	e("(* 1/4 2/3 5)"); // 5/6
	e("(* 1/4 2/3 5 2.)"); // 1.6666666666666667
	e("(/ 1/4 2/3)"); // 3/8
	e("(/ 3/4 2)"); // 3/8
	e("(/ 2 3/4)"); // 8/3

	note("more fractionals");
	e("(/ 5/1)");
	e("(/ 5/0)");
	e("(/ 1/5)");
	e("(/ 1/9223372036854775807 2)");
	e("(* 1/9223372036854775807 2)");
	// > (* 1/9223372036854775807 3/2)
	// ERR: #<int64-overflow-error (* 9223372036854775807 2)>
	// the "need 128 bits temporarily" case, solve in the future
	// to give 3/18446744073709551614
	e("(/ 1/9223372036854775807 3/2)");
	e("(* 2 1/9223372036854775807)");
	e("(* 1/2 1/9223372036854775807)");
	e("(* 1/9223372036854775807 1/2)");
// > (+ 1/9223372036854775807 1/2)
// ERR: #<int64-overflow-error (* 9223372036854775807 2)>
// > (+ 1/9223372036854775807 1/9223372036854775807)
// ERR: #<int64-overflow-error (* 9223372036854775807 9223372036854775807)>
// oh weak");
	e("(- 1/9223372036854775807 0)");
	e("(- 0 1/9223372036854775807)");
	e("(- 1 1/9223372036854775807)");

	note("exact->inexact");
	e("(exact->inexact 3)");
	e("(exact->inexact .3)");
	e("(exact->inexact 3/2)");

	note("invalid");
	e("()"); // empty call
	e("(+ 10 . 20)"); // improper list -- XX say 'argument list'
	e("(+ . 10)"); // improper list -- XX say 'argument list'
	e("(10 20)"); // not a function error

	note("chars");
	e("#\\x40");
	e("(char->integer (string-ref \"\\100\" 0))");
	// e("#o100"); -> 64
	e("(string->list \"Hello\\0 World\\b\\a\\f\\t\\r\\n\")");
	e("(list #\\f #\\esc #\\return #\\xff #\\ufffe #\\U0001effe)");
	e("(list->string (list #\\f #\\esc #\\return #\\x70))");
	e("(list->string (list #\\f #\\esc #\\return #\\xff #\\ufffe #\\U0001effe))");
	e("(string->list \"f\\33\\r\\377\\ufffe\\U0001effe\")");
	e("(char->integer #\\xef)");
	e("#\\xef");
	e("(char->integer #\\uffff)");
	e("(char->integer #\\UFFFE)");
	e("(char->integer #\\uFFFE)");
	e("(char->integer #\\UFFFFffff)");
	e("(char->integer #\\UFFFFFFFF)");
	e("(char->integer #\\U000fffff)");
	e("(char->integer #\\u000fffff)");
	e("(string->list \"a\\33b\")");
	e("(map char->integer (string->list \"\\12\"))");
	e("(map char->integer (string->list \"\\377\"))");
	e("(map char->integer (string->list \"\\378\"))");
	e("(map char->integer (string->list \"\\477\"))");

	note("strings");
	e("(string-append \"a\" \"b c\" \"d\")");
	e("(string-append \"a\")");
	e("(string-append)");

	note("stdlib");
	e("(list? (list \"foo\"))");
	e("(list? (list))");
	e("(list? (cons 1 2))");
	e("(list? \"foo\")");
	e("(inc 10)");
	e("(dec 0)");
	e("dec");
	e("inc");
	e("(fold-right inc (list 10 11 13))");
	e("(fold-right inc #f (list 10 11 13))");
	e("(fold-right cons #f (list 10 11 13))");
	e("(map inc (list 10 11 13))");
	e("(improper->proper-map inc (cons 10 (cons 20 30)))");
	e("(improper->proper-map inc 10)");

	note(".code");
	e("(.code (cons 10 (cons 20 30)))");
	e("(.code (cons 20 30))");
	e("(.code (cons 20 (list)))");
	e("(.code (list (list 10) 20))");
	e("(.code cons)");
}
