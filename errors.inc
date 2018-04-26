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


#define MAX_ERROR_STRING (512)

const Error:NoError = Error:0xFFFFFFFF;

static
    ErrorState[MAX_ERROR_STRING],
    Count,
    bool:ErrorLive;

stock Error:Error(what[], len = sizeof(what)) {
    Count++;
    if(Count > 1) {
        strcat(ErrorState, "\n");
    }
    strcat(ErrorState[Count], what, len);
    return Error:Count; // 0 is always no error
}

stock Handled(Error:e) {
    if(!ErrorLive) {
        print("attempt to handle non-present error");
        PrintAmxBacktrace();
    }

    Count = 0;
    ErrorLive = false;
    return;
}

stock operator=(Error:e) {
    // unused
}

stock operator~(Error:e[], i) {
    #pragma unused e, i
    if(ErrorLive) {
        print("unhandled error:");
        print(ErrorState);
        PrintAmxBacktrace();
    }
    return;
}
