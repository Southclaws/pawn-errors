# pawn-errors

[![sampctl](https://shields.southcla.ws/badge/sampctl-pawn--errors-2f2f2f.svg?style=for-the-badge)](https://github.com/Southclaws/pawn-errors)

This is a work-in-progress proposal for a standardised, minimal, simple and
universal way to handle errors in the Pawn language.

There exists a more complex error handling solution which implements try/catch
exceptions. This library aims to be a simple, pure Pawn (no asm) alternative
which does not introduce new syntax.

The concept is similar to how Go and simple C programs handle errors: when an
error is raised with `Error()`:

```pawn
Error:thisFunctionFails() {
    return Error("failed to be true :(");
}
```

a non-zero `Error:` value is returned with the intention that it be returned
directly from the enclosing function. Meanwhile, at the call site of the
function in question:

```pawn
new Error:e = thisFunctionFails();
if(e) {
    err("something went wrong, but we fixed it!",
        _e(e)); // not implemented but possible logger type for errors
    Handled(e);
}
```

its return value is checked and if it's non-zero, the error information can be
extracted and handled. Finally, the error is marked handled with `Handled()`.

The proposal features the usage of a tagged scope destructor so if an `Error:`
value goes unhandled, when it leaves scope an error message is printed along
with a backtrace. This means that uncaught errors are handled:

```pawn
    e = thisFunctionFails();
    // e exits scope without being handled, print a nasty "panic" message
}
```

There is still much to do before this can be used in production. My aim is to
create a very simple API - which in my opinion is all that is necessary given
that Pawn is procedural and single-threaded. Once an error is generated in a
function, it can be handled at the function call site, it's rare that you ever
get into a situation where you'll have multiple unhandled errors, so this
library doesn't provide that functionality however it may be implemented by
simply appending error strings to the internal string buffer, similar to how the
Go errors package provides `wrap()` to enclose an error with additional
contextual information then pass its value further up the call stack.

This library will also make use of crashdetect tracebacks in error information.

## Installation

Simply install to your project:

```bash
sampctl package install Southclaws/pawn-errors
```

Include in your code and begin using the library:

```pawn
#include <errors>
```

## Usage

See `test.pwn` for an example of usage. Once the API is stable, more thorough
documentation will be added here.

## Testing

Currently, the tests are simply a `main` and some function calls, check out the
code in `test.pwn` and run the package to see how it works.

```bash
sampctl package run
```
