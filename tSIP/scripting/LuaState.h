//---------------------------------------------------------------------------

#ifndef LuaStateH
#define LuaStateH
//---------------------------------------------------------------------------

#include "lua.hpp"

class Lua_State
{
	lua_State *L;
public:
	Lua_State(void):
		L(luaL_newstate())
	{
	}

	~Lua_State() {
		lua_close(L);
	}

	// implicitly act as a lua_State pointer
	inline operator lua_State*() {
		return L;
	}
};

#endif
