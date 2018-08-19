
static bool
betweenIncl (char c, char from, char to) {
	return ((from <= c) && (c <= to));
}

static bool
char_isdigit(char c) {
	return betweenIncl(c, '0', '9');
}

static bool
char_isalpha(char c) {
	return betweenIncl(c, 'a', 'z')
		|| betweenIncl(c, 'A', 'Z');
}

static bool
char_isalphanumeric(char c) {
	return char_isalpha(c) || char_isdigit(c);
}

static bool
char_isword(char c) {
	return char_isalphanumeric(c)
		|| (c == '_');
}
	

static bool
char_doesNotNeedQuoting (char c) {
	return (char_isword(c)
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
