#include "parse.hpp"

bool isWhitespace (char c) {
	return ((c == ' ')
		|| (c == '\n')
		|| (c == '\r')
		|| (c == '\t')
		|| (c == '\f')
		);
}


S skipWhitespaceOnly (S s) {
	while (!s.isNull()) {
		if (!isWhitespace(s.first())) {
			return s;
		}
		s= s.rest();
	}
	return s;
}

// return the position after the end of this line, or eof (always
// successful)
S skipUntilAfterEol(S s) {
	while (!s.isNull()) {
		char c= s.first();
		s= s.rest();
		if (c == '\r') {
			if (s.isNull())
				return s; // odd though
			c= s.first();
			if (c == '\n')
				return s.rest();
			else
				return s;
		}
		if ((c == '\n')
		    // page feed without newline, XX correct?
		    || (c == '\f')) {
			return s;
		}
	}
	return s;
}

// return the position after the first occurrence of str, or eof error
// (naive algorithm)
S skipUntil (S s, const char* str, bool after) {
	while (!s.isNull()) {
		const char* _str= str;
		auto _s=s;
		while (true) {
			char _c= *_str;
			if (!_c)
				return after ? _s : s;
			if (_s.isNull())
				goto eof; // not break, since str wouldn't fit
			if (_c != _s.first())
				break;
			_s=_s.rest();
			_str++;
		}
		s= s.rest();
	}
eof:
	return s.setError(ParseResultCode_UnexpectedEof);
}

bool isWordEndBoundary(S s) {
	return (s.isNull()) || ! isWordChar(s.first());
}

S expectString(S s, const char* str) {
	const char* p= str;
	while (true) {
		if (!*p)
			return s;
		if (s.isNull())
			return s.setError(ParseResultCode_UnexpectedEof);
		if (!(s.first() == *p))
			return s.setError(ParseResultCode_UnexpectedString);
		s= s.rest();
		p++;
	}
}

