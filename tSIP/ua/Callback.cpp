//---------------------------------------------------------------------------


#pragma hdrstop

#include "Callback.h"

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


