#ifndef ActionH
#define ActionH

#include <System.hpp>

struct Action
{
	enum Type
	{
		TYPE_NONE = 0,
		TYPE_SHOWHIDE,		///< show/hide app
		TYPE_BUTTON,		///< pressing console button
		TYPE_ANSWER,
		TYPE_HANGUP,
		TYPE_REDIAL,
		TYPE_CALL_CLIPBOARD,
		TYPE_SHOWHIDE_SIDECAR,	///< show/hide sidecar with programmable buttons
		TYPE_ANSWER_HANGUP,	///< answer or hangup depending on current call state
		TYPE_SCRIPT_FILE,

		TYPE_LIMITER
	} type;
	int id;
	AnsiString scriptFile;

	Action(void):
		type(TYPE_NONE),
		id(0)
	{
	}

	bool operator==(const Action& right) const {
		if (type != right.type ||
			id != right.id ||
            scriptFile != right.scriptFile
			)
		{
			return false;
		}
		return true;
	}

	bool operator!=(const Action& right) const {
    	return !(*this == right);
	}

	static const char* getTypeDescription(enum Type type);

};

#endif
