#include <string.h>

#include "symboltable.hpp"

#include "../SipHash/siphash.c"


static char symboltable_secret[16]; // XX correct?
static bool symboltable_secret_initialized=false;

static void symboltable_secret_perhaps_init() {
	if (symboltable_secret_initialized) return;
	strncpy(symboltable_secret,
		"hell'o worldsx10", 16); // XX suuure
}

symboltablehash_t siphash(const std::string s) {
	symboltablehash_t res;
	symboltable_secret_perhaps_init();
	siphash(s.data(),
		static_cast<const size_t>(s.length()),
		symboltable_secret,
		&res);
	return res;
}


