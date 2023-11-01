#pragma once

struct lua_State;

namespace rbx::implement {
	int mousemoverel(lua_State* R);
	int mouse1click(lua_State* R);
	int mouse1down(lua_State* R);
	int mouse1up(lua_State* R);
	int mouse2click(lua_State* R);
	int mouse2down(lua_State* R);
	int mouse2up(lua_State* R);
}