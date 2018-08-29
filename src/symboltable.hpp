#ifndef _SYMBOLTABLE_HPP
#define _SYMBOLTABLE_HPP

#include <stdint.h>
#include <string>


// the idea is to optimize the particular hashtable by not re-hashing
typedef uint64_t symboltablehash_t;


// mapping strings to symbols

// template <typename S>
// class SymbolTable {
// 	std::unordered_map<std::string, S> m;
// };
// already did that in LilySymbol::intern(std::string s), now adding hash here


// mapping symbols to values

template <typename S, typename V>
class SymbolMapValue {
	SymbolMapValue(S k, V v) : key(k), value(v) {}
	S key;
	V value;
};

template <typename T, int SIZE>
class SymbolMap {
	
};


// copied from SipHash/siphash.c; return value actually seems nonsensical.
// ((outlen == 8) || (outlen == 16))
int siphash(const uint8_t *in, const size_t inlen, const uint8_t *k,
            uint8_t *out, const size_t outlen);

// and a reasonable adaption, please!
inline
int siphash(const char *in, const size_t inlen, const char k[16],
	    // symboltablehash_t* correct? now endianness dependent! Does that bother me?
            symboltablehash_t *out) {
	return siphash((const uint8_t *)in, inlen, (const uint8_t *)k,
		       (uint8_t *)out, sizeof(symboltablehash_t));
}

symboltablehash_t siphash(const std::string s);


#endif // _SYMBOLTABLE_HPP
