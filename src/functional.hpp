#ifndef _FUNCTIONAL_HPP
#define _FUNCTIONAL_HPP

#include <functional>

// template <typename T>
// std::function<bool(T)> complement(std::function<bool(T)> fn) {
// 	return [&](T v) -> bool {
// 		return ! fn(v);
// 	};
// }

// #define COMPLEMENT(T, e) complement<T>(static_cast<std::function<bool(T)>>(e))

// Doesn't work with overloaded functions as fn; what's the problem,
// lame?

#define COMPLEMENT(T, fnsym) [&](T v) -> bool {			\
		return ! fnsym(v);				\
	}


#endif // _FUNCTIONAL_HPP

