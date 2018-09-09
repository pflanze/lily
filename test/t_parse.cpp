#include <iostream>
#include <string>
#include <parse.hpp>
#include <lilyUtil.hpp>

S SC(const char* str) {
	// will leak memory, but no care for tests
	return S(new std::string(str));
}

void p(StringCursor s) {
	if (s.succeeded()) {
		std::cout << "success: ";
	} else {
		std::cout << "failure (code " << (int)s.error()
			  << ", pos " << s.position() << "): ";
	}
	std::cout << show(s.string()) << "\n";
}


int main () {
	p(skipUntilAfterEol(SC("Hello,\n the World\nshould still be visible.")));
	p(skipUntilAfterEol(SC("Testing \r\nDOS")));
	p(skipUntilAfterEol(SC("Testing \r\n\rDOSmix")));
	p(skipUntilAfterEol(SC("Testing \n\rwrong or unixmix")));
	p(skipUntilAfterEol(SC("Testing \rMac")));
	p(skipWhitespaceOnly(SC("\t \n\r\fok\n")));
	p(expectString(SC("Hello!"), "ell")); // fail
	p(expectString(SC("Hello!"), "Hell")); // "o!"
	p(skipUntilAfter(SC("Hello"), "e")); // "llo"
	p(skipUntilAfter(SC("Hello"), "el")); // "lo"
	p(skipUntil(SC("Hello"), "el")); // "ello"
	p(skipUntilAfter(SC("Hello"), "llo")); // ""
	p(skipUntilAfter(SC("Hello"), "o")); // ""
	p(skipUntilAfter(SC("Hello"), "l")); // "lo"
	p(skipUntilAfter(SC("Hello"), "")); // "Hello"
	p(skipUntilAfter(SC("Hello"), "llom")); // fail
	p(skipUntilAfter(SC("Hello"), "la")); // fail
	p(skipUntilAfter(SC("Hello"), "x")); // fail
	p(skipUntilAfter(SC(" #| |# 234"), "|#"));
}
