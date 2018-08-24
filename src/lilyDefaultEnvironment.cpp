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

// #define DEF_PRIMITIVE(name, 

static
LilyObjectPtr lilyAdd(LilyObjectPtr vs) {
	//XXX XLIST_UNWRAP
	return INT((_lilyFold<LilyInt64, int64_t>
		    (XLIST_UNWRAP(vs),
		     [](LilyInt64* num, int64_t res) -> int64_t {
			    return num->value + res; // XX check for overflow!
		    }, 0)));
}

// XX stupid copypaste
static
LilyObjectPtr lilyMult(LilyObjectPtr vs) {
	int64_t total=1;
	LilyObject* p= UNWRAP(vs);
	while (true) {
		if (auto pair= dynamic_cast<LilyPair*>(p)) {
			if (auto num=  UNWRAP_AS(LilyInt64, pair->_car)) {
				total*= num->value; // XX check for overflow!
				p= UNWRAP(pair->_cdr);
			} else {
				throw std::logic_error("not an integer");
			}
		} else if (dynamic_cast<LilyNull*>(p)) {
			return INT(total);
		} else {
			throw std::logic_error("not a proper list");
		}
	}
}


LilyListPtr lilyDefaultEnvironment() {
	LilyListPtr env= LIST(
		PAIR(SYMBOL("+"), PRIMITIVE(lilyAdd, "+")),
		PAIR(SYMBOL("*"), PRIMITIVE(lilyMult, "*")),
		);
	return env;
}


