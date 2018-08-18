#include <lily.hpp>
#include <iostream>

#define CONS(a,b) LILY_NEW(LilyPair(a,b))
#define NIL LilyNull::singleton()
#define TRUE LilyBoolean::True()
#define FALSE LilyBoolean::False()
#define INT(a) LILY_NEW(LilyInt64(a))
#define DOUBLE(a) LILY_NEW(LilyDouble(a))
#define STRING(a) LILY_NEW(LilyString(a))
#define SYMBOL(a) LILY_NEW(LilySymbol(a))

void prt(LilyObjectPtr v) {
	v->onelinePrint(std::cout);
	std::cout << "\n";
}

int main () {
	LilyObjectPtr n= LILY_NEW(LilyInt64(-4113));
	prt(n);
	prt(CONS(INT(10), CONS(INT(20), NIL)));
	prt(CONS(INT(10), INT(20)));
	prt(CONS(INT(10), CONS(INT(20), INT(30))));
	prt(CONS(INT(10), CONS(STRING("20"), INT(30))));
	prt(CONS(INT(10), STRING("Hello\nWorld, \"all \\fine\"")));
	prt(CONS(FALSE, TRUE));
	prt(CONS(CONS(FALSE, TRUE), TRUE));
	prt(CONS(SYMBOL("10"),
		 CONS(SYMBOL("println!"),
		      SYMBOL("Hello\nWorld, \"all | \\| \\fine\""))));
	prt(CONS(NIL,NIL));
}
