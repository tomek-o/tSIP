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

}	// namespace


int luaopen_tsip_winapi (lua_State *L)
{
	static const struct luaL_Reg tsip_winapi[] = {
		{"FindWindow", l_WinapiFindWindow},
		{"SendMessage", l_WinapiSendMessage},
		{"Beep", ScriptExec::l_Beep},
		{"MessageBox", ScriptExec::l_MessageBox},
		{"GetAsyncKeyState", l_WinapiGetAsyncKeyState},
		{NULL, NULL}
	};
	luaL_newlib(L, tsip_winapi);
	return 1;
}