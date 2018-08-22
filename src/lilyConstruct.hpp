#include <iostream>
#include <cstdarg>

#define LILY_NEW(classAndArguments) \
	std::shared_ptr<LilyObject>(new classAndArguments)

#define LILY_LIST_NEW(classAndArguments)				\
	std::shared_ptr<LilyList>(new classAndArguments)

#define CONS(a,b) LILY_NEW(LilyPair(a,b))
#define LIST_CONS(a,b) LILY_LIST_NEW(LilyPair(a,b))
// provide both or rename?
#define PAIR(a,b) LILY_NEW(LilyPair(a,b))
#define LIST_PAIR(a,b) LILY_LIST_NEW(LilyPair(a,b))
// /both
#define NIL LilyNull::singleton()
#define TRUE LilyBoolean::True()
#define FALSE LilyBoolean::False()
#define VOID LilyVoid::singleton()
#define INT(a) LILY_NEW(LilyInt64(a))
#define DOUBLE(a) LILY_NEW(LilyDouble(a))
#define STRING(a) LILY_NEW(LilyString(a))
#define SYMBOL(a) LilySymbol::intern(a)
#define BOOLEAN(a) ((a) ? TRUE : FALSE)
#define PRIMITIVE(a) LILY_NEW(LilyPrimitive(a))
#define FRAME(vs,es) LILY_NEW(LilyContinuationFrame(vs,es))

static inline
LilyObjectPtr WRITELN(LilyObjectPtr v, std::ostream& out= std::cout) {
	v->onelinePrint(out);
	out << "\n";
	return VOID;
}

static inline
LilyListPtr _LIST(std::initializer_list<LilyObjectPtr> vs) {
	LilyListPtr res= NIL;
	// std::initializer_list does not offer reverse() nor rbegin
	// nor indices in C++11. So use reversion afterwards.
	for (auto v : vs) {
		res= LIST_CONS(v, res);
	}
	return reverse(res);
}

#define LIST(...) _LIST({ __VA_ARGS__ })


