#pragma once

struct lua_State;

namespace rbx::implement {
	int getrawmetatable(lua_State* R);
	int getnamecallmethod(lua_State* R);
	int setnamecallmethod(lua_State* R);
	int setreadonly(lua_State* R);
	int isreadonly(lua_State* R);
}
