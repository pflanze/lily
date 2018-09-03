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
		auto v= LILY_NEW(LilySymbol(s,siphash(s)));
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
LilyFractional64::~LilyFractional64() {};
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
		p= LIST_UNWRAP(p->rest());
		isNull= p->isNull();
		if (isNull) {
			out << " ";
		}
	}
	out << ")";
}

// XX const? can we bring it into the program segment?
static auto quote= SYMBOL("quote");
static auto quasiquote= SYMBOL("quasiquote");
static auto unquote= SYMBOL("unquote");

void
LilyPair::onelinePrint(std::ostream& out) {
	LETU_AS(cdr, LilyPair, _cdr);
	if (cdr && is_LilyNull(&*(cdr->_cdr))) {
		if (_car==quote) { out << "'";  goto end_special; }
		else if (_car==quasiquote) { out << "`";  goto end_special; }
		else if (_car==unquote) { out << ",";  goto end_special; }
		else {goto otherwise; }
	end_special:
		cdr->_car->onelinePrint(out);
		return;
	}
otherwise:
	// list or improper list
	LilyPair* p= this;
	out << "(";
	while (true) {
		p->_car->onelinePrint(out);
		LilyObject* d= &(*(p->_cdr));
		if ((p= is_LilyPair(d))) {
			out << " ";
		} else if (is_LilyNull(d)) {
			break;
		} else {
			out << " . ";
			d->onelinePrint(out);
			break;
		}
	}
	out << ")";
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
	bool needsQuoting= false;
	bool containsNonDigit= false;
	for(char c : string) {
		if (needsSymbolQuoting(c)) {
			needsQuoting= true;
			break;
		}
		if (!isDigit(c))
			containsNonDigit= true;
	}
	if (needsQuoting || (!containsNonDigit) || string.length()==0) {
		out << '|';
		string_onelinePrint(string, out, '|');
		out << '|';
	} else {
		out << string;
	}
#if 0
	out<< "[" << hash << "]";
#endif
}


void
LilyInt64::onelinePrint(std::ostream& out) {
	out << value;
}

void
LilyFractional64::onelinePrint(std::ostream& out) {
	out << _numerator << "/" << _denonimator;
}

void
LilyDouble::onelinePrint(std::ostream& out) {
	out << value;
}

// could output addresses, but then testing via stringification
// becomes non-deterministic
void
LilyNativeProcedure::onelinePrint(std::ostream& out) {
	out << "#<native-procedure "<< _name <<">";
}

void
LilyNativeMacroexpander::onelinePrint(std::ostream& out) {
	out << "#<native-macro-expander "<< _name <<">";
}

void
LilyNativeEvaluator::onelinePrint(std::ostream& out) {
	out << "#<native-evaluator "<< _name <<">";
}

// and yeah, decided to make it a LilyObject, so now have to define
// the stuff that might be user accessed
void
LilyContinuationFrame::onelinePrint(std::ostream& out) {
	bool deep=1;
	if (deep) {
		(out << "#<continuation-frame "
		 << (_maybeHead ? show(_maybeHead) : "NULL")
		 << " "
		 << show(_rvalues) // XX use onelinePrint directly please..
		 << " "
		 << show(_expressions)
		 << ">");
	} else {
		(out << "#<continuation-frame 0x"
		 << std::hex
		 << _rvalues << " 0x"
		 << _expressions
		 << std::dec
		 << ">");
	}
}

void
LilyParseError::onelinePrint(std::ostream& out) {
	(out
	 << "#<parse-error "
	 << show(STRING(_msg)) // for string formatting
	 << " "
	 << _pos // no need for formatting, ok?
	 << ">");
}



std::string LilyParseError::what() {
	return STR(_msg << " (position " << _pos << ")");
}



