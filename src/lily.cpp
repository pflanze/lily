#include "lily.hpp"

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


LilyObjectPtr
LilyNull::singleton() {
	static LilyObjectPtr v (new LilyNull());
	return v;
}



LilyListPair::~LilyListPair() {};
LilyPair::~LilyPair() {};
LilyBoolean::~LilyBoolean() {};
LilyString::~LilyString() {};
LilySymbol::~LilySymbol() {};
LilyNumber::~LilyNumber() {};
LilyInt64::~LilyInt64() {};
LilyDouble::~LilyDouble() {};


