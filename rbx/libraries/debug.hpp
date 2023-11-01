#pragma once 

struct lua_State;

namespace rbx::debug {
	void init(lua_State* R);
}