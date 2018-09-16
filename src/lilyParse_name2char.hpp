// This was partially auto-generated. Better don't edit!


// Return type is *not* lily_char_t, since it needs to be signed for
// the -1; don't need to be able to represent the full range of
// unicode anyway here.

static
int32_t name2char (S str, int32_t len) {
	auto stringEq= [=](S& str, const char* str2) -> bool {
		return str.subStringEq(str2, len);
	};
	auto otherwise= [=]() -> int32_t {
		return -1;
	};
	if (len > 100)
		return otherwise();
	switch ((char)len) {
	case 1:
		return str.first();
	case 3:
		return [&]() -> int32_t {
			if (stringEq(str, "nul")) {
				return 0;
			} else if (stringEq(str, "tab")) {
				return 9;
			} else if (stringEq(str, "esc")) {
				return 27;
			} else {
				return otherwise();
			}
		}();
	case 4:
		return [&]() -> int32_t {
			if (stringEq(str, "vtab")) {
				return 11;
			} else if (stringEq(str, "page")) {
				return 12;
			} else {
				return otherwise();
			}
		}();
	case 5:
		return [&]() -> int32_t {
			if (stringEq(str, "alarm")) {
				return 7;
			} else if (stringEq(str, "space")) {
				return 32;
			} else {
				return otherwise();}
		}();
	case 6:
		return [&]() -> int32_t {
			if (stringEq(str, "return")) {
				return 13;
			} else if (stringEq(str, "delete")) {
				return 127;
			} else {
				return otherwise();
			}
		}();
	case 7:
		return [&]() -> int32_t {
			if (stringEq(str, "newline")) {
				return 10;
			} else {
				return otherwise();
			}
		}();
	case 9:
		return [&]() -> int32_t {
			if (stringEq(str, "backspace")) {
				return 8;
			} else {
				return otherwise();
			}
		}();
	default:
		return otherwise();
	}
}

