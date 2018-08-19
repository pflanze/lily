
static bool
betweenIncl (char c, char from, char to) {
	return ((from <= c) && (c <= to));
}

static bool
isDigit(char c) {
	return betweenIncl(c, '0', '9');
}

static bool
isAlpha(char c) {
	return betweenIncl(c, 'a', 'z')
		|| betweenIncl(c, 'A', 'Z');
}

static bool
isAlphanumeric(char c) {
	return isAlpha(c) || isDigit(c);
}

static bool
isWordChar(char c) {
	return isAlphanumeric(c)
		|| (c == '_');
}
	

static bool
doesNotNeedSymbolQuoting (char c) {
	return (isWordChar(c)
		|| (c == '!')
		|| (c == '?')
		|| (c == '.')
		|| (c == ':')
		|| (c == '/')
		|| (c == '%')
		|| (c == '$')
		|| (c == '-')
		|| (c == '+')
		|| (c == '*')
		|| (c == '_')
		|| (c == '&')
		|| (c == '=')
		|| (c == '<')
		|| (c == '>')
		);
}

LilyObjectPtr lilyParse (std::string s);
