
class LilyObject {

};

enum LilySpecialKind {
	FALSE,
	TRUE,
	NIL,
};

class LilySpecial : LilyObject {
	int kind;
};

class LilyPair : LilyObject {
	LilyObject* car;
	LilyObject* cdr;
};

class LilyString : LilyObject {
	std::string value;
};

class LilySymbol : LilyObject {
	std::string value;
};

class LilyNumber : LilyObject {
};

class LilyInt64 : LilyNumber {
	int64 value;
};

class LilyDouble : LilyNumber {
	double value;
};

