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
	NotAFloatSuffix,
	NotAFloat,
	NotANumber,
	Int64Overflow,
	DivisionByZero,
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
	case ParseResultCode::NotAFloatSuffix: return "NotAFloatSuffix";
	case ParseResultCode::NotAFloat: return "NotAFloat";
	case ParseResultCode::NotANumber: return "NotANumber";
	case ParseResultCode::Int64Overflow: return "Int64Overflow";
	case ParseResultCode::DivisionByZero: return "DivisionByZero";
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

// all the character that will introduce another token after a number,
// or also symbol or keyword?
static bool isSeparation (char c) {
	return isWhitespace(c)
		|| needsSymbolQuoting(c);
}
static bool isSeparation (S s) {
	bool r= s.isNull() || isSeparation(s.first());
	WARN("isSeparation on " << show(s.string()) << " = " << r);
	return r;
}


static
S skipWhitespaceAndComments (Sm s) {
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


typedef const ParseResult<LilyObjectPtr> PR;
typedef ParseResult<LilyObjectPtr> PRm;

static
PR OK(LilyObjectPtr v, S s) {
	assert(s.success());
	return PR(v,s);
}

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
				return OK(VOID, r);
		}
		return parseError(r, ParseResultCode::UnknownBangSpecial);
	} else if (c1 == '|') {
		// skipWhitespaceAndComments should have eliminated
		// this case
		throw std::logic_error("bug");
	} else {
		if (isWordEndBoundary(r)) {
			if (c1 == 'f') {
				return OK(FALSE, r);
			} else if (c1 == 't') {
				return OK(TRUE, r);
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
PR parseStringLike(Sm s,
		   char quoteChar,
		   std::function<PR(const std::string&, S rest)>
		   cont) {
	std::string str;
	while (true) {
		if (s.isNull())
			return parseError(s, ParseResultCode::UnexpectedEof);
		char c= s.first();
		s= s.rest();
		if (c == quoteChar)
			return cont(str, s);
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


// A positive integer is unable to hold the most negative integer we
// could parse, hence not usable for both positive and negative cases;
// hence, assume negative number and return such; negate outside for
// the positive case. Does not check for boundary after number, this
// is left for the upper level: necessary since this is used as part
// of parseFloat, too.
PR parseNegativeInteger(Sm s) {
	if (s.isNull())
		return parseError(s, ParseResultCode::MissingInput);
	int64_t res=0;
	bool isnumber= false;
	bool isoverflow= false;
	while (true) {
		if (s.isNull())
			break;
		char c= s.first();
		if (isDigit(c)) {
			s=s.rest();
			if (!isoverflow) {
				auto d = c - '0';
				try {
					res= lily_sub(lily_mul(res, 10), d);
				} catch (std::overflow_error) {
					isoverflow= true;
				}
			}
			isnumber=true;
		} else {
			break;
		}
	}
	if (isnumber) {
		if (isoverflow)
			return parseError(s, ParseResultCode::Int64Overflow);
		else
			return OK(INT(res), s);
	} else {
		// length is not zero. isnumber is an odd (left-over)
		// way to do that perhaps.
		return parseError(s, ParseResultCode::NotAnInteger);
	}
}

PR parseNegate(PR pr) {
	if (!pr.success())
		return pr;
	LETU_AS(v, LilyInt64, pr.value());
	assert(v);
	try {
		return OK(INT(lily_negate(v->value)), pr.remainder());
	} catch (std::overflow_error) {
		return parseError(pr.remainder(), ParseResultCode::Int64Overflow);
	}
}

// CAREFUL: this is only in fact able to parse positive integers; for
// the general case use parseNegativeInteger!
PR parsePositiveInteger(S s) {
	return parseNegate(parseNegativeInteger(s));
}

// could have a leading + or -; does not verify boundary after the
// integer
PR parseInteger(S s) {
	if (s.isNull())
		return parseError(s, ParseResultCode::MissingInput);
	char c0= s.first();
	if (c0=='-') {
		return parseNegativeInteger(s.rest());
	} else if (c0=='+') {
		return parsePositiveInteger(s.rest());
	} else {
		return parsePositiveInteger(s);
	}
}

// R5RS:

// (decimal 10) -> (uinteger 10) (suffix)
//    | . (digit 10)+ #* (suffix)
//    | (digit 10)+ . (digit 10)* #* (suffix)
//    | (digit 10)+ #+ . #* (suffix)

// (uinteger R) -> (digit R)+ #*

// (suffix) -> (empty)
//    | (eponent marker) (sign) (digit 10)+

// (eponent marker) -> e | s | f | d | l

// (sign) -> (empty) | + | -


typedef std::pair<char,int64_t> suffixValue; // <exponent marker, exponent>
typedef ParseResult<suffixValue> PR_suffix;
static PR_suffix parseError_suffix(S s, ParseResultCode error) {
	return PR_suffix(suffixValue(0,0), s.setError(error));
}

PR_suffix parseFloat_suffix(S s) {
	if (s.isNull())
		return parseError_suffix(s, ParseResultCode::NotAFloatSuffix);
	char c0= s.first();
	switch (c0) {
	case 'e':
	case 's':
	case 'f':
	case 'd':
	case 'l': {
		PR exponent= parseInteger(s.rest());
		if (exponent.failed())
			// Note: need to pass along exponent.error()
			// so as to pass along overflow errors;
			// overflow must be 'sticky' in the error
			// path. (Automate?)
			return parseError_suffix(exponent.remainder(),
						 exponent.error());
		// if (isSeparation(exponent.remainder())) // XX or is this done outside?
		// 	return
		return PR_suffix
			(suffixValue(c0,
				     UNWRAP_AS(LilyInt64,
					       exponent.value())->value),
			 exponent.remainder());
	}
	default:
		return parseError_suffix(s, ParseResultCode::NotAFloatSuffix);
	}
}


// s is after the predot part; in the case of a leading dot, 0 is
// passed as predot value
PR parseFloat(int64_t predot, Sm s) {
	WARN("parseFloat: " << predot << ", " << show(s.string()));
	if (s.isNull())
		return parseError(s, ParseResultCode::NotAFloat);
	int64_t postdot= 0;
	bool hasDot= false;
	bool overflow= false;
	char c0= s.first();
	switch (c0) {
	case '.': {
		WARN("  has dot, " << show(s.rest().string()));
		hasDot= true;
		s= s.rest();
		PR _postdot= parsePositiveInteger(s);
		if (_postdot.success()) {
			postdot= XUNWRAP_AS(LilyInt64, _postdot.value())->value;
			s= _postdot.remainder();
			WARN("  got postdot, " << postdot << ", " << show(s.string()));
		} else {
			if (_postdot.error() == ParseResultCode::Int64Overflow) {
				overflow= true;
				// need to go on parsing, hence force
				// success; will then fail again at
				// the end via checking 'overflow'
				s= _postdot.remainder().setSuccess();
				WARN("  postdot but overflowed");
			} else {
				WARN("  no postdot");
			}
		}
		break;
	}
	}
	PR_suffix suffix= parseFloat_suffix(s);
	if (suffix.success()) {
		// we have a suffix
		char exponentMarker= suffix.value().first;
		int64_t exponent= suffix.value().second;
		return OK(STRING(STR("float with suffix: "
				     << predot
				     << " . "
				     << postdot
				     << " "
				     << exponentMarker
				     << " "
				     << exponent)),
			  suffix.remainder());
	} else {
		// there is no suffix
		WARN("  there is no suffix, hasDot="<<hasDot<<", ç");
		if (hasDot) {
			if (overflow)
				// it is following float syntax, but
				// error parsing it
				return parseError(s, ParseResultCode::Int64Overflow);
			else
				return OK(STRING(STR("float without suffix: "
						     << predot
						     << " . "
						     << postdot)),
					  s);
		} else {
			return parseError(s, ParseResultCode::NotAFloat);
		}
	}
}

PR parseNumber(S s) {
	PRm result= parseInteger(s);
	if (! result.success())
		return result;

	if (result.remainder().isNull())
		return result;

	{
		PR f= parseFloat(UNWRAP_AS(LilyInt64, result.value())->value,
				 result.remainder());
		if (f.success()) {
			WARN("  parseFloat returned success, " << show(f.value()));
			result= f;
			goto successsofar;
		}
		if (f.error() == ParseResultCode::Int64Overflow) {
			// syntax follows float, just couldn't hold in
			// words. Need to pass along failure. Here it
			// is actually fine to let result be an error
			// case.
			result= f;
			goto successsofar;
		}
		WARN("  parseFloat returned failure, " << ParseResultCode_string(f.error()));
	}

	{
		// XX make a parseFractional like parseFloat ?
		char c= result.remainder().first();
		DEBUGWARN("     c="<<c);
		switch (c) {
		case '/': {
			// 1/-3 should be parsed as a symbol, at least
			// Gambit does that, hence we have to do this
			auto s= result.remainder().rest();
			if (s.isNull())
				goto notanumber;
			auto num2= parsePositiveInteger(s);
			if (num2.success()) {
				// Oh, and we have to check again what
				// follows it. Evil syntax? Tokenizer
				// actually makes sense for *this*
				// reason. Should have proper boundary
				// detection functions to achieve the
				// same (todo).
				s= num2.remainder();
				if ((! s.isNull()) && (s.first() == '/'))
					goto notanumber;
				XLETU_AS(n, LilyInt64, result.value());
				XLETU_AS(d, LilyInt64, num2.value());
				try {
					// todo location keeping
					result= OK(Divide(n, d), s);
					goto successsofar;
				} catch (LilyDivisionByZeroError) {
					// XX ever report start, not end?
					goto notanumber;
				}
			} else {
				// returning num would be wrong now
				// that we know it would be valid as a
				// fractional up to this point? I
				// mean, we can and should fall back
				// to parsing as a symbol now. Weird
				// case?
				return num2;
			}
		}
		default:
			goto successsofar;
		}
	}
successsofar:
	// now there must be a proper separation after the number,
	// otherwise it's not one. (XX could there be cases where
	// other options should be tried? I.e. every option should be
	// followed by isSeparation check?)
	WARN("successsofar: checking remainder "<<show(result.remainder().string()));
	if (isSeparation(result.remainder()))
		return result;
notanumber:
	return parseError(s, ParseResultCode::NotANumber);
}

// unquoted (no '|' around it) symbols
PR parseSymbol(Sm s) {
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
	else if ((len > 1) && (str[len-1] == ':')) {
		str.pop_back();
		return OK(KEYWORD(str, false), s);
	}
	// check (str[0] == ':') for CL style keywords, give them another type?
	else
		return OK(SYMBOL(str, false), s);
}

// move to header file? but then also need to move the above classes.
PR lilyParse (S s);

// s is after the '('
PR parseList(Sm s) {
	s= skipWhitespaceAndComments(s);
	if (s.isNull())
		return parseError(s, ParseResultCode::UnexpectedEof);
	char c=s.first();
	if (c==')')
		return OK(NIL, s.rest());
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
				return OK(r1.value(), s2.rest());
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
			return OK(CONS(res.value(), tail.value()),
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
		return OK(CONS(res.value(), tail.value()),
			  tail.remainder());
	}
}


static
PR newString(const std::string& str, S rest) {
	return OK(STRING(str), rest);
}
static
PR newSymbolOrKeyword(const std::string& str, S rest) {
	if ((! rest.isNull()) && (rest.first() == ':'))
		return OK(KEYWORD(str, true), rest.rest());
	else
		return OK(SYMBOL(str, true), rest);
}


PR lilyParse (Sm s) {
	s= skipWhitespaceAndComments (s);
	if (s.isNull())
		return parseError(s, ParseResultCode::MissingInput);
	char c= s.first();
	auto s1= s.rest();
	LilyObjectPtr* special_symbol;
	if (c=='(') {
		return parseList(s1);
	} else if (c=='#') {
		return parseHashitem(s1);
	} else if (c=='"') {
		return parseStringLike(s1, '"', newString);
	} else if (c=='|') {
		return parseStringLike(s1, '|', newSymbolOrKeyword);
	} else if (c==';') {
		// until the end of the line; if s is 1 line then that
		// will be eof, but make it generic so actually check:
		return lilyParse(skipUntilAfterEol(s1));
	} else if (c=='\'') {
		special_symbol= &lilySymbol_quote; goto special;
	} else if (c=='`') {
		special_symbol= &lilySymbol_quasiquote; goto special;
	} else if (c==',') {
		special_symbol= &lilySymbol_unquote; goto special;
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
	throw std::logic_error("unreachable");
special:
	auto r= lilyParse(s1);
	return OK(CONS(*special_symbol, CONS(r.value(), NIL)),
		  r.remainder());
}

// convenience function
LilyObjectPtr lilyParse (std::string s, bool requireTotal) /* noexcept */ {
	PR r= lilyParse(StringCursor(&s));
	if (r.success()) {
		if (requireTotal) {
		    // check that there's nothing after the parsed
		    // expression
		    auto end= skipWhitespaceAndComments(r.remainder());
		    if (end.isNull())
			    return r.value();
		    else
			    return PARSEERROR("non-whitespace after expression",
					      end.position());
		} else {
			return r.value();
		}
	} else {
		// exception? error result? Or s-expression for
		// producing the error? :
		ParseResultCode code= r.error();
		return PARSEERROR(ParseResultCode_string(code),
				  r.remainder().position());
	}
}
