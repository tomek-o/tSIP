//---------------------------------------------------------------------------

/** Locking portaudio: shared by UA and AudioDevicesList
*/

#pragma hdrstop

#include "PortaudioLock.h"
#include <portaudio.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

//---------------------------------------------------------------------------

#pragma package(smart_init)

static bool initialized = false;
static CRITICAL_SECTION cs;

extern "C" void lockFn(void)
{
	EnterCriticalSection(&cs);
}

extern "C" void unlockFn(void)
{
	LeaveCriticalSection(&cs);
}

int PortaudioLockInit(void)
{
	InitializeCriticalSection(&cs);
	Pa_SetLocks(lockFn, unlockFn);
	initialized = true;
	return 0;
}

void PortaudioLockShutdown(void)
{
	if (initialized)
		DeleteCriticalSection(&cs);
}
