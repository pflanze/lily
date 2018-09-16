#include "lilyConstruct.hpp"

using namespace lilyConstruct;

LilyObjectPtr lilyConstruct::WRITELN(LilyObjectPtr v, std::ostream& out) {
	v->write(out);
	out << "\n";
	return VOID;
}

LilyObjectPtr lilyConstruct::WRITELN(LilyObjectPtr v) {
	return WRITELN(v, std::cout);
}

void lilyConstruct::wr(LilyObjectPtr v) {
	WRITELN(v, std::cout);
}


LilyListPtr lilyConstruct::_LIST(std::initializer_list<LilyObjectPtr> vs) {
	LilyListPtr res= NIL;
	// std::initializer_list does not offer reverse() nor rbegin
	// nor indices in C++11. So use reversion afterwards.
	for (auto v : vs) {
		res= CONS(v, res);
	}
	return reverse(res);
}

