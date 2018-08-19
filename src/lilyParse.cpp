#include <assert.h>
#include <functional>
#include "lily.hpp"
#include "lilyConstruct.hpp"
#include "lilyParse.hpp"

#define WARN(e) std::cerr<< e <<"\n"

enum class ParseResultCode : char {
	Success = 0,
		MissingInput,
		UnexpectedEof,
		Unimplemented,
		UnknownSyntax,
		UnknownBangSpecial,
		UnknownSpecial,
		UnexpectedString,
		NotAnInteger,
		Int64Overflow,
		NotASymbol,
		InvalidDottedList,
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
	case ParseResultCode::NotAnInteger: return "NotAnInteger";
	case ParseResultCode::Int64Overflow: return "Int64Overflow";
	case ParseResultCode::NotASymbol: return "NotASymbol";
	case ParseResultCode::InvalidDottedList: return "InvalidDottedList";
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
		if (pos1 <= _string->length()) {
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
		return (_error==ParseResultCode::Success);
	}
	StringCursor setError (ParseResultCode error) {
		return StringCursor(_string, _position, error);
	}
	uint32_t position() {
		return _position;
	}
	std::string& string() {
		return *_string;
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
	return (s.isNull()) || !isWordChar(s.first());
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


ParseResult parseInteger(S s) {
	int64_t res=0;
	if (s.isNull())
		return parseError(s, ParseResultCode::MissingInput);
	char c0= s.first();
	bool isneg= false;
	bool isnumber= false;
	bool isoverflow= false;
	if (c0=='-') {
		isneg= true;
	} else if (c0=='+') {
		isneg= false;
	} else if (isDigit(c0)) {
		auto d = c0-'0';
		res= isneg ? -d : d;
		isnumber=true;
	} else {
		return parseError(s, ParseResultCode::NotAnInteger);
	}
	s=s.rest();
	auto checkOverflow= [&]() {
		if (isoverflow)
			return;
		if ((isneg && (res>0)) ||
		    ((!isneg) && (res<0)))
			isoverflow=true;
	};
	while (true) {
		if (s.isNull())
			break;
		char c= s.first();
		s=s.rest();
		if (isDigit(c)) {
			auto d = c-'0';
			res= res*10;
			// copypaste of below, XX is this required?
			checkOverflow();
			res= isneg ? res - d : res + d;
			checkOverflow();
			isnumber=true;
		} else {
			// equivalent to isWordEndBoundary, isNull
			// check already done
			if (isWordChar(c))
				isnumber=false;
			break;
		}
	}
	if (isnumber) {
		if (isoverflow)
			return parseError(s, ParseResultCode::Int64Overflow);
		else
			return ParseResult(INT(res), s);
	} else {
		return parseError(s, ParseResultCode::NotAnInteger);
	}
}

ParseResult parseNumber(S s) {
	auto num= parseInteger(s);
	if (num.success())
		return num;
	// num= p.. XXX
	//throw std::logic_error("unfinished");
	return num;
}
ParseResult parseSymbol(S s) {
	std::string str;
	while (true) {
		if (s.isNull())
			break;
		char c= s.first();
		if (doesNotNeedSymbolQuoting(c))
			str.push_back(c);
		else
			break;
		s=s.rest();
	}
	// XX also check that the last character isn't a : (keyword)
	if ((str.length() == 1) && str[0] == '.')
		return parseError(s, ParseResultCode::NotASymbol);
	else
		return ParseResult(SYMBOL(str), s);
}

ParseResult lilyParse (S s); // XX move to header file? but then also need to move the above classes.

// s is after the '('
ParseResult parseList(S s) {
	s= skipWhitespace(s);
	if (s.isNull())
		return parseError(s, ParseResultCode::UnexpectedEof);
	char c=s.first();
	if (c==')')
		return ParseResult(NULL, s);
	if (c=='.') {
		auto s1=s.rest();
		if (s1.isNull())
			return parseError(s1, ParseResultCode::UnexpectedEof);
		c=s1.first();
		// whitespace,  or " ( |  would all be valid after real dot. Otherwise symbol. XXUH
		if (isWhitespace(c)) {
			// dotted pair, expect 1 element then ")"
			auto r1= lilyParse(s1);
			if (!r1.success())
				return r1; // XX cutting away all the stored stuff. OK?
			auto s2= r1.remainder();
			s2= skipWhitespace(s2);
			if (s2.isNull())
				return parseError(s1, ParseResultCode::UnexpectedEof);
			char c2= s2.first();
			if (c2 == ')')
				return ParseResult(r1.value(), s2.rest());
			else
				return parseError(s2, ParseResultCode::InvalidDottedList);
		} else {
			// must be a symbol starting with a dot, right?  XX
			auto res= parseSymbol(s);
			if (!res.success())
				return res; // ditto cutting away
			auto tail= parseList(res.remainder());
			if (!tail.success())
				return tail; // ditto, and, this is our 'failure monad'
			return ParseResult(CONS(res.value(), tail.value()),
					   tail.remainder());
		}
	} else {
		// parse an item, then the remainder of the list
		auto res= lilyParse(s.rest());
		//XX from here copy-paste from above!
		if (!res.success())
			return res;
		auto tail= parseList(res.remainder());
		if (!tail.success())
			return tail;
		return ParseResult(CONS(res.value(), tail.value()),
				   tail.remainder());
	}
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
		// if it was a proper number but just overflowed,
		// return it as such:
		if (v.error() == ParseResultCode::Int64Overflow)
			return v;
		// not parseable as a number (even potentially
		// bignum), try symbol:
		v= parseSymbol(s);
		if (v.success())
			return v;
		return parseError(s, ParseResultCode::UnknownSyntax);
	}
}

// convenience function
LilyObjectPtr lilyParse (std::string s) {
	ParseResult r= lilyParse(StringCursor(&s, 0));
	if (r.success()) {
		return r.value();
	} else {
		// exception? error result? Or s-expression for
		// producing the error? :
		ParseResultCode code= r.error();
		return LIST(SYMBOL("parse-error"),
			    // INT(code),  XX how can I convert it?
			    STRING(ParseResultCode_string(code)),
			    // SYMBOL("pos:"),
			    INT(r.remainder().position()));
	}
}
