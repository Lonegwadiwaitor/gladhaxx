#pragma once

struct lua_State;
namespace rbx::implement {
	inline void* orequire;
	int require(lua_State* R);
	int getsimulationradius(lua_State* R);
	int setsimulationradius(lua_State* R);
	int fireclickdetector(lua_State* R);
	int firetouchinterest(lua_State* R);
	int fireproximityprompt(lua_State* R);
	int getinstances(lua_State* R);
	int getnilinstances(lua_State* R);
	int getsignalfunctions(lua_State* R);
	int getconnections(lua_State* R);
	void initialize_connection_metatable(lua_State* R);
}
