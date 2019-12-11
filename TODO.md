# TODO

## Various

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

## Challenging

- error handling: casting with XAS etc. throws exceptions, which is
  fine currently that this is the only error handling mechanism. Once
  Scheme has its own error handling mechanism, these usages have to be
  split and converted into calling Scheme error handler (making a new
  continuation and process it).

