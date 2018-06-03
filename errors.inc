// built-in include guard removal
// just in case the user has a local dependency with the same file name
#if defined _inc_errors
	#undef _inc_errors
#endif
// custom include-guard to ensure we don't duplicate
#if defined _errors_included
	#endinput
#endif
#define _errors_included


#include <crashdetect>


// PRINT_ON_ERROR if set will make each Error() call also print the error
#if defined PRINT_ON_ERROR
    #define _PRINT_ON_ERROR
#endif

// MAX_BUBBLED_ERRORS sets the maximum error depth
#if !defined MAX_BUBBLED_ERRORS
    #define MAX_BUBBLED_ERRORS (16)
#endif

// MAX_ERROR_STRING controls the maximum size of the error string buffer
#if !defined MAX_ERROR_STRING
    #define MAX_ERROR_STRING (MAX_BUBBLED_ERRORS * 128)
#endif


// Error should be called and the return value returned from any function that
// fails to do what it should.
forward Error:Error(what[], ...);

// Handled should be called on an error when it has been resolved and no longer
// needs to be stored. This erases all current errors and resets error state.
forward Handled(Error:e);

// GetErrorCount returns the amount of errors that are currently stacked up and
// awaiting being `Handled()`.
forward GetErrorCount();

// GetError returns the current error state.
forward GetError(output[], len = sizeof output);

// PrintError simply prints the current error state.
forward PrintError();

// When an Error: leaves scope without being handled, a nasty error is printed!
forward operator~(Error:e[], i);

// NoError is to be returned by an Error: tagged function if it was successful.
const Error:NoError = Error:0xFFFFFFFF;
const ErrorTag = tagof(Error:);

static
    // ErrorBuffer simply contains all error messages separated by newlines.
    ErrorBuffer[MAX_ERROR_STRING],
    // A heap space to store large backtraces to avoid stack overflows.
    ErrorBacktrace[1024],
    // Each error is marked as unhandled (1) until Error or Handled is called.
    ErrorState[MAX_BUBBLED_ERRORS],
    // Since Pawn is procedural, Error IDs are simply an incrementing value.
    Count;


stock Error:Error(what[]) {
    Count++;
    if(Count > 1) {
        if(Count == MAX_BUBBLED_ERRORS) {
            printf("[error] %d too many unhandled errors:\n%s", Count, ErrorBuffer);
            PrintAmxBacktrace();
        }
        strcat(ErrorBuffer, "\n");
    } else {
        ErrorBuffer[0] = EOS;
    }

    // Get a backtrace
    GetBacktrace(ErrorBacktrace);
    // Trim off the beginning by scanning for the end of the first line
    // lines look like:
    // - # addr in function (args) at file:line
    // first, find the location of the last element of the first trace line
    // then from that position, find the first ` at ` then add 4 to skip over.
    new begin = strfind(ErrorBacktrace, " at ", false,
        strfind(ErrorBacktrace, "errors.inc")
    ) + 4;
    // the end of the line is simply the location of the first \n starting from
    // the `begin` position.
    new end = strfind(ErrorBacktrace, "\n", false, begin);
    // zero out the end of the line
    ErrorBacktrace[end] = 0;

    // ErrorBacktrace[begin] points to the beginning of the line number
    strcat(ErrorBuffer, ErrorBacktrace[begin]);
    // This extra bit is to satisfy the vscode proglem matcher regex
    strcat(ErrorBuffer, " (runtime) ");
    // And finally, the actual error message itself.
    strcat(ErrorBuffer, what);

    ErrorState[Count] = 1;
    if(Count > 1) {
        ErrorState[Count - 1] = 0;
    }

    #if defined _PRINT_ON_ERROR
    printf("[ERROR] %d %s", Count, what);
    #endif

    return Error:Count;
}

stock Handled(Error:e) {
    if(Count == 0) {
        print("attempt to handle non-present error");
        PrintAmxBacktrace();
        return 1;
    }

    ErrorBuffer[0] = EOS;
    ErrorState[_:e] = 0;
    Count = 0;

    return 0;
}

stock GetErrorCount() {
    return Count;
}

stock GetError(output[], len = sizeof output) {
    return strcat(output, ErrorBuffer, len);
}

stock PrintError() {
    printf("[error] %d errors:\n%s", ErrorBuffer);
}

stock operator~(Error:e[], c) {
    for(new i; i < c; i++) {
        if(e[i] != NoError) {
            if(ErrorState[_:e[i]]) {
                printf("[error] %d unhandled errors:\n%s", _:e, ErrorBuffer);
                ErrorState[_:e[i]] = 0;
                ErrorBuffer[0] = EOS;
                Count = 0;
            }
        }
    }
    return;
}
