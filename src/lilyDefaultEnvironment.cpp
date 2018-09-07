#include "lily.hpp"
#include "lilyConstruct.hpp"
#include "lilyDefaultEnvironment.hpp"


template <typename LilyT, typename T>
static
T _lilyFold(LilyList* vs, std::function<T(LilyT*,T)> fn, T start) {
	T res= start;
	while (true) {
		if (auto pair= dynamic_cast<LilyPair*>(vs)) {
			if (auto v=  UNWRAP_AS(LilyT, pair->_car)) {
				res= fn(v, res);
				vs= LIST_UNWRAP(pair->_cdr);
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

#define DEF_FOLD_UP_NATIVE(name, LilyT, T, OP, START)			\
	static								\
	LilyObjectPtr name(LilyListPtr* vs,				\
			   LilyListPtr* _ctx,				\
			   LilyListPtr* _cont) {			\
		return LILY_NEW						\
			(LilyT(_lilyFold<LilyT, T>			\
			       (XLIST_UNWRAP(*vs),			\
				[](LilyT* num, T res) -> T {		\
				       /* XX check for overflow! */	\
				       return res OP num->value;	\
			        },					\
				START)));				\
	}

DEF_FOLD_UP_NATIVE(lilyAdd, LilyInt64, int64_t, +, 0);
DEF_FOLD_UP_NATIVE(lilyMult, LilyInt64, int64_t, *, 1);

// LONESTART is used when there's only one argument
#define DEF_FOLD_DOWN_NATIVE(name, LilyT, T, OP, LONESTART)		\
	static								\
	LilyObjectPtr name(LilyListPtr* vs,				\
			   LilyListPtr* _ctx,				\
			   LilyListPtr* _cont) {			\
	auto fn= [](LilyT* num, T res) -> T {				\
		/* XX check for overflow? */				\
		return res OP num->value;				\
	};								\
	LilyList* _vs= &**vs;						\
	if (is_LilyNull(_vs))						\
		throw std::logic_error(#OP ": wrong number of arguments"); \
	T res;								\
	LilyList* r= &*(_vs->rest());					\
	if (is_LilyNull(r))						\
		res= fn(XUNWRAP_AS(LilyT, _vs->first()), LONESTART);	\
	else								\
		res= _lilyFold<LilyT, T>				\
			(r, fn, XUNWRAP_AS(LilyT, _vs->first())->value); \
	return LILY_NEW(LilyT(res));					\
	}

DEF_FOLD_DOWN_NATIVE(lilySub, LilyInt64, int64_t, -, 0);
DEF_FOLD_DOWN_NATIVE(lilyQuotient, LilyInt64, int64_t, /, 1);
DEF_FOLD_DOWN_NATIVE(lilyRemainder, LilyInt64, int64_t, %, 1);

// DEF_NATIVE(lilyDiv, LilyInt64, int64_t, /); // generic


static
LilyObjectPtr lilyCons(LilyListPtr* vs,
		       LilyListPtr* _ctx,
		       LilyListPtr* _cont) {
	// WARN("cons: "<<show(vs));
	LETU_AS(vs0, LilyPair, *vs);
	if (vs0) {
		LETU_AS(vs1, LilyPair, vs0->_cdr);
		if (vs1) {
			LETU_AS(vs2, LilyNull, vs1->_cdr);
			if (vs2) {
				return CONS(vs0->_car, vs1->_car);
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
				return p->_car;
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
				return p->_cdr;
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
	// arguments == ((12 13 14))
	LET_AS(_arguments, LilyList, *arguments);
	if (is_LilyNull(&*(_arguments->rest()))) {
		// we have just one argument, cool.
		auto l= _arguments->first(); // (12 13 14) or "hello" or something
		int64_t len= 0;
		while (true) {
			LET_AS(p, LilyPair, l);
			if (p) {
				len++; // check overflow? 64bit int is pretty large though :)
				l= p->cdr();
			} else {
				LET_AS(null, LilyNull, l);
				if (null) {
					break;
				} else {
					throw std::logic_error
						(STR("not a list: "
						     << show(_arguments->first())));
				}
			}
		}
		return INT(len);
	} else {
		throw std::logic_error("length received more than one argument");
	}
		
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

#define ENTRY(str, proc) PAIR(SYMBOL(str, false), NATIVE_PROCEDURE(proc, str))

LilyListPtr lilyDefaultEnvironment() {
	static LilyListPtr env= LIST(
		ENTRY("+", lilyAdd),
		ENTRY("*", lilyMult),
		ENTRY("-", lilySub),
		ENTRY("quotient", lilyQuotient),
		ENTRY("remainder", lilyRemainder),
		// ENTRY("integer./", lilyDiv),
		ENTRY("cons", lilyCons),
		ENTRY("car", lilyCar),
		ENTRY("cdr", lilyCdr),
		ENTRY("quote", lilyQuote),
		ENTRY("list", lilyList),
		ENTRY("length", lilyLength),
		ENTRY("reverse", lilyReverse),
		ENTRY("define", lilyDefine),
		ENTRY("begin", lilyBegin),
		);
	return env;
}


