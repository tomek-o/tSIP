//---------------------------------------------------------------------------

#pragma hdrstop

#ifdef _DEBUG

#include "SelfTests.h"
#include "common/TelecomUtils.h"
#include "Log.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

void RunSelfTests(void)
{
	int failedCase;

	failedCase = TestExtractNumberFromUri();
	if (failedCase == 0)
	{
		LOG("SelfTests: TestExtractNumberFromUri OK\n");
	}
	else
	{
		LOG("SelfTests: TestExtractNumberFromUri FAILED at case #%d\n", failedCase);
	}
}

#endif // _DEBUG
