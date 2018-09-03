#ifndef _LILY_HPP
#define _LILY_HPP

#include <string>
#include <memory>
#include <stdexcept>
#include <ostream>
#include <functional>
#include <assert.h>

#include "lilyUtil.hpp"
#include "parse.hpp"
#include "symboltable.hpp"
#include "xxx.hpp"


enum class LilyEvalOpcode : char {
	Null,
	Void,
	Pair,
	Boolean,
	String,
	Symbol,
	Int64,
	Fractional64,
	Double,
	NativeProcedure,
	NativeMacroexpander,
	NativeEvaluator,
	InvalidIsFrame,
	ParseError,
};

class LilyObject;
class LilyList;
class LilyNumber;
class LilyContinuationFrame;
// class LilySymbol;

typedef std::shared_ptr<LilyObject> LilyObjectPtr;
typedef std::shared_ptr<LilyList> LilyListPtr;
typedef std::shared_ptr<LilyContinuationFrame> LilyContinuationFramePtr;
typedef std::shared_ptr<LilyNumber> LilyNumberPtr;


// move to lilyConstruct (see WRITELN)?, or both to lilyUtil?
std::string show(const LilyObjectPtr& v);
std::string show(LilyObject* v);


class LilyObject {
public:
	std::string onelineString();
	virtual const char* typeName()=0;
	virtual void onelinePrint(std::ostream& out)=0;
	LilyEvalOpcode evalId; // XX no way to make that const? come on..?
};


// -- type-enforced proper lists

class LilyNull;
class LilyPair;

class LilyList : public LilyObject {
public:
	virtual LilyObjectPtr first() = 0;
	virtual LilyListPtr rest() = 0;
	virtual LilyObjectPtr car() = 0;
	virtual LilyObjectPtr cdr() = 0;
	virtual bool isNull() = 0;
	virtual bool isPair() = 0;
	virtual void onelinePrint(std::ostream& out);
	// virtual ~LilyList();
};

struct LilyNull : public LilyList {
public:
	LilyNull () {
		evalId= LilyEvalOpcode::Null;
	}
	virtual LilyObjectPtr first() {
		throw std::logic_error("end of list");
	};
	virtual LilyListPtr rest() {
		throw std::logic_error("end of list");
	};
	virtual LilyObjectPtr car() {
		throw std::logic_error("end of list");
	};
	virtual LilyObjectPtr cdr() {
		throw std::logic_error("end of list");
	};
	virtual bool isNull() { return true; }
	virtual bool isPair() { return false; }
	static LilyListPtr singleton();
	virtual const char* typeName();
	// virtual ~LilyNull();
};
static inline LilyNull* is_LilyNull(LilyObject* v) {
	return dynamic_cast<LilyNull*>(v);
}


struct LilyPair : public LilyList {
public:
	LilyPair(LilyObjectPtr car, LilyObjectPtr cdr)
		: _car(car), _cdr(cdr) {
		assert(car);
		assert(cdr);
		evalId= LilyEvalOpcode::Pair;
	}
	virtual LilyObjectPtr first() { return _car; }
	virtual LilyListPtr rest() {
#if 0
		// this gives "dereferencing type-punned pointer will
		// break strict-aliasing rules" warning, but it allows
		// me to check whether conversion succeeded and still
		// only do the casting work once.
		if (dynamic_cast<LilyList*>(&*_cdr)) {
			return *((LilyListPtr*)(&_cdr));
		} else {
			throw std::logic_error(STR("improper list: " <<
						   show(_cdr)));
		}
#else
		// Does this cost a pair of reference counting
		// operations?
		auto res= std::dynamic_pointer_cast<LilyList>(_cdr);
		if (!res)
			throw std::logic_error(STR("improper list: " <<
						   show(_cdr)));
		return res;
#endif
	}
	virtual LilyObjectPtr car() { return _car; }
	virtual LilyObjectPtr cdr() { return _cdr; }
	//^ XX evil?...
	virtual bool isNull() { return false; }
	virtual bool isPair() { return true; }
	virtual void onelinePrint(std::ostream& out);
	virtual const char* typeName();
	virtual ~LilyPair();
	LilyObjectPtr _car;
	LilyObjectPtr _cdr;
};
static inline LilyPair* is_LilyPair(LilyObject* v) {
	return dynamic_cast<LilyPair*>(v);
}


// Atoms

class LilyVoid : public LilyObject {
private:
	LilyVoid() {
		evalId= LilyEvalOpcode::Void;
	};
public:
	virtual bool isNull() { return true; }
	static LilyObjectPtr singleton();
	virtual void onelinePrint(std::ostream& out);
	virtual const char* typeName();
	// virtual ~LilyVoid();
};

