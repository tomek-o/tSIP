//---------------------------------------------------------------------------


#pragma hdrstop

#include "Call.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

void Call::reset(void)
{
	Call defaultCall;
    *this = defaultCall;
}

