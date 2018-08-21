#include <functional>
#include <assert.h>
#include "lily.hpp"
#include "lilyConstruct.hpp"
#include "lilyParse.hpp"
#include "lilyUtil.hpp"

enum class ParseResultCode : char {
	// the ones hard-coded in parse.cpp
	Success=0,
	UnexpectedEof=1,
	UnexpectedString=2,

	Whitespace, // parser found whitespace at point, not object
	MissingInput,
	Unimplemented,

	UnknownSyntax,
	UnknownBangSpecial,
	UnknownSpecial,
	NotAnInteger,
	Int64Overflow,
	NotASymbol,
	InvalidDottedList,
};

const char* ParseResultCode_string (ParseResultCode c) {
	switch (c) {
	case ParseResultCode::Success: return "Success";
	case ParseResultCode::UnexpectedEof: return "UnexpectedEof";
	case ParseResultCode::UnexpectedString: return "UnexpectedString";

	case ParseResultCode::Whitespace: return "Whitespace"; // shouldn't happen, meta
	case ParseResultCode::MissingInput: return "MissingInput";
	case ParseResultCode::Unimplemented: return "Unimplemented";

	case ParseResultCode::UnknownSyntax: return "UnknownSyntax";
	case ParseResultCode::UnknownBangSpecial: return "UnknownBangSpecial";
	case ParseResultCode::UnknownSpecial: return "UnknownSpecial";
	case ParseResultCode::NotAnInteger: return "NotAnInteger";
	case ParseResultCode::Int64Overflow: return "Int64Overflow";
	case ParseResultCode::NotASymbol: return "NotASymbol";
	case ParseResultCode::InvalidDottedList: return "InvalidDottedList";
	}
	// how can I make the compiler warn about missing cases but
	// otherwise be silent, without the throw ?
	throw std::logic_error("missing case");
}


bool needsSymbolQuoting (char c) {
	return !(isWordChar(c)
		 || (c == '!')
		 || (c == '?')
		 || (c == '.')
		 || (c == ':')
		 || (c == '/')
		 || (c == '%')
		 || (c == '$')
		 || (c == '-')
		 || (c == '+')
		 || (c == '*')
		 || (c == '_')
		 || (c == '&')
		 || (c == '=')
		 || (c == '<')
		 || (c == '>')
		 || (c == '#')
		);
}


static
S skipWhitespaceAndComments (S s) {
	s= skipWhitespaceOnly(s);
	if (s.isNull())
		return s;
	char c= s.first();
	if (c == ';') {
		return skipWhitespaceAndComments(skipUntilAfterEol(s));
	} else {
		auto rest= s.rest();
		if ((c == '#') && (! rest.isNull()) && (rest.first() == '|'))
			return skipWhitespaceAndComments
				(skipUntilAfter(rest.rest(), "|#"));
		else
			return s;
	}
}


typedef ParseResult<LilyObjectPtr> PR;

static
PR parseError(S s, ParseResultCode error) {
	return PR(VOID, s.setError(error));
}



