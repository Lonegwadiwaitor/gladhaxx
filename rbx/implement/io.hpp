struct lua_State;

namespace rbx::implement {
	int read_file(lua_State* R);
	int write_file(lua_State* R);
	int append_file(lua_State* R);
	int load_file(lua_State* R);
	int list_files(lua_State* R);
	int is_file(lua_State* R);
	int is_folder(lua_State* R);
	int make_folder(lua_State* R);
	int delete_folder(lua_State* R);
	int delete_file(lua_State* R);
}