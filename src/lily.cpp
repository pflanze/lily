#include <unordered_map>
#include <sstream>
#include "lily.hpp"
#include "lilyConstruct.hpp" // oh well, why separate those then?
#include "lilyParse.hpp"
#include "lilyUtil.hpp"
#include <limits>

using namespace lily;

// XX weird wanted that to be fully abstract
LilyNumberPtr LilyNumber::multiply(const LilyNumberPtr& b) {XXX};
LilyNumberPtr LilyNumber::divideBy(const LilyNumberPtr& b) {XXX};
LilyNumberPtr LilyNumber::add(const LilyNumberPtr& b) {XXX};
LilyNumberPtr LilyNumber::subtract(const LilyNumberPtr& b) {XXX};
double LilyNumber::toDouble() {XXX};

void LilyNumber::write(std::ostream& out) {XXX};
const char* LilyNumber::typeName() { return "LilyNumber"; }
// /weird


LilyObjectPtr
LilyBoolean::True() {
	static LilyObjectPtr v (new LilyBoolean(true));
	return v;
}

LilyObjectPtr
LilyBoolean::False() {
	static LilyObjectPtr v (new LilyBoolean(false));
	return v;
}


LilyListPtr
LilyNull::singleton() {
	static LilyListPtr v (new LilyNull());
	return v;
}

LilyObjectPtr
LilyVoid::singleton() {
	static LilyObjectPtr v (new LilyVoid());
	return v;
}


// XX careful threads! danger?

// XX currently does not free unused symbols. Periodic sweep? Weak
// somehow, how? (Special table needed, or weak will wrapper.)

typedef std::unordered_map<std::string, LilySymbollikePtr> lilySymbollikeTable;

template <typename T>
static /* inline since only used once per type? But static should do the same, right? */
LilySymbollikePtr lilySymbollikeIntern(lilySymbollikeTable* t, std::string s,
				       bool mayNeedQuoting) {
	auto it= t->find(s);
	if (it != t->end()) {
		return it->second;
		// why 'is this a tuple and iterator at same time?'?
		// Overloaded dereference, right? (Uh?)
	} else {
		auto v= LILY_NEW(T,(s, siphash(s), false));
		if (mayNeedQuoting) {
			// check whether it would work unquoted:
			// simply try to parse its unquoted
			// representation back
			auto v1= lilyParse(show(v), true);
			LETU_AS(v1p, T, v1);
			if (!v1p || !(v1p->string() == s))
				v= LILY_NEW(T,(s, siphash(s), true));
		}
		// else (did not come with quotes) we already showed
		// that it doesn't need quoting.
		(*t)[s]= v;
		return v;
	}
}

static lilySymbollikeTable * lilySymbolTable;
LilySymbollikePtr
LilySymbol::intern(std::string s, bool nq) {
	return lilySymbollikeIntern<LilySymbol>(lilySymbolTable, s, nq);
}

static lilySymbollikeTable * lilyKeywordTable;
LilySymbollikePtr
LilyKeyword::intern(std::string s, bool nq) {
	return lilySymbollikeIntern<LilyKeyword>(lilyKeywordTable, s, nq);
}


LilyPair::~LilyPair() {};
//LilyVoid::~LilyVoid() {};
//LilyBoolean::~LilyBoolean() {};
LilyChar::~LilyChar() {};
LilyString::~LilyString() {};
LilySymbol::~LilySymbol() {};
LilyKeyword::~LilyKeyword() {};
LilyInt64::~LilyInt64() {};
LilyFractional64::~LilyFractional64() {};
LilyDouble::~LilyDouble() {};


std::string
LilyObject::onelineString() {
	std::ostringstream res;
	this->write(res);
	return res.str();
}

	
void
LilyList::write(std::ostream& out) {
	LilyList* p= this;
	out << "(";
	bool isNull= p->isNull();
	while (!isNull) {
		p->first()->write(out);
		p= LIST_UNWRAP(p->rest());
		isNull= p->isNull();
		if (isNull) {
			out << " ";
		}
	}
	out << ")";
}

