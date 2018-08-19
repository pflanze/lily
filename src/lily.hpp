#include <string>
#include <memory>
#include <stdexcept>
#include <ostream>


class LilyObject;
class LilyList;

typedef std::shared_ptr<LilyObject> LilyObjectPtr;
typedef std::shared_ptr<LilyList> LilyListPtr;

class LilyObject {
public:
	std::string onelineString();
	virtual const char* typeName()=0;
	virtual void onelinePrint(std::ostream& out)=0;
	virtual LilyObjectPtr eval(LilyObjectPtr v, LilyListPtr ctx)=0;
};


// -- type-enforced proper lists
class LilyList : public LilyObject {
public:
	virtual LilyObjectPtr first() = 0;
	virtual LilyListPtr rest() = 0;
	virtual bool isNull() = 0;
	virtual void onelinePrint(std::ostream& out);
	// virtual ~();
};

class LilyNull : public LilyList {
public:
	virtual LilyObjectPtr first() {
		throw std::logic_error("end of list");
	};
	virtual LilyListPtr rest() {
		throw std::logic_error("end of list");
	};
	virtual bool isNull() { return true; }
	static LilyObjectPtr singleton();
	virtual LilyObjectPtr eval(LilyObjectPtr v, LilyListPtr ctx);
	virtual const char* typeName();
	// virtual ~LilyNull();
};
static inline LilyNull* is_LilyNull(LilyObject* v) {
	return dynamic_cast<LilyNull*>(v);
}

class LilyListPair : public LilyList {
private:
	LilyObjectPtr _first;
	LilyListPtr _rest;
public:
	LilyListPair(LilyObjectPtr first, LilyListPtr rest) :
		_first(first), _rest(rest) {}
	virtual LilyObjectPtr first() { return _first; }
	virtual LilyListPtr rest() { return _rest; }
	virtual bool isNull() { return false; }
	virtual LilyObjectPtr eval(LilyObjectPtr v, LilyListPtr ctx);
	virtual const char* typeName();
	virtual ~LilyListPair();
};
// / type-enforced proper lists


class LilyPair : public LilyObject {
public:
	LilyPair(LilyObjectPtr a, LilyObjectPtr d) : car(a), cdr(d) {}
	LilyObjectPtr car;
	LilyObjectPtr cdr;
	virtual void onelinePrint(std::ostream& out);
	virtual LilyObjectPtr eval(LilyObjectPtr v, LilyListPtr ctx);
	virtual const char* typeName();
	virtual ~LilyPair();
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
	virtual LilyObjectPtr eval(LilyObjectPtr v, LilyListPtr ctx);
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
	virtual LilyObjectPtr eval(LilyObjectPtr v, LilyListPtr ctx);
	virtual const char* typeName();
	//virtual ~LilyBoolean();
};

class LilyString : public LilyObject {
public:
	LilyString(std::string s) : string(s) {}
	std::string string;
	virtual void onelinePrint(std::ostream& out);
	virtual LilyObjectPtr eval(LilyObjectPtr v, LilyListPtr ctx);
	virtual const char* typeName();
	virtual ~LilyString();
};

class LilySymbol : public LilyObject {
public:
	LilySymbol(std::string s) : string(s) {}
	std::string string;
	virtual void onelinePrint(std::ostream& out);
	virtual LilyObjectPtr eval(LilyObjectPtr v, LilyListPtr ctx);
	virtual const char* typeName();
	virtual ~LilySymbol();
};

class LilyNumber : public LilyObject {
public:
	// virtual LilyObjectPtr eval(LilyObjectPtr v, LilyListPtr ctx);
	// virtual ~LilyNumber();
};

class LilyInt64 : public LilyNumber {
public:
	LilyInt64(int64_t v) : value(v) {};
	int64_t value;
	virtual void onelinePrint(std::ostream& out);
	virtual LilyObjectPtr eval(LilyObjectPtr v, LilyListPtr ctx);
	virtual const char* typeName();
	virtual ~LilyInt64();
};

class LilyDouble : public LilyNumber {
public:
	double value;
	virtual void onelinePrint(std::ostream& out);
	virtual LilyObjectPtr eval(LilyObjectPtr v, LilyListPtr ctx);
	virtual const char* typeName();
	virtual ~LilyDouble();
};


// utils
LilyObjectPtr reverse(LilyObjectPtr l);
