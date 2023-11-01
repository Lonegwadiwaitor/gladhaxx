#pragma once

#include "../../src/pch.hpp"
#include "../../directx/draw/element.hpp"

struct lua_State;

namespace rbx::draw {
	void init(lua_State* R);
	extern void clear_element_cache();
	extern void clear_metatable_cache();
}
