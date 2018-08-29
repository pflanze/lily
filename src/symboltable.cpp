#include <string.h>
#include <stdio.h>
#include "symboltable.hpp"
#include "lilyUtil.hpp"

#include "../SipHash/siphash.c"


// XX correct?
#define SECRET_SIZE 16

static char symboltable_secret[SECRET_SIZE];
static bool symboltable_secret_initialized=false;

static void symboltable_secret_perhaps_init() {
	if (symboltable_secret_initialized) return;
	// WARN("HEY");
	FILE* in= fopen("/dev/urandom", "r");
	if (!in)
		throwWithStrerror("opening /dev/urandom for reading");
	auto len= fread(symboltable_secret, SECRET_SIZE, 1, in);
	if (len != 1)
		throwWithStrerror
			(STR("could not read enough bytes from /dev/urandom: "
			     << len << " instead of 1*" << SECRET_SIZE));
	if (fclose(in))
		throwWithStrerror("close /dev/urandom");
	symboltable_secret_initialized= true;
}

symboltablehash_t siphash(const std::string s) {
	uint64_t res;
	symboltable_secret_perhaps_init();
	siphash(s.data(),
		static_cast<const size_t>(s.length()),
		symboltable_secret,
		&res);
	return static_cast<symboltablehash_t>(res);
}