// XX const? can we bring it into the program segment?
LilyObjectPtr lilySymbol_quote;
LilyObjectPtr lilySymbol_quasiquote;
LilyObjectPtr lilySymbol_unquote;

void lily_init() {
	lilySymbolTable= new lilySymbollikeTable();
	lilyKeywordTable= new lilySymbollikeTable();
	lilySymbol_quote= SYMBOL("quote", false);
	lilySymbol_quasiquote= SYMBOL("quasiquote", false);
	lilySymbol_unquote= SYMBOL("unquote", false);

}




void
LilyPair::write(std::ostream& out) {
	LETU_AS(cdr, LilyPair, _cdr);
	if (cdr && is_LilyNull(&*(cdr->_cdr))) {
		if (_car == lilySymbol_quote) { out << "'";  goto end_special; }
		else if (_car == lilySymbol_quasiquote) { out << "`";  goto end_special; }
		else if (_car == lilySymbol_unquote) { out << ",";  goto end_special; }
		else {goto otherwise; }
	end_special:
		cdr->_car->write(out);
		return;
	}
otherwise:
	// list or improper list
	LilyPair* p= this;
	out << "(";
	while (true) {
		p->_car->write(out);
		LilyObject* d= &(*(p->_cdr));
		if ((p= is_LilyPair(d))) {
			out << " ";
		} else if (is_LilyNull(d)) {
			break;
		} else {
			out << " . ";
			d->write(out);
			break;
		}
	}
	out << ")";
}

void
LilyVoid::write(std::ostream& out) {
	out << "#!void";
}

void
LilyBoolean::write(std::ostream& out) {
	out << (_value ? "#t" : "#f");
}

static
char unsafe_hexdigit (int d) {
	return (d < 10) ? (d + '0') : (d + 'a');
}

static
void writehex(std::ostream& out, uint32_t v, int numdigits) {
	for (int i=0; i< numdigits; i++) {
		out << unsafe_hexdigit((v >> ((numdigits - 1 - i) * 4)) & 15);
	}
}


void
LilyChar::write(std::ostream& out) {
	lily_char_t c= _char;
	out << "#\\";
	const char* maybeStr= lilyCharMaybeName(c);
	if (maybeStr) {
		out << maybeStr;
	} else if ((c >= 33) && (c <= 126)) {
		out << (char)_char; // XXX properly handle with locale based encoding lib
	} else if (c < (1<<(2*4))) {
		out << "x";
		writehex(out, c, 2);
	} else if (c < (1<<(4*4))) {
		out << "u";
		writehex(out, c, 4);
	} else {
		out << "U";
		writehex(out, c, 8);
	}
}

void
LilyString::write(std::ostream& out) {
	out << '"';
	string_onelinePrint(string, out, '"');
	out << '"';
}

void
LilySymbollike::write(std::ostream& out) {
	if (needsQuoting) {
		out << '|';
		string_onelinePrint(_string, out, '|');
		out << '|';
	} else {
		out << _string;
	}
#if 0
	out<< "[" << hash << "]";
#endif
}

void
LilyKeyword::write(std::ostream& out) {
	LilySymbollike::write(out);
	out << ":";
}

void
LilyInt64::write(std::ostream& out) {
	out << value;
}

void
LilyFractional64::write(std::ostream& out) {
	out << _numerator << "/" << _denominator;
}