const char* LilyNull::typeName() {return "Null";}
const char* LilyVoid::typeName() {return "Void";}
const char* LilyPair::typeName() {return "Pair";}
const char* LilyBoolean::typeName() {return "Boolean";}
const char* LilyString::typeName() {return "String";}
const char* LilySymbol::typeName() {return "Symbol";}
const char* LilyInt64::typeName() {return "Int64";}
const char* LilyFractional64::typeName() {return "Fractional64";}
const char* LilyDouble::typeName() {return "Double";}
const char* LilyNativeProcedure::typeName() {return "NativeProcedure";}
const char* LilyNativeMacroexpander::typeName() {return "NativeMacroexpander";}
const char* LilyNativeEvaluator::typeName() {return "NativeEvaluator";}
const char* LilyContinuationFrame::typeName() {return "ContinuationFrame";}
const char* LilyParseError::typeName() {return "ParseError";}


void throwOverflow(int64_t a, const char*op, int64_t b) {
	throw std::overflow_error(STR("int64 overflow: " << a << " " << op << " " << b));
}
void throwOverflow(const char*op, int64_t a) {
	throw std::overflow_error(STR("int64 overflow: " << op << " " << a));
}
void throwDivByZero(int64_t a, const char*op) {
	// no div by zero exception, huh? XX?
	throw std::runtime_error(STR("int64 division by zero: " << a << " " << op << " " << 0));
}



// can't use gcd from <numeric> as it doesn't check for number overflow

// Scheme code from scheme 48
// (define (gcd . integers)
//   (reduce (lambda (x y)
//             (cond ((< x 0) (gcd (- 0 x) y))
//                   ((< y 0) (gcd x (- 0 y)))
//                   ((< x y) (euclid y x))
//                   (else (euclid x y))))
//           0
//           integers))
int64_t lily_euclid(int64_t x, int64_t y);
int64_t lily_gcd(int64_t x, int64_t y) {
	if (x < 0)
		return lily_gcd(lily_negate(x), y);
	else if (y < 0)
		return lily_gcd(x, lily_negate(y));
	else if (x < y)
		return lily_euclid(y, x);
	else
		return lily_euclid(x, y);
}
// (define (euclid x y)
//   (if (= y 0)
//       (if (and (inexact? y)
//                (exact? x))
//           (exact->inexact x)
//           x)
//       (euclid y (remainder x y))))
int64_t lily_euclid(int64_t x, int64_t y) {
	if (y == 0)
		return x;
	else
		return lily_euclid(y, lily_remainder(x, y));
}
// (define (lcm . integers)
//   (reduce (lambda (x y)
//             (let ((g (gcd x y)))
//               (cond ((= g 0) g)
//                     (else (* (quotient (abs x) g) (abs y))))))
//           1
//           integers))
int64_t lily_lcm(int64_t x, int64_t y) {
	int64_t g= lily_gcd(x, y);
	if (g == 0)
		return g;
	else
		return lily_mul(lily_quotient(lily_abs(x), g), lily_abs(y));
}


// XX test
double LilyInt64::asDouble() {
	return static_cast<double>(value);
}
double LilyFractional64::asDouble() {
	return static_cast<double>(_numerator) / static_cast<double>(_numerator);
}
double LilyDouble::asDouble() {
	return value;
}

LilyNumberPtr LilyInt64::multiply(const LilyNumberPtr& b) {
	auto* b0= dynamic_cast<LilyInt64*>(&*b);
	if (b0) return Multiply(this, b0);
	auto b1= dynamic_cast<LilyFractional64*>(&*b);
	if (b1) return Multiply(this, b1);
	throw std::logic_error("unimplemented number operation");
}
LilyNumberPtr LilyInt64::divideBy(const LilyNumberPtr& b) {
	auto* b0= dynamic_cast<LilyInt64*>(&*b);
	if (b0) return Divide(this, b0);
	auto b1= dynamic_cast<LilyFractional64*>(&*b);
	if (b1) return Divide(this, b1);
	throw std::logic_error("unimplemented number operation");
}
LilyNumberPtr LilyInt64::add(const LilyNumberPtr& b) {
	throw std::logic_error("unimplemented number operation");
}
LilyNumberPtr LilyInt64::subtract(const LilyNumberPtr& b) {
	throw std::logic_error("unimplemented number operation");
}

