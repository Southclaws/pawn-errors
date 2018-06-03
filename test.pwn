#define PRINT_ON_ERROR
#include "errors.inc"

#define RUN_TESTS
#include <YSI\y_testing>

Error:failsOnTrue(bool:fails) {
	if(fails) {
		return Error("i failed :(");
	}

	return NoError;
}

Error:failsOn5(input) {
	new bool:fails;
	if(input == 5) {
		fails = true;
	}

	new Error:e = failsOnTrue(fails);
	if(e) {
		return Error("value was not equal to 5");
	}

	return NoError;
}

Error:failsOnOdd(input) {
	new fails;
	if(input % 2 != 0) {
		fails = 5;
	}

	new Error:e = failsOn5(fails);
	if(e) {
		return Error("value was not odd");
	}

	return NoError;
}

Test:ErrorDepth1() {
	{
		new Error:e;

		e = failsOnTrue(true);
		ASSERT(e == Error:1);

		new count = GetErrorCount();
		ASSERT(count == 1);

		new
			gotError[512],
			wantError[] = "F:\\Projects\\pawn-errors\\test.pwn:9 (runtime) i failed :(";
		GetError(gotError);
		ASSERT(!strcmp(gotError, wantError));
		print(gotError);

		ASSERT(Handled(e) == 0);
		ASSERT(GetErrorCount() == 0);
	}
}

Test:ErrorDepth2() {
	{
		new Error:e;

		e = failsOn5(5);
		ASSERT(e == Error:2);

		new count = GetErrorCount();
		ASSERT(count == 2);

		new
			gotError[512],
			wantError[] = "F:\\Projects\\pawn-errors\\test.pwn:9 (runtime) i failed :(\nF:\\Projects\\pawn-errors\\test.pwn:23 (runtime) value was not equal to 5";
		GetError(gotError);
		ASSERT(!strcmp(gotError, wantError));
		print(gotError);

		ASSERT(Handled(e) == 0);
		ASSERT(GetErrorCount() == 0);
	}
}

Test:ErrorDepth3() {
	{
		new Error:e;

		e = failsOnOdd(5);
		ASSERT(e == Error:3);

		new count = GetErrorCount();
		ASSERT(count == 3);

		new
			gotError[512],
			wantError[] = "F:\\Projects\\pawn-errors\\test.pwn:9 (runtime) i failed :(\nF:\\Projects\\pawn-errors\\test.pwn:23 (runtime) value was not equal to 5\nF:\\Projects\\pawn-errors\\test.pwn:37 (runtime) value was not odd";
		GetError(gotError);
		ASSERT(!strcmp(gotError, wantError));
		print(gotError);

		ASSERT(Handled(e) == 0);
		ASSERT(GetErrorCount() == 0);
	}
}

Test:ErrorUnhandled() {
	{
		new Error:e;

		e = failsOnTrue(true);
		ASSERT(e == Error:1);

		new count = GetErrorCount();
		ASSERT(count == 1);

		new
			gotError[512],
			wantError[] = "F:\\Projects\\pawn-errors\\test.pwn:9 (runtime) i failed :(";
		GetError(gotError);
		ASSERT(!strcmp(gotError, wantError));
		print(gotError);
	}

	ASSERT(Handled(Error:0) == 1);
	ASSERT(GetErrorCount() == 0);
}
