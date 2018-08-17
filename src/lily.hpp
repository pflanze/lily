#include <string>
#include <memory>
#include <stdexcept>

class LilyObject;
class LilyList;

typedef std::shared_ptr<LilyObject> LilyObjectPtr;
typedef std::shared_ptr<LilyList> LilyListPtr;

class LilyObject {
public:
	virtual LilyObjectPtr eval(LilyObjectPtr v, LilyListPtr ctx);
};


// -- type-enforced proper lists
class LilyList : public LilyObject {
public:
	virtual LilyObjectPtr first() = 0;
	virtual bool isNull() = 0;
	// virtual ~();
};

class LilyNull : public LilyList {
public:
	virtual LilyObjectPtr first() {
		throw std::logic_error("end of list");
	};
	virtual bool isNull() { return true; }
	static LilyObjectPtr singleton();
	// virtual ~LilyNull();
};

class LilyListPair : public LilyList {
public:
	LilyObjectPtr _first;
	LilyListPtr _rest;
	virtual LilyObjectPtr first() { return _first; }
	virtual LilyListPtr rest() { return _rest; }
	virtual bool isNull() { return false; }
	virtual ~LilyListPair();
};
// / type-enforced proper lists


class LilyPair : public LilyObject {
public:
	LilyObjectPtr fst;
	LilyObjectPtr snd;
	virtual ~LilyPair();
};



// Atoms

class LilyBoolean : public LilyObject {
private:
	LilyBoolean(bool v) : value(v) {};
public:
	bool value;
	static LilyObjectPtr True();
	static LilyObjectPtr False();
	virtual ~LilyBoolean();
};

class LilyString : public LilyObject {
public:
	std::string string;
	virtual ~LilyString();
};

class LilySymbol : public LilyObject {
public:
	std::string string;
	virtual ~LilySymbol();
};

class LilyNumber : public LilyObject {
public:
	virtual ~LilyNumber();
};

class LilyInt64 : public LilyNumber {
public:
	int64_t value;
	virtual ~LilyInt64();
};

class LilyDouble : public LilyNumber {
public:
	double value;
	virtual ~LilyDouble();
};

