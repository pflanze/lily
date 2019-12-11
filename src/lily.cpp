#include <unordered_map>
#include <sstream>
#include "lily.hpp"
#include "lilyConstruct.hpp" // oh well, why separate those then?
#include "lilyParse.hpp"
#include "lilyUtil.hpp"
#include <limits>

using namespace lilyConstruct;
using lily::show;


#define DEFINE_(Nam)							\
	std::string Lily##Nam::typeName() {return STR(STRINGIFY(Nam));}

LILY_DEFINE_FOR_ALL_OPCODES;
#undef DEFINE_


#if LILY_MEMORY_STATISTICS
uint64_t lily_allocation_count;
uint64_t lily_deallocation_count;

#else

#endif


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



// XX weird wanted that to be fully abstract
LilyNumberPtr LilyNumber::multiply(const LilyNumberPtr& b) {NOIMPLEMENTATION};
LilyNumberPtr LilyNumber::divideBy(const LilyNumberPtr& b) {NOIMPLEMENTATION};
LilyNumberPtr LilyNumber::add(const LilyNumberPtr& b) {NOIMPLEMENTATION};
LilyNumberPtr LilyNumber::subtract(const LilyNumberPtr& b) {NOIMPLEMENTATION};
double LilyNumber::toDouble() {NOIMPLEMENTATION};

void LilyNumber::write(std::ostream& out) {NOIMPLEMENTATION};
std::string LilyNumber::typeName() { return STR("LilyNumber"); }
// /weird


LilyBooleanPtr
LilyBoolean::True() {
	static LilyBooleanPtr v (new LilyBoolean(true));
	return v;
}

LilyBooleanPtr
LilyBoolean::False() {
	static LilyBooleanPtr v (new LilyBoolean(false));
	return v;
}


LilyNullPtr
LilyNull::singleton() {
	static LilyNullPtr v (new LilyNull());
	return v;
}

