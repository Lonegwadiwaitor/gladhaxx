#pragma once

#include "../../src/pch.hpp"

struct lua_State;

namespace rbx::runtime {
	using yieldable_cfunc = std::function<int(lua_State*)>;

	class yielding
	{
	public:
		yielding(lua_State* R);

		int execute(const std::function<yieldable_cfunc()>& func) const;

	private:
		struct ThreadRef {
			uint8_t _[4];
			lua_State* ls;

			ThreadRef(lua_State* L) : ls(L) {};
		};

		lua_State* L = nullptr;
	};
}