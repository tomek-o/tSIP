//---------------------------------------------------------------------------


#pragma hdrstop

#include "Hotkeys.h"
#include "HotKeyConf.h"
#include "Log.h"
#include "common/OS.h"
#include <windows.h>

//---------------------------------------------------------------------------

#pragma package(smart_init)

#define MOD_NOREPEAT 0x4000

HotKeys::HotKeys(void):
	nextId(MIN_ID)
{
}

const HotKeyConf* HotKeys::Find(const std::list<HotKeyConf> &conf, WORD &Key, TShiftState Shift, bool global)
{
	std::list<HotKeyConf>::const_iterator iter;
	for (iter = conf.begin(); iter != conf.end(); ++iter)
	{
		const HotKeyConf &cfg = *iter;
		if (cfg.global == global && Key == cfg.vkCode)
		{
			if 	(
					Shift.Contains(ssShift) == (bool)(cfg.modifiers & HotKeyConf::SHIFT) &&
					Shift.Contains(ssCtrl) == (bool)(cfg.modifiers & HotKeyConf::CTRL) &&
					Shift.Contains(ssAlt) == (bool)(cfg.modifiers & HotKeyConf::ALT) &&
					((cfg.modifiers & HotKeyConf::WIN) == 0) 
				)
			{
				return &cfg;
			}
		}
	}
	return NULL;
}

const HotKeyConf* HotKeys::FindGlobal(int id)
{
	std::list<GlobalHotKey>::iterator it;
	for (it = globalHotKeys.begin(); it != globalHotKeys.end(); ++it)
	{
		if (it->registered && it->id == id)
		{
			return &it->hotKeyConf;
		}
	}
	return NULL;
}

int HotKeys::RegisterGlobal(const std::list<HotKeyConf> &conf, HWND hwnd)
{
	int rc = 0;

	std::list<GlobalHotKey>::iterator it;
	// mark for removal by default
	for (it = globalHotKeys.begin(); it != globalHotKeys.end(); ++it)
	{
		it->remove = true;
	}

	std::list<HotKeyConf>::const_iterator iter;
	for (iter = conf.begin(); iter != conf.end(); ++iter)
	{
		const HotKeyConf &cfg = *iter;
		if (cfg.global == true)
		{
			bool exist = false;
			// check if already registered
			for (it = globalHotKeys.begin(); it != globalHotKeys.end(); ++it)
			{
				GlobalHotKey &ghk = *it;
				if (ghk.hotKeyConf.keyCode == cfg.keyCode && ghk.hotKeyConf.modifiers == cfg.modifiers)
				{
					exist = true;
					it->remove = false;
					break;
				}
			}
			if (exist == false)
			{
				GlobalHotKey newGhk;
				newGhk.hotKeyConf = cfg;
				globalHotKeys.push_back(newGhk);
				it = --globalHotKeys.end();
			}
			if (it->registered == false)
			{
				// try to register
				UINT fsModifiers = 0;
				if (IsWin7OrLater())
				{
					fsModifiers |= MOD_NOREPEAT;	// not supported by Vista and earlier, use own antirepeat
				}
				if (cfg.modifiers & HotKeyConf::CTRL)
				{
					fsModifiers |= MOD_CONTROL;
				}
				if (cfg.modifiers & HotKeyConf::SHIFT)
				{
					fsModifiers |= MOD_SHIFT;
				}
				if (cfg.modifiers & HotKeyConf::ALT)
				{
					fsModifiers |= MOD_ALT;
				}
				int id = FindNextId();
				if (RegisterHotKey(hwnd, id, fsModifiers, cfg.vkCode) == 0)
				{
					AnsiString desc;
					if (cfg.modifiers & HotKeyConf::CTRL)
					{
						desc = "Ctrl";
					}
					if (cfg.modifiers & HotKeyConf::SHIFT)
					{
						if (desc != "")
							desc += "+";
						desc += "Shift";
					}
					if (cfg.modifiers & HotKeyConf::ALT)
					{
						if (desc != "")
							desc += "+";
						desc += "Alt";
					}
					if (desc != "")
						desc += "+";
					desc += cfg.keyCode;
					LOG("Failed to register global hotkey %s\n", desc.c_str());
					rc++;
				}
				else
				{
					it->registered = true;
					it->id = id;
				}
			}
		}
	}


	// remove all marked for removal
	for (it = globalHotKeys.begin(); it != globalHotKeys.end(); )
	{
		if (it->remove)
		{
			if (it->registered)
			{
            	UnregisterHotKey(hwnd, it->id);
			}
			it = globalHotKeys.erase(it);
		}
		else
		{
			++it;
		}
	}
	
	return rc;
}

void HotKeys::Unregister(HWND hwnd)
{
	std::list<GlobalHotKey>::iterator it;
	for (it = globalHotKeys.begin(); it != globalHotKeys.end(); ++it)
	{
		if (it->registered)
		{
			UnregisterHotKey(hwnd, it->id);
			it->registered = false;
		}
	}
}

int HotKeys::FindNextId(void)
{
	for (int i=MIN_ID; i<=MAX_ID; i++)
	{
		std::list<GlobalHotKey>::iterator it;
		for (it = globalHotKeys.begin(); it != globalHotKeys.end(); ++it)
		{
			if (it->id == nextId)
			{
				break;
			}
		}
		if (it == globalHotKeys.end())
		{
			int rc = nextId;
			nextId++;
			if (nextId > MAX_ID)
			{
				nextId = 0;
			}
			return rc;
		}
	}
	return MIN_ID-1;
}