LilyVoidPtr
LilyVoid::singleton() {
	static LilyVoidPtr v (new LilyVoid());
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
				// v= LILY_NEW(T,(s, siphash(s), true));
				v->needsQuoting(true);
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


LilyPair::~LilyPair() noexcept {};
//LilyVoid::~LilyVoid() {};
//LilyBoolean::~LilyBoolean() {};
LilyChar::~LilyChar() noexcept {};
LilyString::~LilyString() noexcept {};
LilySymbol::~LilySymbol() noexcept {};
LilyKeyword::~LilyKeyword() noexcept {};
LilyInt64::~LilyInt64() noexcept {};
LilyFractional64::~LilyFractional64() noexcept {};
LilyDouble::~LilyDouble() noexcept {};

LilyCallable::~LilyCallable() noexcept {};
LilyNativeEvaluator::~LilyNativeEvaluator() noexcept {};
LilyContinuationFrame::~LilyContinuationFrame() noexcept {};
LilyNativeProcedure::~LilyNativeProcedure() noexcept {};
// ::~() noexcept {};
// ::~() noexcept {};


void
LilyNull::write(std::ostream& out) {
	out << "()";
}



// XX const? can we bring them into the program segment?
#define _DEFINE_(nam) LilyObjectPtr lilySymbol_##nam;
LILY_DEFINE_FOR_ALL_SPECIAL_SYMBOLS
#undef _DEFINE_

void lily::init() {
	lilySymbolTable= new lilySymbollikeTable();
	lilyKeywordTable= new lilySymbollikeTable();

#define _DEFINE_(nam) lilySymbol_##nam= SYMBOL(STRINGIFY(nam), false);
	LILY_DEFINE_FOR_ALL_SPECIAL_SYMBOLS
#undef _DEFINE_
}


void
LilyPair::write(std::ostream& out) {
	LETU_AS(cdr, LilyPair, _cdr);
	if (cdr && is_LilyNull(&*(cdr->_cdr))) {
		const char* q;
		if (_car == lilySymbol_quote)
			q= "'";
		else if (_car == lilySymbol_quasiquote)
			q= "`";
		else if (_car == lilySymbol_unquote)
			q= ",";
		else
			goto notspecialsyntax;
		out << q;
		cdr->_car->write(out);
		return;
	}
notspecialsyntax:
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
	return (d < 10) ? (d + '0') : (d - 10 + 'a');
}

static
void writehex(std::ostream& out, uint32_t v, int numdigits) {
	for (int i=0; i< numdigits; i++) {
		out << unsafe_hexdigit((v >> ((numdigits - 1 - i) * 4)) & 15);
	}
}


void
LilyChar::write(std::ostream& out) {
	lily_char_t c= _value;
	out << "#\\";
	const char* maybeStr= lilyCharMaybeName(c);
	if (maybeStr) {
		out << maybeStr;
	} else if ((c >= 33) && (c <= 126)) {
		out << (char)c; // XXX properly handle with locale based encoding lib
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


void lily::write(const std::string& str, std::ostream& out) {
	stringlike_write(str, out, '"');
}

void
LilyString::write(std::ostream& out) {
	lily::write(_value, out);
}

void
LilySymbollike::write(std::ostream& out) {
	if (_needsQuoting) {
		stringlike_write(_string, out, '|');
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
	out << std::dec << _value;
}

void
LilyFractional64::write(std::ostream& out) {
	out << _enumerator << "/" << _denominator;
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
	o << _value;
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
		out << "#<continuation-frame ";
		if (_maybeHead)
			_maybeHead->write(out);
		else
			out << "NULL";
		out << " ";
		_rvalues->write(out);
		out << " ";
		_expressions->write(out);
		out << ">";
	} else {
		out << "#<continuation-frame 0x"
		    << std::hex
		    << _rvalues << " 0x"
		    << _expressions
		    << std::dec
		    << ">";
	}
}

#define DEFINE_(kind,nam)					\
	void							\
	Lily##kind##Error::write(std::ostream& out) {		\
		out << "#<" nam "-error ";			\
		auto rem= CONS(INT(_b), NIL);			\
		CONS(SYMBOL(_op),				\
		     _unary ? rem : CONS(INT(_a), rem))		\
			->write(out);				\
		out << ">";					\
	}
DEFINE_(Int64Overflow, "int64-overflow");
DEFINE_(Int64Underflow, "int64-underflow");
// XX could this one have been done via 
#undef DEFINE_


void
LilyDivisionByZeroError::write(std::ostream& out) {
	out << "#<division-by-zero-error ";
	lily::write(_msg, out);
	out << ">";
}

void
LilyParseError::write(std::ostream& out) {
	out << "#<parse-error ";
	lily::write(_msg, out);
	out << " "
	    << _pos // no need for formatting, ok?
	    << ">";
}


LilyObjectPtr LilyNull::toCode(LilyObjectPtr self) {
	return LIST(lilySymbol_quote, self);
}
LilyObjectPtr LilyPair::toCode(LilyObjectPtr self) {
	if (lily::isList(self)) {
		return CONS(SYMBOL("list"),
			    lily::map(lily::toCode,
				      XAS<LilyList>(self)));
	} else if (UNWRAP_AS(LilyPair, _cdr)) {
		return CONS(SYMBOL("improper-list"),
			    lily::improper_to_proper_map(
				    lily::toCode,
				    self));
	} else {
		return LIST(lilySymbol_cons,
			    lily::toCode(_car),
			    lily::toCode(_cdr));
	}
	return NIL;
}
LilyObjectPtr LilyVoid::toCode(LilyObjectPtr self) {
	return LIST(lilySymbol_void);
}
LilyObjectPtr LilyBoolean::toCode(LilyObjectPtr self) {
	return self;
}
LilyObjectPtr LilyChar::toCode(LilyObjectPtr self) {
	return self;
}
LilyObjectPtr LilyString::toCode(LilyObjectPtr self) {
	return self;
}
LilyObjectPtr LilySymbollike::toCode(LilyObjectPtr self) {
	return LIST(lilySymbol_quote, self);
}
LilyObjectPtr LilyKeyword::toCode(LilyObjectPtr self) {
	return self;
}
LilyObjectPtr LilyNumber::toCode(LilyObjectPtr self) {
	return self;
}
LilyObjectPtr LilyInt64::toCode(LilyObjectPtr self) {
	return self;
}
LilyObjectPtr LilyFractional64::toCode(LilyObjectPtr self) {
	return self;
}
LilyObjectPtr LilyDouble::toCode(LilyObjectPtr self) {
	return self;
}
LilyObjectPtr LilyNativeProcedure::toCode(LilyObjectPtr self) {
	// XX assumes it's defined in the 'toplevel', etc., but that's
	// a problem not checked for in all the other cases, too.
	return SYMBOL(_name);
}
LilyObjectPtr LilyNativeMacroexpander::toCode(LilyObjectPtr self) {
	UNIMPLEMENTED("LilyNativeMacroexpander::toCode");
}
LilyObjectPtr LilyNativeEvaluator::toCode(LilyObjectPtr self) {
	UNIMPLEMENTED("LilyNativeEvaluator::toCode");
}
LilyObjectPtr LilyContinuationFrame::toCode(LilyObjectPtr self) {
	UNIMPLEMENTED("LilyContinuationFrame::toCode");
}
// right level?
LilyObjectPtr LilyErrorWithWhat::toCode(LilyObjectPtr self) {
	UNIMPLEMENTED("LilyErrorWithWhat::toCode"); // make mixin for _a _op _b _unary class? needs friend method accesses
}
LilyObjectPtr LilyDivisionByZeroError::toCode(LilyObjectPtr self) {
	return LIST(SYMBOL("division-by-zero-error"), STRING(_msg));
}
LilyObjectPtr LilyParseError::toCode(LilyObjectPtr self) {
	return LIST(SYMBOL("parse-error"), STRING(_msg), INT(_pos));
}


// XX include the exception type or not?

#define DEFINE_(kind)							\
	std::string Lily##kind##Error::what() {			\
		/* XX this, unlike the others, formats using Scheme syntax; */ \
		/* bad or good? */					\
		std::ostringstream o;					\
		write(o);						\
		return o.str();						\
	}
DEFINE_(Int64Overflow);
DEFINE_(Int64Underflow);
#undef DEFINE_



std::string LilyDivisionByZeroError::what() {
	return STR("division by zero: " << _msg );
}

std::string LilyParseError::what() {
	return STR("parse error: " <<_msg << " (position " << _pos << ")");
}

std::string LilyForeignBase::what() {
	// XX is this OK? Don't want to generate more code to produce
	// messages, probably unused anyway (these are *not* exception
	// objects)?
	std::ostringstream out;
	write(out);
	return out.str();
}



LilyObjectPtr LilyForeignPointerBase::toCode(LilyObjectPtr self) {
	return LIST(SYMBOL(STR(tName()<<'*')),
		    INT(valuep_as_uint()));
}
std::string LilyForeignPointerBase::typeName() {
	// this is the full type name, not just T
	return STR("ForeignPointer<" <<
		   tName()
		   << ">");
}
void LilyForeignPointerBase::write(std::ostream& out) {
	out << "#<foreign-pointer "
	    << tName();
	out << " 0x" << std::hex << valuep_as_uint() << std::dec;
	out << ">";
}



LilyObjectPtr LilyForeignValueBase::toCode(LilyObjectPtr self) {
	UNIMPLEMENTED("LilyForeignValueBase::toCode");
}
std::string LilyForeignValueBase::typeName() {
	// this is the full type name, not just T
	return STR("ForeignValue<" <<
		   tName()
		   << ">");
}
void LilyForeignValueBase::write(std::ostream& out) {
	out << "#<foreign-value "
	    << tName();
	out << " (hidden)"; // ?
	out << ">";
}




#define DEFINE_(throwOverflow, LilyInt64OverflowError, overflow)	\
	/* these are also std::overflow_error errors XX correct? */	\
	void throwOverflow(int64_t a, const char* op, int64_t b) {	\
		throw LilyInt64OverflowError(a, op, b);			\
		(STR("int64 " overflow ": " << a << " " << op << " " << b)); \
	}								\
	void throwOverflow(const char* op, int64_t a) {			\
		throw LilyInt64OverflowError(op, a);			\
		(STR("int64 " overflow ": " << op << " " << a));	\
	}
DEFINE_(throwOverflow, LilyInt64OverflowError, "overflow");
DEFINE_(throwUnderflow, LilyInt64UnderflowError, "underflow")
#undef DEFINE_

void throwDivByZero(int64_t a, const char* op) {
	throw LilyDivisionByZeroError
		(STR("int64 division by zero: " << a << " " << op << " " << 0));
}



// Can't use gcd from <numeric> as it doesn't check for number
// overflow. Originally translated from Scheme code from s48.

static
int64_t lily_euclid(int64_t x, int64_t y) {
	if (y == 0)
		return x;
	else
		return lily_euclid(y, x % y);
}

static
int64_t _lily_gcd_positive(int64_t x, int64_t y) {
	if (x < y)
		return lily_euclid(y, x);
	else
		return lily_euclid(x, y);
}

int64_t lily_gcd(int64_t x, int64_t y) {
	return _lily_gcd_positive(lily_abs(x), lily_abs(y));
}

// static
// int64_t lily_lcm(int64_t x, int64_t y) {
// 	int64_t g= lily_gcd(x, y);
// 	if (g == 0)
// 		return g;
// 	else
// 		return lily_mul(lily_quotient(lily_abs(x), g), lily_abs(y));
// }

// really just Divide(int64_t,int64_t)
static
LilyNumberPtr simplifiedFractional64(int64_t n, int64_t d) {
	if (d == 0)
		// even though the message is not actually going to be
		// used in the case of the parser.  XX just include
		// the arguments in the obj, not the string
		throw LilyDivisionByZeroError
			(STR("Divide(" << n << ", " << d << ")"));

	if (d == 1)
		// stupid in some cases since might just refcnt++ a,
		// but then the API wouldn't work.
		return std::shared_ptr<LilyNumber>(new LilyInt64(n));

	if (d == -1)
		return std::shared_ptr<LilyNumber>
			(new LilyInt64(lily_negate(n)));

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
	DEBUGWARN("simplifiedFractional64: from "<<n<<"/"<<d<<" to "<<n1<<"/"<<d1);
	return std::shared_ptr<LilyNumber>(new LilyFractional64(n1, d1));
}

LilyNumberPtr Divide(LilyInt64* a, LilyInt64* b) {
	return simplifiedFractional64(a->value(), b->value());
}


LilyNumberPtr Add(LilyInt64* a, LilyFractional64* b) {
	// XX better algo less likely to hit max int?
	int64_t n= b->enumerator();
	int64_t d= b->denominator();
	return simplifiedFractional64(lily_add(n, lily_mul(a->value(), d)),
				      d);
}

LilyNumberPtr Subtract(LilyInt64* a, LilyFractional64* b) {
	// XX better algo less likely to hit max int?
	int64_t n= b->enumerator();
	int64_t d= b->denominator();
	return simplifiedFractional64(lily_sub(lily_mul(a->value(), d), n),
				      d);
}

LilyNumberPtr Subtract(LilyFractional64* a, LilyInt64* b) {
	// XX better algo less likely to hit max int?
	int64_t n= a->enumerator();
	int64_t d= a->denominator();
	return simplifiedFractional64(lily_sub(n, lily_mul(b->value(), d)),
				      d);
}

LilyNumberPtr Subtract(LilyFractional64* a, LilyFractional64* b) {
	// XX better algo less likely to hit max int?
	return simplifiedFractional64
		(lily_sub(lily_mul(a->enumerator(), b->denominator()),
			  lily_mul(b->enumerator(), a->denominator())),
		 lily_mul(a->denominator(), b->denominator()));
}

LilyNumberPtr Add(LilyFractional64* a, LilyFractional64* b) {
	// XX better algo less likely to hit max int?
	return simplifiedFractional64
		(lily_add(lily_mul(a->enumerator(), b->denominator()),
			  lily_mul(b->enumerator(), a->denominator())),
		 OVERFLOW2UNDERFLOW64
		 (lily_mul(a->denominator(), b->denominator())));
}

LilyNumberPtr Multiply(LilyInt64* a, LilyFractional64* b) {
	// XX better algo less likely to hit max int?
	return simplifiedFractional64(lily_mul(b->enumerator(),
					       a->value()),
				      b->denominator());
}

LilyNumberPtr Multiply(LilyFractional64* a, LilyFractional64* b) {
	// XX better algo less likely to hit max int?
	return simplifiedFractional64(lily_mul(a->enumerator(),
					       b->enumerator()),
				      OVERFLOW2UNDERFLOW64 // correct?
				      (lily_mul(a->denominator(),
						b->denominator())));
}

LilyNumberPtr Divide(LilyInt64* a, LilyFractional64* b) {
	// XX better algo less likely to hit max int?
	return simplifiedFractional64(lily_mul(b->denominator(),
					       a->value()),
				      b->enumerator());
}
LilyNumberPtr Divide(LilyFractional64* a, LilyInt64* b) {
	// XX better algo less likely to hit max int?
	return simplifiedFractional64(a->enumerator(),
				      OVERFLOW2UNDERFLOW64
				      (lily_mul(a->denominator(),
						b->value())));
}
LilyNumberPtr Divide(LilyFractional64* a, LilyFractional64* b) {
	// XX better algo less likely to hit max int?
	return simplifiedFractional64(lily_mul(a->enumerator(),
					       b->denominator()),
				      OVERFLOW2UNDERFLOW64 // correct?
				      (lily_mul(a->denominator(),
						b->enumerator())));
}



// XX test
double LilyInt64::toDouble() {
	return static_cast<double>(_value);
}
double LilyFractional64::toDouble() {
	return static_cast<double>(_enumerator)
		/ static_cast<double>(_denominator);
}
double LilyDouble::toDouble() {
	return _value;
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




// careful: returns NULL (not NIL) on failure
static
LilyObjectPtr
alistMaybeRef (LilyListPtr l, LilyObjectPtr key) {
	while (! l->isNull()) {
		LETU_AS(p, LilyPair, l->first()); // safe as l is a LilyListPtr
		if (p->car() == key)
			return p->cdr();
		l= l->rest(); // throws exception if improper list
	}
	return NULL;
}


LilyObjectPtr
lily::apply1ary(const char* procname,
		std::function<LilyObjectPtr(LilyObjectPtr)> proc,
		LilyListPtr* vs) {
	IF_LETU_AS(vs0, LilyPair, *vs) {
		IF_LETU_AS(vs1, LilyNull, vs0->cdr()) {
			return proc(vs0->car());
		}
	}
	throw std::logic_error(STR(procname << " needs 1 argument"));
}


LilyObjectPtr
lily::eval(LilyObjectPtr code,
	   LilyListPtr ctx,
	   LilyListPtr cont) {
	LilyObjectPtr acc;
	while (true) {
	eval:
		DEBUGWARN("eval: "<< show(code) << " in: " << show(cont));
		switch (code->evalId) {
		case LilyEvalOpcode::Null:
			// XX don't mis-use logic_error for this (and
			// other cases)
			throw std::logic_error("empty call");
			break;
		case LilyEvalOpcode::Pair: {
			LETU_AS(p, LilyPair, code); // safe as just checked opcode
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
			cont= CONS(FRAME(NULL, NIL, p->rest()), cont);
			code= p->first();
			// need to look at code again, but don't have
			// acc to use from this iteration, hence short
			// it:
			goto eval;
		}
		case LilyEvalOpcode::Symbol:
			acc= alistMaybeRef(ctx, code);
			if (!acc)
				throw std::logic_error
					(STR("variable not bound: "
					     << show(code)));
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
		case LilyEvalOpcode::Int64OverflowError:
		case LilyEvalOpcode::Int64UnderflowError:
		case LilyEvalOpcode::DivisionByZeroError:
		case LilyEvalOpcode::ContinuationFrame:
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

			XLET_AS(frame, LilyContinuationFrame, cont->first());
			cont= cont->rest();
			bool accIsHead= ! frame->maybeHead();
			if (accIsHead) {
				// acc contains the evaluated
				// head. Now we can check whether it
				// is a function, macro or evaluator
				// application.
				IF_LET_AS(evaluator, LilyNativeEvaluator, acc) {
					auto expressions= frame->expressions();
					acc= evaluator->_eval
						(&expressions, &ctx, &cont);
					// ^ ditto XX
					DEBUGWARN("evaluator returned: "<<show(acc)
					     <<", while cont="<<show(cont));
					// pass acc to cont
					goto next_cont;
				}
				IF_LET_AS(expander, LilyMacroexpander, acc) {
					auto expressions= frame->expressions();
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
			XLET_AS(expressions, LilyList, frame->expressions());
			auto rvalues= accIsHead ? frame->rvalues()
				: CONS(acc, frame->rvalues());
			if (expressions->isNull()) {
				// ready to call the continuation
				DEBUGWARN("ready to call the continuation");
				LilyListPtr arguments= lily::reverse(rvalues);
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
				cont= CONS(FRAME(head,
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


LilyListPtr lily::reverse(LilyObjectPtr l) {
	LilyListPtr res= NIL;
	while (true) {
		if (LilyPair*p= is_LilyPair(&*l)) {
			res= CONS(p->car(), res);
			l= p->cdr();
		} else if (is_LilyNull(&*l)) {
			break;
		} else {
			throw std::logic_error("improper list");
		}
	}
	return res;
}

bool lily::isList(LilyObjectPtr v) {
	while (true) {
		IF_LETU_AS(vp, LilyPair, v)
			v= vp->cdr();
		else if (UNWRAP_AS(LilyNull, v))
			return true;
		else
			return false;
	}
	return false;
}


// XX change to not use the C++ stack!
LilyObjectPtr lily::fold_right(
	std::function<LilyObjectPtr(LilyObjectPtr, LilyObjectPtr)> fn,
	LilyObjectPtr start,
	LilyListPtr l) {
	std::function<LilyObjectPtr(LilyObjectPtr)> rec=
		[&](LilyObjectPtr v) -> LilyObjectPtr {

		IF_LETU_AS(vp, LilyPair, v) {
			return fn(vp->car(),
				  rec(vp->cdr()));
		} else if (UNWRAP_AS(LilyNull, v)) {
			return start;
		} else {
			throwTypeError("proper list", v);
		}
	};
	return rec(l);
}

// XX change to not use the C++ stack!
LilyObjectPtr lily::improper_fold_right(
	std::function<LilyObjectPtr(LilyObjectPtr, LilyObjectPtr)> fn,
	LilyObjectPtr start,
	LilyObjectPtr v) {
	std::function<LilyObjectPtr(LilyObjectPtr)> rec=
		[&](LilyObjectPtr v) -> LilyObjectPtr {

		IF_LETU_AS(vp, LilyPair, v) {
			return fn(vp->car(),
				  rec(vp->cdr()));
		} else if (UNWRAP_AS(LilyNull, v)) {
			return start;
		} else {
			return fn(v, start);
		}
	};
	return rec(v);
}


// XX even these (even when fixing the above) are guilty of using the
// C++ stack, even if just one frame (call/cc!).
LilyObjectPtr lily::map(
	std::function<LilyObjectPtr(LilyObjectPtr)> fn,
	LilyListPtr l) {
	return lily::fold_right([&](LilyObjectPtr v,
				    LilyObjectPtr tail) {
					return CONS(fn(v), tail);
				}, NIL, l);
}

// XX ditto
LilyObjectPtr lily::improper_to_proper_map(
	std::function<LilyObjectPtr(LilyObjectPtr)> fn,
	LilyObjectPtr v) {
	return lily::improper_fold_right
		([&](LilyObjectPtr v,
		     LilyObjectPtr tail) {
			return CONS(fn(v), tail);
		}, NIL, v);
}

// Cut off leading "Lily" from type names; does not handle unicode
// since there's no unicode used in those. XX will become obsolete
// once using namespaces.
static
std::string typenameForUser(const std::string s) {
	auto len= s.length();
	if (len > 4) {
		auto start= s.substr(0,4);
		if (start == "Lily") {
			auto t= s.substr(4, len-4);
			// not safe for unicode, but unicode is not
			// used in C++ type names.
			t[0]= tolower(t[0]);
			return t;
		} else {
			return s;
		}
	} else {
		return s;
	}
}


void throwTypeError(const char* typeid_str, LilyObjectPtr v) {
	throw std::logic_error
		(STR("not a "
		     << typenameForUser(typeidToTypename(typeid_str))
		     << ": "
		     << show(v)));
}

