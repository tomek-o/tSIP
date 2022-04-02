//---------------------------------------------------------------------------
#pragma hdrstop

#include "LuaWinapi.h"
#include "ScriptExec.h"	// sharing some functions
#include "lua.hpp"

//---------------------------------------------------------------------------

#pragma package(smart_init)

namespace
{

int l_WinapiFindWindow(lua_State* L)
{
	const char* className = lua_tostring(L, 1);
	const char* windowName = lua_tostring(L, 2);
	HWND hWnd = FindWindow(className, windowName);
	lua_pushnumber(L, reinterpret_cast<unsigned int>(hWnd));
	return 1;
}

int l_WinapiSendMessage(lua_State* L)
{
	unsigned int hWnd = lua_tointegerx(L, 1, NULL);
	unsigned int msg = lua_tointegerx(L, 2, NULL);
	unsigned int wParam = lua_tointegerx(L, 3, NULL);
	unsigned int lParam = lua_tointegerx(L, 4, NULL);
	int ret = SendMessage((HWND)hWnd, msg, wParam, lParam);
	lua_pushnumber(L, ret);
	return 1;
}

int l_WinapiGetAsyncKeyState(lua_State* L)
{
	int vKey = lua_tointegerx(L, 1, NULL);
	int ret = GetAsyncKeyState(vKey);
	lua_pushnumber(L, ret);
	return 1;
}

int l_WinapiPlaySound(lua_State* L)
{
	const char* pszSound = lua_tostring(L, 1);
	unsigned int hmod = lua_tointegerx(L, 2, NULL);
	unsigned fdwSound = lua_tointegerx(L, 3, NULL);
	int ret = PlaySound(pszSound, reinterpret_cast<HMODULE>(hmod), fdwSound);
	lua_pushnumber(L, ret);
	return 1;
}

int l_Winapi_keybd_event(lua_State* L)
{
	unsigned int bVk = lua_tointegerx(L, 1, NULL);
	unsigned int bScan = lua_tointegerx(L, 2, NULL);
	unsigned int dwFlags = lua_tointegerx(L, 3, NULL);
	unsigned int dwExtraInfo = 0;
	keybd_event(bVk, bScan, dwFlags, dwExtraInfo);
	return 0;
}

}	// namespace


int luaopen_tsip_winapi (lua_State *L)
{
	static const struct luaL_Reg tsip_winapi[] = {
		{"FindWindow", l_WinapiFindWindow},
		{"SendMessage", l_WinapiSendMessage},
		{"Beep", ScriptExec::l_Beep},
		{"MessageBox", ScriptExec::l_MessageBox},
		{"GetAsyncKeyState", l_WinapiGetAsyncKeyState},
		{"PlaySound", l_WinapiPlaySound},
		{"keybd_event", l_Winapi_keybd_event},
		{NULL, NULL}
	};
	luaL_newlib(L, tsip_winapi);
	return 1;
}