// really copy-paste; XX templates possible?
LilyNumberPtr LilyFractional64::multiply(const LilyNumberPtr& b) {
	auto b0= dynamic_cast<LilyInt64*>(&*b);
	if (b0) return Multiply(this, b0);
	auto b1= dynamic_cast<LilyFractional64*>(&*b);
	if (b1) return Multiply(this, b1);
	throw std::logic_error("unimplemented number operation");
}
LilyNumberPtr LilyFractional64::divideBy(const LilyNumberPtr& b) {
	throw std::logic_error("unimplemented number operation");
}
LilyNumberPtr LilyFractional64::add(const LilyNumberPtr& b) {
	throw std::logic_error("unimplemented number operation");
}
LilyNumberPtr LilyFractional64::subtract(const LilyNumberPtr& b) {
	throw std::logic_error("unimplemented number operation");
}


LilyNumberPtr LilyDouble::multiply(const LilyNumberPtr& b) {
	auto f= dynamic_cast<LilyDouble*>(&*b);
	if (f) return Multiply(this, f);
	throw std::logic_error("unimplemented number operation");
}
LilyNumberPtr LilyDouble::divideBy(const LilyNumberPtr& b) {
	throw std::logic_error("unimplemented number operation");
}
LilyNumberPtr LilyDouble::add(const LilyNumberPtr& b) {
	throw std::logic_error("unimplemented number operation");
}
LilyNumberPtr LilyDouble::subtract(const LilyNumberPtr& b) {
	throw std::logic_error("unimplemented number operation");
}



LilyObjectPtr
LilyNativeProcedure::call(LilyListPtr* args,
			  LilyListPtr* ctx,
			  LilyListPtr* cont) {
	DEBUGWARN("NativeProcedure: " << _name << show(*args));
	return _proc(args, ctx, cont);
}

LilyObjectPtr
LilyNativeMacroexpander::call(LilyListPtr* expressions,
			      LilyListPtr* ctx,
			      LilyListPtr* cont) {
	DEBUGWARN("NativeMacroexpander: " << _name << show(*expressions));
	return _expander(expressions, ctx, cont);
}

LilyObjectPtr
LilyNativeEvaluator::call(LilyListPtr* expressions,
			  LilyListPtr* ctx,
			  LilyListPtr* cont) {
	DEBUGWARN("NativeEvaluator: " << _name << show(*expressions));
	return _eval(expressions, ctx, cont);
}




// returns NULL on failure
LilyObjectPtr
alistMaybeGet (LilyListPtr l, LilyObjectPtr key) {
	while (! l->isNull()) {
		LETU_AS(p, LilyPair, l->first());
		if (p->car() == key)
			return p->cdr();
		l= l->rest();
	}
	return NULL;
}


LilyObjectPtr
apply1ary(const char* procname,
	  std::function<LilyObjectPtr(LilyObjectPtr)> proc,
	  LilyListPtr* vs) {
	LETU_AS(vs0, LilyPair, *vs);
	if (vs0) {
		LETU_AS(vs1, LilyNull, vs0->_cdr);
		if (vs1) {
			return proc(vs0->_car);
		}
	}
	throw std::logic_error(STR(procname << " needs 1 argument"));
}


// just casting...
static LilyListPtr frameExpressionsList(const LilyContinuationFramePtr& frame) {
	LET_AS(expressions, LilyList, frame->expressions());
	if (!expressions)
		throw std::logic_error("ill-formed special form");
	return expressions;
}

