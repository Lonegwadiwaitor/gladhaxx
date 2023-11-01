#pragma once

struct lua_State;

namespace rbx {
	extern lua_State* base_state;
	extern lua_State* roblox_state;
	lua_State* newthread(lua_State* R, bool safe);
}
