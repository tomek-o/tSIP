#include "ScriptSource.h"

const char* GetScriptSourceName(enum ScriptSource source)
{
	switch (source) {
	case SCRIPT_SRC_BUTTON:
		return "button";
	case SCRIPT_SRC_ON_MAKING_CALL:
		return "on making call";
	case SCRIPT_SRC_ON_CALL_STATE:
		return "on call state";
	case SCRIPT_SRC_ON_STREAMING_STATE:
		return "on streaming state";
	case SCRIPT_SRC_ON_REGISTRATION_STATE:
		return "on registration state";
	case SCRIPT_SRC_ON_STARTUP:
		return "on startup";
	case SCRIPT_SRC_ON_TIMER:
		return "on timer";
	case SCRIPT_SRC_ON_DIALOG_INFO:
		return "on dialog info";
	case SCRIPT_SRC_ON_DIAL:
		return "on dial";
	case SCRIPT_SRC_SCRIPT_WINDOW:
		return "script window";
	case SCRIPT_SRC_ON_AUDIO_ERROR:
		return "on audio error";
	case SCRIPT_SRC_PLUGIN_QUEUE:
		return "plugin queue";
	case SCRIPT_SRC_ON_CUSTOM_REQUEST_REPLY:
		return "on custom request reply";
	case SCRIPT_SRC_ON_CONTACT_NOTE_OPEN:
		return "on contact note open";
	case SCRIPT_SRC_ON_RECORDER_STATE:
		return "on recorder state";
	case SCRIPT_SRC_COMMAND_LINE:
		return "command line";
	case SCRIPT_SRC_BUTTON_MOUSE_UP_DOWN:
		return "button mouse up/down";
	case SCRIPT_SRC_ON_ENCRYPTION_STATE:
		return "on encryption state";
	case SCRIPT_SRC_HOTKEY:
		return "hotkey";
	default:
		return "???";
	}
}