void
LilyDouble::write(std::ostream& out) {
	/* Boost has boost::lexical_cast which is said to work for
	   choosing the correct number of digits, but don't want to
	   depend on that. This about works but not sure it's correct,
	   it is giving back different values than parsed, who is to
	   blame? */

	/* Have to check whether the output already contains a dot or
	   exponent, hence capture into a string first, meh. */
	std::ostringstream o;
	o.precision(std::numeric_limits<double>::max_digits10 - 2);
	// ^ XX "- 2": evil, hide the parsing (or precision?) problems under the carpet
	o << value;
	auto s= o.str();
	auto len= s.length();
	if ((len >= 3)
	    && (((s[0]=='i') && (s[1]=='n') && (s[2]=='f'))
		    ||
		((s[1]=='i') && (s[2]=='n')))) {
		out << ((s[0]=='-') ? "-inf.0" : "+inf.0");
	} else {
		auto e= s.find('e');
		bool has_e= !(e == std::string::npos);
		if (has_e) {
			e++;
			if ((e < len) && (s[e] == '+')) {
				s.replace(e, 1, "");
			}
		}
		out << s;
		if ((s.find('.') == std::string::npos) && !has_e)
			out << ".";
	}
}

// could output addresses, but then testing via stringification
// becomes non-deterministic
void
LilyNativeProcedure::write(std::ostream& out) {
	out << "#<native-procedure "<< _name <<">";
}

void
LilyNativeMacroexpander::write(std::ostream& out) {
	out << "#<native-macro-expander "<< _name <<">";
}

void
LilyNativeEvaluator::write(std::ostream& out) {
	out << "#<native-evaluator "<< _name <<">";
}

// and yeah, decided to make it a LilyObject, so now have to define
// the stuff that might be user accessed
void
LilyContinuationFrame::write(std::ostream& out) {
	bool deep=1;
	if (deep) {
		(out << "#<continuation-frame "
		 << (_maybeHead ? show(_maybeHead) : "NULL")
		 << " "
		 << show(_rvalues) // XX use write directly please..
		 << " "
		 << show(_expressions)
		 << ">");
	} else {
		(out << "#<continuation-frame 0x"
		 << std::hex
		 << _rvalues << " 0x"
		 << _expressions
		 << std::dec
		 << ">");
	}
}

void
LilyDivisionByZeroError::write(std::ostream& out) {
	(out
	 << "#<division-by-zero-error "
	 << show(STRING(_msg))
	 << ">");
}

void
LilyParseError::write(std::ostream& out) {
	(out
	 << "#<parse-error "
	 << show(STRING(_msg)) // for string formatting
	 << " "
	 << _pos // no need for formatting, ok?
	 << ">");
}


// XX include the exception type or not?

std::string LilyDivisionByZeroError::what() {
	return STR("division by zero: " << _msg );
}

std::string LilyParseError::what() {
	return STR("parse error: " <<_msg << " (position " << _pos << ")");
}



const char* LilyNull::typeName() {return "Null";}
const char* LilyVoid::typeName() {return "Void";}
const char* LilyPair::typeName() {return "Pair";}
const char* LilyBoolean::typeName() {return "Boolean";}
const char* LilyChar::typeName() {return "Char";}
const char* LilyString::typeName() {return "String";}
const char* LilySymbol::typeName() {return "Symbol";}
const char* LilyKeyword::typeName() {return "Keyword";}
const char* LilyInt64::typeName() {return "Int64";}
const char* LilyFractional64::typeName() {return "Fractional64";}
const char* LilyDouble::typeName() {return "Double";}
const char* LilyNativeProcedure::typeName() {return "NativeProcedure";}
const char* LilyNativeMacroexpander::typeName() {return "NativeMacroexpander";}
const char* LilyNativeEvaluator::typeName() {return "NativeEvaluator";}
const char* LilyContinuationFrame::typeName() {return "ContinuationFrame";}
const char* LilyDivisionByZeroError::typeName() {return "DivisionByZeroError";}
const char* LilyParseError::typeName() {return "ParseError";}


void throwOverflow(int64_t a, const char*op, int64_t b) {
	throw std::overflow_error(STR("int64 overflow: " << a << " " << op << " " << b));
}
void throwOverflow(const char*op, int64_t a) {
	throw std::overflow_error(STR("int64 overflow: " << op << " " << a));
}
void throwDivByZero(int64_t a, const char*op) {
	throw LilyDivisionByZeroError(STR("int64 division by zero: " << a << " " << op << " " << 0));
}



// can't use gcd from <numeric> as it doesn't check for number overflow

