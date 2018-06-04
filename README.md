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

### Basic Errors

Functions that could potentially fail should be tagged `Error:` and either
return `NoError()` or `Error()`. For example, this is a function that always
fails:

```pawn
Error:thisFunctionFails() {
    return Error(1, "I always fail!");
}
```

The first argument is an error code, this is the actual value that is returned
from the function, it can be used by the call site to determine exactly went
wrong. You can optionally export named constants to simplify the error checking
process.

This return value should be checked at the call site using `IsError`:

```pawn
new Error:e = thisFunctionFails();
if(IsError(e)) {
    printf("ERROR: thisFunctionFails has failed");
    Handled();
}
```

### Nested Errors

If an error has been returned and the call site cannot handle it, you can simply
return another `Error()` to the next caller. Errors will stack along with full
file and line information so when you handle them, you have all the data
available.

Lets modify the above example to pass an error further up the chain:

```pawn
Error:doSomething() {
    new Error:e = thisFunctionFails();
    if(IsError(e)) {
        return Error(1, "thisFunctionFails has failed and I don't know what to do, maybe my caller does");
    }
}

public OnSomething() {
    new Error:e = doSomething();
    if(IsError(e)) {
        print("something went wrong");
        Handled();
    }
}
```

### Marking Errors as Handled

At the end of these examples, the error is marked handled with `Handled()`. This
will erase the current stack of errors and indicates that the script has
returned to a safe state.

If a single error or a stack of errors is unhandled, the error information will
be printed once the current stack has returned (in other words, once the current
callback has finished).

### GetErrors

So far you've seen a lot of creating errors with text content but not a lot of
getting that text content back for worthwhile descriptions of errors.

Well `GetErrors` does exactly that. We can modify the above example to include a
`GetErrors` call and then the usage of a custom logger which may store the
information in a database for later analysis or send it to a developer's channel
on IRC, Slack or Discord:

```pawn
public OnSomething() {
    new Error:e = doSomething();
    if(IsError(e)) {
        new errorInfo[1024];
        GetErrors(errorInfo);
        customLogger(errorInfo); // send it to a logging database or something
        Handled();
    }
}
```

`GetErrors` returns a string that looks like this:

```text
F:\Projects\pawn-errors\test.pwn:11 (warning) #1: i failed :(
F:\Projects\pawn-errors\test.pwn:25 (warning) #2: value was not equal to 5
F:\Projects\pawn-errors\test.pwn:39 (warning) #3: value was not odd
```

This format uses the same standard pattern that the vscode-pawn problem matcher
expects, that means your errors will show up in the editor if you use the
`Run Package` task:

![https://i.imgur.com/EP7uqs1.png](https://i.imgur.com/EP7uqs1.png)

### NoError and Semantics of Return Values

You can also return `NoError()` to indicate that the function did not fail,
however this function does take an argument:

```pawn
Error:temperature(input) {
    if(input > 100) {
        return Error(1, "Too hot to survive!");
    }
    if(input > 50) {
        return NoError(2); // can survive, but too hot to go outside
    }
    return NoError(); // it's cool
}
```

Here, the semantics are important. The first branch returns a full on error,
something has gone wrong that the function can not deal with internally and must
raise an error.

The second branch declares that there's no error, but it still returns a
non-zero exit code which indicates to the call site that the function did not
complete but it wasn't because of instability, it was merely something else less
important.

Take for example an account load function, it has three exit states:

- 1: Account was corrupt in some way
- 2: Account is banned
- 0: Account is fine

The first state is an error, something has gone wrong with the system that has
resulted in a corrupt file. The second state is more mild, the account wasn't
loaded because the user is banned, that's not an error that's just a situation
where the function did not complete but it was an expected outcome. And finally
the zero return code is the success state. Functions only ever need a single
success state, otherwise they are too complex.

Here's the code version of that example:

```pawn
stock Error:LoadPlayerAccount(playerid, file[], fileData[]) {
    new Error:error;

    error = ReadFile(file, fileData);
    if(IsError(error)) {
        return Error(1, "failed to read player account file");
    }

    if(fileData[E_PLAYER_BANNED]) {
        return NoError(2); // player is banned, no point doing more work
    }

    // do some processing on the player's account now that we know that
    // 1. it's not corrupt
    // 2. they are not banned

    return NoError(); // default value is 0
}
```

This pattern makes use of guard clauses as points in code to catch errors early
and return them up the stack to be handled.

## Testing

To run the tests:

```bash
sampctl package run
```
