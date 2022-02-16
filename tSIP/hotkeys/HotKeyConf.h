#ifndef HotKeyConfH
#define HotKeyConfH

#include "Action.h"
#include <System.hpp>

struct HotKeyConf
{
	enum Modifier
	{
		SHIFT =	1<<0,
		CTRL =	1<<1,
		ALT =	1<<2,
		WIN =	1<<3
	};
	AnsiString keyCode;
	int vkCode;			///< duplicates keyCode, used to speed up mapping, not stored in configuration
	int modifiers;
	bool global;		///< register hotkey as global

	Action action;

	bool operator==(const HotKeyConf& right) const {
		if (keyCode != right.keyCode ||
			modifiers != right.modifiers ||
			global != right.global ||
			action != right.action)
		{
			return false;
		}
		return true;
	}

	HotKeyConf(void) {
		keyCode = "";
		vkCode = -1;
		modifiers = SHIFT | CTRL | ALT;
		global = false;
	}

	AnsiString GetDescription(void) const;
};

#endif