// Scheme code from scheme 48
// (define (gcd . integers)
//   (reduce (lambda (x y)
//             (cond ((< x 0) (gcd (- 0 x) y))
//                   ((< y 0) (gcd x (- 0 y)))
//                   ((< x y) (euclid y x))
//                   (else (euclid x y))))
//           0
//           integers))

// (define (euclid x y)
//   (if (= y 0)
//       (if (and (inexact? y)
//                (exact? x))
//           (exact->inexact x)
//           x)
//       (euclid y (remainder x y))))

// (define (lcm . integers)
//   (reduce (lambda (x y)
//             (let ((g (gcd x y)))
//               (cond ((= g 0) g)
//                     (else (* (quotient (abs x) g) (abs y))))))
//           1
//           integers))

int64_t lily_euclid(int64_t x, int64_t y) {
	if (y == 0)
		return x;
	else
		return lily_euclid(y, x % y);
}

int64_t _lily_gcd_positive(int64_t x, int64_t y) {
	if (x < y)
		return lily_euclid(y, x);
	else
		return lily_euclid(x, y);
}

int64_t lily_gcd(int64_t x, int64_t y) {
	return _lily_gcd_positive(lily_abs(x), lily_abs(y));
}

int64_t lily_lcm(int64_t x, int64_t y) {
	int64_t g= lily_gcd(x, y);
	if (g == 0)
		return g;
	else
		return lily_mul(lily_quotient(lily_abs(x), g), lily_abs(y));
}

LilyNumberPtr simplifiedFractional64(int64_t n, int64_t d) {
	int64_t f= lily_gcd(n, d);
	int64_t n1;
	int64_t d1;
	if (f > 1) {
		n1= n / f;
		d1= d / f;
		if (d1 == 1) {
			DEBUGWARN("simplifiedFractional64: from "<<n<<"/"<<d<<" to "<<n1);
			return std::shared_ptr<LilyNumber>
				(new LilyInt64(n1));
		}
	} else {
		n1= n;
		d1= d;
	}
	assert(!(d1 == 1));
	DEBUGWARN("simplifiedFractional64: from "<<n<<"/"<<d<<" to "<<n1<<"/"<<d1);
	return std::shared_ptr<LilyNumber>(new LilyFractional64(n1, d1));
}

LilyNumberPtr Divide(LilyInt64* a, LilyInt64* b) {
	int64_t bv= b->value;
	if (bv == 0)
		// even though the message is not actually going to be
		// used in the case of the parser
		throw LilyDivisionByZeroError(STR("Divide(" << a << ", " << b << ")"));
	if (bv == 1)
		// stupid since might just refcnt++ a, but then the API wouldn't work.
		return std::shared_ptr<LilyNumber>(new LilyInt64(bv));
	return simplifiedFractional64(a->value, bv);
}


LilyNumberPtr Add(LilyInt64* a, LilyFractional64* b) {
	// XX better algo less likely to hit max int?
	int64_t n= b->numerator();
	int64_t d= b->denominator();
	return simplifiedFractional64(lily_add(n, lily_mul(a->value, d)), d);
}

LilyNumberPtr Subtract(LilyInt64* a, LilyFractional64* b) {
	// XX better algo less likely to hit max int?
	int64_t n= b->numerator();
	int64_t d= b->denominator();
	return simplifiedFractional64(lily_sub(lily_mul(a->value, d), n),
				      d);
}

LilyNumberPtr Subtract(LilyFractional64* a, LilyInt64* b) {
	// XX better algo less likely to hit max int?
	int64_t n= a->numerator();
	int64_t d= a->denominator();
	return simplifiedFractional64(lily_sub(n, lily_mul(b->value, d)), d);
}

LilyNumberPtr Subtract(LilyFractional64* a, LilyFractional64* b) {
	// XX better algo less likely to hit max int?
	return simplifiedFractional64
		(lily_sub(lily_mul(a->numerator(), b->denominator()),
			  lily_mul(b->numerator(), a->denominator())),
		 lily_mul(a->denominator(), b->denominator()));
}

