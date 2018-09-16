#include "lily.hpp"
#include "lilyConstruct.hpp"
#include "lilyDefaultEnvironment.hpp"

using namespace lily;


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


static LilyObjectPtr
lilyExactInexact(LilyListPtr* vs,
		 LilyListPtr* _ctx,
		 LilyListPtr* _cont) {
	return apply1ary("exact->inexact", [](LilyObjectPtr v) {
			// XX optim?: return v if already a double
			return DOUBLE(XAS<LilyNumber>(v)->toDouble());
		}, vs);
}

static
LilyObjectPtr lilyCons(LilyListPtr* vs,
		       LilyListPtr* _ctx,
		       LilyListPtr* _cont) {
	// WARN("cons: "<<show(vs));
	LETU_AS(vs0, LilyPair, *vs);
	if (vs0) {
		LETU_AS(vs1, LilyPair, vs0->cdr());
		if (vs1) {
			LETU_AS(vs2, LilyNull, vs1->cdr());
			if (vs2) {
				return CONS(vs0->car(), vs1->car());
			}
		}
	}
	throw std::logic_error("cons needs 2 arguments");
}

static LilyObjectPtr lilyCar(LilyListPtr* vs,
			     LilyListPtr* _ctx,
			     LilyListPtr* _cont) {
	return apply1ary("car", [](LilyObjectPtr v) {
			LETU_AS(p, LilyPair, v);
			if (p)
				return p->car();
			else
				throw std::logic_error(STR("not a pair: "
							   << show(v)));
		}, vs);
}

static LilyObjectPtr lilyCdr(LilyListPtr* vs,
			     LilyListPtr* _ctx,
			     LilyListPtr* _cont) {
	return apply1ary("cdr", [](LilyObjectPtr v) {
			LETU_AS(p, LilyPair, v);
			if (p)
				return p->cdr();
			else
				throw std::logic_error(STR("not a pair: "
							   << show(v)));
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
				LET_AS(p, LilyPair, l);
				if (p) {
					len++;
					// ^ check overflow? 64bit int
					// is pretty large though :)
					l= p->cdr();
					// XX optim: take pointers
					// instead (avoid
					// refcounting)? Measure!
				} else {
					LET_AS(null, LilyNull, l);
					if (null) {
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
	return apply1ary("string->list", [](LilyObjectPtr v) {
			XLETU_AS(s, LilyString, v);
			LilyListPtr res= NIL;
			for (auto i= s->value().rbegin(); i != s->value().rend(); i++)
				res= CONS(CHAR(*i), res);
			return res;
		}, arguments);
}

static LilyObjectPtr lilyIntegerToChar(LilyListPtr* arguments,
				       LilyListPtr* _ctx,
				       LilyListPtr* _cont) {
	return apply1ary("integer->char", [](LilyObjectPtr v) {
			XLETU_AS(i, LilyInt64, v);
			// XX check for correct range
			return CHAR(i->value());
		}, arguments);
}

static LilyObjectPtr lilyCharToInteger(LilyListPtr* arguments,
				       LilyListPtr* _ctx,
				       LilyListPtr* _cont) {
	return apply1ary("char->integer", [](LilyObjectPtr v) {
			XLETU_AS(i, LilyChar, v);
			return INT(i->value());
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
	LET_AS(var, LilySymbol, var_or_pair);
	if (var) {
		LET_AS(es1, LilyPair, es0->cdr());
		if (!es1)
			throw std::logic_error("define: if getting a symbol as 1st argument, need one more argument");
		LET_AS(end, LilyNull, es1->cdr());
		if (!end)
			throw std::logic_error("define: if getting a symbol as 1st argument, accepting just one more argument");
		auto expr= es1->car();
		// ç
	}
	LET_AS(bindform, LilyPair, var_or_pair);
	if (bindform) {
		LET_AS(var, LilySymbol, bindform->first());
		if (var) {
			throw std::logic_error("XX functions not yet implemented");
		} else {
			throw std::logic_error(STR("define: expecting symbol, got: "
						   << show(bindform->first())));
		}
	}

	throw std::logic_error("define needs a symbol as the first argument");
	const LilyObjectPtr& _es1= es0->cdr();
	LET_AS(es1, LilyPair, _es1);
	if (es1) {
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

#define ENTRY(str, proc) PAIR(SYMBOL(str), NATIVE_PROCEDURE(proc, str))

LilyListPtr lilyDefaultEnvironment() {
	static LilyListPtr env= LIST(
		ENTRY("+", lilyAdd),
		ENTRY("*", lilyMult),
		ENTRY("-", lilySub),
		ENTRY("/", lilyDiv),
		ENTRY("quotient", lilyQuotient),
		ENTRY("remainder", lilyRemainder),
		ENTRY("modulo", lilyModulo),
		ENTRY("integer./", lilyIntegerDiv),
		ENTRY("double./", lilyDoubleDiv),
		ENTRY("cons", lilyCons),
		ENTRY("car", lilyCar),
		ENTRY("cdr", lilyCdr),
		ENTRY("quote", lilyQuote),
		ENTRY("list", lilyList),
		ENTRY("length", lilyLength),
		ENTRY("reverse", lilyReverse),
		ENTRY("define", lilyDefine),
		ENTRY("begin", lilyBegin),
		ENTRY("exact->inexact", lilyExactInexact),
		ENTRY("string->list", lilyStringToList),
		ENTRY("integer->char", lilyIntegerToChar),
		ENTRY("char->integer", lilyCharToInteger),
		);
	return env;
}


