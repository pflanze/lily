#ifndef _LILY_HPP
#define _LILY_HPP

#include <string>
#include <memory>
#include <stdexcept>
#include <ostream>
#include <functional>
#include <assert.h>
 

class LilyObject;
class LilyList;

typedef std::shared_ptr<LilyObject> LilyObjectPtr;
typedef std::shared_ptr<LilyList> LilyListPtr;

class LilyObject {
public:
	std::string onelineString();
	virtual const char* typeName()=0;
	virtual void onelinePrint(std::ostream& out)=0;
	virtual LilyObjectPtr eval(LilyListPtr ctx)=0;
};


// -- type-enforced proper lists

class LilyNull;
class LilyPair;

class LilyList : public LilyObject {
public:
	virtual LilyObjectPtr first() = 0;
	virtual LilyObjectPtr rest() = 0;
	virtual bool isNull() = 0;
	virtual bool isPair() = 0;
	virtual void onelinePrint(std::ostream& out);
	// virtual ~LilyList();
};

struct LilyNull : public LilyList {
public:
	virtual LilyObjectPtr first() {
		throw std::logic_error("end of list");
	};
	virtual LilyObjectPtr rest() {
		throw std::logic_error("end of list");
	};
	virtual bool isNull() { return true; }
	virtual bool isPair() { return false; }
	static LilyListPtr singleton();
	virtual LilyObjectPtr eval(LilyListPtr ctx);
	virtual const char* typeName();
	// virtual ~LilyNull();
};
static inline LilyNull* is_LilyNull(LilyObject* v) {
	return dynamic_cast<LilyNull*>(v);
}


struct LilyPair : public LilyList {
public:
	LilyPair(LilyObjectPtr a, LilyObjectPtr d) : car(a), cdr(d) {
		assert(a);
		assert(d);
	}
	virtual LilyObjectPtr first() { return car; }
	virtual LilyObjectPtr rest() {
		assert(dynamic_cast<LilyList*>(&*(cdr)));
		return cdr;
	}
	//^ XX evil?...
	virtual bool isNull() { return false; }
	virtual bool isPair() { return true; }
	virtual void onelinePrint(std::ostream& out);
	virtual LilyObjectPtr eval(LilyListPtr ctx);
	virtual const char* typeName();
	virtual ~LilyPair();
	LilyObjectPtr car;
	LilyObjectPtr cdr;
};
static inline LilyPair* is_LilyPair(LilyObject* v) {
	return dynamic_cast<LilyPair*>(v);
}


// Atoms

class LilyVoid : public LilyObject {
private:
	LilyVoid() {};
public:
	virtual bool isNull() { return true; }
	static LilyObjectPtr singleton();
	virtual void onelinePrint(std::ostream& out);
	virtual LilyObjectPtr eval(LilyListPtr ctx);
	virtual const char* typeName();
	// virtual ~LilyVoid();
};

class LilyBoolean : public LilyObject {
private:
	LilyBoolean(bool v) : value(v) {};
public:
	bool value;
	static LilyObjectPtr True();
	static LilyObjectPtr False();
	virtual void onelinePrint(std::ostream& out);
	virtual LilyObjectPtr eval(LilyListPtr ctx);
	virtual const char* typeName();
	//virtual ~LilyBoolean();
};

class LilyString : public LilyObject {
public:
	LilyString(std::string s) : string(s) {}
	std::string string;
	virtual void onelinePrint(std::ostream& out);
	virtual LilyObjectPtr eval(LilyListPtr ctx);
	virtual const char* typeName();
	virtual ~LilyString();
};

class LilySymbol : public LilyObject {
public:
	LilySymbol(std::string s) : string(s) {}
	static LilyObjectPtr intern(std::string s);
	std::string string;
	virtual void onelinePrint(std::ostream& out);
	virtual LilyObjectPtr eval(LilyListPtr ctx);
	virtual const char* typeName();
	virtual ~LilySymbol();
};

class LilyNumber : public LilyObject {
public:
	// virtual LilyObjectPtr eval(LilyListPtr ctx);
	// virtual ~LilyNumber();
};

class LilyInt64 : public LilyNumber {
public:
	LilyInt64(int64_t v) : value(v) {};
	int64_t value;
	virtual void onelinePrint(std::ostream& out);
	virtual LilyObjectPtr eval(LilyListPtr ctx);
	virtual const char* typeName();
	virtual ~LilyInt64();
};

class LilyDouble : public LilyNumber {
public:
	double value;
	virtual void onelinePrint(std::ostream& out);
	virtual LilyObjectPtr eval(LilyListPtr ctx);
	virtual const char* typeName();
	virtual ~LilyDouble();
};

typedef std::function<LilyObjectPtr(LilyObjectPtr)> LilyPrimitive_t;

struct LilyPrimitive : public LilyObject {
public:
	LilyPrimitive(LilyPrimitive_t primitive)
		: _primitive(primitive) {}
	LilyPrimitive_t primitive() { return _primitive; }
	virtual const char* typeName();
	virtual void onelinePrint(std::ostream& out);
	virtual LilyObjectPtr eval(LilyListPtr ctx);
	LilyPrimitive_t _primitive;
};


// utils
LilyListPtr reverse(LilyObjectPtr l);

// casting that also unwraps it from the shared_ptr
#define UNWRAP_AS(t, e) dynamic_cast<t*>(&*(e))
#define UNWRAP(e) UNWRAP_AS(LilyObject,e)
#define LIST_UNWRAP(e) UNWRAP_AS(LilyList,e)

#define WARN(e) std::cerr<< e <<"\n"


#endif

