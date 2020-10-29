#define NO_BACKTRACES
#define DEBUG_PRINTS
#include "errors.inc"

#define RUN_TESTS
#include <YSI_Core\y_testing>


Error:failsOnTrue(bool:fails) {
	if(fails) {
		return Error(1, "i failed :(");
	}

	return NoError();
}

Error:failsOn5(input) {
	new bool:fails;
	if(input == 5) {
		fails = true;
	}

	new Error:e = failsOnTrue(fails);
	if(IsError(e)) {
		return Error(1);
	}

	return NoError();
}

Error:failsOnOdd(input) {
	new fails;
	if(input % 2 != 0) {
		fails = 5;
	}

	new Error:e = failsOn5(fails);
	if(IsError(e)) {
		return Error(1, "value was not odd");
	}

	return NoError();
}

Error:failsOn5WarnsOn6(input) {
	if(input == 6) {
		return NoError(2);
	}
	if(input == 5) {
		return Error(1, "function incomplete, can not continue");
	}

	return NoError();
}

Test:ErrorDepth1() {
	print("\n\n\n--- ErrorDepth1 ---\n\n\n");
	{
		new Error:e;

		e = failsOnTrue(true);
		ASSERT(e == Error:1);

		new count = GetErrorCount();
		ASSERT(count == 1);

		new
			gotError[512],
			wantFind[] = "(error) #1: i failed :(\n";
		GetErrors(gotError);
		printf("'%s'", gotError);
		ASSERT(strfind(gotError, wantFind) != -1);

		PrintErrors();
		ASSERT(Handled() == 0);
		ASSERT(GetErrorCount() == 0);
	}

	ASSERT(GetErrorCount() == 0);
	ASSERT(Handled() == 1);
}

Test:ErrorDepth2() {
	print("\n\n\n--- ErrorDepth2 ---\n\n\n");
	{
		new Error:e;

		e = failsOn5(5);
		ASSERT(e == Error:1);

		new count = GetErrorCount();
		ASSERT(count == 2);

		new
			gotError[512],
			wantFind[] = "(error) #2: (none)";
		GetErrors(gotError);
		print(gotError);
		ASSERT(strfind(gotError, wantFind) != -1);

		PrintErrors();
		ASSERT(Handled() == 0);
		ASSERT(GetErrorCount() == 0);
	}

	ASSERT(GetErrorCount() == 0);
	ASSERT(Handled() == 1);
}

Test:ErrorDepth3() {
	print("\n\n\n--- ErrorDepth3 ---\n\n\n");
	{
		new Error:e;

		e = failsOnOdd(5);
		ASSERT(e == Error:1);

		new count = GetErrorCount();
		ASSERT(count == 3);

		new
			gotError[512],
			wantFind[] = "(error) #3: value was not odd\n";
		GetErrors(gotError);
		print(gotError);
		ASSERT(strfind(gotError, wantFind) != -1);

		PrintErrors();
		ASSERT(Handled() == 0);
		ASSERT(GetErrorCount() == 0);
	}

	ASSERT(GetErrorCount() == 0);
	ASSERT(Handled() == 1);
}

Test:ErrorNoneWithCode() {
	print("\n\n\n--- ErrorNoneWithCode ---\n\n\n");
	{
		new Error:e;

		e = failsOn5WarnsOn6(5);
		ASSERT(e == Error:1);

		new count = GetErrorCount();
		printf("%d", count);
		ASSERT(count == 1);

		new
			gotError[512];
		GetErrors(gotError);
		print(gotError);

		Handled();

		e = failsOn5WarnsOn6(6);
		ASSERT(e == Error:2);

		ASSERT(GetErrorCount() == 0);
	}

	ASSERT(GetErrorCount() == 0);
}

Test:ErrorUnhandled() {
	print("\n\n\n--- ErrorUnhandled ---\n\n\n");
	{
		new Error:e;

		e = failsOnTrue(true);
		ASSERT(e == Error:1);

		new count = GetErrorCount();
		ASSERT(count == 1);

		new
			gotError[512];
		GetErrors(gotError);
		print(gotError);
	}

	ASSERT(GetErrorCount() != 0);
}
