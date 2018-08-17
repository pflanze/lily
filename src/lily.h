#include <std::string>
#include <std::shared_ptr>

class LilyObject;
class LilyList;

typedef std::shared_ptr<LilyObject> LilyObjectPtr;
typedef std::shared_ptr<LilyList> LilyListPtr;

class LilyObject {
	virtual LilyObjectPtr eval(LilyObjectPtr v, LilyListPtr ctx);
};

enum class LilySpecialKind {
	FALSE,
	TRUE,
};

class LilySpecial : LilyObject {
	int kind;
};

class LilyList {
	virtual LilyObjectPtr first() = 0;
	virtual bool isNull() = 0;
};

class LilyNull : LilyList {
	virtual LilyObjectPtr first() {
		throw std::logic_error("end of list");
	};
	virtual bool isNull() { return true; }
	static LilyObjectPtr lilyNull() {
		static LilyObjectPtr v (new LilyNull());
		return v;
	}
};

class LilyListPair : LilyList {
	LilyObjectPtr _first;
	LilyListPtr _rest;
	virtual LilyObjectPrt first() { return _first; }
	virtual LilyListPtr rest() { return _rest; }
	virtual bool isNull() { return false; }
};

class LilyPair : LilyObject {
	LilyObjectPtr fst;
	LilyObjectPtr snd;
};

class LilyString : LilyObject {
	std::string string;
};

class LilySymbol : LilyObject {
	std::string string;
};

class LilyNumber : LilyObject {
};

class LilyInt64 : LilyNumber {
	int64_t value;
};

class LilyDouble : LilyNumber {
	double value;
};

