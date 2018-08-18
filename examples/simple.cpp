#include <lily.hpp>
#include <lilyConstruct.hpp>

int main () {
	LilyObjectPtr n= LILY_NEW(LilyInt64(-4113));
	PRINT(n);
	PRINT(CONS(INT(10), CONS(INT(20), NIL)));
	PRINT(CONS(INT(10), INT(20)));
	PRINT(CONS(INT(10), CONS(INT(20), INT(30))));
	PRINT(CONS(INT(10), CONS(STRING("20"), INT(30))));
	PRINT(CONS(INT(10), STRING("Hello\nWorld, \"all \\fine\"")));
	PRINT(CONS(FALSE, TRUE));
	PRINT(CONS(CONS(FALSE, TRUE), TRUE));
	PRINT(CONS(SYMBOL("10"),
		   CONS(SYMBOL("println!"),
			SYMBOL("Hello\nWorld, \"all | \\| \\fine\""))));
	PRINT(CONS(NIL,NIL));
}

