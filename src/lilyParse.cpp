#include <assert.h>
#include <functional>
#include "lily.hpp"
#include "lilyParse.hpp"
#include "lilyConstruct.hpp"


enum class ParseResultCode : char {
	Success = 0,
		MissingInput,
		UnexpectedEof,
		Unimplemented,
		UnknownSyntax,
		UnknownBangSpecial,
		UnknownSpecial,
		UnexpectedString,
};

const char* ParseResultCode_string (ParseResultCode c) {
	switch (c) {
	case ParseResultCode::Success: return "Success";
	case ParseResultCode::MissingInput: return "MissingInput";
	case ParseResultCode::UnexpectedEof: return "UnexpectedEof";
	case ParseResultCode::Unimplemented: return "Unimplemented";
	case ParseResultCode::UnknownSyntax: return "UnknownSyntax";
	case ParseResultCode::UnknownBangSpecial: return "UnknownBangSpecial";
	case ParseResultCode::UnknownSpecial: return "UnknownSpecial";
	case ParseResultCode::UnexpectedString: return "UnexpectedString";
	}
	// how can I make the compiler warn about missing cases but
	// otherwise be silent, without the throw ?
	throw std::logic_error("missing case");
}

// StringCursor can also represent parse errors (including the
// location of their appearance)

// XX this is optimized for 64 bit architectures.
class StringCursor {
public:
	StringCursor(std::string* string, uint32_t position,
		     ParseResultCode error= ParseResultCode::Success)
		: _string(string), _error(error), _position(position) {}
	char first () {
		return (*_string)[_position];
	}
	StringCursor rest () {
		uint32_t pos1= _position+1;
		assert(pos1 > 0);
		if (pos1 < _string->length()) {
			return StringCursor(_string, pos1, _error);
		} else {
			throw std::logic_error("end of input");
		}
	}
	bool isNull () {
		return !(_position < _string->length());
	}
	ParseResultCode error () {
		return _error;
	}
	bool success () {
		return !(_error==ParseResultCode::Success);
	}
	StringCursor setError (ParseResultCode error) {
		return StringCursor(_string, _position, error);
	}
	uint32_t position() {
		return _position;
	}
private:
	std::string* _string;
	ParseResultCode _error;
	uint32_t _position;
};

class ParseResult {
public:
	ParseResult(LilyObjectPtr result, StringCursor remainder)
		: _result(result), _remainder(remainder) {}
	LilyObjectPtr value() {
		return _result;
	}
	ParseResultCode error () {
		return _remainder.error();
	}
	bool success() {
		return _remainder.success();
	}
	StringCursor remainder() {
		return _remainder;
	}
private:
	LilyObjectPtr _result;	
	StringCursor _remainder;
};

typedef StringCursor S;

static
ParseResult parseError(S s, ParseResultCode error) {
	return ParseResult(VOID, s.setError(error));
}


// ----------------------------------------------------------------

static
bool isWhitespace (char c) {
	return ((c == ' ')
		|| (c == '\n')
		|| (c == '\r')
		|| (c == '\t')
		|| (c == '\f')
		);
}


static
S skipWhitespace (S s) {
	while (!s.isNull()) {
		if (!isWhitespace(s.first())) {
			return s;
		}
		s= s.rest();
	}
	return s;
}

// return the position after c, or eof (always successful)
// static
// S skipUntil (S s, char c) {
// 	while (!s.isNull()) {
// 		char c1= s.first();
// 		s= s.rest();
// 		if (c1 == c) {
// 			return s;
// 		}
// 	}
// 	return s;
// }

// return the position after the end of this line, or eof (always
// successful)
static
S skipUntilEol(S s) {
	while (!s.isNull()) {
		char c= s.first();
		s= s.rest();
		if (c == '\r') {
			if (s.isNull())
				return s; // odd though
			c= s.first();
			s= s.rest();
		}
		if ((c == '\n')
		    // page feed without newline, XX correct?
		    || (c == '\f')) {
			return s;
		}
	}
	return s;
}

static
bool isWordEndBoundary(S s) {
	return (s.isNull()) || !char_isword(s.first());
}

