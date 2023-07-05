//---------------------------------------------------------------------------


#pragma hdrstop

#include "Call.h"
#include "ControlQueue.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

void Call::reset(void)
{
	Call defaultCall;
    *this = defaultCall;
}

void Call::hold(bool state)
{
	if (holdState == state)
		return;
	UA->Hold(uid, holdState);
}

