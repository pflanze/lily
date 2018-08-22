#include <sstream>
#include "lily.hpp"
#include "lilyConstruct.hpp" // oh well, why separate those then?
#include "lilyParse.hpp"
#include "lilyUtil.hpp"

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


LilyListPtr
LilyNull::singleton() {
	static LilyListPtr v (new LilyNull());
	return v;
}

LilyObjectPtr
LilyVoid::singleton() {
	static LilyObjectPtr v (new LilyVoid());
	return v;
}



LilyPair::~LilyPair() {};
//LilyVoid::~LilyVoid() {};
//LilyBoolean::~LilyBoolean() {};
LilyString::~LilyString() {};
LilySymbol::~LilySymbol() {};
LilyInt64::~LilyInt64() {};
LilyDouble::~LilyDouble() {};


std::string
LilyObject::onelineString() {
	std::ostringstream res;
	this->onelinePrint(res);
	return res.str();
}

	
void
LilyList::onelinePrint(std::ostream& out) {
	LilyList* p= this;
	out << "(";
	bool isNull= p->isNull();
	while (!isNull) {
		p->first()->onelinePrint(out);
		p= dynamic_cast<LilyList*>(&*(p->rest()));
		isNull= p->isNull();
		if (isNull) {
			out << " ";
		}
	}
	out << ")";
}


void
LilyPair::onelinePrint(std::ostream& out) {
	LilyPair* p= this;
	out << "(";
	while (true) {
		p->car->onelinePrint(out);
		LilyObject* d= &(*(p->cdr));
		if ((p= is_LilyPair(d))) {
			out << " ";
		} else if (is_LilyNull(d)) {
			out << ")";
			break;
		} else {
			out << " . ";
			d->onelinePrint(out);
			out << ")";
			break;
		}
	}
}


void
LilyBoolean::onelinePrint(std::ostream& out) {
	out << (value ? "#t" : "#f");
}

void
LilyVoid::onelinePrint(std::ostream& out) {
	out << "#!void";
}


void
LilyString::onelinePrint(std::ostream& out) {
	out << '"';
	string_onelinePrint(string, out, '"');
	out << '"';
}

void
LilySymbol::onelinePrint(std::ostream& out) {
	bool needsQuoting=0;
	for(char c : string) {
		if (needsSymbolQuoting(c)) {
			needsQuoting=1;
			break;
		}
	}
	if (needsQuoting
	    || string.length()==0
	    || isDigit(string[0])) {
		out << '|';
		string_onelinePrint(string, out, '|');
		out << '|';
	} else {
		out << string;
	}
}


void
LilyInt64::onelinePrint(std::ostream& out) {
	out << value;
}

void
LilyDouble::onelinePrint(std::ostream& out) {
	out << value;
}

void
LilyPrimitive::onelinePrint(std::ostream& out) {
	out << "#<function>"; // XX decide how to handle these
}



LilyObjectPtr
LilyNull::eval(LilyListPtr ctx) {
	throw std::logic_error("not implemented yet");
};
LilyObjectPtr
LilyVoid::eval(LilyListPtr ctx) {
	throw std::logic_error("not implemented yet");
};
LilyObjectPtr
LilyPair::eval(LilyListPtr ctx) {
	throw std::logic_error("not implemented yet");
};
LilyObjectPtr
LilyBoolean::eval(LilyListPtr ctx) {
	throw std::logic_error("not implemented yet");
};
LilyObjectPtr
LilyString::eval(LilyListPtr ctx) {
	throw std::logic_error("not implemented yet");
};
LilyObjectPtr
LilySymbol::eval(LilyListPtr ctx) {
	throw std::logic_error("not implemented yet");
};
LilyObjectPtr
LilyInt64::eval(LilyListPtr ctx) {
	throw std::logic_error("not implemented yet");
};
LilyObjectPtr
LilyDouble::eval(LilyListPtr ctx) {
	throw std::logic_error("not implemented yet");
};
LilyObjectPtr
LilyPrimitive::eval(LilyListPtr ctx) {
	throw std::logic_error("not implemented yet");
};


const char* LilyNull::typeName() {return "Null";}
const char* LilyVoid::typeName() {return "Void";}
const char* LilyPair::typeName() {return "Pair";}
const char* LilyBoolean::typeName() {return "Boolean";}
const char* LilyString::typeName() {return "String";}
const char* LilySymbol::typeName() {return "Symbol";}
const char* LilyInt64::typeName() {return "Int64";}
const char* LilyDouble::typeName() {return "Double";}
const char* LilyPrimitive::typeName() {return "Primitive";}


// utils
LilyListPtr reverse(LilyObjectPtr l) {
	LilyListPtr res= NIL;
	while (true) {
		if (LilyPair*p= is_LilyPair(&*l)) {
			res= LIST_CONS(p->car, res);
			l= p->cdr;
		} else if (is_LilyNull(&*l)) {
			break;
		} else {
			throw std::logic_error("improper list");
		}
	}
	return res;
}