// s is after '#'
static
PR parseHashitem(S s) {
	if (s.isNull())
		return parseError(s, ParseResultCode::UnexpectedEof);
	char c1= s.first();
	auto r= s.rest();
	if (c1 == '!') {
		// special object
		r= expectString(r, "void");
		if (r.success()) {
			if (isWordEndBoundary(r))
				return PR(VOID, r);
		}
		return parseError(r, ParseResultCode::UnknownBangSpecial);
	} else if (c1 == '|') {
		// skipWhitespaceAndComments should have eliminated
		// this case
		throw std::logic_error("bug");
	} else {
		if (isWordEndBoundary(r)) {
			if (c1 == 'f') {
				return PR(FALSE, r);
			} else if (c1 == 't') {
				return PR(TRUE, r);
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
PR parseStringLike(S s, char quoteChar,
			    std::function<LilyObjectPtr(const std::string&)>
			    constructor) {
	std::string str;
	while (true) {
		if (s.isNull())
			return parseError(s, ParseResultCode::UnexpectedEof);
		char c= s.first();
		s= s.rest();
		if (c == quoteChar)
			return PR(constructor(str), s);
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


PR parseInteger(S s) {
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
		if (isDigit(c)) {
			s=s.rest();
			auto d = c-'0';
			res= res*10;
			checkOverflow(); // this one required?
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
			return PR(INT(res), s);
	} else {
		return parseError(s, ParseResultCode::NotAnInteger);
	}
}

PR parseNumber(S s) {
	auto num= parseInteger(s);
	if (num.success())
		return num;
	// num= p.. XXX
	//throw std::logic_error("unfinished");
	return num;
}

PR parseSymbol(S s) {
	std::string str;
	while (true) {
		if (s.isNull())
			break;
		char c= s.first();
		if (!needsSymbolQuoting(c))
			str.push_back(c);
		else
			break;
		s=s.rest();
	}
	auto len= str.length();
	if ((len == 1) && str[0] == '.')
		return parseError(s, ParseResultCode::NotASymbol);
	else if ((len > 1) && ((str[0] == ':') || (str[len-1] == ':')))
		// keywords
		return parseError(s, ParseResultCode::Unimplemented);
	else
		return PR(SYMBOL(str), s);
}

// move to header file? but then also need to move the above classes.
PR lilyParse (S s);

// s is after the '('
PR parseList(S s) {
	s= skipWhitespaceAndComments(s);
	if (s.isNull())
		return parseError(s, ParseResultCode::UnexpectedEof);
	char c=s.first();
	if (c==')')
		return PR(NIL, s.rest());
	if (c=='.') {
		auto s1=s.rest();
		if (s1.isNull())
			return parseError(s1, ParseResultCode::UnexpectedEof);
		auto s2= skipWhitespaceAndComments(s1);
		if (s2.position() > s1.position()) {
			// dot stands by itself, i.e. dotted pair; expect 1 element then ")"
			auto r1= lilyParse(s2);
			if (!r1.success())
				return r1; // cutting away all the stored stuff. OK?
			auto s2= r1.remainder();
			s2= skipWhitespaceAndComments(s2);
			if (s2.isNull())
				return parseError(s2, ParseResultCode::UnexpectedEof);
			char c2= s2.first();
			if (c2 == ')')
				return PR(r1.value(), s2.rest());
			else
				return parseError(s2, ParseResultCode::InvalidDottedList);
		} else {
			// must be a symbol starting with a dot, right?  XX
			auto res= parseSymbol(s);
			if (!res.success()) {
				if (res.error() == ParseResultCode::NotASymbol)
					return parseError(s2, ParseResultCode::InvalidDottedList);
				else
					return res; // ditto cutting away
			}
			auto tail= parseList(res.remainder());
			if (!tail.success())
				return tail; // ditto, and, this is our 'failure monad'
			return PR(CONS(res.value(), tail.value()),
				  tail.remainder());
		}
	} else {
		// parse an item, then the remainder of the list
		PR res= lilyParse(s);
		if (!res.success())
			return res;
		auto tail= parseList(res.remainder());
		if (!tail.success())
			return tail;
		return PR(CONS(res.value(), tail.value()),
			  tail.remainder());
	}
}


static
LilyObjectPtr newString(const std::string& str) {
	return STRING(str);
}
static
LilyObjectPtr newSymbol(const std::string& str) {
	return SYMBOL(str);
}


PR lilyParse (S s) {
	s= skipWhitespaceAndComments (s);
	if (s.isNull())
		return parseError(s, ParseResultCode::MissingInput);
	char c= s.first();
	auto r= s.rest();
	if (c=='(') {
		return parseList(r);
	} else if (c=='#') {
		return parseHashitem(r);
	} else if (c=='"') {
		return parseStringLike(r, '"', newString);
	} else if (c=='|') {
		return parseStringLike(r, '|', newSymbol);
	} else if (c==';') {
		// until the end of the line; if s is 1 line then that
		// will be eof, but make it generic so actually check:
		return lilyParse(skipUntilAfterEol(r));
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
		// return parseError(s, ParseResultCode::UnknownSyntax);

		// actually return the symbol parsing error since
		// symbol is what it remains to be interpreted as
		// anyway?
		return v;
	}
}

// convenience function
LilyObjectPtr lilyParse (std::string s) {
	PR r= lilyParse(StringCursor(&s));
	if (r.success()) {
		return r.value();
	} else {
		// exception? error result? Or s-expression for
		// producing the error? :
		ParseResultCode code= r.error();
		return LIST(SYMBOL("parse-error"),
			    STRING(ParseResultCode_string(code)),
			    INT(r.remainder().position()));
	}
}
