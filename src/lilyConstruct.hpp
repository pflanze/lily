#include <iostream>
#include <cstdarg>

#define LILY_NEW(classAndArguments) \
	std::shared_ptr<LilyObject>(new classAndArguments)

#define CONS(a,b) LILY_NEW(LilyPair(a,b))
#define NIL LilyNull::singleton()
#define TRUE LilyBoolean::True()
#define FALSE LilyBoolean::False()
#define VOID LilyVoid::singleton()
#define INT(a) LILY_NEW(LilyInt64(a))
#define DOUBLE(a) LILY_NEW(LilyDouble(a))
#define STRING(a) LILY_NEW(LilyString(a))
#define SYMBOL(a) LILY_NEW(LilySymbol(a))

static inline
LilyObjectPtr WRITELN(LilyObjectPtr v, std::ostream& out= std::cout) {
	v->onelinePrint(out);
	out << "\n";
	return VOID;
}

static inline
LilyObjectPtr _LIST(std::initializer_list<LilyObjectPtr> vs) {
	LilyObjectPtr res= NIL;
	// std::initializer_list does not offer reverse() nor rbegin
	// nor indices in C++11. So use reversion afterwards.
	for (auto v : vs) {
		res= CONS(v, res);
	}
	return reverse(res);
}

#define LIST(...) _LIST({ __VA_ARGS__ })


