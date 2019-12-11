# Hacking

Some guidance on how to work on the Lily system.

## Principles

* The goal of this Scheme system is not to be fast in running
  programs; it is to be simple, small in code size, and fast in
  getting from a string representation of an S-expression to running
  the represented program (i.e. low latency).

* Almost everything is done with (singly linked) lists (the same ones
  accessible as first class list data type (well, pair and null data
  types) within Scheme programs; this is to keep it simple, and
  perhaps to allow (/ make easier) interesting introspection features
  from Scheme programs):

    * the code (the interpreter works directly on S-expressions,
      there's no bytecode or even 'solidified' AST; this should help
      keep latency low)
    * the lexical environment ("ctx")
    * the stack ("cont")

* CPP macros are used quite heavily; the author feels it makes the
  code cleaner, but it does require understanding them (TODO:
  document them)

* `...Ptr` (e.g. `LilyObjectPtr`) types are `shared_ptr`-wrapped
  types, they are representing the first-class values available to
  Scheme programs. In some places in the code references to the
  embedded objects are passed (in the hopes for some speed
  improvement--TODO: is this ill-advised?  Is passing shared_ptr
  zero-overhead in move situations? Other situations? Does C++14
  improve on this?).

    * There are macros to get these low-level pointers in
      [lily.hpp](src/lily.hpp): `UNWRAP_AS`, `LIST_UNWRAP`, `LETU_AS`,
      which return/assign null pointers if the casting (type check)
      fails (as well as `IF_LETU_AS` which dispatches depending on the
      check result, think of it as pattern matching), and the variants
      `XUNWRAP_AS`,`XLIST_UNWRAP`, `XLETU_AS` which throw exceptions
      on casting errors.
      
    * For just dynamic type handling without unwrapping, there's also
      the templated `XAS` function. (This one uses non-standardized
      type meta information instead of source stringification; OTOH it
      works when itself used in templates.)  As well as `LET_AS` and
      `IF_LET_AS`.

* Currently C++ exceptions are used for signalling errors from Scheme
  operations; this is due to be changed to something that Scheme
  programs (not just the C++ layer) can catch, though (dynamic
  variables (Scheme 'parameters') with handler functions and call/cc).

* There are C++ functions and macros acting as constructors that
  mirror the functions with the same names in Scheme, like `LIST`,
  `CONS`, etc. (see [src/lilyConstruct.hpp](src/lilyConstruct.hpp)).
  
    * Gotcha: Scheme calls the list null pointer `null`; but `NULL` is
      the C/C++ null pointer. So it's called `NIL` instead (like in
      Common Lisp). Remember not to accidentally use `NULL` as that
      *will* compile but give a segfault (sad C++ doesn't support
      non-nullable types (yet).)


## Naming

* `__varname` for 'magic' variables (introduced by some macros)


## In flux

Things that are inbetween design decisions:

* [src/lilyDefaultEnvironment.cpp](src/lilyDefaultEnvironment.cpp) is
  mostly defining toplevel functions then embedding those in the env
  constructed by `lilyDefaultEnvironment`, because that was the
  original way to do things; now that there's the `DEFPRIM..` macros,
  this may be the way to go (it *will* not make it possible to export
  those functions on the C++ level, though, which may or may not be
  desirable).

