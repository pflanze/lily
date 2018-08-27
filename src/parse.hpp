#ifndef _PARSE_HPP
#define _PARSE_HPP

#include <string>
#include <assert.h>
#include <stdexcept>


// StringCursor, shortened S, is the argument for all parsers and the
// return value for non-capturing parsers. ParseResult, shortened PR,
// is the return value for capturing parsers.

enum class ParseResultCode : char; // to be defined by user

// These ParseResultCode values are hard coded as integers and
// unsafely casted here--make sure your definition of ParseResultCode
// assigns them correct meaning, e.g.

// enum class ParseResultCode : char {
// 	Success=0,
// 	UnexpectedEof=1,
// 	UnexpectedString=2,
// 	// .. your own codes ..
// };

// static ParseResultCode ParseResultCode_Success= (ParseResultCode)0;
// static ParseResultCode ParseResultCode_UnexpectedEof= (ParseResultCode)1;
// static ParseResultCode ParseResultCode_UnexpectedString= (ParseResultCode)2;
// XX only to avoid warnings:
#define ParseResultCode_Success ((ParseResultCode)0)
#define ParseResultCode_UnexpectedEof ((ParseResultCode)1)
#define ParseResultCode_UnexpectedString ((ParseResultCode)2)


// StringCursor (and thus indirectly also ParseResult) represents both
// the remainder of an input, and success or parse errors (including
// their context)

// An (unverified) attempt has been made to fit StringCursor into just
// two 64-bit machine words.

// (Idea: make it fit into just one by requiring the user to custom
// allocate the string to a reduced-address-entropy region, and then
// only store 24 bits of pointer information.)

// (BTW won't need to maintain line numbering, could obtain line
// number from an on-demand re-scanning run in the rare case when
// needed.)

typedef uint32_t parse_position_t;

class StringCursor {
public:
	StringCursor(std::string* string,
		     parse_position_t position = 0,
		     ParseResultCode error = ParseResultCode_Success)
		: _string(string), _error(error), _position(position) {}
	char first () {
		return (*_string)[_position];
	}
	StringCursor rest () {
		parse_position_t pos1= _position+1;
		assert(pos1 > 0); // XX not strictly portable
		if (pos1 <= _string->length()) {
			return StringCursor(_string, pos1, _error);
		} else {
			throw std::logic_error("read past end of input");
		}
	}
	bool isNull () {
		return !(_position < _string->length());
	}
	ParseResultCode error () {
		return _error;
	}
	bool success () {
		return (_error==ParseResultCode_Success);
	}
	StringCursor setError (ParseResultCode error) {
		return StringCursor(_string, _position, error);
	}
	parse_position_t position() {
		return _position;
	}
	std::string& string() {
		return *_string;
	}
	std::string stringRemainder() {
		return _string->substr(_position,
				       _string->length() - _position);
	}
private:
	std::string* _string;
	ParseResultCode _error;
	parse_position_t _position;
};

template <typename T>
class ParseResult {
public:
	ParseResult(T result, StringCursor remainder)
		: _result(result), _remainder(remainder) {
		assert(result);
	}
	T value() {
		return _result;
	}
	ParseResultCode error () {
		return _remainder.error();
	}
	bool success() {
		return _remainder.success();
	}
	StringCursor remainder() {
		return _remainder;
	}
private:
	T _result;	
	StringCursor _remainder;
};

typedef StringCursor S;
// typedef ParseResult<YourType> PR;


// ----------------------------------------------------------------


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

bool isWhitespace (char c);

S skipWhitespaceOnly (S s);

// return the position after the end of this line, or eof (always
// successful)
S skipUntilAfterEol(S s);

// return the position after the first occurrence of str, or eof error
// (naive algorithm)
S skipUntil (S s, const char* str, bool after=false);

static inline
S skipUntilAfter (S s, const char* str) {
	return skipUntil(s, str, true);
}

bool isWordEndBoundary(S s);

S expectString(S s, const char* str);


#endif