class LilyBoolean : public LilyObject {
private:
	LilyBoolean(bool v) : value(v) {
		evalId= LilyEvalOpcode::Boolean;
	};
public:
	bool value;
	static LilyObjectPtr True();
	static LilyObjectPtr False();
	virtual void onelinePrint(std::ostream& out);
	virtual const char* typeName();
	//virtual ~LilyBoolean();
};

class LilyString : public LilyObject {
public:
	LilyString(std::string s) : string(s) {
		evalId= LilyEvalOpcode::String;
	}
	std::string string;
	virtual void onelinePrint(std::ostream& out);
	virtual const char* typeName();
	virtual ~LilyString();
};

class LilySymbol : public LilyObject {
public:
	LilySymbol(std::string s, symboltablehash_t h)
		: string(s), hash(h) {
		evalId= LilyEvalOpcode::Symbol;
	}
	static LilyObjectPtr intern(std::string s);
	const std::string string;
	const symboltablehash_t hash;
	virtual void onelinePrint(std::ostream& out);
	virtual const char* typeName();
	virtual ~LilySymbol();
};

class LilyNumber : public LilyObject {
public:
	virtual LilyNumberPtr multiply(const LilyNumberPtr& b)=0;
	virtual LilyNumberPtr divideBy(const LilyNumberPtr& b)=0;
	virtual LilyNumberPtr add(const LilyNumberPtr& b)=0;
	virtual LilyNumberPtr subtract(const LilyNumberPtr& b)=0;
	virtual double asDouble()=0;
};

class LilyExact : public LilyNumber {
public:
};

class LilyInexact : public LilyNumber {
public:
};

class LilyInt64 : public LilyExact {
public:
	LilyInt64(int64_t v) : value(v) {
		evalId= LilyEvalOpcode::Int64;
	};
	int64_t value;
	virtual void onelinePrint(std::ostream& out);
	virtual const char* typeName();
	virtual double asDouble();
	virtual LilyNumberPtr multiply(const LilyNumberPtr& b);
	virtual LilyNumberPtr divideBy(const LilyNumberPtr& b);
	virtual LilyNumberPtr add(const LilyNumberPtr& b);
	virtual LilyNumberPtr subtract(const LilyNumberPtr& b);
	virtual ~LilyInt64();
};

class LilyFractional64 : public LilyExact {
public:
	LilyFractional64 (int64_t numerator, int64_t denonimator)
		: _numerator(numerator), _denonimator(denonimator) {
		evalId= LilyEvalOpcode::Fractional64;
	}
	int64_t _numerator;
	int64_t _denonimator;
	virtual void onelinePrint(std::ostream& out);
	virtual const char* typeName();
	virtual double asDouble();
	virtual LilyNumberPtr multiply(const LilyNumberPtr& b);
	virtual LilyNumberPtr divideBy(const LilyNumberPtr& b);
	virtual LilyNumberPtr add(const LilyNumberPtr& b);
	virtual LilyNumberPtr subtract(const LilyNumberPtr& b);
	virtual ~LilyFractional64();
};

class LilyDouble : public LilyInexact {
public:
	LilyDouble (double x) : value(x) {
		evalId= LilyEvalOpcode::Double;
	}
	double value;
	virtual void onelinePrint(std::ostream& out);
	virtual const char* typeName();
	virtual double asDouble();
	virtual LilyNumberPtr multiply(const LilyNumberPtr& b);
	virtual LilyNumberPtr divideBy(const LilyNumberPtr& b);
	virtual LilyNumberPtr add(const LilyNumberPtr& b);
	virtual LilyNumberPtr subtract(const LilyNumberPtr& b);
	virtual ~LilyDouble();
};


void throwOverflow(int64_t a, const char*op, int64_t b);
void throwOverflow(const char*op, int64_t a);
void throwDivByZero(int64_t a, const char*op);

