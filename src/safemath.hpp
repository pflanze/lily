
#if 0

// https://stackoverflow.com/questions/199333/how-to-detect-integer-overflow

#include <cstddef>
#if defined( _MSC_VER )
#  include <intrin.h>
#endif

inline size_t query_intel_x86_eflags( const size_t query_bit_mask )
{
#if defined( _MSC_VER )
    return __readeflags() & query_bit_mask;
#elif defined( __GNUC__ )
    // this code will work only on 64-bit GNU-C machines;
    // Tested and does NOT work with Intel C++ 10.1!
    size_t eflags;
    __asm__ __volatile__(
        "pushfq \n\t"
        "pop %%rax\n\t"
        "movq %%rax, %0\n\t"
        :"=r"(eflags)
        :
        :"%rax"
        );
    return eflags & query_bit_mask;
#else
#pragma message("No inline assembly will work with this compiler!")
    return 0;
#endif
}


#define FLAG_OVERFLOW 0x800
#define FLAG_CARRY 1

inline int64_t lily_add(int64_t a, int64_t b) {
	int64_t res= a + b;
	if (query_intel_x86_eflags(FLAG_OVERFLOW))
		throwOverflow(a, "+", b);
	return res;
}
inline int64_t lily_sub(int64_t a, int64_t b) {
	int64_t res= a - b;
	if (query_intel_x86_eflags(FLAG_OVERFLOW | FLAG_CARRY))
		throwOverflow(a, "-", b);
	return res;
}
inline int64_t lily_mul(int64_t a, int64_t b) {
	int64_t res= a * b;
	if (query_intel_x86_eflags(FLAG_OVERFLOW | FLAG_CARRY))
		throwOverflow(a, "*", b);
	return res;
}
//XX ^ f check all of that  which flags  pls ?

#elsif 0

// Alternative: __int128 type support by gcc, supported already in
// that older version

// As an extension the integer scalar type `__int128' is supported for
// targets having an integer mode wide enough to hold 128-bit.  Simply
// write `__int128' for a signed 128-bit integer, or `unsigned __int128'
// for an unsigned 128-bit integer.  There is no support in GCC to express
// an integer constant of type `__int128' for targets having `long long'
// integer with less then 128 bit width.

inline int64_t lily_add(int64_t a, int64_t b) {
	__int128 res= a + b;
	if (res >> 64)
		throwOverflow(a, "+", b);
	return res;
}
inline int64_t lily_sub(int64_t a, int64_t b) {
	__int128 res= a - b;
	if (res >> 64)
		throwOverflow(a, "-", b);
	return res;
}
inline int64_t lily_mul(int64_t a, int64_t b) {
	__int128 res= a * b;
	if (res >> 64)
		throwOverflow(a, "*", b);
	return res;
}

#else

#include <limits.h>


#ifndef INT64_MAX
#  define INT64_MAX LLONG_MAX
#  define INT64_MIN LLONG_MIN
#endif

inline int64_t lily_add(int64_t a, int64_t x) {
	if (((x > 0) && (a > INT64_MAX - x)) ||
	    ((x < 0) && (a < INT64_MIN - x)))
		throwOverflow(a, "+", x);
	return a + x;
}
inline int64_t lily_sub(int64_t a, int64_t x) {
	if (((x < 0) && (a > INT64_MAX + x)) ||
	    ((x > 0) && (a < INT64_MIN + x)))
		throwOverflow(a, "-", x);
	return a - x;
}
inline int64_t lily_mul(int64_t a, int64_t x) {
	if (((a == -1) && (x == INT64_MIN)) ||
	    ((x == -1) && (a == INT64_MIN)) ||
	    (a > INT64_MAX / x) ||
	    (a < INT64_MIN / x))
		throwOverflow(a, "*", x);
	return a * x;
}



#endif
