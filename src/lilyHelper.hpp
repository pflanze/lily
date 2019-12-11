#ifndef _LILYHELPER_HPP
#define _LILYHELPER_HPP

// Helpers for defining primitives ("FFI" but defined from within C++)

// Usage:

// 1. '#define DEFPRIM_ENVIRONMENT environment' where environment is
//    your variable holding the environment to add the primitives to.

// 2. use the DEFPRIM* macros to define your primitives.


#include "lily.hpp"

#define NATIVE_PROCEDURE_BINDING(str, proc) \
	PAIR(SYMBOL(str), NATIVE_PROCEDURE(proc, str))

#define NATIVE_EVALUATOR_BINDING(str, proc) \
	PAIR(SYMBOL(str), NATIVE_EVALUATOR(proc, str))

#define BINDPRIM(k,v) DEFPRIM_ENVIRONMENT= CONS(NATIVE_PROCEDURE_BINDING(k,v),	\
						DEFPRIM_ENVIRONMENT);


inline
LilyObjectPtr
apply0(const char* procname,
       std::function<LilyObjectPtr()> proc,
       LilyListPtr* vs)
{
	if (UNWRAP_AS(LilyNull, *vs)) {
		return proc();
	}
	throw std::logic_error(STR(procname << " needs 0 arguments"));
}

template <typename A>
LilyObjectPtr
apply1(const char* procname,
       std::function<LilyObjectPtr(std::shared_ptr<A>)> proc,
       LilyListPtr* vs)
{
	IF_LETU_AS(vs0, LilyPair, *vs) {
		if (UNWRAP_AS(LilyNull, vs0->cdr())) {
			return proc(XAS<A>(vs0->car()));
		}
	}
	throw std::logic_error(STR(procname << " needs 1 argument"));
}

template <typename A, typename B>
LilyObjectPtr
apply2(const char* procname,
       std::function<LilyObjectPtr(std::shared_ptr<A>,
				   std::shared_ptr<B>)> proc,
       LilyListPtr* vs)
{
	IF_LETU_AS(vs0, LilyPair, *vs) {
		IF_LETU_AS(vs1, LilyPair, vs0->cdr()) {
			if (UNWRAP_AS(LilyNull, vs1->cdr())) {
				return proc(XAS<A>(vs0->car()),
					    XAS<B>(vs1->car()));
			}
		}
	}
	throw std::logic_error(STR(procname << " needs 2 arguments"));
}

template <typename A, typename B, typename C>
LilyObjectPtr
apply3(const char* procname,
       std::function<LilyObjectPtr(std::shared_ptr<A>,
				   std::shared_ptr<B>,
				   std::shared_ptr<C>)> proc,
       LilyListPtr* vs)
{
	IF_LETU_AS(vs0, LilyPair, *vs) {
		IF_LETU_AS(vs1, LilyPair, vs0->cdr()) {
			IF_LETU_AS(vs2, LilyPair, vs1->cdr()) {
				if (UNWRAP_AS(LilyNull, vs2->cdr())) {
					return proc(XAS<A>(vs0->car()),
						    XAS<B>(vs1->car()),
						    XAS<C>(vs2->car()));
				}
			}
		}
	}
	throw std::logic_error(STR(procname << " needs 3 arguments"));
}


// this is ~gcc specific: disable warnings about *possibly unused*
// variables
#define UNUSED(t_and_var) __attribute__((unused)) t_and_var

#define LAMBDA0(scmnamstr, body)					\
	[&](LilyListPtr* __vs,						\
	    UNUSED(LilyListPtr* __ctx),					\
	    UNUSED(LilyListPtr* __cont)) -> LilyObjectPtr {		\
		return apply0(scmnamstr,				\
			      [&]() -> LilyObjectPtr body,		\
			      __vs);					\
	}

#define LAMBDA1(scmnamstr, T1, v1, body)				\
	[&](LilyListPtr* __vs,						\
	    UNUSED(LilyListPtr* __ctx),					\
	    UNUSED(LilyListPtr* __cont)) -> LilyObjectPtr {		\
		return apply1<T1>(scmnamstr,				\
				  [&](std::shared_ptr<T1> v1)		\
				  -> LilyObjectPtr body,		\
				  __vs);				\
	}

#define LAMBDA2(scmnamstr, T1, v1, T2, v2, body)			\
	[&](LilyListPtr* __vs,						\
	    UNUSED(LilyListPtr* __ctx),					\
	    UNUSED(LilyListPtr* __cont)) -> LilyObjectPtr {		\
		return apply2<T1,T2>(scmnamstr,				\
				     [&](std::shared_ptr<T1> v1,	\
					 std::shared_ptr<T2> v2)	\
				     -> LilyObjectPtr body,		\
				     __vs);				\
	}

#define LAMBDA3(scmnamstr, T1, v1, T2, v2, T3, v3, body)		\
	[&](LilyListPtr* __vs,						\
	    UNUSED(LilyListPtr* __ctx),					\
	    UNUSED(LilyListPtr* __cont)) -> LilyObjectPtr {		\
		return apply3<T1,T2,T3>(scmnamstr,			\
					[&](std::shared_ptr<T1> v1,	\
					    std::shared_ptr<T2> v2,	\
					    std::shared_ptr<T3> v3)	\
					-> LilyObjectPtr body,		\
					__vs);				\
	}


#define DEFPRIM0(var, scmvar, body)			\
	auto var = LAMBDA0(scmvar, body);		\
	BINDPRIM(scmvar, var);
#define DEFPRIM1(var, scmvar, t1, arg1, body)		\
	auto var = LAMBDA1(scmvar, t1, arg1, body);	\
	BINDPRIM(scmvar, var);
#define DEFPRIM2(var, scmvar, t1, arg1, t2, arg2, body)		\
	auto var = LAMBDA2(scmvar, t1, arg1, t2, arg2, body);	\
	BINDPRIM(scmvar, var);
#define DEFPRIM3(var, scmvar, t1, arg1, t2, arg2, t3, arg3, body)	\
	auto var = LAMBDA3(scmvar, t1, arg1, t2, arg2, t3, arg3, body);	\
	BINDPRIM(scmvar, var);
	

#endif
