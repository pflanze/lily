#include <functional>
#include <assert.h>
#include "lily.hpp"
#include "lilyConstruct.hpp"
#include "lilyParse.hpp"
#include "lilyUtil.hpp"
#include "functional.hpp"

// man exp10
#ifndef _GNU_SOURCE
#  define _GNU_SOURCE
#endif
#include <math.h>


enum class ParseResultCode : char {
	// the ones hard-coded in parse.hpp
	Success=0,
	UnexpectedEof=1,
	UnexpectedString=2,

	Whitespace, // parser found whitespace at cursor, not object
	MissingInput,
	Unimplemented,

	UnknownSyntax,
	UnknownBangSpecial,
	UnknownSpecial,
	ImproperlyPlacedDot,
	NotAnInteger,
	NotAFloatSuffix,
	NotAFloat,
	NotANumber,
	Int64Overflow,
	DivisionByZero,
	NotASymbol,
	InvalidDottedList,
	InvalidCharname,
	InvalidHexdigit,
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
	case ParseResultCode::ImproperlyPlacedDot: return "ImproperlyPlacedDot";
	case ParseResultCode::NotAnInteger: return "NotAnInteger";
	case ParseResultCode::NotAFloatSuffix: return "NotAFloatSuffix";
	case ParseResultCode::NotAFloat: return "NotAFloat";
	case ParseResultCode::NotANumber: return "NotANumber";
	case ParseResultCode::Int64Overflow: return "Int64Overflow";
	case ParseResultCode::DivisionByZero: return "DivisionByZero";
	case ParseResultCode::NotASymbol: return "NotASymbol";
	case ParseResultCode::InvalidDottedList: return "InvalidDottedList";
	case ParseResultCode::InvalidCharname: return "InvalidCharname";
	case ParseResultCode::InvalidHexdigit: return "InvalidHexdigit";
	}
	// how can I make the compiler warn about missing cases but
	// otherwise be silent, without the throw ?
	throw std::logic_error("missing case");
}


