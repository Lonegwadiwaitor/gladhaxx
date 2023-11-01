#pragma once

#include "../../src/pch.hpp"
#include "../../directx/imgui/component.hpp"

struct lua_State;

namespace rbx::ig {
	void init(lua_State* R);
	extern void clear_component_cache();
	extern void clear_callback_cache();
	extern void clear_metatable_cache();
}
