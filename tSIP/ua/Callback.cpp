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


const char* Callback::GetRegStateName(enum reg_state_e state)
{
	switch (state)
	{
	case REG_STATE_UNREGISTER_OK:
		return "UNREGISTER_OK";
	case REG_STATE_REGISTERING:
		return "REGISTERING";
	case REG_STATE_REGISTER_OK:
		return "REGISTER_OK";
	case REG_STATE_REGISTER_FAIL:
		return "REGISTER_FAIL";
	case REG_STATE_UNREGISTERING:
		return "UNREGISTERING";
	case REG_STATE_UNREGISTER_FAIL:
		return "UNREGISTER_FAIL";
	default:
		return "???";
	}
}

const char* Callback::GetRegStateDescription(enum reg_state_e state)
{
	switch (state)
	{
	case REG_STATE_UNREGISTER_OK:
		return "Unregistered";
	case REG_STATE_REGISTERING:
		return "Registering";
	case REG_STATE_REGISTER_OK:
		return "Registered";
	case REG_STATE_REGISTER_FAIL:
		return "Error registering";
	case REG_STATE_UNREGISTERING:
		return "Unregistering...";
	case REG_STATE_UNREGISTER_FAIL:
		return "Unregistering error";
	default:
		return "???";
	}
}

AnsiString Callback::GetRegStateTranslatedName(enum reg_state_e state)
{
	switch (state)
	{
	case REG_STATE_UNREGISTER_OK:
		return Translate2("Registration.stateName.UNREGISTER_OK", GetRegStateName(state));
	case REG_STATE_REGISTERING:
		return Translate2("Registration.stateName.REGISTERING", GetRegStateName(state));
	case REG_STATE_REGISTER_OK:
		return Translate2("Registration.stateName.REGISTER_OK", GetRegStateName(state));
	case REG_STATE_REGISTER_FAIL:
		return Translate2("Registration.stateName.REGISTER_FAIL", GetRegStateName(state));
	case REG_STATE_UNREGISTERING:
		return Translate2("Registration.stateName.UNREGISTERING", GetRegStateName(state));
	case REG_STATE_UNREGISTER_FAIL:
		return Translate2("Registration.stateName.UNREGISTER_FAIL", GetRegStateName(state));
	default:
		return "???";
	}
}

AnsiString Callback::GetRegStateTranslatedDescription(enum reg_state_e state)
{
	switch (state)
	{
	case REG_STATE_UNREGISTER_OK:
		return Translate2("Registration.stateDescription.UNREGISTER_OK", GetRegStateDescription(state));
	case REG_STATE_REGISTERING:
		return Translate2("Registration.stateDescription.REGISTERING", GetRegStateDescription(state));
	case REG_STATE_REGISTER_OK:
		return Translate2("Registration.stateDescription.REGISTER_OK", GetRegStateDescription(state));
	case REG_STATE_REGISTER_FAIL:
		return Translate2("Registration.stateDescription.REGISTER_FAIL", GetRegStateDescription(state));
	case REG_STATE_UNREGISTERING:
		return Translate2("Registration.stateDescription.UNREGISTERING", GetRegStateDescription(state));
	case REG_STATE_UNREGISTER_FAIL:
		return Translate2("Registration.stateDescription.UNREGISTER_FAIL", GetRegStateDescription(state));
	default:
		return "???";
	}
}