static
S expectString(S s, const char* str) {
	const char* p= str;
	while (true) {
		if (!*p)
			return s;
		if (s.isNull())
			return s.setError(ParseResultCode::UnexpectedEof);
		if (!(s.first() == *p))
			return s.setError(ParseResultCode::UnexpectedString);
		s= s.rest();
	}
}


// s is after '#'
static
ParseResult parseBooleanOrSpecial(S s) {
	if (s.isNull())
		return parseError(s, ParseResultCode::UnexpectedEof);
	char c1= s.first();
	auto r= s.rest();
	if (c1 == '!') {
		r= expectString(r, "void");
		if (r.success()) {
			if (isWordEndBoundary(r))
				return ParseResult(VOID, r);
		}
		return parseError(r, ParseResultCode::UnknownBangSpecial);
	} else {
		if (isWordEndBoundary(r)) {
			if (c1 == 'f') {
				return ParseResult(FALSE, r);
			} else if (c1 == 't') {
				return ParseResult(TRUE, r);
			} else {
				return parseError(s, ParseResultCode::UnknownSpecial);
			}
		} else {
			return parseError(r, ParseResultCode::UnknownSpecial);
		}
	}
}

// s is after '"' or '|'
static
ParseResult parseStringLike(S s, char quoteChar,
			    std::function<LilyObjectPtr(const std::string&)>
			    constructor) {
	std::string str;
	while (true) {
		if (s.isNull())
			return parseError(s, ParseResultCode::UnexpectedEof);
		char c= s.first();
		s= s.rest();
		if (c == quoteChar)
			return ParseResult(constructor(str), s);
		if (c == '\\') {
			if (s.isNull())
				return parseError(s, ParseResultCode::UnexpectedEof);
			c = s.first();
			s= s.rest();
			if (c=='n') {
				c='\n';
			} else if (c=='r') {
				c='\r';
			} else if (c=='t') {
				c='\t';
			} else if (c=='f') {
				c='\f';
			} else if (c=='0') {
				c='\0';
			}
			// XX handle lots of other cases?

			// else leave c as is
		}
		// XX handle unicode?
		str.push_back(c);
	}
}

ParseResult parseList(S s) {
	throw std::logic_error("unfinished");
}
ParseResult parseNumber(S s) {
	throw std::logic_error("unfinished");
}
ParseResult parseSymbol(S s) {
	throw std::logic_error("unfinished");
}

// XX move?
static
LilyObjectPtr newString(const std::string& str) {
	return STRING(str);
}
static
LilyObjectPtr newSymbol(const std::string& str) {
	return SYMBOL(str);
}


ParseResult lilyParse (S s) {
	s= skipWhitespace (s);
	if (s.isNull())
		return parseError(s, ParseResultCode::MissingInput);
	char c= s.first();
	auto r= s.rest();
	if (c=='(') {
		return parseList(r);
	} else if (c=='#') {
		// XX also handle ranged ("multi-line") comments
		return parseBooleanOrSpecial(r);
	} else if (c=='"') {
		return parseStringLike(r, '"', newString);
	} else if (c=='|') {
		return parseStringLike(r, '|', newSymbol);
	} else if (c==';') {
		// until the end of the line; if s is 1 line then that
		// will be eof, but make it generic so actually check:
		return lilyParse(skipUntilEol(r));
	} else if (c=='\'') {
		return parseError(s, ParseResultCode::Unimplemented);
	} else if (c=='`') {
		return parseError(s, ParseResultCode::Unimplemented);
	} else if (c==',') {
		return parseError(s, ParseResultCode::Unimplemented);
	} else {
		// attempt numbers, plain symbol (correct in that order?)
		auto v= parseNumber(s);
		if (v.success())
			return v;
		v= parseSymbol(s);
		if (v.success())
			return v;
		return parseError(s, ParseResultCode::UnknownSyntax);
	}
}

LilyObjectPtr lilyParse (std::string& s) {
	ParseResult r= lilyParse(StringCursor(&s, 0));
	if (r.success()) {
		return r.value();
	} else {
		// exception? error result? Or s-expression for
		// producing the error? :
		ParseResultCode code= r.error();
		return LIST(SYMBOL("error"),
			    STRING("parse error:"),
			    // INT(code),  XX how can I convert it?
			    STRING(ParseResultCode_string(code)),
			    SYMBOL("pos:"),
			    INT(r.remainder().position()));
	}
}