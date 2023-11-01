#include "../../src/pch.hpp"

#include "util.hpp"

#include "../luau/VM/src/lapi.h"
#include "../luau/VM/src/lstate.h"

struct Color3 {
	float r;
	float g;
	float b;
};

struct Vector2 {
	float x;
	float y;
};

inline void** rbx::allocate(lua_State* R) {
	return static_cast<void**>(lua_newuserdata(R, sizeof(void**), 0));
}

inline bool rbx::compare(const char* str, const char* str1) {
	return strcmp(str, str1) == 0;
}

void rbx::push_method(lua_State* R, const char* name, lua_CFunction f, int nups) {
	lua_pushcclosure(R, f, name, nups);

	//makeshift_setfield(R, -2, name);
	lua_setfield(R, -2, name);
	std::cout << "successful pushmethod call\n";
}

void rbx::push_color3(lua_State* R, const ImVec4& vec) {
	lua_getglobal(R, "Color3");
	lua_getfield(R, -1, "new");

	lua_pushnumber(R, vec.x);
	lua_pushnumber(R, vec.y);
	lua_pushnumber(R, vec.z);
	
	lua_pcall(R, 3, 1, 0);
	lua_remove(R, -2);
}

ImVec4 rbx::to_color3(lua_State* R, int idx) {
	const auto [r, g, b] = *static_cast<Color3*>(lua_touserdata(R, idx));

	return { r, g, b, 1 };
}

inline bool rbx::ig::is_same_line(lua_State* R) {
	return lua_type(R, 3) == LUA_TBOOLEAN && lua_toboolean(R, 3);
}

void rbx::push_vector2(lua_State* R, const ImVec2& vec) {
	lua_getglobal(R, "Vector2");
	lua_getfield(R, -2, "new");

	lua_pushnumber(R, vec.x);
	lua_pushnumber(R, vec.y);

	lua_call(R, 2, 1);
	lua_remove(R, -2);
}

ImVec2 rbx::to_vector2(lua_State* R, int idx) {
	const auto [x, y] = *static_cast<Vector2*>(lua_touserdata(R, idx));

	return { x, y };
}
