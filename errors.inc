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


#include <a_samp>
#include <crashdetect>


// MAX_BACKTRACE_SIZE is the size of the buffer to store backtrace in
#if !defined MAX_BACKTRACE_SIZE
    #define MAX_BACKTRACE_SIZE (2048)
#endif

// MAX_STACKED_ERRORS sets the maximum error depth
#if !defined MAX_STACKED_ERRORS
    #define MAX_STACKED_ERRORS (16)
#endif

// MAX_ERROR_MESSAGE controls the maximum size of an error message
#if !defined MAX_ERROR_MESSAGE
    #define MAX_ERROR_MESSAGE (256)
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
// needs to be stored. This erases all current errors and resets error state. If
// called while there are no errors present, will print an error unless `silent`
// is set to true.
forward Handled(silent = false);

// GetErrorCount returns the amount of errors that are currently stacked up and
// awaiting being `Handled()`.
forward GetErrorCount();

// GetLastErrorCause writes the most recent error cause text into `out`.
forward GetLastErrorCause(out[], len = sizeof out);

// GetErrorCause writes the specified error cause into `out`.
forward GetErrorCause(index, out[], len = sizeof out);

// GetLastErrorTrace writes the most recent error trace text into `out`.
forward GetLastErrorTrace(out[], len = sizeof out);

// GetErrorTrace writes the specified error trace into `out`.
forward GetErrorTrace(index, out[], len = sizeof out);

// GetErrors returns the current error state.
forward GetErrors(output[], len = sizeof output);

// PrintErrors simply prints the current error state.
forward PrintErrors();


forward _errors_resolveUnhandled();

static
    // CauseBuffer contains all stacked error messages.
    CauseBuffer[MAX_BACKTRACE_SIZE][MAX_ERROR_MESSAGE],
    // TraceBuffer contains all stacked error backtraces.
    TraceBuffer[MAX_STACKED_ERRORS][MAX_BACKTRACE_SIZE],
    // Since Pawn is procedural, Error IDs are simply an incrementing value.
    Count,
    // Error starts this timer and Handled kills it.
    ErrorSourceTimer;

// 91 characters long:
// 15 + 1 = "AMX backtrace:""
// 44 + 1 = "#0 native GetBacktrace () in crashdetect.DLL"
const FIRST_LINE_LEN = 91;


stock Error:Error(code, const what[] = "") {
    if(Count == MAX_STACKED_ERRORS - 1) {
        printf("[error] %d too many unhandled errors:", Count);
        // If we've hit the error limit, don't squash this error, instead handle
        // it and reset the index counter so this error can be buffered.
        _errors_resolveUnhandled();
        // Kill the existing deferred handler timer so a new one can be started.
        // This will happen because the resolveUnhandled function resets Count.
        KillTimer(ErrorSourceTimer);
    }

    new bufferIndex = Count;
    Count++;

    // store the error cause and backtrace into the buffer.
    if(what[0] == EOS) {
        format(CauseBuffer[bufferIndex], MAX_ERROR_MESSAGE, "(none)");
    } else {
        format(CauseBuffer[bufferIndex], MAX_ERROR_MESSAGE, what);
    }
    GetBacktrace(TraceBuffer[bufferIndex]);

    // if this is the first error being generated within this call stack, defer
    // a function call to handle unresolved errors in the future.
    if(bufferIndex == 0) {
        ErrorSourceTimer = SetTimer("_errors_resolveUnhandled", 0, 0);
    }

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

stock Handled(silent = false) {
    if(Count == 0) {
        if(!silent) {
            print("[debug] attempt to handle non-present error");
            PrintAmxBacktrace();
        }
        return 1;
    }

    Count = 0;
    KillTimer(ErrorSourceTimer);

    return 0;
}

stock GetErrorCount() {
    return Count;
}

stock GetLastErrorCause(out[], len = sizeof out) {
    format(out, len, CauseBuffer[Count - 1]);
    return 0;
}

stock GetErrorCause(index, out[], len = sizeof out) {
    if(index >= Count) {
        return 1;
    }
    format(out, len, CauseBuffer[index]);
    return 0;
}

stock GetLastErrorTrace(out[], len = sizeof out) {
    format(out, len, TraceBuffer[Count - 1]);
    return 0;
}

stock GetErrorTrace(index, out[], len = sizeof out) {
    if(index >= Count) {
        return 1;
    }
    format(out, len, TraceBuffer[index]);
    return 0;
}

stock GetErrors(output[], len = sizeof output) {
    new
        start, end,
        line[256];

    format(output, len, "[error] Error stack with %d errors:\n", Count);
    for(new i; i < Count; ++i) {
        format(output, len, "%s[error] Error #%d: '%s' trace:\n", output, i, CauseBuffer[i]);

        // skip the first two lines, they are useless.
        start = strfind(TraceBuffer[i], "\n", false, end + FIRST_LINE_LEN) + 1;
        end = strfind(TraceBuffer[i], "\n", false, start);
        if(end == -1) {
            end = strlen(TraceBuffer[i]); // when the trace is only 3 lines long
        }
        do {
            start = strfind(TraceBuffer[i], " ", false, start) + 1;
            strmid(line, TraceBuffer[i], start, end);

            // this is the only difference between Print and Get
            format(line, sizeof line, "[error] %s\n", line);
            strcat(output, line, len);

            start = end + 1;
            end = strfind(TraceBuffer[i], "\n", false, start);
        } while(end != -1);
    }
}

stock PrintErrors() {
    new
        start, end,
        line[256];

    printf("[error] Error stack with %d errors:", Count);
    for(new i; i < Count; ++i) {
        printf("[error] Error #%d: '%s' trace:", i, CauseBuffer[i]);

        // skip the first two lines, they are useless.
        start = strfind(TraceBuffer[i], "\n", false, end + FIRST_LINE_LEN) + 1;
        end = strfind(TraceBuffer[i], "\n", false, start);
        if(end == -1) {
            end = strlen(TraceBuffer[i]); // when the trace is only 3 lines long
        }
        do {
            start = strfind(TraceBuffer[i], " ", false, start) + 1;
            strmid(line, TraceBuffer[i], start, end);

            printf("[error] %s", line);

            start = end + 1;
            end = strfind(TraceBuffer[i], "\n", false, start);
        } while(end != -1);
    }
}
