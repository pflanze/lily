#include "lilyDefaultEnvironment.hpp"
#include "lilyConstruct.hpp"

static
LilyObjectPtr lilyAdd(LilyObjectPtr vs) {
	int64_t total=0;
	LilyObject* p= UNWRAP(vs);
	while (true) {
		if (auto pair= dynamic_cast<LilyPair*>(p)) {
			if (auto num=  UNWRAP_AS(LilyInt64, pair->_car)) {
				total+= num->value; // XX check for overflow!
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
		);
	return env;
}




