#include <lily.hpp>
#include <iostream>

int main () {
	LilyObjectPtr n= LILY_NEW(LilyInt64(-4113));
	n->onelinePrint(std::cout);
	std::cout << "\n";
	// LilyObjectPtr l=
	// 	LILY_NEW(LilyPair(
	// 			 LILY_NEW())
}
