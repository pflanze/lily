#ifndef _LILY_HPP
#define _LILY_HPP

#include <string>
#include <memory>
#include <stdexcept>
#include <ostream>
#include <functional>
#include <assert.h>

enum class LilyEvalOpcode : char {
	Null,
	Void,
	Pair,
	Boolean,
	String,
	Symbol,
	Int64,
	Double,
	Primitive
};

class LilyObject;
class LilyList;
// class LilySymbol;

typedef std::shared_ptr<LilyObject> LilyObjectPtr;
typedef std::shared_ptr<LilyList> LilyListPtr;
// typedef std::shared_ptr<LilyList> LilySymbolPtr;

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
		assert(dynamic_cast<LilyList*>(&*_cdr));
		return *((LilyListPtr*)(&_cdr));
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



class LilyCallable : public LilyObject {
public:
	virtual LilyObjectPtr call(LilyListPtr args) = 0;
};

typedef std::function<LilyObjectPtr(LilyObjectPtr)> LilyPrimitive_t;

struct LilyPrimitive : public LilyCallable {
public:
	LilyPrimitive(LilyPrimitive_t primitive,
		      const char* name)
		: _primitive(primitive), _name(name) {
		evalId= LilyEvalOpcode::Primitive;
	}
	LilyPrimitive_t primitive() { return _primitive; }
	virtual const char* typeName();
	virtual void onelinePrint(std::ostream& out);
	virtual LilyObjectPtr call(LilyListPtr args);
	LilyPrimitive_t _primitive;
	const char* _name;
};

struct LilyContinuationFrame : public LilyObject {
public:
	LilyContinuationFrame(LilyListPtr rvalues, LilyListPtr expressions)
		: _rvalues(rvalues), _expressions(expressions) {}
	LilyListPtr rvalues() { return _rvalues; }
	LilyListPtr expressions() { return _expressions; }
private: // XX make struct readonly?
	virtual const char* typeName();
	virtual void onelinePrint(std::ostream& out);
	LilyListPtr _rvalues; // i.e. evaluated
	LilyListPtr _expressions; // i.e. unevaluated
};


// utils

LilyObjectPtr eval(LilyObjectPtr& code,
		   LilyListPtr ctx,
		   LilyListPtr cont= LilyNull::singleton());

LilyListPtr reverse(LilyObjectPtr l);

// casting that also unwraps it from the shared_ptr
#define UNWRAP_AS(t, e) dynamic_cast<t*>(&*(e))
#define UNWRAP(e) UNWRAP_AS(LilyObject,e)
#define LIST_UNWRAP(e) UNWRAP_AS(LilyList,e)
// let unwrapped
#define LET_AS_U(var, t, e) t* var= UNWRAP_AS(t, e)
#define LET_U(var, e) LET_AS_U(var, LilyObject, e)

#define WARN(e) std::cerr<< e <<"\n"


#endif