inline int64_t lily_add(int64_t a, int64_t b) {
	int64_t res;
	if (__builtin_saddl_overflow(a, b, &res))
		throwOverflow(a, "+", b);
	return res;
}
inline int64_t lily_sub(int64_t a, int64_t b) {
	int64_t res;
	if (__builtin_ssubl_overflow(a, b, &res))
		throwOverflow(a, "-", b);
	return res;
}
inline int64_t lily_mul(int64_t a, int64_t b) {
	int64_t res;
	if (__builtin_smull_overflow(a, b, &res))
		throwOverflow(a, "*", b);
	return res;
}
inline int64_t lily_quotient(int64_t a, int64_t b) {
	if (b==0)
		throwDivByZero(a, "/");
	return a/b;
}
inline int64_t lily_remainder(int64_t a, int64_t b) {
	if (b==0)
		throwDivByZero(a, "%");
	return a % b;
}
inline int64_t lily_negate(int64_t a) {
	// wow (= (- (arithmetic-shift 1 63)) (arithmetic-shift -1 63)) holds true
	if (a == (-1ll << 63ll))
		throwOverflow("-", a);
	return -a;
}
inline int64_t lily_abs(int64_t a) {
	return (a < 0) ? lily_negate(a) : a;
}



int64_t lily_gcd(int64_t a, int64_t b);

// Number operations via static dispatch; normally use the virtual
// methods instead (which are using these)
static inline LilyNumberPtr Multiply(LilyDouble* a, LilyDouble* b) {
	return std::shared_ptr<LilyNumber>(new LilyDouble(a->value * b->value));
}
// don't need all the combinations: only inexact*inexact and then the
// combinations of the exact variants; complex numbers pending.
static inline LilyNumberPtr Multiply(LilyFractional64* a, LilyFractional64* b) {
	// return lilyGcd();
	XXX
}
static inline LilyNumberPtr Multiply(LilyInt64* a, LilyFractional64* b) {
	// return lilyGcd();
	XXX
}
static inline LilyNumberPtr Multiply(LilyFractional64* a, LilyInt64* b) {
	return Multiply(b,a);
}
static inline LilyNumberPtr Multiply(LilyInt64* a, LilyInt64* b) {
	return std::shared_ptr<LilyNumber>(new LilyInt64(lily_mul(a->value, b->value)));
}


LilyNumberPtr Divide(LilyInt64* a, LilyInt64* b);

static inline LilyNumberPtr Divide(LilyInt64* a, LilyFractional64* b) {
	XXX
}



// LilyNative_t is also used for syntax, hence args could be values or
// expressions
typedef std::function<LilyObjectPtr(LilyListPtr* args,  // XX const LilyListPtr&? Say const about the pointer target, really, well, both. Refcounting ?
				    LilyListPtr* ctx,
				    LilyListPtr* cont)> LilyNative_t;


// Currently using callable for both pre-application and application
// phases; not sure this can remain that way in the future.
class LilyCallable : public LilyObject {
public:
	//XXQ do I have to use () syntax for making it clear it's a
	//method or would something using LilyNative_t work?
	virtual LilyObjectPtr call(LilyListPtr* args,
				   LilyListPtr* ctx,
				   LilyListPtr* cont) = 0;
};

struct LilyNativeProcedure : public LilyCallable {
public:
	LilyNativeProcedure(LilyNative_t proc,
			    const char* name)
		: _proc(proc), _name(name) {
		evalId= LilyEvalOpcode::NativeProcedure;
	}
	virtual const char* typeName();
	virtual void onelinePrint(std::ostream& out);
	virtual LilyObjectPtr call(LilyListPtr* args,
				   LilyListPtr* ctx,
				   LilyListPtr* cont);
	LilyNative_t _proc;
	const char* _name;
};


// A macro expander takes code (as an LilyObject) and returns code;
// it's like a function but used in an earlier phase during evaluation
class LilyMacroexpander : public LilyCallable {
public:
	// needs to be virtual since calling guest language function
	// based variant will be different, correct?
	virtual LilyObjectPtr call(LilyListPtr* expressions,
				   LilyListPtr* ctx,
				   LilyListPtr* cont) = 0;
};


struct LilyNativeMacroexpander : public LilyMacroexpander {
public:
	LilyNativeMacroexpander(LilyNative_t expander,
				const char* name)
		: _expander(expander), _name(name) {
		evalId= LilyEvalOpcode::NativeMacroexpander;
		// ^ but can't use this to dispatch in eval, as the
		// pair around it is the dispatch point (and then it's
		// hidden behind a symbol first, too; and then in the
		// future in another context)
	}
	virtual const char* typeName();
	virtual void onelinePrint(std::ostream& out);
	virtual LilyObjectPtr call(LilyListPtr* expressions,
				   LilyListPtr* ctx,
				   LilyListPtr* cont);
	LilyNative_t _expander;
	const char* _name;
};


// An evaluator receives code and evaluates it to a value (not code,
// unlike macros).

// XX not sure making it a LilyCallable is right here at all since it
// requires context(s?), too. Ah but then would want to give macros
// access to the same, too.