LilyNumberPtr Add(LilyFractional64* a, LilyFractional64* b) {
	// XX better algo less likely to hit max int?
	return simplifiedFractional64
		(lily_add(lily_mul(a->numerator(), b->denominator()),
			  lily_mul(b->numerator(), a->denominator())),
		 lily_mul(a->denominator(), b->denominator()));
}

LilyNumberPtr Multiply(LilyInt64* a, LilyFractional64* b) {
	// XX better algo less likely to hit max int?
	return simplifiedFractional64(lily_mul(b->numerator(), a->value),
				      b->denominator());
}

LilyNumberPtr Multiply(LilyFractional64* a, LilyFractional64* b) {
	// XX better algo less likely to hit max int?
	return simplifiedFractional64(lily_mul(a->numerator(), b->numerator()),
				      lily_mul(a->denominator(), b->denominator()));
}

LilyNumberPtr Divide(LilyInt64* a, LilyFractional64* b) {
	// XX better algo less likely to hit max int?
	return simplifiedFractional64(lily_mul(b->denominator(), a->value),
				      b->numerator());
}
LilyNumberPtr Divide(LilyFractional64* a, LilyInt64* b) {
	// XX better algo less likely to hit max int?
	return simplifiedFractional64(a->numerator(),
				      lily_mul(a->denominator(), b->value));
}
LilyNumberPtr Divide(LilyFractional64* a, LilyFractional64* b) {
	// XX better algo less likely to hit max int?
	return simplifiedFractional64(lily_mul(a->numerator(), b->denominator()),
				      lily_mul(a->denominator(), b->numerator()));
}



// XX test
double LilyInt64::toDouble() {
	return static_cast<double>(value);
}
double LilyFractional64::toDouble() {
	return static_cast<double>(_numerator)
		/ static_cast<double>(_denominator);
}
double LilyDouble::toDouble() {
	return value;
}

LilyNumberPtr LilyInt64::multiply(const LilyNumberPtr& b) {
	auto* b0= dynamic_cast<LilyInt64*>(&*b);
	if (b0) return Multiply(this, b0);
	auto b1= dynamic_cast<LilyFractional64*>(&*b);
	if (b1) return Multiply(this, b1);
	auto b3= dynamic_cast<LilyDouble*>(&*b);
	if (b3) return Multiply(this, b3);
	throw std::logic_error(STR("unimplemented number operation: multiply "
				   << show(this) << " " << show(b)));
}
LilyNumberPtr LilyInt64::divideBy(const LilyNumberPtr& b) {
	auto* b0= dynamic_cast<LilyInt64*>(&*b);
	if (b0) return Divide(this, b0);
	auto b1= dynamic_cast<LilyFractional64*>(&*b);
	if (b1) return Divide(this, b1);
	auto b3= dynamic_cast<LilyDouble*>(&*b);
	if (b3) return Divide(this, b3);
	throw std::logic_error(STR("unimplemented number operation: divide "
				   << show(this) << " " << show(b)));
}
LilyNumberPtr LilyInt64::add(const LilyNumberPtr& b) {
	auto* b0= dynamic_cast<LilyInt64*>(&*b);
	if (b0) return Add(this, b0);
	auto b1= dynamic_cast<LilyFractional64*>(&*b);
	if (b1) return Add(this, b1);
	auto b3= dynamic_cast<LilyDouble*>(&*b);
	if (b3) return Add(this, b3);
	throw std::logic_error(STR("unimplemented number operation: add "
				   << show(this) << " " << show(b)));
}
LilyNumberPtr LilyInt64::subtract(const LilyNumberPtr& b) {
	auto* b0= dynamic_cast<LilyInt64*>(&*b);
	if (b0) return Subtract(this, b0);
	auto b1= dynamic_cast<LilyFractional64*>(&*b);
	if (b1) return Subtract(this, b1);
	auto b3= dynamic_cast<LilyDouble*>(&*b);
	if (b3) return Subtract(this, b3);
	throw std::logic_error(STR("unimplemented number operation: subtract "
				   << show(this) << " " << show(b)));
}

