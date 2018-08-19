#include "lily.hpp"
#include "lilyConstruct.hpp" // oh well, why separate those then?
#include <sstream>
#include "lilyParse.hpp"

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

LilyObjectPtr
LilyVoid::singleton() {
	static LilyObjectPtr v (new LilyVoid());
	return v;
}



LilyListPair::~LilyListPair() {};
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
		p= &(*(p->rest()));
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


static void
string_onelinePrint(std::string& str, std::ostream& out, char quoteChar) {
	for (char c : str) {
		if (c==quoteChar) {
			out << '\\' << c;
		} else if (c== '\n') {
			out << "\\n";
		} else if (c== '\\') {
			out << "\\\\";
		} else {
			out << c;
			// XX utf8
		}
	}
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
		if (!doesNotNeedSymbolQuoting(c)) {
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



LilyObjectPtr
LilyNull::eval(LilyObjectPtr v, LilyListPtr ctx) {
	throw std::logic_error("not implemented yet");
};
LilyObjectPtr
LilyVoid::eval(LilyObjectPtr v, LilyListPtr ctx) {
	throw std::logic_error("not implemented yet");
};
LilyObjectPtr
LilyListPair::eval(LilyObjectPtr v, LilyListPtr ctx) {
	throw std::logic_error("not implemented yet");
};
LilyObjectPtr
LilyPair::eval(LilyObjectPtr v, LilyListPtr ctx) {
	throw std::logic_error("not implemented yet");
};
LilyObjectPtr
LilyBoolean::eval(LilyObjectPtr v, LilyListPtr ctx) {
	throw std::logic_error("not implemented yet");
};
LilyObjectPtr
LilyString::eval(LilyObjectPtr v, LilyListPtr ctx) {
	throw std::logic_error("not implemented yet");
};
LilyObjectPtr
LilySymbol::eval(LilyObjectPtr v, LilyListPtr ctx) {
	throw std::logic_error("not implemented yet");
};
LilyObjectPtr
LilyInt64::eval(LilyObjectPtr v, LilyListPtr ctx) {
	throw std::logic_error("not implemented yet");
};
LilyObjectPtr
LilyDouble::eval(LilyObjectPtr v, LilyListPtr ctx) {
	throw std::logic_error("not implemented yet");
};


const char* LilyNull::typeName() {return "Null";}
const char* LilyVoid::typeName() {return "Void";}
const char* LilyListPair::typeName() {return "ListPair";}
const char* LilyPair::typeName() {return "Pair";}
const char* LilyBoolean::typeName() {return "Boolean";}
const char* LilyString::typeName() {return "String";}
const char* LilySymbol::typeName() {return "Symbol";}
const char* LilyInt64::typeName() {return "Int64";}
const char* LilyDouble::typeName() {return "Double";}


// utils
LilyObjectPtr reverse(LilyObjectPtr l) {
	LilyObjectPtr res= NIL;
	while (true) {
		if (LilyPair*p= is_LilyPair(&*l)) {
			res= CONS(p->car, res);
			l= p->cdr;
		} else if (is_LilyNull(&*l)) {
			break;
		} else {
			throw std::logic_error("improper list");
		}
	}
	return res;
}

