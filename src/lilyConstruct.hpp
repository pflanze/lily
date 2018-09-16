#ifndef _LILYCONSTRUCT_HPP
#define _LILYCONSTRUCT_HPP

#include <iostream>
#include <cstdarg>
#include "lily.hpp"

#define LILY_NEW(class, arguments)			\
	std::shared_ptr<class>(new class arguments)

namespace lilyConstruct {

	// provide both ?
	inline LilyListPtr CONS(LilyObjectPtr a, LilyObjectPtr b) {
		return LILY_NEW(LilyPair,(a,b));
	}
	inline LilyListPtr PAIR(LilyObjectPtr a, LilyObjectPtr b) {
		return LILY_NEW(LilyPair,(a,b));
	}
	// /both
	static LilyNullPtr NIL = LilyNull::singleton();
	#ifdef TRUE
	#  undef TRUE
	#endif
	static LilyBooleanPtr TRUE = LilyBoolean::True();
	#ifdef FALSE
	#  undef FALSE
	#endif
	static LilyBooleanPtr FALSE = LilyBoolean::False();
	static LilyVoidPtr VOID = LilyVoid::singleton();
	inline LilyInt64Ptr INT(int64_t a) {
		return LILY_NEW(LilyInt64,(a));
	}
	inline LilyFractional64Ptr FRACTIONAL(int64_t n, int64_t d) {
		return LILY_NEW(LilyFractional64,(n,d));
	}
	inline LilyDoublePtr DOUBLE(double a) {
		return  LILY_NEW(LilyDouble,(a));
	}
	inline LilyStringPtr STRING(std::string a) {
		return LILY_NEW(LilyString,(a));
	}

	inline LilyCharPtr CHAR(lily_char_t c) {
		return LILY_NEW(LilyChar,(c));
	}

	inline LilySymbollikePtr SYMBOL(std::string str,
					bool mayNeedQuoting = true) {
		return LilySymbol::intern(str, mayNeedQuoting);
	}

	inline LilySymbollikePtr KEYWORD(std::string str,
					 bool mayNeedQuoting = true) {
		return LilyKeyword::intern(str, mayNeedQuoting);
	}

	inline LilyBooleanPtr BOOLEAN(bool a) {
		return ((a) ? TRUE : FALSE);
	}
	inline LilyNativeProcedurePtr NATIVE_PROCEDURE(LilyNative_t proc,
						       const char* name) {
		return LILY_NEW(LilyNativeProcedure,(proc, name));
	}
	inline LilyNativeMacroexpanderPtr
	NATIVE_MACROEXPANDER(LilyNative_t expander,
			     const char* name) {
		return LILY_NEW(LilyNativeMacroexpander,(expander, name));
	}
	inline LilyNativeEvaluatorPtr NATIVE_EVALUATOR(LilyNative_t eval,
				const char* name) {
		return LILY_NEW(LilyNativeEvaluator,(eval, name));
	}
	inline LilyContinuationFramePtr FRAME(LilyObjectPtr maybeHead,
					      LilyListPtr rvalues,
					      LilyListPtr expressions) {
		return LILY_NEW(LilyContinuationFrame,
				(maybeHead,rvalues,expressions));
	}
	inline LilyParseErrorPtr PARSEERROR(const std::string msg,
					    parse_position_t pos) {
		return LILY_NEW(LilyParseError,(msg,pos));
	}

	LilyObjectPtr WRITELN(LilyObjectPtr v, std::ostream& out);
	LilyObjectPtr WRITELN(LilyObjectPtr v);
	// for GDB, since it doesn't recognize the return type of those above:
	void wr(LilyObjectPtr v);

	LilyListPtr _LIST(std::initializer_list<LilyObjectPtr> vs);

}

// XX no way to put LIST into a namespace, right?
#define LIST(...) lilyConstruct::_LIST({ __VA_ARGS__ })


#endif