// really copy-paste; XX templates possible?
LilyNumberPtr LilyFractional64::multiply(const LilyNumberPtr& b) {
	auto b0= dynamic_cast<LilyInt64*>(&*b);
	if (b0) return Multiply(this, b0);
	auto b1= dynamic_cast<LilyFractional64*>(&*b);
	if (b1) return Multiply(this, b1);
	auto b3= dynamic_cast<LilyDouble*>(&*b);
	if (b3) return Multiply(this, b3);
	throw std::logic_error(STR("unimplemented number operation: multiply "
				   << show(this) << " " << show(b)));
}
LilyNumberPtr LilyFractional64::divideBy(const LilyNumberPtr& b) {
	auto b0= dynamic_cast<LilyInt64*>(&*b);
	if (b0) return Divide(this, b0);
	auto b1= dynamic_cast<LilyFractional64*>(&*b);
	if (b1) return Divide(this, b1);
	auto b3= dynamic_cast<LilyDouble*>(&*b);
	if (b3) return Divide(this, b3);
	throw std::logic_error(STR("unimplemented number operation: divideBy "
				   << show(this) << " " << show(b)));
}
LilyNumberPtr LilyFractional64::add(const LilyNumberPtr& b) {
	auto b0= dynamic_cast<LilyInt64*>(&*b);
	if (b0) return Add(b0, this);
	auto b1= dynamic_cast<LilyFractional64*>(&*b);
	if (b1) return Add(this, b1);
	auto b3= dynamic_cast<LilyDouble*>(&*b);
	if (b3) return Add(this, b3);
	throw std::logic_error(STR("unimplemented number operation: add "
				   << show(this) << " " << show(b)));
}
LilyNumberPtr LilyFractional64::subtract(const LilyNumberPtr& b) {
	auto b0= dynamic_cast<LilyInt64*>(&*b);
	if (b0) return Subtract(this, b0);
	auto b1= dynamic_cast<LilyFractional64*>(&*b);
	if (b1) return Subtract(this, b1);
	auto b3= dynamic_cast<LilyDouble*>(&*b);
	if (b3) return Subtract(this, b3);
	throw std::logic_error(STR("unimplemented number operation: subtract "
				   << show(this) << " " << show(b)));
}


LilyNumberPtr LilyDouble::multiply(const LilyNumberPtr& b) {
	auto b0= dynamic_cast<LilyExact*>(&*b);
	if (b0) return Multiply(this, b0);
	auto b3= dynamic_cast<LilyDouble*>(&*b);
	if (b3) return Multiply(this, b3);
	throw std::logic_error(STR("unimplemented number operation: multiply "
				   << show(this) << " " << show(b)));
}
LilyNumberPtr LilyDouble::divideBy(const LilyNumberPtr& b) {
	auto b0= dynamic_cast<LilyExact*>(&*b);
	if (b0) return Divide(this, b0);
	auto b3= dynamic_cast<LilyDouble*>(&*b);
	if (b3) return Divide(this, b3);
	throw std::logic_error(STR("unimplemented number operation: divideBy "
				   << show(this) << " " << show(b)));
}
LilyNumberPtr LilyDouble::add(const LilyNumberPtr& b) {
	// can't flip the arguments (which would allow to use the
	// dispatch tree there) as the current api wants a Ptr as b,
	// and this is not a Ptr. XX solution?
	auto b0= dynamic_cast<LilyExact*>(&*b);
	if (b0) return Add(b0, this);
	auto b3= dynamic_cast<LilyDouble*>(&*b);
	if (b3) return Add(this, b3);
	throw std::logic_error(STR("unimplemented number operation: add "
				   << show(this) << " " << show(b)));
}
LilyNumberPtr LilyDouble::subtract(const LilyNumberPtr& b) {
	auto b0= dynamic_cast<LilyExact*>(&*b);
	if (b0) return Subtract(this, b0);
	auto b3= dynamic_cast<LilyDouble*>(&*b);
	if (b3) return Subtract(this, b3);
	throw std::logic_error(STR("unimplemented number operation: subtract "
				   << show(this) << " " << show(b)));
}



