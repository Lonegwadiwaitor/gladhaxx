#include "../../src/pch.hpp"

#include "../luau/VM/include/lua.h"
#include "../luau/VM/src/lapi.h"
#include "../luau/VM/src/lstate.h"
#include "../luau/VM/src/lgc.h"
#include "env.hpp"

#include "hooks.hpp"
#include "http.hpp"

#include "../runtime/async_closure.hpp"

#include "lualib.h"

int internal_httpget(lua_State* R) {
	rbx::runtime::yielding yield(R);

	std::string url = lua_tostring(R, 2);

	cpr::Header headers;

	headers.insert({ "User-Agent", "Roblox/WinInet" });

	auto result = cpr::Get(cpr::Url{ url }, headers);

	lua_pushstring(R, result.text.c_str());
	return 1;
}

lua_CFunction onamecall;
lua_CFunction oindex;

int namecall_hook(lua_State* R) {
	if (R->extraspace->identity == 8) {
		auto namecall_method = std::string_view(R->namecall->data);

		if (namecall_method == "GetObjects") { // pretty much same as below
			std::printf("nc getobj\n");
			auto args = lua_gettop(R);
			lua_pushcclosure(R, rbx::implement::getobjects, nullptr, 0);
			lua_insert(R, 1);
			lua_pcall(R, args, 1, 0);

			return 1;
		}

		if (namecall_method == "HttpGet" || namecall_method == "HttpGetAsync") {
			auto args = lua_gettop(R);
			lua_pushcclosure(R, internal_httpget, nullptr, 0);
			lua_insert(R, 1);
			lua_pcall(R, args, 1, 0);

			return lua_gettop(R);
		}
	}

	return onamecall(R);
}

int index_hook(lua_State* R) {
	if (R->extraspace->identity == 8) {
		//auto self = index2addr(R, 1);
		auto index = std::string_view(lua_tostring(R, 2));
		//size_t size;
		//std::printf("Self: %p, index: %s\n", luaL_tolstring(R, 1, &size), luaL_tolstring(R, 2, &size));

		if (index == "GetObjects") { // we infact have a re-implementation for this!
			std::printf("getobj\n");
			lua_pushcclosure(R, rbx::implement::getobjects, nullptr, 0);
			return 1;
		}

		if (index == "HttpGet" || index == "HttpGetAsync")
		{
			lua_pushcclosurek(R, internal_httpget, nullptr, 0, 0);
			return 1;
		}
	}

	return oindex(R);
}

int rbx::implement::hooks::init(lua_State* R) {
	lua_getglobal(R, "game");
	const TValue* obj = index2addr(R, -1);
	Table* object = uvalue(obj)->metatable;

	sethvalue(R, R->top, object)
	incr_top(R)

	lua_rawgetfield(R, -1, "__index");
	const auto index = clvalue(index2addr(R, -1));
	oindex = index->c.f;
	lua_pop(R, 1);

	lua_rawgetfield(R, -1, "__namecall");
	const auto namecall = clvalue(index2addr(R, -1));
	onamecall = namecall->c.f;
	lua_pop(R, 1);

	index->c.f = index_hook;
	namecall->c.f = namecall_hook;

	lua_pop(R, 2);

	return 1;
}