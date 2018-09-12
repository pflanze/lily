#ifndef _LILYPARSE_HPP
#define _LILYPARSE_HPP

#include "parse.hpp"

bool needsSymbolQuoting (lily_char_t c);

const char* lilyCharMaybeName(lily_char_t c);

// never throws exceptions: 1/0 is parsed as a symbol, parsing errors
// are returned as LilyParseError objects
LilyObjectPtr lilyParse (std::string s, bool requireTotal=false) /* noexcept */;


#endif
