#include "lily.hpp"
#include "lilyConstruct.hpp"
#include "lilyHelper.hpp"

#include "lilyDefaultEnvironment.hpp"

using namespace lily;
using namespace lilyConstruct;


static LilyInt64Ptr _zero= std::dynamic_pointer_cast<LilyInt64>(INT(0));
static LilyInt64Ptr _one= std::dynamic_pointer_cast<LilyInt64>(INT(1));


template <typename LilyT, typename T>
static
T _lilyFold(LilyList* vs, std::function<T(LilyT*,T)> fn, T start) {
	T res= start;
	while (true) {
		if (auto pair= dynamic_cast<LilyPair*>(vs)) {
			if (auto v=  UNWRAP_AS(LilyT, pair->car())) {
				res= fn(v, res);
				vs= LIST_UNWRAP(pair->cdr());
			} else {
				throw std::logic_error("not an integer");
			}
		} else if (dynamic_cast<LilyNull*>(vs)) {
			return res;
		} else {
			throw std::logic_error(STR("not a proper list, ending in: "
						   << show(vs)));
		}
	}
}

// Tv and Tres have to be the unwrapped types!
template <typename Tv, typename Tres>
static
std::shared_ptr<Tres>
lilyFold(LilyList* vs,
	 std::function<std::shared_ptr<Tres>(std::shared_ptr<Tv>,
					     std::shared_ptr<Tres>)> fn,
	 LilyObjectPtr start) {
	// assert(start);
	auto res= XAS<Tres>(start);
	
	while (true) {
		if (auto pair= dynamic_cast<LilyPair*>(vs)) {
			res= fn(XAS<Tv>(pair->car()),
				res);
			vs= XLIST_UNWRAP(pair->cdr());
		} else if (dynamic_cast<LilyNull*>(vs)) {
			return res;
		} else {
			throw std::logic_error(STR("not a proper list, ending in: "
						   << show(vs)));
		}
	}
}


#define DEF_FOLD_UP_NATIVE(name, _opschemename, Tv, Tres, OP, START)	\
	static								\
	LilyObjectPtr name(LilyListPtr* vs,				\
			   LilyListPtr* _ctx,				\
			   LilyListPtr* _cont) {			\
		return lilyFold<Tv, Tres>(XLIST_UNWRAP(*vs),		\
					  OP,				\
					  START);			\
	}

DEF_FOLD_UP_NATIVE(lilyAdd, "+", LilyNumber, LilyNumber,
		   [](LilyNumberPtr v, LilyNumberPtr res) -> LilyNumberPtr {
			   return res->add(v);
		   }, _zero);
DEF_FOLD_UP_NATIVE(lilyMult, "*", LilyNumber, LilyNumber,
		   [](LilyNumberPtr v, LilyNumberPtr res) -> LilyNumberPtr {
			   return res->multiply(v);
		   }, _one);

// LONESTART is used when there's only one argument
#define DEF_FOLD_DOWN_NATIVE(name, opschemename, Tv, Tres, OP, LONESTART) \
	static								\
	LilyObjectPtr name(LilyListPtr* vs,				\
			   LilyListPtr* _ctx,				\
			   LilyListPtr* _cont) {			\
		auto fn= OP;						\
		LilyList* _vs= &**vs;					\
		if (is_LilyNull(_vs))					\
			throw std::logic_error(opschemename ": wrong number of arguments"); \
		LilyList* r= &*(_vs->rest());				\
		if (is_LilyNull(r))					\
			return fn(std::dynamic_pointer_cast<Tv>(_vs->first()), \
				  std::dynamic_pointer_cast<Tres>(LONESTART)); \
		else							\
			return lilyFold<Tv,Tres>(r, fn, _vs->first());	\
	}

DEF_FOLD_DOWN_NATIVE(lilySub, "-", LilyNumber, LilyNumber,
		     [](LilyNumberPtr v, LilyNumberPtr res) -> LilyNumberPtr {
			     return res->subtract(v);
		     }, _zero);
