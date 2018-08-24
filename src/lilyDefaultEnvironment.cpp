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

#define DEF_UP_PRIMITIVE(name, LilyT, T, OP, START)			\
	static								\
	LilyObjectPtr name(LilyObjectPtr vs) {				\
		return LILY_NEW						\
			(LilyT(_lilyFold<LilyT, T>			\
			       (XLIST_UNWRAP(vs),			\
				[](LilyT* num, T res) -> T {		\
				       /* XX check for overflow! */	\
				       return res OP num->value;	\
			        },					\
				START)));				\
	}

DEF_UP_PRIMITIVE(lilyAdd, LilyInt64, int64_t, +, 0);
DEF_UP_PRIMITIVE(lilyMult, LilyInt64, int64_t, *, 1);

#define DEF_DOWN_PRIMITIVE(name, LilyT, T, OP)				\
	static								\
	LilyObjectPtr name(LilyObjectPtr vs) {				\
		LilyList* _vs= XLIST_UNWRAP(vs);			\
		return LILY_NEW						\
			(LilyT(_lilyFold<LilyT, T>			\
			       (XLIST_UNWRAP(_vs->rest()),		\
				[](LilyT* num, T res) -> T {		\
				       /* XX check for overflow? */	\
				       return res OP num->value;	\
			        },					\
				XUNWRAP_AS(LilyT, _vs->first())->value))); \
	}

DEF_DOWN_PRIMITIVE(lilySub, LilyInt64, int64_t, -);
DEF_DOWN_PRIMITIVE(lilyQuotient, LilyInt64, int64_t, /);
DEF_DOWN_PRIMITIVE(lilyRemainder, LilyInt64, int64_t, %);

// DEF_PRIMITIVE(lilyDiv, LilyInt64, int64_t, /); // generic


static
LilyObjectPtr lilyCons(LilyObjectPtr vs) {
	// WARN("cons: "<<show(vs));
	LETU_AS(vs0, LilyPair, vs);
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

static
LilyObjectPtr apply1ary(const char* procname,
			std::function<LilyObjectPtr(LilyObjectPtr)> proc,
			LilyObjectPtr vs) {
	LETU_AS(vs0, LilyPair, vs);
	if (vs0) {
		LETU_AS(vs1, LilyNull, vs0->_cdr);
		if (vs1) {
			return proc(vs0->_car);
		}
	}
	throw std::logic_error(STR(procname << " needs 1 argument"));
}

static LilyObjectPtr lilyCar(LilyObjectPtr vs) {
	return apply1ary("car", [](LilyObjectPtr v) {
			LETU_AS(p, LilyPair, v);
			if (p)
				return p->_car;
			else
				throw std::logic_error(STR("not a pair: "
							   << show(v)));
		}, vs);
}

static LilyObjectPtr lilyCdr(LilyObjectPtr vs) {
	return apply1ary("cdr", [](LilyObjectPtr v) {
			LETU_AS(p, LilyPair, v);
			if (p)
				return p->_cdr;
			else
				throw std::logic_error(STR("not a pair: "
							   << show(v)));
		}, vs);
}

LilyListPtr lilyDefaultEnvironment() {
	LilyListPtr env= LIST(
		PAIR(SYMBOL("+"), PRIMITIVE(lilyAdd, "+")),
		PAIR(SYMBOL("*"), PRIMITIVE(lilyMult, "*")),
		PAIR(SYMBOL("-"), PRIMITIVE(lilySub, "-")),
		PAIR(SYMBOL("quotient"), PRIMITIVE(lilyQuotient, "quotient")),
		PAIR(SYMBOL("remainder"), PRIMITIVE(lilyRemainder, "remainder")),
		// PAIR(SYMBOL("integer./"), PRIMITIVE(lilyDiv, "integer./")),
		PAIR(SYMBOL("cons"), PRIMITIVE(lilyCons, "cons")),
		PAIR(SYMBOL("car"), PRIMITIVE(lilyCar, "car")),
		PAIR(SYMBOL("cdr"), PRIMITIVE(lilyCdr, "cdr")),
		);
	return env;
}


