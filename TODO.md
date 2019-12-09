- use Qt strings directly: how?
- handle unicode; with and without using Qt strings?
- check `\0` handling
- proper `\r\n` handling
- protect against stack overflows during parsing and pretty-printing?
- check for number overflows
- `lilyParse`: report begin of erroneous elements instead of end
- make data structure const where applicable
- change continuations data structure into an intrusive list or more
  optimized pure stack
- use `LETU_AS` instead of `LET_AS` in places where it's safe, check
  performance difference
- provide an interface hierarchy separate from implementation?
- replace `assert` with an `ASSERT` that throws (what about null
  pointer exceptions?)
- don't use C++ exceptions for Scheme errors, rather continuations and
  dynamic variables (and catch (and throw?) C++ exceptions at native
  boundaries)
