#include <lily.hpp>
#include <lilyConstruct.hpp>

int main () {
	lily_init();
	LilyObjectPtr n= LILY_NEW(LilyInt64,(-4113));
	WRITELN(n);
	WRITELN(CONS(INT(10), CONS(INT(20), NIL)));
	WRITELN(CONS(INT(10), INT(20)));
	WRITELN(CONS(INT(10), CONS(INT(20), INT(30))));
	WRITELN(CONS(INT(10), CONS(STRING("20"), INT(30))));
	WRITELN(CONS(INT(10), STRING("Hello\nWorld, \"all \\fine\"")));
	auto ft= CONS(FALSE, TRUE);
	auto ft2= CONS(ft, TRUE);
	WRITELN(ft);
	WRITELN(ft2);
	// XX really should provide a SYMBOL constructor that works
	// without the second argument.
	WRITELN(CONS(SYMBOL("10", true),
		     CONS(SYMBOL("println!", false),
			  SYMBOL("Hello\nWorld, \"all | \\| \\fine\"", true))));
	WRITELN(WRITELN(CONS(NIL,NIL)));
	WRITELN(LIST(NIL,FALSE,TRUE,SYMBOL("", true)));
}