bool needsSymbolQuoting (lily_char_t c) {
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


// (map (lambda (i) (list i (.char i))) (.. 0 260))

// regenerate lilyParse_name2char.hpp if changed!
const char* lilyCharMaybeName(lily_char_t c) {
	switch (c) {
	case 0: return "nul";
	case 7: return "alarm";
	case 8: return "backspace";
	case 9: return "tab";
	case 10: return "newline";
	case 11: return "vtab";
	case 12: return "page";
	case 13: return "return";
	case 27: return "esc";
	case 32: return "space";
	case 127: return "delete";
	default: return 0;
	}
}

#include "lilyParse_name2char.hpp"


// all the characters that will introduce another token after a
// number, or also symbol or keyword?
static bool isSeparation (lily_char_t c) {
	return isWhitespace(c)
		|| needsSymbolQuoting(c);
}
static bool isSeparation (S s) {
	bool r= s.isNull() || isSeparation(s.first());
	DEBUGWARN("isSeparation on " << show(s.string()) << " = " << r);
	return r;
}


static
S skipWhitespaceAndComments (Sm s) {
	s= skipWhitespaceOnly(s);
	if (s.isNull())
		return s;
	lily_char_t c= s.first();
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
	assert(s.succeeded());
	return PR(v,s);
}

static
PR ERR(ParseResultCode error, S s) {
	return PR(VOID, s.setError(error));
}



static
int hexdigit2int (lily_char_t c) {
	if ((c >= '0') && (c <= '9'))
		return c - '0';
	else if ((c >= 'a') && (c <= 'f'))
		return c - 'a' + 10;
	else if ((c >= 'A') && (c <= 'F'))
		return c - 'A' + 10;
	else
		return -1;
}

// read a hex-encoded LilyChar
static
PR parseCharInHex(Sm s, int lenToRead, int lenTillSeparator) {
	if (lenToRead != lenTillSeparator)
		// might be because of eof, though, ?
		return ERR(ParseResultCode::InvalidCharname, s);

	// assert(lenToRead <= 8);
	uint32_t c= 0;
	for (int i=0; i < lenToRead; i++) {
		// if (s.isNull())
		// 	return ERR(ParseResultCode::UnexpectedEof, s);
		// ^ can't happen since checked beforehand

		// use PR for hexdigit2int?... 'heavy'?
		auto d= hexdigit2int(s.first());
		WARN("hexdigit2int("<<s.first()<<")="<<d);
		if (d < 0)
			return ERR(ParseResultCode::InvalidHexdigit, s);
		c = (c << 4) + d;
		s= s.rest();
	}
	return OK(CHAR(c), s);
}


// s is after '#'
static
PR parseHashitem(S s) {
	if (s.isNull())
		return ERR(ParseResultCode::UnexpectedEof, s);
	char c1= s.first();
	auto r= s.rest();
	if (c1 == '!') {
		// special object
		r= expectString(r, "void");
		if (r.succeeded()) {
			if (isWordEndBoundary(r))
				return OK(VOID, r);
		}
		return ERR(ParseResultCode::UnknownBangSpecial, r);
	} else if (c1 == '|') {
		// skipWhitespaceAndComments should have eliminated
		// this case
		throw std::logic_error("bug");
	} else if (c1 == '\\') {
		// character
		if (r.isNull())
			return ERR(ParseResultCode::UnexpectedEof, s);
		auto r1= dropWhile(r.rest(),
				   COMPLEMENT(lily_char_t, isSeparation));
		auto len= r.positionDifferenceTo(r1);
		if (len > 1) {
			switch (r.first()) {
			case 'x':
				return parseCharInHex(r.rest(), 2, len-1);
			case 'u':
				return parseCharInHex(r.rest(), 4, len-1);
			case 'U':
				return parseCharInHex(r.rest(), 8, len-1);
			}
			// default: not a hex code
		}
		auto c= name2char(r, len);
		if (c == -1)
			return ERR(ParseResultCode::InvalidCharname, r1);
		return OK(CHAR(c), r1);
	} else {
		if (isWordEndBoundary(r)) {
			if (c1 == 'f') {
				return OK(FALSE, r);
			} else if (c1 == 't') {
				return OK(TRUE, r);
			} else {
				return ERR(ParseResultCode::UnknownSpecial, s);
			}
		} else {
			return ERR(ParseResultCode::UnknownSpecial, r);
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
			return ERR(ParseResultCode::UnexpectedEof, s);
		char c= s.first();
		s= s.rest();
		if (c == quoteChar)
			return cont(str, s);
		if (c == '\\') {
			if (s.isNull())
				return ERR(ParseResultCode::UnexpectedEof, s);
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
		return ERR(ParseResultCode::MissingInput, s);
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
			return ERR(ParseResultCode::Int64Overflow, s);
		else
			return OK(INT(res), s);
	} else {
		// length is not zero. isnumber is an odd (left-over)
		// way to do that perhaps.
		return ERR(ParseResultCode::NotAnInteger, s);
	}
}

PR parseNegate(PR pr) {
	if (!pr.succeeded())
		return pr;
	LETU_AS(v, LilyInt64, pr.value());
	assert(v);
	try {
		return OK(INT(lily_negate(v->value)), pr.remainder());
	} catch (std::overflow_error) {
		return ERR(ParseResultCode::Int64Overflow, pr.remainder());
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
		return ERR(ParseResultCode::MissingInput, s);
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


typedef std::pair<lily_char_t,int64_t> suffixValue; // <exponent marker, exponent>
typedef ParseResult<suffixValue> PR_suffix;
static PR_suffix ERR_suffix(S s, ParseResultCode error) {
	return PR_suffix(suffixValue(0,0), s.setError(error));
}

PR_suffix parseFloat_suffix(S s) {
	if (s.isNull())
		return ERR_suffix(s, ParseResultCode::NotAFloatSuffix);
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
			return ERR_suffix(exponent.remainder(),
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
		return ERR_suffix(s, ParseResultCode::NotAFloatSuffix);
	}
}


// s is after the predot part (including any possible sign); in the
// case of a leading dot, 0 is passed as predot value. If negate is
// true then the result needs to be negated (can also be false and
// predot already negative).
PR parseFloat(Sm s, bool negate, int64_t predot) {
	DEBUGWARN("parseFloat: " << predot << ", " << show(s.string()));
	if (s.isNull())
		return ERR(ParseResultCode::NotAFloat, s);
	int64_t postdot= 0;
	int32_t postdotLength= 0;
	// ^ signed so that we can negate it before passing to exp10
	bool hasDot= false;
	bool overflow= false;
	char c0= s.first();
	switch (c0) {
	case '.': {
		DEBUGWARN("  has dot, " << show(s.rest().string()));
		hasDot= true;
		s= s.rest();
		PR _postdot= parsePositiveInteger(s);
		if (_postdot.succeeded()) {
			postdot= XUNWRAP_AS(LilyInt64, _postdot.value())->value;
			auto p0= s.position();
			auto p1= _postdot.remainder().position();
			assert(p1 > p0);
			assert((p1 - p0) < 10000);
			// ^ should have returned with Int64Overflow
			//   long before that (can't have that many
			//   digits in an int64_t)
			postdotLength= p1 - p0;
			s= _postdot.remainder();
			DEBUGWARN("  got postdot, " << postdot << ", " << show(s.string()));
		} else {
			if (_postdot.error() == ParseResultCode::Int64Overflow) {
				overflow= true;
				// need to go on parsing, hence force
				// success; will then fail again at
				// the end via checking 'overflow'
				s= _postdot.remainder().setSucceeded();
				DEBUGWARN("  postdot but overflowed");
			} else {
				DEBUGWARN("  no postdot");
			}
		}
		break;
	}
	}

	double f= exp10(-postdotLength);
	auto predotabs= (predot < 0) ? -predot : predot;
	if (predot < 0)
		negate= !negate;
	DEBUGWARN("  f= "<<f<<", predot= "<<predot<<", predotabs="<<predotabs<<", negate="<<negate);

	PR_suffix suffix= parseFloat_suffix(s);
	if (suffix.succeeded()) {
		// we have a suffix
		//char exponentMarker= suffix.value().first;   XX verify
		int64_t exponent= suffix.value().second;

		double result=
			(static_cast<double>(predotabs)
			 + static_cast<double>(postdot)
			 * f)
			* exp10(exponent);
		DEBUGWARN("  have suffix. exponent="<<exponent<<", result="<<result);
		return OK(DOUBLE(negate ? -result : result),
			  suffix.remainder());
	} else {
		// there is no suffix
		if (hasDot) {
			if (overflow)
				// it is following float syntax, but
				// error parsing it
				return ERR(ParseResultCode::Int64Overflow, s);
			else {
				double result=
					static_cast<double>(predotabs)
					+ static_cast<double>(postdot)
					* f;
				DEBUGWARN("  no suffix. result="<<result);
				return OK(DOUBLE(negate ? -result : result),
					  s);
			}
		} else {
			return ERR(ParseResultCode::NotAFloat, s);
		}
	}
}

PR parseNumber(Sm s) {
	// if (s.isNull())
	// 	return ERR(ParseResultCode::MissingInput, s);
	//(^ automate such, please..; should never get here that way
	//   tho--so *where* is MissingInput useful?)

	bool isneg= false;
	char c0= s.first();
	if (c0=='-') {
		isneg= true;
		s= s.rest();
	} else if (c0=='+') {
		s= s.rest();
	}

	if (s.isNull())
		return ERR(ParseResultCode::NotANumber, s);

	PRm result;
	if (s.first() == '.') {
		// ugly? parseFloat is going to (and has to) analyze the dot again.
		result= parseFloat(s, isneg, 0);
		if (result.remainder().position() > s.rest().position())
			goto successsofar; // XX correct to do that in failure case? rename label!
		else
			// no digits left to the dot and none (or
			// suffix) right to it either: it's just a dot or
			// +. by itself
			return ERR(ParseResultCode::NotANumber, s);
	}

	result= isneg ? parseNegativeInteger(s) : parsePositiveInteger(s);
	if (result.error() == ParseResultCode::Int64Overflow)
		// not success of course, but that will check for
		// boundary, and the error is kept.
		goto successsofar;
	if (result.failed()) 
		return result;

	// parseInteger succeededç 
	if (result.remainder().isNull())
		// automatically a boundary
		return result;

	{
		PR f= parseFloat(result.remainder(),
				 false,
				 UNWRAP_AS(LilyInt64, result.value())->value);
		if (f.succeeded()) {
			DEBUGWARN("  parseFloat returned success, " << show(f.value()));
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
		DEBUGWARN("  parseFloat returned failure, " << ParseResultCode_string(f.error()));
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
			if (num2.succeeded()) {
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
	DEBUGWARN("successsofar: checking remainder "<<show(result.remainder().string()));
	if (isSeparation(result.remainder()))
		return result;
notanumber:
	return ERR(ParseResultCode::NotANumber, s);
}

// unquoted (no '|' around it) symbols or keywords
PR parseSymbolOrKeyword(Sm s) {
	std::string str;
	while (true) {
		if (s.isNull())
			break;
		lily_char_t c= s.first();
		if (!needsSymbolQuoting(c))
			str.push_back(c);
		else
			break;
		s=s.rest();
	}
	auto len= str.length();
	if ((len == 1) && str[0] == '.')
		//return ERR(ParseResultCode::NotASymbol, s);
		throw std::logic_error("bug, dot should already have been checked");
	else if ((len > 1) && (str[len-1] == ':')) {
		str.pop_back();
		return OK(KEYWORD(str, false), s);
	}
	// check (str[0] == ':') for CL style keywords, give them another type?
	else if (len >= 1)
		return OK(SYMBOL(str, false), s);
	else
		return ERR(ParseResultCode::NotASymbol, s);
}

// move to header file? but then also need to move the above classes.
PR lilyParse (S s);

// s is after the '('
PR parseList(Sm s) {
	s= skipWhitespaceAndComments(s);
	if (s.isNull())
		return ERR(ParseResultCode::UnexpectedEof, s);
	lily_char_t c=s.first();
	if (c==')')
		return OK(NIL, s.rest());
	// parse an item, then the remainder of the list; parsing the
	// dot the same way as other items except that it won't be a
	// success result; handle that case (the only place where it
	// isn't to be returned to the user)
	PR res= lilyParse(s);
	if (res.error() == ParseResultCode::ImproperlyPlacedDot) {
		// dotted pair; expect 1 element then ")"
		auto s0 = skipWhitespaceAndComments
			(res.remainder().setSucceeded());
		DEBUGWARN("parse remainder after dot: "<<show(s0.string()));
		if (s0.isNull())
			return ERR(ParseResultCode::UnexpectedEof, s0);
		if (s0.first() == ')')
			return ERR(ParseResultCode::InvalidDottedList, s0);
		auto r1= lilyParse(s0);
		if (r1.failed())
			return r1; // cutting away all the stored stuff. OK?
		auto s2= skipWhitespaceAndComments(r1.remainder());
		if (s2.isNull())
			return ERR(ParseResultCode::UnexpectedEof, s2);
		lily_char_t c2= s2.first();
		if (c2 == ')')
			return OK(r1.value(), s2.rest());
		else
			return ERR(ParseResultCode::InvalidDottedList, s2);
	} else {
		if (res.failed())
			return res;

		LETU_AS(sym, LilySymbol, res.value());
		DEBUGWARN("parseList: got a "<<show(res.value()->typeName())<<", "
		     << (sym ? show(sym->string()) : "not a symbol")
		     << ", now going to parse: "<<show(res.remainder().string()));
		auto tail= parseList(skipWhitespaceAndComments(res.remainder()));
		if (tail.failed())
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
		return ERR(ParseResultCode::MissingInput, s);
	lily_char_t c= s.first();
	auto s1= s.rest();
	LilyObjectPtr* special_symbol;
	if (c=='(') {
		return parseList(s1);
	} else if (c=='#') {
		return parseHashitem(s1);
	} else if (c=='"') {
		return parseStringLike(s1, '"', newString);
	} else if (c=='|') {
		DEBUGWARN("parseStringLike on: "<<show(s1.string())); 
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
		if ((c=='.') && isSeparation(s1))
			return ERR(ParseResultCode::ImproperlyPlacedDot, s1);

		// attempt numbers, plain symbol (correct in that order?)
		auto v= parseNumber(s);
		if (v.succeeded())
			return v;
		// if it was a proper number but just overflowed,
		// return it as such:
		if (v.error() == ParseResultCode::Int64Overflow)
			return v;
		// not parseable as a number (even potentially
		// bignum), try symbol:
		v= parseSymbolOrKeyword(s);
		if (v.succeeded())
			return v;
		// return ERR(ParseResultCode::UnknownSyntax, s);

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
	if (r.succeeded()) {
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
