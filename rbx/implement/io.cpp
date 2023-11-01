#include "../../src/pch.hpp"

#include "io.hpp"

#include "lualib.h"

// can't write nor read on UWP, therefore we use pipes to communicate with our UI which does all the work for us.
// TODO: finish


std::unordered_map<std::string, std::string> file_cache;



int rbx::implement::read_file(lua_State* R) {
	luaL_checktype(R, 1, LUA_TSTRING);

	const auto file = lua_tostring(R, 1);

	if (!file_cache.contains(file))
		luaL_error(R, "No such file exists");

	lua_pushstring(R, file_cache[file].data());
	return 1;
}
int rbx::implement::write_file(lua_State* R) {
	luaL_checktype(R, 1, LUA_TSTRING);
	luaL_checktype(R, 2, LUA_TSTRING);

	const auto file = lua_tostring(R, 1);
	const auto contents = lua_tostring(R, 2);

	file_cache[file] = contents; // temporary
	return 0;
}
int rbx::implement::append_file(lua_State* R) {
	luaL_checktype(R, 1, LUA_TSTRING);
	luaL_checktype(R, 2, LUA_TSTRING);

	const auto file = lua_tostring(R, 1);

	if (!file_cache.contains(file))
		luaL_error(R, "No such file exists");

	const auto contents = lua_tostring(R, 2);

	file_cache[file].append(contents);

	return 0;
}
int rbx::implement::load_file(lua_State* R) {
	return 0;
}
int rbx::implement::list_files(lua_State* R) {
	lua_createtable(R, 0, 0);

	int i = 0;

	for (auto file: file_cache) {
		lua_pushstring(R, file.first.data());
		lua_rawseti(R, -2, ++i);
	}

	return 1;
}
int rbx::implement::is_file(lua_State* R) {
	luaL_checktype(R, 1, LUA_TSTRING);

	const auto file = lua_tostring(R, 1);

	lua_pushboolean(R, file_cache.contains(file));
	return 1;
}
int rbx::implement::is_folder(lua_State* R) {
	lua_pushboolean(R, true);
	return 1;
}
int rbx::implement::make_folder(lua_State* R) {
	return 0;
}
int rbx::implement::delete_folder(lua_State* R) {
	return 0;
}
int rbx::implement::delete_file(lua_State* R) {
	luaL_checktype(R, 1, LUA_TSTRING);

	const auto file = lua_tostring(R, 1);

	if (!file_cache.contains(file)) {
		luaL_error(R, "File doesn't exist!");
	}

	file_cache.erase(file);
	return 0;
}
