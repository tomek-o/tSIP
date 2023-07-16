//---------------------------------------------------------------------------


#pragma hdrstop

#include "Call.h"
#include "Calls.h"
#include "ControlQueue.h"
#include "Settings.h"
#include "buttons/ProgrammableButtons.h"
#include "Globals.h"
#include "Log.h"

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

void Call::setHold(bool state)
{
	if (connected == false && progress == false)
		state = false;
	if (hold == state)
	{
		LOG("Call %u: no need to change hold state to %d\n", uid, static_cast<int>(state));
		return;
	}
	LOG("Call %u: changing hold state to %d\n", uid, static_cast<int>(state));
	hold = state;
	UA->Hold(uid, state);
	if (uid == Calls::GetCurrentCallUid())
	{
		buttons.UpdateBtnState(Button::HOLD, state);
	}
}

void Call::setMute(bool state)
{
	if (connected == false && progress == false)
		state = false;
	if (mute == state)
	{
		LOG("Call %u: no need to change mute state to %d\n", uid, static_cast<int>(state));
		return;
	}
	LOG("Call %u: changing mute state to %d\n", uid, static_cast<int>(state));
	mute = state;
	UA->Mute(uid, state);
	buttons.UpdateBtnState(Button::MUTE, state);
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

AnsiString Call::getStateName(void) const
{
	return Callback::GetCallStateName(state);
}

AnsiString Call::getStateDescription(void) const
{
	if (sipReason != "")
		return sipReason;
	return Callback::GetCallStateDescription(state);
}