// XX Gambit allows inexact integers here !
DEF_FOLD_DOWN_NATIVE(lilyQuotient, "quotient", LilyInt64, LilyInt64,
		     [](LilyInt64Ptr v, LilyInt64Ptr res) -> LilyInt64Ptr {
			     return INT(lily_quotient(res->value(),
						      v->value()));
		     }, _one);
DEF_FOLD_DOWN_NATIVE(lilyRemainder, "remainder", LilyInt64, LilyInt64,
		     [](LilyInt64Ptr v, LilyInt64Ptr res) -> LilyInt64Ptr {
			     return INT(lily_remainder(res->value(),
						       v->value()));
		     }, _one);
DEF_FOLD_DOWN_NATIVE(lilyModulo, "modulo", LilyInt64, LilyInt64,
		     [](LilyInt64Ptr v, LilyInt64Ptr res) -> LilyInt64Ptr {
			     return INT(lily_modulo(res->value(),
						    v->value()));
		     }, _one);

// inputs must be integers, but result can be fractionals.
// XX also check the type of the start value
DEF_FOLD_DOWN_NATIVE(lilyIntegerDiv, "integer./", LilyInt64, LilyNumber,
		     [](LilyInt64Ptr v, LilyNumberPtr res) -> LilyNumberPtr {
			     return res->divideBy(v);
		     }, _one);

// XX also check the type of the start value
DEF_FOLD_DOWN_NATIVE(lilyDoubleDiv, "double./", LilyDouble, LilyDouble,
		     [](LilyDoublePtr v, LilyDoublePtr res) -> LilyDoublePtr {
			     // return res->divideBy(v);
			     // that's giving LilyNumberPtr, just do it directly?
			     return DOUBLE(res->value() / v->value());
		     }, _one);

DEF_FOLD_DOWN_NATIVE(lilyDiv, "/", LilyNumber, LilyNumber,
		     [](LilyNumberPtr v, LilyNumberPtr res) -> LilyNumberPtr {
			     return res->divideBy(v);
		     }, _one);


LilyStringPtr _emptyString= STRING("");

DEF_FOLD_UP_NATIVE(lilyStringAppend, "string-append", LilyString, LilyString,
		   [](LilyStringPtr v, LilyStringPtr res) -> LilyStringPtr {
			   return STRING(res->value() + v->value());
		   }, _emptyString);


static LilyObjectPtr
lilyExactInexact(LilyListPtr* vs,
		 LilyListPtr* _ctx,
		 LilyListPtr* _cont) {
	return apply1<LilyNumber>("exact->inexact",
				  [](LilyNumberPtr v) -> LilyObjectPtr {
			// XX optim?: return v if already a double
			return DOUBLE(v->toDouble());
		}, vs);
}

static
LilyObjectPtr lilyCons(LilyListPtr* vs,
		       LilyListPtr* _ctx,
		       LilyListPtr* _cont) {
	// WARN("cons: "<<show(vs));
	IF_LETU_AS(vs0, LilyPair, *vs) {
		IF_LETU_AS(vs1, LilyPair, vs0->cdr()) {
			IF_LETU_AS(vs2, LilyNull, vs1->cdr()) {
				return CONS(vs0->car(), vs1->car());
			}
		}
	}
	throw std::logic_error("cons needs 2 arguments");
}

static LilyObjectPtr lilyCar(LilyListPtr* vs,
			     LilyListPtr* _ctx,
			     LilyListPtr* _cont) {
	return apply1<LilyPair>("car", [](LilyPairPtr p) {
			return p->car();
		}, vs);
}

static LilyObjectPtr lilyCdr(LilyListPtr* vs,
			     LilyListPtr* _ctx,
			     LilyListPtr* _cont) {
	return apply1<LilyPair>("cdr", [](LilyPairPtr p) {
			return p->cdr();
		}, vs);
}

static LilyObjectPtr lilyQuote(LilyListPtr* es,
			       LilyListPtr* _ctx,
			       LilyListPtr* _cont) {
	return apply1ary("quote", [](LilyObjectPtr e) {
			DEBUGWARN("quote: " << show(e));
			return e;
		}, es);
}

