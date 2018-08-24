#include <lily.hpp>
#include <lilyConstruct.hpp>
#include <lilyParse.hpp>

void pr(const char* s) {
	auto v= lilyParse(std::string(s));
	std::cout << v->typeName() << ": ";
	WRITELN(v);
}

void note(const char* s) {
	std::cout << "---- " << s << " ----\n";
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

	note("overflow");
	pr("9223372036854775808"); // 2^63
	pr("9223372036854775807"); // 2^63-1
	pr("-9223372036854775809");
	pr("-9223372036854775808");
	pr("922337203685477580844A"); // symbol, not number overflow error
	pr("  (1 2 3)");
	pr("hi:all there");
	pr("(hi \"there\")");
	pr("(hi .)");
	pr("(hi .x)");
	pr("(hi . x)");
	pr("(hi .x");
	pr("(hi . x");
	pr("(hi .x())");
	pr("(hi . x())"); // InvalidDottedList
	pr("(\n hi\"there\" ) ");
	pr("(a b . (c))"); // (a b c)

	note("line comments");
	pr("(\n hi\"there\" ;; all good\n) ");
	pr("(hi \"there\")3");
	pr("(hi \"there\""); // UnexpectedEof
	pr("(()5())");
	pr("(hi () ( ;\n) \"there\")");
	pr("(hi () ( ;\n) \"there\""); // UnexpectedEof
	pr("(hi . ; \n x)");
	pr("(hi .; \n x)");

	note("in-line comments"); // or 'range comments' or whatever
	pr(" #|hi|# there"); // "there"
	pr(" #| |# 234"); // 234
	pr(" #| |#| 234|"); // | 234|
	pr(" #| |#  234|"); // 234
	pr("  234|"); // 234

	note("  all: (hi . x)");
	pr("(hi .#||#x)"); // Gambit parses .#| as invalid token!
	pr("(hi .#|||#x)");
	pr("(hi .#||\\|#x)"); // Gambit does not allow escaping of the end marker
	pr("(hi .#|\n|#x)");
	pr("(hi .#||; |\n||# x)");

	note("symbol, unknown special, symbols:");
	pr(".#a"); // symbol
	pr("(1 .#a)"); // len 2
	pr("#a"); // UnknownSpecial; Gambit: invalid token
	pr("..");
	pr("(a ..)");
	pr("(a ...)");
	pr("(a . ..)");
	pr("(a .. .)"); // invalid dotted list
	pr("(#!void #f #t)");
	pr("(#!void#f #t)"); // XX currently same as above; Gambit: Invalid '#!' name: "void#f"
	pr("(#t#f)"); // XX currently accepted as 2 values; Gambit: invalid token

	note("keywords (not implemented)");
	pr("foo:bar");
	pr("foo: bar");
	pr("(foo :bar)");
	pr("(|foo:||:bar|)");
	pr(" : ");
	pr(" :: "); // Yes, Gambit parses this as a keyword

	note("syntactic sugar");
	pr("'a"); // 'a
	pr("(quote a)"); // 'a
	pr("'()"); // '()
	pr("  ' ; \r\n \t(;\n)"); // '()
	pr(" ` ( , ; \n a )"); // `(,a)
	pr(" ` #| asdf \n wew |# (a . ,b)"); // `(a unquote b)
	pr("(quasiquote)");
	pr("(quasiquote a b)");
	pr("(quasiquote a)");

	note("demo");
	pr("(+ 10 ; \n  20)");
}

