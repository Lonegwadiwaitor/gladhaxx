#pragma once

#include "../../src/pch.hpp"

struct lua_State;

namespace rbx::implement {
	void init(lua_State* R);
}