static LilyObjectPtr lilyLength(LilyListPtr* arguments,
				LilyListPtr* _ctx,
				LilyListPtr* _cont) {
	return apply1ary("length", [&](LilyObjectPtr l) {
			int64_t len= 0;
			while (true) {
				IF_LET_AS(p, LilyPair, l) {
					len++;
					// ^ check overflow? 64bit int
					// is pretty large though :)
					l= p->cdr();
					// XX optim: take pointers
					// instead (avoid
					// refcounting)? Measure!
				} else {
					IF_LET_AS(null, LilyNull, l) {
						break;
					} else {
						throw std::logic_error
							(STR("not a list: "
							     << show((*arguments)->first())));
					}
				}
			}
			return INT(len);
		}, arguments);
}

static LilyObjectPtr lilyList(LilyListPtr* arguments,
			      LilyListPtr* _ctx,
			      LilyListPtr* _cont) {
	return *arguments;
}

static LilyObjectPtr lilyReverse(LilyListPtr* arguments,
				 LilyListPtr* _ctx,
				 LilyListPtr* _cont) {
	return apply1ary("reverse", [](LilyObjectPtr v) {
			return reverse(v);
		}, arguments);
}

static LilyObjectPtr lilyStringToList(LilyListPtr* arguments,
				      LilyListPtr* _ctx,
				      LilyListPtr* _cont) {
	return apply1<LilyString>("string->list", [](LilyStringPtr s) {
			auto str= s->value();
			LilyListPtr res= NIL;
			for (auto i= str.rbegin(); i != str.rend(); i++) {
				unsigned char uc= *i; // XX unicode
				res= CONS(CHAR(uc), res);
			}
			return res;
		}, arguments);
}

static LilyObjectPtr lilyListToString(LilyListPtr* arguments,
				      LilyListPtr* _ctx,
				      LilyListPtr* _cont) {
	return apply1<LilyList>("list->string", [](LilyListPtr l) {
			std::string str;
			while (! (l->isNull())) {
				auto c= XAS<LilyChar>(l->first())->value();
				str.push_back(c); // XX unicode
				l= l->rest();
			}
			return STRING(str);
		}, arguments);
}

static LilyObjectPtr lilyIntegerToChar(LilyListPtr* arguments,
				       LilyListPtr* _ctx,
				       LilyListPtr* _cont) {
	return apply1<LilyInt64>("integer->char", [](LilyInt64Ptr i) {
			// XX check for correct range
			return CHAR(i->value());
		}, arguments);
}

static LilyObjectPtr lilyCharToInteger(LilyListPtr* arguments,
				       LilyListPtr* _ctx,
				       LilyListPtr* _cont) {
	return apply1<LilyChar>("char->integer", [](LilyCharPtr c) {
			return INT(c->value());
		}, arguments);
}

static LilyObjectPtr lilySysAllocationCounts(LilyListPtr* arguments,
					     LilyListPtr* _ctx,
					     LilyListPtr* _cont) {
	return apply0("sys:allocation-counts", []() {
			auto a= lilyAllocationCount();
			auto d= lilyDeallocationCount();
			return LIST(INT(a), INT(d));
		}, arguments);
}

static LilyObjectPtr lilySysObjectCount(LilyListPtr* arguments,
					LilyListPtr* _ctx,
					LilyListPtr* _cont) {
	return apply0("sys:object-count", []() {
			auto a= lilyAllocationCount();
			auto d= lilyDeallocationCount();
			return INT(a-d);
		}, arguments);
}

static LilyObjectPtr lilyToCode(LilyListPtr* arguments,
				LilyListPtr* _ctx,
				LilyListPtr* _cont) {
	return apply1<LilyObject>(".code", toCode, arguments);
}

static LilyObjectPtr lilyIsList(LilyListPtr* arguments,
				LilyListPtr* _ctx,
				LilyListPtr* _cont) {
	return apply1<LilyObject>("list?", [](LilyObjectPtr v) {
			return lily::isList(v) ? TRUE : FALSE;
		}, arguments);
}


