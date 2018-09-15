#ifndef ActionH
#define ActionH

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
		
		TYPE_LIMITER
	} type;
	int id;

	Action(void):
		type(TYPE_NONE),
		id(0)
	{
	}
	static const char* getTypeDescription(enum Type type);

};

#endif
