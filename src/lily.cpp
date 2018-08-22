#include <unordered_map>
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


// XX careful threads! danger?
std::unordered_map<std::string, LilyObjectPtr> lilySymbolTable {};

// XX currently does not free unused symbols. Periodic sweep? Weak
// somehow, how? (Special table needed, or weak will wrapper.)
LilyObjectPtr
LilySymbol::intern(std::string s) {
	auto it= lilySymbolTable.find(s);
	if (it != lilySymbolTable.end()) {
		return it->second;
		// why 'is this a tuple and iterator at same time?'?
		// Overloaded dereference, right? (Uh?)
	} else {
		auto v= LILY_NEW(LilySymbol(s));
		lilySymbolTable[s]= v;
		return v;
	}
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
		p->_car->onelinePrint(out);
		LilyObject* d= &(*(p->_cdr));
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

// and yeah, decided to make it LilyObject, so now have to define the
// stuff that might be user accessed
void
LilyContinuationFrame::onelinePrint(std::ostream& out) {
	out << "#<continuation-frame>"; // XX decide how to handle these
}


const char* LilyNull::typeName() {return "Null";}
const char* LilyVoid::typeName() {return "Void";}
const char* LilyPair::typeName() {return "Pair";}
const char* LilyBoolean::typeName() {return "Boolean";}
const char* LilyString::typeName() {return "String";}
const char* LilySymbol::typeName() {return "Symbol";}
const char* LilyInt64::typeName() {return "Int64";}
const char* LilyDouble::typeName() {return "Double";}
const char* LilyPrimitive::typeName() {return "Primitive";}
const char* LilyContinuationFrame::typeName() {return "ContinuationFrame";}



LilyObjectPtr
LilyPrimitive::call(LilyListPtr args) {
	return _primitive(args);
}


LilyObjectPtr eval(LilyObjectPtr& code,
		   LilyListPtr ctx,
		   LilyListPtr cont) {
	LilyObjectPtr acc;
	while (true) {
		switch (code->evalId) {
		case LilyEvalOpcode::Null:
			throw std::logic_error("empty call");
			break;
		case LilyEvalOpcode::Void:
			acc= code;
			break;
		case LilyEvalOpcode::Pair:
			throw std::logic_error("not implemented yet");
			break;
		case LilyEvalOpcode::Boolean:
			acc= code;
			break;
		case LilyEvalOpcode::String:
			acc= code;
			break;
		case LilyEvalOpcode::Symbol:
			throw std::logic_error("not implemented yet");
			break;
		case LilyEvalOpcode::Int64:
			acc= code;
			break;
		case LilyEvalOpcode::Double:
			acc= code;
			break;
		case LilyEvalOpcode::Primitive:
			acc= code;
			break;
		default:
			throw std::logic_error("invalid opcode");
		}
		if (cont->isNull()) {
			return acc;
		} else {
			LET_AS_U(frame, LilyContinuationFrame, cont->first());
			LET_AS_U(expressions, LilyList, frame->expressions());

			LilyListPtr rvalues1= LIST_CONS(acc, frame->rvalues());
			if (expressions->isNull()) {
				// ready to call the continuation
				LilyListPtr values= reverse(frame->rvalues());
				LET_AS_U(f, LilyCallable, values->first());
				acc= f->call(values->rest());
				cont= cont->rest();
			} else {
				// update continuation
				cont= LIST_CONS(FRAME(rvalues1,
						      expressions->rest()),
						cont->rest());
				code= expressions->first();
			}
		}
	}
}



// utils
LilyListPtr reverse(LilyObjectPtr l) {
	LilyListPtr res= NIL;
	while (true) {
		if (LilyPair*p= is_LilyPair(&*l)) {
			res= LIST_CONS(p->_car, res);
			l= p->_cdr;
		} else if (is_LilyNull(&*l)) {
			break;
		} else {
			throw std::logic_error("improper list");
		}
	}
	return res;
}

