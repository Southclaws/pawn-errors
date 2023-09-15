#include "errors.inc"

#define RUN_TESTS
#include <YSI_Core\y_testing>


Error:failsOnTrue(bool:fails) {
	if(fails) {
		return Error(1, "i failed :(");
	}

	return Ok();
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

	return Ok();
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

	return Ok();
}

Error:failsOn5WarnsOn6(input) {
	if(input == 6) {
		return Ok(2);
	}
	if(input == 5) {
		return Error(1, "function incomplete, can not continue");
	}

	return Ok();
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
			wantFind[] = "Error:failsOnTrue (bool:fails=true)";
		GetErrors(gotError);
		printf("'%s'", gotError);
		ASSERT(strfind(gotError, wantFind) != -1);

		PrintErrors();
		ASSERT(Handled() == 0);
		ASSERT(GetErrorCount() == 0);
	}

	ASSERT(GetErrorCount() == 0);
	ASSERT(Handled(true) == 1);
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
			gotError[1024],
			wantFind[] = "(none)";
		GetErrors(gotError);
		printf("'%s'", gotError);
		ASSERT(strfind(gotError, wantFind) != -1);

		PrintErrors();
		ASSERT(Handled() == 0);
		ASSERT(GetErrorCount() == 0);
	}

	ASSERT(GetErrorCount() == 0);
	ASSERT(Handled(true) == 1);
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
			gotError[2048],
			wantFind[] = "value was not odd";
		GetErrors(gotError);
		print(gotError);
		ASSERT(strfind(gotError, wantFind) != -1);

		PrintErrors();
		ASSERT(Handled() == 0);
		ASSERT(GetErrorCount() == 0);
	}

	ASSERT(GetErrorCount() == 0);
	ASSERT(Handled(true) == 1);
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

Test:GetLastErrorCause() {
	print("\n\n\n--- GetLastErrorCause ---\n\n\n");
	{
		new Error:e;

		e = failsOnTrue(true);
		ASSERT(e == Error:1);

		new cause[128];
		new ret = GetLastErrorCause(cause);
		ASSERT(ret == 0);

		new
			wantFind[] = "i failed :(";
		printf("'%s'", cause);
		ASSERT(strfind(cause, wantFind) != -1);

		PrintErrors();
		ASSERT(Handled() == 0);
		ASSERT(GetErrorCount() == 0);
	}
}

Test:GetErrorCause() {
	print("\n\n\n--- GetErrorCause ---\n\n\n");
	{
		new Error:e;

		e = failsOnTrue(true);
		ASSERT(e == Error:1);

		e = failsOnTrue(true);
		ASSERT(e == Error:1);

		new cause[128];
		new ret = GetErrorCause(0, cause);
		ASSERT(ret == 0);

		new
			wantFind[] = "i failed :(";
		printf("'%s'", cause);
		ASSERT(strfind(cause, wantFind) != -1);

		ret = GetErrorCause(1, cause);
		printf("'%s'", cause);
		ASSERT(strfind(cause, wantFind) != -1);

		PrintErrors();
		ASSERT(Handled() == 0);
		ASSERT(GetErrorCount() == 0);
	}
}
