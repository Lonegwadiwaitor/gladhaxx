struct lua_State;

namespace rbx::implement {
	int get_script_hash(lua_State* R);
	int get_script_bytecode(lua_State* R);
	int get_script_closure(lua_State* R);
	int load_string(lua_State* R);
	int identify_executor(lua_State* R);
}