static LilyObjectPtr lilyInc(LilyListPtr* arguments,
			     LilyListPtr* _ctx,
			     LilyListPtr* _cont) {
	return apply1<LilyInt64>("inc", [](LilyInt64Ptr v) {
			return INT(lily_add(v->value(), 1));
		}, arguments);
}
static LilyObjectPtr lilyDec(LilyListPtr* arguments,
			     LilyListPtr* _ctx,
			     LilyListPtr* _cont) {
	return apply1<LilyInt64>("dec", [](LilyInt64Ptr v) {
			return INT(lily_sub(v->value(), 1));
		}, arguments);
}


static LilyObjectPtr lilyFoldRight(LilyListPtr* arguments,
				   LilyListPtr* ctx,
				   LilyListPtr* cont) {
	return apply3<LilyCallable, LilyObject, LilyList>
		("fold-right", [&](LilyCallablePtr fn,
				   LilyObjectPtr start,
				   LilyListPtr l) {
			return lily::fold_right(
				[&](LilyObjectPtr v,
				    LilyObjectPtr res) {
					auto args= LIST(v, res);
					return fn->call(&args, // well
							ctx,
							cont);
				},
				start,
				l);
							
		}, arguments);
}

// almost-copy-paste of above
static LilyObjectPtr lilyImproperFoldRight(LilyListPtr* arguments,
					   LilyListPtr* ctx,
					   LilyListPtr* cont) {
	return apply3<LilyCallable, LilyObject, LilyObject>
		("improper-fold-right",
		 [&](LilyCallablePtr fn,
		     LilyObjectPtr start,
		     LilyObjectPtr v) {
			return lily::improper_fold_right(
				[&](LilyObjectPtr v,
				    LilyObjectPtr res) {
					auto args= LIST(v, res);
					return fn->call(&args, // well
							ctx,
							cont);
				},
				start,
				v);
							
		}, arguments);
}


static LilyObjectPtr lilyMap(LilyListPtr* arguments,
			     LilyListPtr* ctx,
			     LilyListPtr* cont) {
	return apply2<LilyCallable, LilyList>
		("map",
		 [&](LilyCallablePtr fn,
		     LilyListPtr l) {
			return lily::map(
				[&](LilyObjectPtr v) {
					auto args= LIST(v);
					return fn->call(&args, // well
							ctx,
							cont);
				},
				l);
							
		}, arguments);
}

// almost-copy
static LilyObjectPtr lilyImproperToProperMap(LilyListPtr* arguments,
					     LilyListPtr* ctx,
					     LilyListPtr* cont) {
	return apply2<LilyCallable, LilyObject>
		("improper->proper-map",
		 [&](LilyCallablePtr fn,
		     LilyObjectPtr v) {
			return lily::improper_to_proper_map(
				[&](LilyObjectPtr v) {
					auto args= LIST(v);
					return fn->call(&args, // well
							ctx,
							cont);
				},
				v);
							
		}, arguments);
}



static LilyObjectPtr lilyDefine(LilyListPtr* es,
				LilyListPtr* ctx,
				LilyListPtr* cont) {
	LET_AS(es0, LilyPair, *es);
	if (!es0)
		throw std::logic_error("define needs at least 1 argument");
	// match 1st argument
	auto var_or_pair= es0->car();
	IF_LET_AS(var, LilySymbol, var_or_pair) {
		LET_AS(es1, LilyPair, es0->cdr());
		if (!es1)
			throw std::logic_error("define: if getting a symbol as 1st argument, need one more argument");
		LET_AS(end, LilyNull, es1->cdr());
		if (!end)
			throw std::logic_error("define: if getting a symbol as 1st argument, accepting just one more argument");
		auto expr= es1->car();
		// ç
	}
	IF_LET_AS(bindform, LilyPair, var_or_pair) {
		IF_LET_AS(var, LilySymbol, bindform->first()) {
			throw std::logic_error("XX functions not yet implemented");
		} else {
			throw std::logic_error(STR("define: expecting symbol, got: "
						   << show(bindform->first())));
		}
	}

	throw std::logic_error("define needs a symbol as the first argument");
	const LilyObjectPtr& _es1= es0->cdr();
	IF_LET_AS(es1, LilyPair, _es1) {
		//const LilyObjectPtr& 
	} else {
		// set variable to void (or remove it altogether?)
		//XXX
	}
}


