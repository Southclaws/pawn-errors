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


// DEBUG_PRINTS if set will make each Error() call also print the error
#if defined DEBUG_PRINTS
    #define _DEBUG_PRINTS
#endif

// PRINT_BACKTRACES if set will print backtraces on internal errors
#if defined PRINT_BACKTRACES
    #define _PRINT_BACKTRACES
#endif

// MAX_STACKED_ERRORS sets the maximum error depth
#if !defined MAX_STACKED_ERRORS
    #define MAX_STACKED_ERRORS (16)
#endif

// MAX_ERROR_MESSAGE controls the maximum size of an error message
#if !defined MAX_ERROR_MESSAGE
    #define MAX_ERROR_MESSAGE (256)
#endif

// MAX_ERROR_LOCATION controls the maximum size of an error location string
#if !defined MAX_ERROR_LOCATION
    #define MAX_ERROR_LOCATION (128)
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

// GetErrors returns the current error state.
forward GetErrors(output[], len = sizeof output);

// PrintError simply prints the current error state.
forward PrintError();

// When an Error: leaves scope without being handled, a nasty error is printed!
// forward operator~(Error:e[], i);

// NoError is to be returned by an Error: tagged function if it was successful.
const Error:NoError = Error:0;
const ErrorTag = tagof(Error:);


enum E_ERROR_INFO {
    // Each error is marked as unhandled (1) until Error or Handled is called.
    E_ERROR_STATE,
    // Each error has a short, concise message associated with it
    E_ERROR_MESSAGE[MAX_ERROR_MESSAGE],
    // A location is a file name and a line number
    E_ERROR_LOCATION[MAX_ERROR_LOCATION],
}

static
    // ErrorBuffer simply contains all error messages separated by newlines.
    ErrorBuffer[MAX_STACKED_ERRORS][E_ERROR_INFO],
    // A heap space to store large backtraces to avoid stack overflows.
    ErrorBacktrace[1024],
    // Since Pawn is procedural, Error IDs are simply an incrementing value.
    Count,
    // Error starts this timer and Handled kills it.
    ErrorSourceTimer;


stock Error:Error(what[]) {
    if(Count == MAX_STACKED_ERRORS - 1) {
        printf("[error] %d too many unhandled errors:\n%s", Count, ErrorBuffer);

        #if defined _PRINT_BACKTRACES
        PrintAmxBacktrace();
        #endif
    }

    new bufferIndex = Count;
    Count++;

    ErrorBuffer[bufferIndex][E_ERROR_MESSAGE][0] = EOS;
    ErrorBuffer[bufferIndex][E_ERROR_LOCATION][0] = EOS;

    // And finally, the actual error message itself.
    strcat(ErrorBuffer[bufferIndex][E_ERROR_MESSAGE], what, MAX_ERROR_MESSAGE);

    // Get a backtrace
    ErrorBacktrace[0] = EOS;
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
    strcat(
        ErrorBuffer[bufferIndex][E_ERROR_LOCATION],
        ErrorBacktrace[begin], MAX_ERROR_LOCATION);

    ErrorBuffer[bufferIndex][E_ERROR_STATE] = 1;
    if(bufferIndex == 0) {
        ErrorSourceTimer = SetTimer("_errors_resolveUnhandled", 0, 0);
    } else {
        ErrorBuffer[bufferIndex - 1][E_ERROR_STATE] = 0;
    }

    #if defined _DEBUG_PRINTS
    printf("[error] %d %s: %s",
        Count,
        ErrorBuffer[bufferIndex][E_ERROR_LOCATION],
        ErrorBuffer[bufferIndex][E_ERROR_MESSAGE]);
    #endif

    return Error:Count;
}

stock Handled(Error:e) {
    if(Count == 0) {
        print("attempt to handle non-present error");

        #if defined _PRINT_BACKTRACES
        PrintAmxBacktrace();
        #endif

        return 1;
    }

    if(e != Error:Count) {
        print("e != count in error handler");

        #if defined _PRINT_BACKTRACES
        PrintAmxBacktrace();
        #endif
    }

    #if defined _DEBUG_PRINTS
    printf("[handled] resolved error stack of %d errors", Count);
    #endif

    for(new i; i < Count; ++i) {
        ErrorBuffer[i][E_ERROR_STATE] = 0;
    }
    Count = 0;
    KillTimer(ErrorSourceTimer);

    return 0;
}

stock GetErrorCount() {
    return Count;
}

stock GetErrors(output[], len = sizeof output) {
    new line[MAX_ERROR_MESSAGE + MAX_ERROR_LOCATION];
    for(new i; i < Count; ++i) {
        format(
            line,
            sizeof line,
            "%s (warning) #%d: %s\n",
            ErrorBuffer[i][E_ERROR_LOCATION],
            i + 1,
            ErrorBuffer[i][E_ERROR_MESSAGE]
        );
        strcat(output, line, len);
    }
    return 0;
}

stock PrintError() {
    printf("[error] %d errors:\n%s", ErrorBuffer);
}

// stock Error:operator=(Error:e) {
//     new bufferIndex = _:e - 1;
//     printf("[assign] index %d", bufferIndex);
//     if(0 <= bufferIndex < Count) {
//         ErrorBuffer[bufferIndex][E_ERROR_STATE] = 1;
//     }
//     return e;
// }

// stock operator~(Error:e[], c) {
//     #pragma unused c

//     if(c == 0) {
//         print("[destruct] empty");
//         return;
//     }

//     if(e[0] == NoError) {
//         print("[destruct] NoError");
//         return;
//     }

//     new bufferIndex = _:e[0] - 1;

//     if(ErrorBuffer[bufferIndex][E_ERROR_STATE] == 0) {
//         print("[destruct] state 0");
//         return;
//     }

//     printf("\n[destruct] bufferIndex %d count %d", bufferIndex, Count);

//     printf("[error] Unhandled error #%d left scope:", bufferIndex + 1);
//     for(new j; j < Count; ++j) {
//         printf("%s (error) #%d: %s",
//             ErrorBuffer[j][E_ERROR_LOCATION],
//             j + 1,
//             ErrorBuffer[j][E_ERROR_MESSAGE]
//         );
//     }

//     Count = 0;

//     #if defined _PRINT_BACKTRACES
//     PrintAmxBacktrace();
//     #endif

//     return;
// }