LilyObjectPtr
LilyNativeProcedure::call(LilyListPtr* args,
			  LilyListPtr* ctx,
			  LilyListPtr* cont) {
	DEBUGWARN("NativeProcedure: " << _name << show(*args));
	return _proc(args, ctx, cont);
}

LilyObjectPtr
LilyNativeMacroexpander::call(LilyListPtr* expressions,
			      LilyListPtr* ctx,
			      LilyListPtr* cont) {
	DEBUGWARN("NativeMacroexpander: " << _name << show(*expressions));
	return _expander(expressions, ctx, cont);
}

LilyObjectPtr
LilyNativeEvaluator::call(LilyListPtr* expressions,
			  LilyListPtr* ctx,
			  LilyListPtr* cont) {
	DEBUGWARN("NativeEvaluator: " << _name << show(*expressions));
	return _eval(expressions, ctx, cont);
}




// returns NULL on failure
LilyObjectPtr
alistMaybeGet (LilyListPtr l, LilyObjectPtr key) {
	while (! l->isNull()) {
		LETU_AS(p, LilyPair, l->first());
		if (p->car() == key)
			return p->cdr();
		l= l->rest();
	}
	return NULL;
}


LilyObjectPtr
apply1ary(const char* procname,
	  std::function<LilyObjectPtr(LilyObjectPtr)> proc,
	  LilyListPtr* vs) {
	LETU_AS(vs0, LilyPair, *vs);
	if (vs0) {
		LETU_AS(vs1, LilyNull, vs0->cdr());
		if (vs1) {
			return proc(vs0->car());
		}
	}
	throw std::logic_error(STR(procname << " needs 1 argument"));
}


// just casting...
static LilyListPtr frameExpressionsList(const LilyContinuationFramePtr& frame) {
	LET_AS(expressions, LilyList, frame->expressions());
	if (!expressions)
		throw std::logic_error("ill-formed special form");
	return expressions;
}

