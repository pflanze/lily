#include <iostream>

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

