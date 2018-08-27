#ifndef _LILYCONSTRUCT_HPP
#define _LILYCONSTRUCT_HPP

#include <iostream>
#include <cstdarg>
#include "lily.hpp"

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
#define NATIVE_PROCEDURE(a, name) LILY_NEW(LilyNativeProcedure(a, name))
#define NATIVE_MACROEXPANDER(a, name) LILY_NEW(LilyNativeMacroexpander(a, name))
#define NATIVE_EVALUATOR(a, name) LILY_NEW(LilyNativeEvaluator(a, name))
#define FRAME(maybeHead,rvalues,expressions) LILY_NEW(LilyContinuationFrame(maybeHead,rvalues,expressions))
#define PARSEERROR(msg,pos) LILY_NEW(LilyParseError(msg,pos))

LilyObjectPtr WRITELN(LilyObjectPtr v, std::ostream& out);
LilyObjectPtr WRITELN(LilyObjectPtr v);
// for GDB, since it doesn't recognize the return type of those above:
void wr(LilyObjectPtr v);

LilyListPtr _LIST(std::initializer_list<LilyObjectPtr> vs);

#define LIST(...) _LIST({ __VA_ARGS__ })


#endif
