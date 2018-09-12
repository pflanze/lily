#ifndef _FUNCTIONAL_HPP
#define _FUNCTIONAL_HPP

#include <functional>

template <typename T>
std::function<bool(T)> complement(std::function<bool(T)> fn) {
	return [&](T v) -> bool {
		return ! fn(v);
	};
}


#endif // _FUNCTIONAL_HPP

