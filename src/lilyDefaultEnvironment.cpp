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

#define DEF_PRIMITIVE(name, LilyT, T, OP)				\
	static								\
	LilyObjectPtr name(LilyObjectPtr vs) {				\
		return INT((_lilyFold<LilyT, T>				\
			    (XLIST_UNWRAP(vs),				\
			     [](LilyT* num, T res) -> T {		\
				    /* XX check for overflow! */	\
				    return num->value OP res;		\
			    }, 0)));					\
	}

DEF_PRIMITIVE(lilyAdd, LilyInt64, int64_t, +);
DEF_PRIMITIVE(lilyMult, LilyInt64, int64_t, *);


LilyListPtr lilyDefaultEnvironment() {
	LilyListPtr env= LIST(
		PAIR(SYMBOL("+"), PRIMITIVE(lilyAdd, "+")),
		PAIR(SYMBOL("*"), PRIMITIVE(lilyMult, "*")),
		);
	return env;
}


