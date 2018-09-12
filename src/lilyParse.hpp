#ifndef _LILYPARSE_HPP
#define _LILYPARSE_HPP

#include "parse.hpp"

bool needsSymbolQuoting (char c);

const char* lilyCharMaybeName(char c);

// never throws exceptions: 1/0 is parsed as a symbol, parsing errors
// are returned as LilyParseError objects
LilyObjectPtr lilyParse (std::string s, bool requireTotal=false) /* noexcept */;


#endif
