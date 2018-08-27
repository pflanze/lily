#ifndef _LILYPARSE_HPP
#define _LILYPARSE_HPP

#include "parse.hpp"

bool needsSymbolQuoting (char c);

LilyObjectPtr lilyParse (std::string s, bool requireTotal=false);


#endif
