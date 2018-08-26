#ifndef _LILY_HPP
#define _LILY_HPP

#include <string>
#include <memory>
#include <stdexcept>
#include <ostream>
#include <functional>
#include <assert.h>
#include "lilyUtil.hpp"


enum class LilyEvalOpcode : char {
	Null,
	Void,
	Pair,
	Boolean,
	String,
	Symbol,
	Int64,
	Double,
	NativeProcedure,
	NativeMacroexpander,
	NativeEvaluator,
	InvalidIsFrame
};

class LilyObject;
class LilyList;
// class LilySymbol;

typedef std::shared_ptr<LilyObject> LilyObjectPtr;
typedef std::shared_ptr<LilyList> LilyListPtr;
// typedef std::shared_ptr<LilyList> LilySymbolPtr;


// move to lilyConstruct (see WRITELN)?, or both to lilyUtil?
std::string show(const LilyObjectPtr& v);
std::string show(LilyObject* v);


class LilyObject {
public:
	std::string onelineString();
	virtual const char* typeName()=0;
	virtual void onelinePrint(std::ostream& out)=0;
	LilyEvalOpcode evalId;
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
		if (dynamic_cast<LilyList*>(&*_cdr)) {
			return *((LilyListPtr*)(&_cdr));
		} else {
			throw std::logic_error(STR("improper list: " <<
						   show(_cdr)));
		}
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
	LilySymbol(std::string s) : string(s) {
		evalId= LilyEvalOpcode::Symbol;
	}
	static LilyObjectPtr intern(std::string s);
	std::string string;
	virtual void onelinePrint(std::ostream& out);
	virtual const char* typeName();
	virtual ~LilySymbol();
};

class LilyNumber : public LilyObject {
public:
	// virtual ~LilyNumber();
};

class LilyInt64 : public LilyNumber {
public:
	LilyInt64(int64_t v) : value(v) {
		evalId= LilyEvalOpcode::Int64;
	};
	int64_t value;
	virtual void onelinePrint(std::ostream& out);
	virtual const char* typeName();
	virtual ~LilyInt64();
};

class LilyDouble : public LilyNumber {
public:
	LilyDouble (double x) : value(x) {
		evalId= LilyEvalOpcode::Double;
	}
	double value;
	virtual void onelinePrint(std::ostream& out);
	virtual const char* typeName();
	virtual ~LilyDouble();
};


// Currently using callable for both pre-application and application
// phases; not sure this can remain that way in the future.
class LilyCallable : public LilyObject {
public:
	virtual LilyObjectPtr call(LilyListPtr args,
				   LilyListPtr ctx,
				   LilyListPtr cont) = 0;
};

typedef std::function<LilyObjectPtr(LilyObjectPtr,
				    LilyObjectPtr,
				    LilyObjectPtr)> LilyNative_t;

struct LilyNativeProcedure : public LilyCallable {
public:
	LilyNativeProcedure(LilyNative_t proc,
			    const char* name)
		: _proc(proc), _name(name) {
		evalId= LilyEvalOpcode::NativeProcedure;
	}
	virtual const char* typeName();
	virtual void onelinePrint(std::ostream& out);
	virtual LilyObjectPtr call(LilyListPtr args,
				   LilyListPtr ctx,
				   LilyListPtr cont);
	LilyNative_t _proc;
	const char* _name;
};


// A macro expander takes code (as an LilyObject) and returns code;
// it's like a function but used in an earlier phase during evaluation
class LilyMacroexpander : public LilyCallable {
public:
	// needs to be virtual since calling guest language function
	// based variant will be different, correct?
	virtual LilyObjectPtr call(LilyListPtr expressions,
				   LilyListPtr ctx,
				   LilyListPtr cont) = 0;
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
	virtual LilyObjectPtr call(LilyListPtr expressions,
				   LilyListPtr ctx,
				   LilyListPtr cont);
	LilyNative_t _expander;
	const char* _name;
};


// An evaluator receives code and evaluates it to a value (not code,
// unlike macros).

// XX not sure making it a LilyCallable is right here at all since it
// requires context(s?), too. Ah but then would want to give macros
// access to the same, too.

typedef std::function<LilyObjectPtr(LilyObjectPtr, LilyListPtr, LilyListPtr)> LilyEval_t;

struct LilyNativeEvaluator : public LilyCallable {
public:
	LilyNativeEvaluator(LilyEval_t eval,
		      const char* name)
		: _eval(eval), _name(name) {
		evalId= LilyEvalOpcode::NativeEvaluator;
	}
	virtual const char* typeName();
	virtual void onelinePrint(std::ostream& out);
	virtual LilyObjectPtr call(LilyListPtr args,
				   LilyListPtr ctx,
				   LilyListPtr cont);
	LilyEval_t _eval;
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
	  LilyObjectPtr vs);

// casting that also unwraps it from the shared_ptr; NOTE: returns
// NULL if invalid
#define UNWRAP_AS(t, e) dynamic_cast<t*>(&*(e))
#define UNWRAP(e) UNWRAP_AS(LilyObject,e)
#define LIST_UNWRAP(e) UNWRAP_AS(LilyList,e)
// let unwrapped (or "unsafe")
#define LETU_AS(var, t, e) t* var= UNWRAP_AS(t, e)
#define LETU(var, e) LETU_AS(var, LilyObject, e)

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

