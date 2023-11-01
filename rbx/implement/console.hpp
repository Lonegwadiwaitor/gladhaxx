struct lua_State;

namespace rbx::implement::console {
	int init(lua_State* R);
	int rconsoleprint(lua_State* R);
	int rconsoleclear(lua_State* R);
	int rconsoleinfo(lua_State* R);
	int rconsoleerr(lua_State* R);
	int rconsolewarn(lua_State* R);
	int rconsoletitle(lua_State* R);
}