struct LilyNativeEvaluator : public LilyCallable {
public:
	LilyNativeEvaluator(LilyNative_t eval,
			    const char* name)
		: _eval(eval), _name(name) {
		evalId= LilyEvalOpcode::NativeEvaluator;
	}
	virtual const char* typeName();
	virtual void onelinePrint(std::ostream& out);
	virtual LilyObjectPtr call(LilyListPtr* args,
				   LilyListPtr* ctx,
				   LilyListPtr* cont);
	LilyNative_t _eval;
	const char* _name;
};


// not a callabe (directly), at least not obviously the right thing to
// do for now
struct LilyContinuationFrame : public LilyObject {
public:
	LilyContinuationFrame(LilyObjectPtr maybeHead,
			      LilyListPtr rvalues, // arguments only
			      LilyListPtr expressions // unevaluated arguments
		)
		: _maybeHead(maybeHead), _rvalues(rvalues), _expressions(expressions) {
		assert(rvalues);
		// WARN("expressions="<<expressions); this is sick:
		// showing 0, assert fails, in upper frame gdb reports
		// as zero, but in this frame gdb insists that it
		// isn't. In both "p expressions ? true : false"
		// (true) as well as "p &*expressions" ((LilyList *)
		// 0x7fffffffd960). Assert works as designed but gdb
		// doesn't agree.
		assert(expressions);
		evalId= LilyEvalOpcode::InvalidIsFrame;
	}
	LilyObjectPtr maybeHead() { return _maybeHead; }
	LilyListPtr rvalues() { return _rvalues; }
	LilyListPtr expressions() { return _expressions; }
private: // XX make struct readonly?
	virtual const char* typeName();
	virtual void onelinePrint(std::ostream& out);
	LilyObjectPtr _maybeHead;
	LilyListPtr _rvalues; // i.e. evaluated
	LilyListPtr _expressions; // i.e. unevaluated
};


// base error class, but will not work that well anymore once guest
// language structs exist and are to be allowed to inherit from a
// separate error base
struct LilyErrorBase : public LilyObject {
};
struct LilyErrorWithWhat : public LilyObject {
	virtual std::string what()=0;
};


struct LilyParseError : public LilyErrorWithWhat {
public:
	LilyParseError(std::string msg, parse_position_t pos)
		: _msg(msg), _pos(pos) {
		evalId= LilyEvalOpcode::ParseError;
	}
	virtual std::string msg() { return _msg; };
	virtual parse_position_t pos() { return _pos; };
	virtual std::string what();
	virtual const char* typeName();
	virtual void onelinePrint(std::ostream& out);
	std::string _msg;
	parse_position_t _pos;
};


// direct s-expr evaluator; ctx is the lexical context, cont the
// dynamic context
LilyObjectPtr eval(LilyObjectPtr code,
		   LilyListPtr ctx,
		   LilyListPtr cont= LilyNull::singleton());


LilyListPtr reverse(LilyObjectPtr l);

// utils

// usable for both syntax and function application (not sure how this
// will remain when moving to guest language exceptions?)
LilyObjectPtr
apply1ary(const char* procname,
	  std::function<LilyObjectPtr(LilyObjectPtr)> proc,
	  LilyListPtr* vs);

// casting that also unwraps it from the shared_ptr; NOTE: returns
// NULL if invalid
#define UNWRAP_AS(t, e) dynamic_cast<t*>(&*(e))
//#define UNWRAP(e) UNWRAP_AS(LilyObject,e)
#define LIST_UNWRAP(e) UNWRAP_AS(LilyList,e)
// let unwrapped (or "unsafe")
#define LETU_AS(var, t, e) t* var= UNWRAP_AS(t, e)
//#define LETU(var, e) LETU_AS(var, LilyObject, e)

// casting without unwrapping
#define LET_AS(var, t, e) auto var= \
		std::dynamic_pointer_cast<t>(e)


// Same thing but throws an exception on cast errors.
#define XUNWRAP_AS(t, e)						\
	([&]() -> t* {							\
		t* _XUNWRAP_AS_v= dynamic_cast<t*>(&*(e));		\
		if (! _XUNWRAP_AS_v)					\
			throw std::logic_error("can't unwrap ");	\
		return  _XUNWRAP_AS_v;					\
	})()
#define XLIST_UNWRAP(e) XUNWRAP_AS(LilyList,e)
// let unwrapped
#define XLETU_AS(var, t, e) t* var= XUNWRAP_AS(t, e)
#define XLETU(var, e) XLETU_AS(var, LilyObject, e)


#endif

