//---------------------------------------------------------------------------


#pragma hdrstop

#include "Call.h"
#include "ControlQueue.h"
#include "Settings.h"

#include <ExtCtrls.hpp>

//---------------------------------------------------------------------------

#pragma package(smart_init)

Call::~Call(void)
{
	if (tmrAutoAnswer)
		delete tmrAutoAnswer;
}

void Call::reset(void)
{
	if (tmrAutoAnswer)
	{
		delete tmrAutoAnswer;
		tmrAutoAnswer = NULL;
	}
	Call defaultCall;
    *this = defaultCall;
}

void Call::hold(bool state)
{
	if (holdState == state)
		return;
	UA->Hold(uid, holdState);
}

AnsiString Call::getPeerUri(void) const
{
	if (appSettings.Display.bUsePAssertedIdentity)
	{
		if (paiPeerUri != "")
			return paiPeerUri;
	}
	return uri;
}

AnsiString Call::getPeerName(void) const
{
	if (appSettings.Display.bUsePAssertedIdentity)
	{
		if (paiPeerUri != "")	// using uri to check if header line is present as it shouldn't be empty
			return paiPeerName;
	}
	return peerName;
}

