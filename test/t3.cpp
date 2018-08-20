#include <lily.hpp>
#include <lilyConstruct.hpp>
#include <lilyParse.hpp>
#include <lilyDefaultEnvironment.hpp>

auto environment= lilyDefaultEnvironment();

void e(const char* codestring) {
	auto codeobject= lilyParse(std::string(codestring));
	auto result= codeobject->eval(environment);
	std::cout << result->typeName() << ": "; WRITELN(result);
}

void weirdtest(LilyListPtr mylist) {
	// (22 23 23455)

	// auto myvalue= PAIR(INT(22), PAIR(INT(23), PAIR(INT(23445),  NIL)));

	auto mylistp= LIST_UNWRAP(mylist);
	while (mylistp->isPair()) {
		//XX add layer that has 'value' method--ah can't, return type
		std::cout <<
			UNWRAP_AS(LilyInt64, mylistp->first())->value
			// XX^ ugly, should provide value method
			  << "\n";
		mylistp= LIST_UNWRAP(mylistp->rest());
	}


	WARN((new LilyPair(INT(33), INT(444)))-> isPair());

	WARN((new LilyNull())-> isPair());
}

#define IS(type, var)  !!dynamic_cast<type*>(&*(var))

// bool handdispatch_to_isPair(LilyListPtr v) {
// 	if (IS(LilyPair, v)) {
// 		return true; 
// 	} else {
// 		return false;
// 	}
// }

// bool isPair(LilyPair* v) {
// 	return true;
// }
// bool isPair(LilyNull* v) {
// 	return false;
// }
// -> doesn't actually work because C++ doesn't check the runtime type
//    information for overloaded functions, only static types.


int main () {
	weirdtest(LIST_PAIR(INT(22),
			    LIST_PAIR(INT(23),
				      LIST_PAIR(INT(23445),
						NIL))));
	e("(+)"); // 0
	e("(+ 10 20)"); // 30
	e("(+ 10 . 20)"); // error
}
