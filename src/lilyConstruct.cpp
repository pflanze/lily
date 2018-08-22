#include "lilyConstruct.hpp"

LilyObjectPtr WRITELN(LilyObjectPtr v, std::ostream& out) {
	v->onelinePrint(out);
	out << "\n";
	return VOID;
}

LilyObjectPtr WRITELN(LilyObjectPtr v) {
	return WRITELN(v, std::cout);
}


LilyListPtr _LIST(std::initializer_list<LilyObjectPtr> vs) {
	LilyListPtr res= NIL;
	// std::initializer_list does not offer reverse() nor rbegin
	// nor indices in C++11. So use reversion afterwards.
	for (auto v : vs) {
		res= LIST_CONS(v, res);
	}
	return reverse(res);
}

