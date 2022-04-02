#include "Action.h"

const char* Action::getTypeDescription(enum Action::Type type)
{
	switch (type)
	{
	case TYPE_NONE:
		return "none";
	case TYPE_SHOWHIDE:
		return "show/hide app";
	case TYPE_BUTTON:
		return "button #";
	case TYPE_ANSWER:
		return "answer";
	case TYPE_HANGUP:
		return "hangup";
	case TYPE_REDIAL:
		return "redial";
	case TYPE_CALL_CLIPBOARD:
		return "call: clipboard";
	case TYPE_SHOWHIDE_SIDECAR:
		return "show/hide sidecar";
	case TYPE_ANSWER_HANGUP:
		return "answer/hangup";
	case TYPE_SCRIPT_FILE:
		return "script file";
	default:
		return "";
	}
}

