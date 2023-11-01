#pragma once 

struct lua_State;

namespace rbx::implement {
	int checkcaller(lua_State* R);
	int iscclosure(lua_State* R);
	int islclosure(lua_State* R);
	int hookfunction(lua_State* R);
	int cclosure_handler(lua_State* R);
	int newcclosure(lua_State* R);
	int hook_metamethod(lua_State* R);
	int clone_ref(lua_State* R);
	int clone_function(lua_State* R);
	int filter_gc(lua_State* R);
	int is_our_closure(lua_State* R);
}