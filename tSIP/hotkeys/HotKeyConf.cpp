#include "HotKeyConf.h"

AnsiString HotKeyConf::GetDescription(void) const
{
	AnsiString ret;
	if (global)
		ret = "(GLOBAL) ";
	else
		ret = "(LOCAL)  ";
		
	if (modifiers & SHIFT)
		ret += "SHIFT+";
	if (modifiers & CTRL)
		ret += "CTRL+";
	if (modifiers & ALT)
		ret += "ALT+";
	ret += "[" + keyCode + "]";
	return ret;
}
