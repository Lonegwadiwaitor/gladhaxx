#include "../../src/pch.hpp"

#include "instance.hpp"
#include "env.hpp"

#include "../libraries/util.hpp"

#include "../luau/VM/src/lstate.h"
#include <lualib.h>

#include "ldo.h"
#include "../luau/VM/src/lapi.h"

// these haven't worked in a while, never bothered fixing any of it until everything else worked

DWORD* get_client(lua_State* R) {
	lua_getglobal(R, "game");
	lua_getfield(R, -1, "GetService");
	lua_pushvalue(R, -2);
	lua_pushstring(R, "Players");
	lua_pcall(R, 2, 1, 0);
	lua_remove(R, -2); // pop game
	lua_getfield(R, -1, "LocalPlayer");
	lua_remove(R, -2); // pop players

	auto client = *static_cast<DWORD**>(lua_touserdata(R, -1));

	lua_pop(R, 1); // pop localplayer

	return client;
}

int rbx::implement::require(lua_State* R) {
	auto identity = lua_getidentity(R);

	lua_setidentity(R, 2);
	auto result = reinterpret_cast<lua_CFunction>(orequire)(R);
	lua_setidentity(R, identity);

	return result;
}

int rbx::implement::getsimulationradius(lua_State* R) {
	//lua_pushnumber(R, get_client(R)->simulationRadius);
	return 0;
}

int rbx::implement::setsimulationradius(lua_State* R) {
	/*auto client = get_client(R);
	client->simulationRadius = rbx::tonumber(R, 1);
	client->maxSimulationRadius = rbx::tonumber(R, 1);*/

	return 0;
}

int rbx::implement::fireclickdetector(lua_State* R) {
	luaL_checktype(R, 1, LUA_TUSERDATA);

	const auto click_detector = *static_cast<std::uintptr_t*>(lua_touserdata(R, 1));
	const auto dist = static_cast<float>(luaL_optnumber(R, 2, 0));
	const auto& player = (std::uintptr_t)get_client(R);

	reinterpret_cast<void(__thiscall*)(std::uintptr_t, std::uintptr_t)>(fireclickdetector_address)(click_detector, player);
	
	return 0;
}

struct __declspec(align(1)) struct_transmitter
{
	BYTE gap0[40];
	DWORD dword28;
	BYTE gap2C[344];
	DWORD world;
	BYTE gap188[138];
	BYTE byte212;
	BYTE gap213;
	BYTE byte214;
};


int rbx::implement::firetouchinterest(lua_State* R) {
	luaL_checktype(R, 1, LUA_TUSERDATA);
	luaL_checktype(R, 2, LUA_TUSERDATA);
	luaL_checktype(R, 3, LUA_TNUMBER);

	auto transmitter = *reinterpret_cast<struct_transmitter**>(*static_cast<uintptr_t*>(lua_touserdata(R, 1)) + 0xD8);
	uintptr_t to_touch = *reinterpret_cast<uintptr_t*>(*static_cast<uintptr_t*>(lua_touserdata(R, 2)) + 0xD8);
	uintptr_t world = transmitter->world;
	const int tt = lua_tointeger(R, 3);

	reinterpret_cast<void(__thiscall*)(std::uintptr_t, struct_transmitter*, std::uintptr_t, int)>(firetouchinterest_address)(world, transmitter, to_touch, tt);

	return 0;
}

int rbx::implement::fireproximityprompt(lua_State* R) {
	luaL_checktype(R, 1, LUA_TUSERDATA);

	const auto proximity_prompt = *static_cast<std::uintptr_t*>(lua_touserdata(R, 1));

	reinterpret_cast<int(__thiscall*)(std::uintptr_t)>(fireproximityprompt_address)(proximity_prompt);

	return 0;
}

__forceinline static auto get_inst_cache(lua_State* R) {
	lua_pushlightuserdata(R, (void*)push_inst);
	lua_gettable(R, LUA_REGISTRYINDEX /* 10000 */);

	return 1;
}

int rbx::implement::getinstances(lua_State* R) {
	int index = 1;

	lua_newtable(R);

	get_inst_cache(R);

	lua_pushnil(R);

	while (lua_next(R, -2) != 0) {
		if (lua_type(R, -1) == LUA_TUSERDATA) {
			auto instance = *static_cast<::Instance**>(lua_touserdata(R, -1));

			lua_pushinstance(R, (Instance*) & instance);
			lua_rawseti(R, -5, index++);
		}

		lua_pop(R, 1);
	}

	lua_pop(R, 1);

	lua_pushvalue(R, -1);

	lua_remove(R, -2);

	return 1;
}

int rbx::implement::getnilinstances(lua_State* R) {
	int index = 1;

	lua_newtable(R);

	get_inst_cache(R);

	lua_pushnil(R);

	while (lua_next(R, -2) != 0) {
		if (lua_type(R, -1) == LUA_TUSERDATA) {
			auto instance = *static_cast<void**>(lua_touserdata(R, -1));
			lua_getfield(R, -1, "Parent");

			if (lua_type(R, -1) == LUA_TNIL) {
				lua_pushinstance(R, (Instance*)&instance);
				lua_rawseti(R, -6, index++);
			}

			lua_pop(R, 1);
		}

		lua_pop(R, 1);
	}

	lua_pop(R, 1);

	lua_pushvalue(R, -1);

	lua_remove(R, -2);

	return 1;
}

class script_connection
{
public:
	class connection_ref
	{
	public:
		uint8_t _[0x60];
		int32_t id;
	};
public:
	bool enabled; 
	uint8_t _[12]; 
	script_connection* next;
	uint8_t __[8];
	connection_ref* ref;
};

