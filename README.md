# Lily - Little Lisp system for Qt

This is an (as of yet unfinished, see status) implementation of the
Scheme language (is there interest in Clojure syntax and semantics?) 
in C++ with a memory model that makes it easy to interface to C++
libraries like Qt, and an execution model that favours low latency
over execution speed.

The idea is to support the development of user interfaces using Qt in
a way so that the UI code runs in its own system thread (or a separate
process), but still allows event handlers to be written in Scheme
(hence enabling rapid development), and allows to communicate with
another, larger, garbage-collected language implementation (Scheme
system, or whatever) via exchange of S-expressions (could be changed
to a binary protocol, or JSON) over stdout/stdin (could be changed to
a socket or multi-thread safe queue).

Lily uses reference counting for GC and has thus deterministic latency
(could be improved further by moving deallocation to a separate system
thread should the need arise). Event handlers running as Scheme code
in Lily will thus not lead to UI jitter.

For more complex or computation intensive operations, the code in the
UI thread (Lily program or C++ via tha Lily library) will communicate
with the main system. This model is similar to the approach taken in
single-page web applications: the "main system" in this case is the
server side (backend), whereas the "Lily program" corresponds to the
client side (JavaScript, Elm, ...) side. An approach where the Lily
side code follows a reactive pattern (similar to React or Elm) could
probably be implemented, in which case the similarity will be very
close. Note that the data exchanged over the pipe to the Lily system
(S-expressions) is actually code (the S-expressions describe Scheme
programs), again similar to the sending of JavaScript to the
client. The C++ code starting the Lily interpreter can decide to
evaluate the received code in a limited environment, though, and can
hence arbitrarily limit what the remote side can do.

Lily supports first-class continuations which should aid in
implementing custom threading approaches (obviating the need for
manual continuation-passing style code (event handler chaining) as in
traditional JavaScript).

## Goals

* easy to understand and maintain implementation
* low latency
* first-class lexical environments in some manner (run programs in an
  environment that is tailored to what the programs should be allowed
  to do)

## Non-goals

* raw speed (but there's an idea to maybe implement a translator from
  Scheme programs to equivalent C++ code that is still working on the
  same dynamic data types but saves the interpreter overhead (and then
  possibly, with type annotations, to statically typed code), that
  should speed up things quite a lot and should make libraries in
  Scheme feasible)

## Status

- S-expressions, evaluation of calls to primitives are working, hence
  calling Qt functionality over the pipe is available. Lambda and
  definitions are not implemented yet, thus local event handlers
  cannot be written in Lily yet.

- `make test` shows broken tests for unicode handling that needs to be
  done

- See [TODO](TODO.md).


## Requirements

A compiler that supports C++ 11 is required.

`make qt` requires Qt 4 or greater (4 and 5 have been tested) to be
installed.

Lily has so far only been tested on Linux (Debian).


## Examples

    $ make
    $ bin/examples/repl
    > (* 2 3)
    6
    > +
    #<native-procedure +>
    > (apply + (map inc '(1 2 3)))
    9
    > (sys:allocation-counts)
    (238 74)

The repl binary will not exit until you hit ctl-d or ctl-c; it reads
lines of Scheme code (of the part of Scheme that it currently
supports) on stdin and writes the result of its evaluation to stdout.
You may want to install the `rlwrap` tool and call it as `rlwrap
bin/examples/repl`.

You can see what functions and syntax is implemented by looking at the
definition of `lilyDefaultEnvironment()` in
[src/lilyDefaultEnvironment.cpp](src/lilyDefaultEnvironment.cpp).

Here's an example that embeds Lily in a Qt application (like the repl
above, it reads Scheme 'command lines' on stdin and writes the result
to stdout):

    $ make qt
    $ examples/qt/lilyExample
    (error-message-dialog "Hello")
    #!void
    (input-dialog:get-text "Hi" "What's your name?")
    "Tom Petty"
    (LOG "Hi")
    #!void

Check [examples/qt/main.cpp](examples/qt/main.cpp) for the "DEFPRIM.." 
forms to see what functionality is supported. (Most tabs in the GUI
don't currently have a function, the Log one is the one that shows the
output from the `LOG` Scheme procedure.)


## Author

Lily is being developed by [Christian
Jaeger](http://christianjaeger.ch), originally for [Arcola
Energy](https://www.arcolaenergy.com/). Thanks to Arcola Energy for
the support of the initial development and for releasing it as Open
Source!


## License

Lily is Open Source software released under the MIT License. See
[COPYING](COPYING.md).