LilyObjectPtr eval(LilyObjectPtr code,
		   LilyListPtr ctx,
		   LilyListPtr cont) {
	LilyObjectPtr acc;
	while (true) {
	eval:
		DEBUGWARN("eval: "<< show(code) << " in: " << show(cont));
		switch (code->evalId) {
		case LilyEvalOpcode::Null:
			throw std::logic_error("empty call");
			break;
		case LilyEvalOpcode::Pair: {
			LETU_AS(p, LilyPair, code);
			// Function, macro or evaluator (base syntax)
			// application; the type of the head element
			// determines which kind. Currently, given
			// we're run-time phase only anyway, implement
			// as Fexpr, i.e. allow head to be a sub-form
			// that calculates the syntax to be used. Use
			// of this feature is completely *deprecated*
			// though, it *will* go away (since it is
			// prohibiting efficiency gains by separating
			// compilation from run-time phase).

			// So, in this implementation, make the
			// continuation of even syntactical work
			// visible to Scheme (via first-class
			// continuation access)
			cont= LIST_CONS(FRAME(NULL, NIL, p->rest()), cont);
			code= p->first();
			// need to look at code again, but don't have
			// acc to use from this iteration, hence short
			// it:
			goto eval;
		}
		case LilyEvalOpcode::Symbol:
			acc= alistMaybeGet(ctx, code);
			if (!acc)
				throw std::logic_error(STR("variable not bound: " << show(code)));
			break;

			// self-evaluating cases
		case LilyEvalOpcode::Void:
		case LilyEvalOpcode::Boolean:
		case LilyEvalOpcode::String:
		case LilyEvalOpcode::Int64:
		case LilyEvalOpcode::Fractional64:
		case LilyEvalOpcode::Double:
			acc= code;
			break;
			
			// invalid to evaluate (arbitrary vs. above?)
		case LilyEvalOpcode::NativeProcedure:
		case LilyEvalOpcode::ParseError:
			throw std::logic_error(STR("ill-formed expression: "
						   << show(code)));
		default:
			throw std::logic_error("bug: unknown opcode");
		}
	next_cont:
		// Who to pass the value to?
		if (cont->isNull()) {
			// pass it back to C++
			break;
		} else {
			// pass it to the Lily continuation (which is
			// two-level, a list of frames and then in
			// each frames, in the case of function
			// application, a list of values to be
			// calculated then called, in the case of
			// macros or base syntax the unevaluated
			// arguments are used hence different
			// continuation created.

			LET_AS(frame, LilyContinuationFrame, cont->first());
			cont= cont->rest();
			bool accIsHead= ! frame->maybeHead();
			if (accIsHead) {
				// acc contains the evaluated
				// head. Now we know whether it is a
				// function, macro or evaluator
				// application.
				LET_AS(evaluator, LilyNativeEvaluator, acc);
				if (evaluator) {
					auto expressions= frameExpressionsList(frame);
					acc= evaluator->_eval
						(&expressions, &ctx, &cont);
					// ^ ditto XX
					DEBUGWARN("evaluator returned: "<<show(acc)
					     <<", while cont="<<show(cont));
					// pass acc to cont
					goto next_cont;
				}
				LET_AS(expander, LilyMacroexpander, acc);
				if (expander) {
					auto expressions= frameExpressionsList(frame);
					// XX missing a reference to
					// the original surrounding
					// list here!
					code = expander->call
						(&expressions, &ctx, &cont);
					// ^ now C++ frame there.! XX
					goto eval;
				}
				// otherwise it's a function application;
				// pass acc to cont
			}
			// pass_to_cont:
			LilyObjectPtr head= accIsHead ? acc : frame->maybeHead();
			LET_AS(expressions, LilyList, frame->expressions());
			auto rvalues= accIsHead ? frame->rvalues()
				: LIST_CONS(acc, frame->rvalues());
			if (expressions->isNull()) {
				// ready to call the continuation
				DEBUGWARN("ready to call the continuation");
				LilyListPtr arguments= reverse(rvalues);
				LETU_AS(f, LilyCallable, head);
				if (!f)
					throw std::logic_error
						(STR("not a function: " <<
						     show(head)));
				acc= f->call(&arguments, &ctx, &cont);
				DEBUGWARN("after finishing the continuation frame, acc="
				     << show(acc));
				// what's next?
				goto next_cont;
			} else {
				// update continuation (XX optim:
				// mutate if refcount is 1 (and no
				// weak refs)?)
				cont= LIST_CONS(FRAME(head,
						      rvalues,
						      expressions->rest()),
						cont);
				code= expressions->first();
			}
		}
	}
	DEBUGWARN("eval is finished, returning " << show(acc));
	return acc;
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

std::string show(const LilyObjectPtr& v) {
	std::ostringstream s;
	v->onelinePrint(s);
	return s.str();
}

// XX completely stupid copy-paste; use templates?
std::string show(LilyObject* v) {
	std::ostringstream s;
	v->onelinePrint(s);
	return s.str();
}
