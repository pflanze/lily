#ifndef _PARSE_HPP
#define _PARSE_HPP

#include <string>
#include <assert.h>
#include <stdexcept>
#include <functional>


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

const parse_position_t no_parse_position= 4294967295;

class StringCursor {
public:
	StringCursor(const std::string* backingString,
		     parse_position_t position = 0,
		     ParseResultCode error = ParseResultCode_Success,
		     parse_position_t endposition= no_parse_position
		)
		: _backingString(backingString),
		  _error(error),
		  _position(position)
		{
			if (endposition == no_parse_position)
				_endposition= backingString->length();
			else
				_endposition= endposition;
					
		}
	StringCursor() {}
	char first () const {
		return (*_backingString)[_position];
	}
	const StringCursor rest () const {
		parse_position_t pos1= _position+1;
		assert(pos1 > 0); // XX not strictly portable
		if (pos1 <= _endposition) {
			return StringCursor(_backingString, pos1, _error);
		} else {
			throw std::logic_error("read past end of input");
		}
	}
	bool isNull () const {
		return !(_position < _endposition);
	}
	ParseResultCode error () const {
		return _error;
	}
	bool succeeded () const {
		return (_error==ParseResultCode_Success);
	}
	const StringCursor setError (ParseResultCode error) const {
		return StringCursor(_backingString, _position, error);
	}
	const StringCursor setSucceeded () const {
		return setError(ParseResultCode_Success);
	}
	parse_position_t position() const {
		return _position;
	}
	const std::string* backingString() const {
		return _backingString;
	}
	const std::string takeString(parse_position_t len) const {
		return _backingString->substr(_position, len);
	}
	bool subStringEq(const char* str, parse_position_t len) const {
		for (parse_position_t i = 0; i < len; i++) {
			if (! ((*_backingString)[_position + i] == str[i]))
				return false;
		}
		return true;
	}
	const std::string string() const {
		return _backingString
			->substr(_position, _endposition - _position);
	}
	parse_position_t positionDifferenceTo(StringCursor& s2) {
		assert(s2._position >= _position);
		// assert(s2._backingString == _backingString);
		return s2._position - _position;
	}
private:
	const std::string* _backingString;
	ParseResultCode _error;
	parse_position_t _position;
	parse_position_t _endposition;
};



template <typename T>
class ParseResult {
public:
	ParseResult(const T value, StringCursor remainder)
		: _value(value), _remainder(remainder) {
		// assert(value); can't, as some T may not have a
		// boolean check! Make this part of T!
	}
	ParseResult() {}
	const T value() const {
		assert(_remainder.succeeded());
		return _value;
	}
	ParseResultCode error () const {
		return _remainder.error();
	}
	bool succeeded() const {
		return _remainder.succeeded();
	}
	bool failed() const {
		return ! succeeded();
	}
	const StringCursor remainder() const {
		return _remainder;
	}
private:
	T _value;
	StringCursor _remainder;
};



typedef const StringCursor S;
typedef StringCursor Sm;

//in your header you could:
// typedef const ParseResult<YourType> PR;
// typedef ParseResult<YourType> PRm;


// ----------------------------------------------------------------


inline bool
betweenIncl (char c, char from, char to) {
	return ((from <= c) && (c <= to));
}

inline bool
isDigit(char c) {
	return betweenIncl(c, '0', '9');
}

inline bool
isAlpha(char c) {
	return betweenIncl(c, 'a', 'z')
		|| betweenIncl(c, 'A', 'Z');
}

inline bool
isAlphanumeric(char c) {
	return isAlpha(c) || isDigit(c);
}

inline bool
isWordChar(char c)  {
	return isAlphanumeric(c)
		|| (c == '_');
}

bool isWhitespace (char c);

// not called skipWhitespace because there will be user-defined
// skipWhitespaceAndComments or similar, too.
S skipWhitespaceOnly (S s);

// return the position after the end of this line, or eof (always
// successful)
S skipUntilAfterEol(S s);

// return the position after the first occurrence of str, or eof error
// (naive algorithm)
S skipUntil (S s, const char* str, bool after=false);

inline
S skipUntilAfter (S s, const char* str) {
	return skipUntil(s, str, true);
}

S dropWhile (S s, std::function<bool(char)> pred);

bool isWordEndBoundary(S s);

// returns failure if str is not at the start of s, and position at
// the point of error or (success case) after str
S expectString(S s, const char* str);


#endif
