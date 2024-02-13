//---------------------------------------------------------------------------


#pragma hdrstop

#include "Callback.h"
#include "Translate.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

const char* Callback::GetCallStateName(enum ua_state_e state)
{
	switch (state)
	{
	case CALL_STATE_CLOSED:
		return "CLOSED";
	case CALL_STATE_INCOMING:
		return "INCOMING";
	case CALL_STATE_OUTGOING:
		return "OUTGOING";
	case CALL_STATE_TRYING:
		return "TRYING";
	case CALL_STATE_RINGING:
		return "RINGING";
	case CALL_STATE_PROGRESS:
		return "PROGRESS";
	case CALL_STATE_ESTABLISHED:
		return "ESTABLISHED";
	case CALL_STATE_TRANSFER:
		return "TRANSFER";
	case CALL_STATE_TRANSFER_OOD:
		return "TRANSFER OOD";

	default:
		return "???";
	}
}

const char* Callback::GetCallStateDescription(enum ua_state_e state)
{
	switch (state)
	{
	case CALL_STATE_CLOSED:
		return " - IDLE - ";
	case CALL_STATE_INCOMING:
		return "Incoming call";
	case CALL_STATE_OUTGOING:
		return "Calling...";
	case CALL_STATE_TRYING:
		return "Trying...";
	case CALL_STATE_RINGING:
		return "Ringback";
	case CALL_STATE_PROGRESS:
		return "Call state progress";
	case CALL_STATE_ESTABLISHED:
		return "Connected";
	case CALL_STATE_TRANSFER:
		return "Transfer";
	case CALL_STATE_TRANSFER_OOD:
		return "Transfer OOD";

	default:
		return "???";
	}
}

AnsiString Callback::GetCallStateTranslatedName(enum ua_state_e state)
{
	switch (state)
	{
	case CALL_STATE_CLOSED:
		return Translate2("Call.stateName.CLOSED", GetCallStateName(state));
	case CALL_STATE_INCOMING:
		return Translate2("Call.stateName.INCOMING", GetCallStateName(state));
	case CALL_STATE_OUTGOING:
		return Translate2("Call.stateName.OUTGOING", GetCallStateName(state));
	case CALL_STATE_TRYING:
		return Translate2("Call.stateName.TRYING", GetCallStateName(state));
	case CALL_STATE_RINGING:
		return Translate2("Call.stateName.RINGING", GetCallStateName(state));
	case CALL_STATE_PROGRESS:
		return Translate2("Call.stateName.PROGRESS", GetCallStateName(state));
	case CALL_STATE_ESTABLISHED:
		return Translate2("Call.stateName.ESTABLISHED", GetCallStateName(state));
	case CALL_STATE_TRANSFER:
		return Translate2("Call.stateName.TRANSFER", GetCallStateName(state));
	case CALL_STATE_TRANSFER_OOD:
		return Translate2("Call.stateName.TRANSFER_OOD", GetCallStateName(state));

	default:
		return "???";
	}
}

AnsiString Callback::GetCallStateTranslatedDescription(enum ua_state_e state)
{
	switch (state)
	{
	case CALL_STATE_CLOSED:
		return Translate2("Call.stateDescription.CLOSED", GetCallStateDescription(state));
	case CALL_STATE_INCOMING:
		return Translate2("Call.stateDescription.INCOMING", GetCallStateDescription(state));
	case CALL_STATE_OUTGOING:
		return Translate2("Call.stateDescription.OUTGOING", GetCallStateDescription(state));
	case CALL_STATE_TRYING:
		return Translate2("Call.stateDescription.TRYING", GetCallStateDescription(state));
	case CALL_STATE_RINGING:
		return Translate2("Call.stateDescription.RINGING", GetCallStateDescription(state));
	case CALL_STATE_PROGRESS:
		return Translate2("Call.stateDescription.PROGRESS", GetCallStateDescription(state));
	case CALL_STATE_ESTABLISHED:
		return Translate2("Call.stateDescription.ESTABLISHED", GetCallStateDescription(state));
	case CALL_STATE_TRANSFER:
		return Translate2("Call.stateDescription.TRANSFER", GetCallStateDescription(state));
	case CALL_STATE_TRANSFER_OOD:
		return Translate2("Call.stateDescription.TRANSFER_OOD", GetCallStateDescription(state));

	default:
		return "???";
	}
}

