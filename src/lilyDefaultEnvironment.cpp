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
DEF_DOWN_PRIMITIVE(lilyModulo, LilyInt64, int64_t, %);

// DEF_PRIMITIVE(lilyDiv, LilyInt64, int64_t, /); // generic


LilyListPtr lilyDefaultEnvironment() {
	LilyListPtr env= LIST(
		PAIR(SYMBOL("+"), PRIMITIVE(lilyAdd, "+")),
		PAIR(SYMBOL("*"), PRIMITIVE(lilyMult, "*")),
		PAIR(SYMBOL("-"), PRIMITIVE(lilySub, "-")),
		PAIR(SYMBOL("quotient"), PRIMITIVE(lilyQuotient, "quotient")),
		PAIR(SYMBOL("modulo"), PRIMITIVE(lilyModulo, "modulo")),
		// PAIR(SYMBOL("integer./"), PRIMITIVE(lilyDiv, "integer./")),
		);
	return env;
}


