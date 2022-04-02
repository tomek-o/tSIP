#ifndef ScriptSourceH
#define ScriptSourceH

enum ScriptSource {
	SCRIPT_SRC_BUTTON = 0,
	SCRIPT_SRC_ON_MAKING_CALL,
	SCRIPT_SRC_ON_CALL_STATE,
	SCRIPT_SRC_ON_STREAMING_STATE,
	SCRIPT_SRC_ON_REGISTRATION_STATE,
	SCRIPT_SRC_ON_STARTUP,					// 5
	SCRIPT_SRC_ON_TIMER,
	SCRIPT_SRC_ON_DIALOG_INFO,
	SCRIPT_SRC_ON_DIAL,
	SCRIPT_SRC_SCRIPT_WINDOW,
	SCRIPT_SRC_ON_AUDIO_ERROR,				// 10
	SCRIPT_SRC_PLUGIN_QUEUE,
	SCRIPT_SRC_ON_CUSTOM_REQUEST_REPLY,
	SCRIPT_SRC_ON_CONTACT_NOTE_OPEN,
	SCRIPT_SRC_ON_RECORDER_STATE,
	SCRIPT_SRC_COMMAND_LINE,				// 15
	SCRIPT_SRC_BUTTON_MOUSE_UP_DOWN,
	SCRIPT_SRC_ON_ENCRYPTION_STATE,
	SCRIPT_SRC_HOTKEY,

	SCRIPT_SRC_LIMITER
};

#endif
