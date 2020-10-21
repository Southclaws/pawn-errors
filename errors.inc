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
forward Error:Error(code, const what[] = "");

// NoError should be called and the return value returned from any function that
// did not fail but exited with some semantic error code.
forward Error:NoError(code = 0);

// IsError is used for checking both if an `Error:` value contains an error code
// and that there are one or more errors on the error buffer.
bool:IsError(Error:e);

// Handled should be called on an error when it has been resolved and no longer
// needs to be stored. This erases all current errors and resets error state.
forward Handled();

// GetErrorCount returns the amount of errors that are currently stacked up and
// awaiting being `Handled()`.
forward GetErrorCount();

// GetErrors returns the current error state.
forward GetErrors(output[], len = sizeof output);

// PrintErrors simply prints the current error state.
forward PrintErrors();

// _E is a samp-logger field for adding error information to an err() call
forward LOGGER_FIELD:_E(Error:e);


const ErrorTag = tagof(Error:);


forward _errors_resolveUnhandled();

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


stock Error:Error(code, const what[] = "") {
    if(Count == MAX_STACKED_ERRORS - 1) {
        printf("[error] %d too many unhandled errors:", Count);
        _errors_resolveUnhandled();
    }

    #if defined _PRINT_BACKTRACES
    PrintAmxBacktrace();
    #endif

    new bufferIndex = Count;
    Count++;

    ErrorBuffer[bufferIndex][E_ERROR_MESSAGE][0] = EOS;
    ErrorBuffer[bufferIndex][E_ERROR_LOCATION][0] = EOS;

    if(what[0] == EOS) {
        strcat(
            ErrorBuffer[bufferIndex][E_ERROR_MESSAGE],
            "(passed)",
            MAX_ERROR_MESSAGE);
    } else {
        strcat(
            ErrorBuffer[bufferIndex][E_ERROR_MESSAGE],
            what,
            MAX_ERROR_MESSAGE);
    }

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
    if(end == -1) {
        end = strlen(ErrorBacktrace);
    } else if(end > strlen(ErrorBacktrace)) {
        end = strlen(ErrorBacktrace);
    }
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

    return Error:code;
}

public _errors_resolveUnhandled() {
    if(Count == 0) {
        return;
    }
    print("[error] UNHANDLED ERRORS:");
    PrintErrors();
    Count = 0;
}

stock Error:NoError(code = 0) {
    return Error:code;
}

stock bool:IsError(Error:e) {
    if(_:e > 0 && Count > 0) {
        return true;
    }
    return false;
}

stock Handled() {
    if(Count == 0) {
        print("attempt to handle non-present error");

        #if defined _PRINT_BACKTRACES
        PrintAmxBacktrace();
        #endif

        return 1;
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

stock PrintErrors() {
    printf("[error] %d errors:", Count);
    for(new i; i < Count; ++i) {
        printf(
            "%s (warning) #%d: %s",
            ErrorBuffer[i][E_ERROR_LOCATION],
            i + 1,
            ErrorBuffer[i][E_ERROR_MESSAGE]
        );
    }
}

stock LOGGER_FIELD:_E(Error:e) {
    new result[256];
    format(
        result,
        sizeof result,
        "errorCode=%d errorLoc=\"%s\" error=\"%s\"",
        _:e,
        ErrorBuffer[0][E_ERROR_LOCATION],
        ErrorBuffer[0][E_ERROR_MESSAGE]
    );

    return LOGGER_FIELD:result;
}
