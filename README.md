# pawn-errors

[![sampctl](https://shields.southcla.ws/badge/sampctl-pawn--errors-2f2f2f.svg?style=for-the-badge)](https://github.com/Southclaws/pawn-errors)

This is a simple library for dealing with function-level errors and making sure
unhandled errors don't get quietly ignored.

There exists a more complex error handling solution which implements try/catch
exceptions. This library aims to be a simple, pure Pawn (no asm) alternative
which does not introduce new syntax.

The concept is similar to how Go and simple C programs handle errors: functions
should return an _error value_ indicating success or failure. If the value is
zero, everything went smoothly but if the error is anything but zero, an error
occurred.

This library enhances that pattern through the use of tags, destructors and a
simple raise-handle model that fits the simple procedural nature of Pawn.

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

Functions that could potentially fail should be tagged `Error:` and either
return `NoError` or call `Error()`. For example, this is a function that always
fails:

```pawn
Error:thisFunctionFails() {
    return Error("I always fail!");
}
```

a non-zero `Error:` value is returned. This return value should be checked at
the call site, this can simply be done by checking for truthiness - `e` is
anything other than 0, that means something went wrong:

```pawn
new Error:e = thisFunctionFails();
if(e) {
    printf("ERROR: thisFunctionFails has failed");
    Handled(e);
}
```

Finally, the error is marked handled with `Handled()`. This will erase the
current stack of errors and indicates that the script has returned to a safe
state.

If a single error or a stack of errors is unhandled, the error information will
be printed once the current stack has returned (in other words, once the current
callback has finished).

## Testing

To run the tests:

```bash
sampctl package run
```