static_assert(offsetof(script_connection, ref) == 0x1C, "Invalid script_connection ref offset");
static_assert(offsetof(script_connection::connection_ref, id) == 0x60, "Invalid connection_ref id offset");
static_assert(offsetof(script_connection, enabled) == 0, "Invalid connection_ref enabled offset");

// little struct shitty shit ima just give u my code, i dont have time

// getconnections seems to work in some sense but the .Function is always nil and connect/disconnect crashes

int rbx::implement::getsignalfunctions(lua_State* R) {
	luaL_error(R, "This function is currently inoperable.");

	lua_getfield(R, 1, "Connect");
	lua_pushvalue(R, 1);
	lua_pushcclosure(R, [](lua_State* R) { return 0; }, "", 0);
	lua_pcall(R, 2, 1, 0);

	auto connection = (*reinterpret_cast<script_connection**>(lua_touserdata(R, -1)))->next;

	lua_getfield(R, -1, "Disconnect");
	lua_pushvalue(R, -2);
	lua_pcall(R, 1, 0, 0);
	lua_pop(R, 2);

	lua_newtable(R);
	int i{};

	while (connection != nullptr) {
		if (connection->enabled) {
			lua_rawgeti(R, LUA_REGISTRYINDEX, connection->ref->id);

			lua_rawseti(R, -2, ++i);
		}

		connection = connection->next;
	}

	return 1;
}

static std::unordered_map<script_connection*, int> connection_map{};

void rbx::implement::initialize_connection_metatable(lua_State* R) {
	lua_pushcclosure(R, [](lua_State* R) {
		auto connection = *reinterpret_cast<script_connection**>(lua_touserdata(R, 1));

		if (connection_map.find(connection) != connection_map.end()) {
			lua_rawgeti(R, LUA_REGISTRYINDEX, connection_map[connection]);

			if (lua_type(R, -1) != LUA_TNIL) {
				lua_rawseti(R, LUA_REGISTRYINDEX, connection->ref->id);

				lua_pushnil(R);
				lua_rawseti(R, LUA_REGISTRYINDEX, connection_map[connection]);
			}
			else {
				lua_pop(R, 1);
			}
		}

		return 0;
	}, "", 0);
	lua_setfield(R, -10003, "__connection_enable");

	lua_pushcclosure(R, [](lua_State* R) {
		const auto connection = *reinterpret_cast<script_connection**>(lua_touserdata(R, 1));

		const bool exists = connection_map.contains(connection);

		if (exists) {
			lua_rawgeti(R, LUA_REGISTRYINDEX, connection_map[connection]);
		}

		const bool is_nil = lua_type(R, -1) == LUA_TNIL;

		lua_pop(R, 1);

		if (!exists || is_nil) {
			lua_rawgeti(R, LUA_REGISTRYINDEX, connection->ref->id);
			connection_map[connection] = lua_ref(R, LUA_REGISTRYINDEX);

			lua_pushcclosure(R, [](lua_State* R) { return 0; }, "", 0);
			lua_rawseti(R, LUA_REGISTRYINDEX, connection->ref->id);
		}

		return 0;
	}, "", 0);
	lua_setfield(R, -10003, "__connection_disable");

	lua_createtable(R, 0, 1);

	push_method(R, "__index", [](lua_State* R) {
		auto connection = *reinterpret_cast<script_connection**>(lua_touserdata(R, 1));
		const char* index = lua_tostring(R, 2);

		if (compare(index, "Function")) {
			if (connection_map.find(connection) != connection_map.end()) {
				lua_rawgeti(R, LUA_REGISTRYINDEX, connection_map[connection]);
			} else {
				lua_pushnil(R);
			}

			if (lua_type(R, -1) == LUA_TNIL) {
				lua_pop(R, -1);
				lua_rawgeti(R, LUA_REGISTRYINDEX, connection->ref->id);
			}
		} else if (compare(index, "Disabled")) {
			lua_rawgeti(R, LUA_REGISTRYINDEX, connection_map[connection]);
			lua_pushboolean(R, lua_type(R, -1) != LUA_TNIL);

			return 1;
		} else if (compare(index, "Enable")) {
			lua_setfield(R, -10003, "__connection_enable");
		} else if (compare(index, "Disable")) {
			lua_setfield(R, -10003, "__connection_disable");
		}  else {

			lua_pushnil(R);
		}
		return 1;
	}, 0);

	lua_setfield(R, -10003, "__connection_mt");
}

int rbx::implement::getconnections(lua_State* R) {
	luaL_error(R, "This function is currently inoperable.");
	lua_getfield(R, 1, "Connect");
	lua_pushvalue(R, 1);
	lua_pushcclosure(R, [](lua_State* R) { return 0; }, "", 0);
	lua_pcall(R, 2, 1, 0);

	auto connection = (*reinterpret_cast<script_connection**>(lua_touserdata(R, -1)))->next;

	lua_getfield(R, -1, "Disconnect");
	lua_pushvalue(R, -2);
	lua_pcall(R, 1, 0, 0);
	lua_pop(R, 2);

	lua_newtable(R);
	int i{};

	while (connection != nullptr) {
		if (connection->enabled) {
			*allocate(R) = connection;
			lua_setfield(R, -10003, "__connection_mt");
			lua_setmetatable(R, -2);
			lua_rawseti(R, -2, ++i);
		}

		connection = connection->next;
	}

	return 1;
}