static LilyObjectPtr lilyBegin(LilyListPtr* es,
			       LilyListPtr* ctx,
			       LilyListPtr* cont) {
	// evaluate all es in turn; drop all results before the last
	return NIL;
}

LilyListPtr lilyDefaultEnvironment() {
	static LilyListPtr env= LIST(
		NATIVE_PROCEDURE_BINDING("inc", lilyInc),
		NATIVE_PROCEDURE_BINDING("dec", lilyDec),
		NATIVE_PROCEDURE_BINDING("+", lilyAdd),
		NATIVE_PROCEDURE_BINDING("*", lilyMult),
		NATIVE_PROCEDURE_BINDING("-", lilySub),
		NATIVE_PROCEDURE_BINDING("/", lilyDiv),
		NATIVE_PROCEDURE_BINDING("quotient", lilyQuotient),
		NATIVE_PROCEDURE_BINDING("remainder", lilyRemainder),
		NATIVE_PROCEDURE_BINDING("modulo", lilyModulo),
		NATIVE_PROCEDURE_BINDING("integer./", lilyIntegerDiv),
		NATIVE_PROCEDURE_BINDING("double./", lilyDoubleDiv),
		NATIVE_PROCEDURE_BINDING("cons", lilyCons),
		NATIVE_PROCEDURE_BINDING("car", lilyCar),
		NATIVE_PROCEDURE_BINDING("first", lilyCar),
		NATIVE_PROCEDURE_BINDING("cdr", lilyCdr),
		NATIVE_PROCEDURE_BINDING("rest", lilyCdr),
		NATIVE_EVALUATOR_BINDING("quote", lilyQuote),
		NATIVE_PROCEDURE_BINDING("list", lilyList),
		NATIVE_PROCEDURE_BINDING("length", lilyLength),
		NATIVE_PROCEDURE_BINDING("reverse", lilyReverse),
		NATIVE_EVALUATOR_BINDING("define", lilyDefine),
		NATIVE_EVALUATOR_BINDING("begin", lilyBegin),
		NATIVE_PROCEDURE_BINDING("exact->inexact", lilyExactInexact),
		NATIVE_PROCEDURE_BINDING("string->list", lilyStringToList),
		NATIVE_PROCEDURE_BINDING("list->string", lilyListToString),
		NATIVE_PROCEDURE_BINDING("integer->char", lilyIntegerToChar),
		NATIVE_PROCEDURE_BINDING("char->integer", lilyCharToInteger),
		NATIVE_PROCEDURE_BINDING("string-append", lilyStringAppend),
		NATIVE_PROCEDURE_BINDING("sys:allocation-counts", lilySysAllocationCounts),
		NATIVE_PROCEDURE_BINDING("sys:object-count", lilySysObjectCount),
		NATIVE_PROCEDURE_BINDING(".code", lilyToCode),
		NATIVE_PROCEDURE_BINDING("list?", lilyIsList),
		NATIVE_PROCEDURE_BINDING("fold-right", lilyFoldRight),
		NATIVE_PROCEDURE_BINDING("improper-fold-right", lilyImproperFoldRight),
		NATIVE_PROCEDURE_BINDING("map", lilyMap),
		NATIVE_PROCEDURE_BINDING("improper->proper-map", lilyImproperToProperMap),
		);

#define DEFPRIM_ENVIRONMENT env
	DEFPRIM2(lilyStringRef, "string-ref",
		 LilyString, s, LilyInt64, i, {
			 // XX handle unicode
			 size_t len= s->value().length();
			 int64_t ii = i->value();
			 uint64_t uii= ii;
			 if ((ii < 0) || (uii >= len)) {
				 throw std::logic_error("argument 2 out of range");
			 } else {
				 auto c= s->value()[i->value()];
				 unsigned char uc= c; // XX unicode
				 return CHAR(uc);
			 }
		 });
	DEFPRIM2(lilyApply, "apply",
		 LilyCallable, proc, LilyList, args, {
			 // XX TCO? (It does tail-call the cont? But
			 // not the C++ stack *?*)
			 auto argslist= XAS<LilyList>(args);
			 return proc->call(&argslist, __ctx, __cont);
		 });
	return env;
}


