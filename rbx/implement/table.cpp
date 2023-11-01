#include "../../src/pch.hpp"

#include "table.hpp"

#include "lua.h"
#include "lualib.h"
#include "../src/lapi.h"
#include "../src/lstate.h"

int rbx::implement::getrawmetatable(lua_State* R) {
	if (lua_getmetatable(R, 1)) {
		lua_setreadonly(R, -1, 0);
	}
	else {
		lua_pushnil(R);
	}
	

	return 1;
}

int rbx::implement::getnamecallmethod(lua_State* R) {
	lua_pushstring(R, R->namecall->data);
	return 1;
}

int rbx::implement::setnamecallmethod(lua_State* R) {
	luaL_checktype(R, 1, LUA_TSTRING);

	R->namecall = tsvalue(index2addr(R, 1));
	return 0;
}

int rbx::implement::setreadonly(lua_State* R) {
	luaL_checktype(R, 1, LUA_TTABLE);
	luaL_checktype(R, 2, LUA_TBOOLEAN);

	lua_setreadonly(R, 1, lua_toboolean(R, 2));
	return 1;
}

int rbx::implement::isreadonly(lua_State* R) {
	luaL_checktype(R, 1, LUA_TTABLE);

	lua_pushboolean(R, hvalue(index2addr(R, 1))->readonly);
	return 1;
}