LilyObjectPtr eval(LilyObjectPtr code,
		   LilyListPtr ctx,
		   LilyListPtr cont) {
	LilyObjectPtr acc;
	while (true) {
	eval:
		DEBUGWARN("eval: "<< show(code) << " in: " << show(cont));
		switch (code->evalId) {
		case LilyEvalOpcode::Null:
			throw std::logic_error("empty call");
			break;
		case LilyEvalOpcode::Pair: {
			LETU_AS(p, LilyPair, code);
			// Function, macro or evaluator (base syntax)
			// application; the type of the head element
			// determines which kind. Currently, given
			// we're run-time phase only anyway, implement
			// as Fexpr, i.e. allow head to be a sub-form
			// that calculates the syntax to be used. Use
			// of this feature is completely *deprecated*
			// though, it *will* go away (since it is
			// prohibiting efficiency gains by separating
			// compilation from run-time phase).

			// So, in this implementation, make the
			// continuation of even syntactical work
			// visible to Scheme (via first-class
			// continuation access)
			cont= LIST_CONS(FRAME(NULL, NIL, p->rest()), cont);
			code= p->first();
			// need to look at code again, but don't have
			// acc to use from this iteration, hence short
			// it:
			goto eval;
		}
		case LilyEvalOpcode::Symbol:
			acc= alistMaybeGet(ctx, code);
			if (!acc)
				throw std::logic_error(STR("variable not bound: " << show(code)));
			break;

			// self-evaluating cases
		case LilyEvalOpcode::Void:
		case LilyEvalOpcode::Boolean:
		case LilyEvalOpcode::String:
		case LilyEvalOpcode::Keyword:
		case LilyEvalOpcode::Int64:
		case LilyEvalOpcode::Fractional64:
		case LilyEvalOpcode::Double:
		case LilyEvalOpcode::Char:
			acc= code;
			break;
			
			// invalid to evaluate (arbitrary vs. above?)
		case LilyEvalOpcode::NativeProcedure:
		case LilyEvalOpcode::ParseError:
		case LilyEvalOpcode::DivisionByZeroError:
			throw std::logic_error(STR("ill-formed expression: "
						   << show(code)));
		default:
			throw std::logic_error("bug: unknown opcode");
		}
	next_cont:
		// Who to pass the value to?
		if (cont->isNull()) {
			// pass it back to C++
			break;
		} else {
			// pass it to the Lily continuation (which is
			// two-level, a list of frames and then in
			// each frames, in the case of function
			// application, a list of values to be
			// calculated then called, in the case of
			// macros or base syntax the unevaluated
			// arguments are used hence different
			// continuation created.

			LET_AS(frame, LilyContinuationFrame, cont->first());
			cont= cont->rest();
			bool accIsHead= ! frame->maybeHead();
			if (accIsHead) {
				// acc contains the evaluated
				// head. Now we know whether it is a
				// function, macro or evaluator
				// application.
				LET_AS(evaluator, LilyNativeEvaluator, acc);
				if (evaluator) {
					auto expressions= frameExpressionsList(frame);
					acc= evaluator->_eval
						(&expressions, &ctx, &cont);
					// ^ ditto XX
					DEBUGWARN("evaluator returned: "<<show(acc)
					     <<", while cont="<<show(cont));
					// pass acc to cont
					goto next_cont;
				}
				LET_AS(expander, LilyMacroexpander, acc);
				if (expander) {
					auto expressions= frameExpressionsList(frame);
					// XX missing a reference to
					// the original surrounding
					// list here!
					code = expander->call
						(&expressions, &ctx, &cont);
					// ^ now C++ frame there.! XX
					goto eval;
				}
				// otherwise it's a function application;
				// pass acc to cont
			}
			// pass_to_cont:
			LilyObjectPtr head= accIsHead ? acc : frame->maybeHead();
			LET_AS(expressions, LilyList, frame->expressions());
			auto rvalues= accIsHead ? frame->rvalues()
				: LIST_CONS(acc, frame->rvalues());
			if (expressions->isNull()) {
				// ready to call the continuation
				DEBUGWARN("ready to call the continuation");
				LilyListPtr arguments= reverse(rvalues);
				LETU_AS(f, LilyCallable, head);
				if (!f)
					throw std::logic_error
						(STR("not a function: " <<
						     show(head)));
				acc= f->call(&arguments, &ctx, &cont);
				DEBUGWARN("after finishing the continuation frame, acc="
				     << show(acc));
				// what's next?
				goto next_cont;
			} else {
				// update continuation (XX optim:
				// mutate if refcount is 1 (and no
				// weak refs)?)
				cont= LIST_CONS(FRAME(head,
						      rvalues,
						      expressions->rest()),
						cont);
				code= expressions->first();
			}
		}
	}
	DEBUGWARN("eval is finished, returning " << show(acc));
	return acc;
}



// utils
LilyListPtr reverse(LilyObjectPtr l) {
	LilyListPtr res= NIL;
	while (true) {
		if (LilyPair*p= is_LilyPair(&*l)) {
			res= LIST_CONS(p->car(), res);
			l= p->cdr();
		} else if (is_LilyNull(&*l)) {
			break;
		} else {
			throw std::logic_error("improper list");
		}
	}
	return res;
}

std::string lily::show(const LilyObjectPtr& v) {
	std::ostringstream s;
	v->write(s);
	return s.str();
}

// XX completely stupid copy-paste; use templates?
std::string lily::show(LilyObject* v) {
	std::ostringstream s;
	v->write(s);
	return s.str();
}


void // noreturn;  use builtin excn values!
throwTypeError(const char* tname, LilyObjectPtr v) {
	// gcc is giving things like "10LilyDouble", sigh?
	while (isDigit(*tname))
		tname++;
	throw std::logic_error(STR("not a " << tname << ": "
				   << show(v)));
}

