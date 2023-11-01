#include "../../src/pch.hpp"

#include "env.hpp"

#include "lualib.h"

#include "../../libraries/os/output.hpp"

#include "../luau/VM/include/lua.h"
#include "../luau/VM/src/lapi.h"
#include "../luau/VM/src/lstate.h"
#include "../luau/VM/src/lmem.h"
#include "../luau/VM/src/ldebug.h"
#include "../luau/VM/src/lgc.h"

#include "../scheduler/task_scheduler.hpp"

#pragma optimize("", off)
void rbx::step_gc(lua_State* R, const std::function<gc_status(GCObject*)>& callback) {
	auto cur_page = R->global->allgcopages;

	while (cur_page)
	{
		char* start = nullptr;
		char* end = nullptr;
		auto block = 0;
		auto size = 0;

		luaM_getpagewalkinfo(cur_page, &start, &end, &block, &size);

		for (auto pos = start; pos != end; pos += size)
		{
			if (const auto gco = reinterpret_cast<GCObject*>(pos); (gco->gch.tt == LUA_TFUNCTION || gco->gch.tt == LUA_TTABLE || gco->gch.tt == LUA_TUSERDATA) && callback(gco) == GC_BREAK) {
				break;
			}
		}

		cur_page = cur_page->gcolistnext;
	}
};
#pragma optimize("", on)

int rbx::implement::getrenv(lua_State* R) {
	// ReSharper disable once CppUnreachableCode
	/*if (sys::DEBUG == 0 && !is_smh_state(R)) {
		return 0;
	}*/

	setobj(R, R->top, index2addr(R->global->mainthread, LUA_GLOBALSINDEX));
	incr_top(R);
	return 1;
}

int rbx::implement::getgenv(lua_State* R) {
	// ReSharper disable once CppUnreachableCode
	/*if (sys::DEBUG == 0 && !is_smh_state(R)) {
		return 0;
	}*/

	R->top->value.p = R->gt;
	R->top->tt = LUA_TTABLE;
	incr_top(R);
	return 1;
}

int rbx::implement::getgc(lua_State* R) {
	bool Table = luaL_optboolean(R, 1, 0);
	//DWORD nextPages = (DWORD)R->global->allgcopages;

	int idx = 0;
	lua_createtable(R, 1600, 0);

	global_State* global = R->global;
	auto cur_page = global->allgcopages;
	auto old_gcstate = global->gcstate;
	global->gcstate = -1; /* probably a good idea to pause the GC until we finish our work */

	while (cur_page)
	{
		char* start = nullptr;
		char* end = nullptr;
		auto block = 0;
		auto size = 0;

		luaM_getpagewalkinfo(cur_page, &start, &end, &block, &size);

		for (auto pos = start; pos != end; pos += size)
		{
			if (auto&& gco = reinterpret_cast<GCObject*>(pos); gco && gco->gch.tt == LUA_TFUNCTION || (Table && (gco->gch.tt == LUA_TTABLE || gco->gch.tt == LUA_TUSERDATA))) {
				if (isdead(R->global, gco)) {
					continue;
				}

				R->top->value.p = gco;
				R->top->tt = gco->cl.tt;
				incr_top(R);
				lua_rawseti(R, -2, ++idx);
			}
		}

		cur_page = cur_page->gcolistnext;
	}


	global->gcstate = old_gcstate;

	return 1;
}

#pragma optimize("", off)
int rbx::implement::getsenv(lua_State* R) {
	luaL_checktype(R, 1, LUA_TUSERDATA);

	luaL_getmetafield(R, 1, "__type");
	if (strcmp(lua_tostring(R, -1), "Instance") != 0) {
		lua_pop(R, 1);
		luaL_argerror(R, 1, "Instance expected");
	}

	lua_pop(R, 1);

	lua_getfield(R, 1, "ClassName");

	std::string_view str = lua_tostring(R, -1);

	lua_pop(R, 1);

	if (str == "LocalScript" || str == "ModuleScript") {
		auto script = *(std::uintptr_t*)lua_touserdata(R, 1);

		auto node = *reinterpret_cast<std::uintptr_t*>(script + 300);
		auto wtr = *reinterpret_cast<std::uintptr_t*>(node + 4);
		auto first_node = *reinterpret_cast<std::uintptr_t*>(wtr + 12);
		auto state_container = *reinterpret_cast<std::uintptr_t*>(first_node + 16);

		if (state_container) {
			// script is running
			lua_State* state = *reinterpret_cast<lua_State**>(state_container + 4);

			if (state)
			{
				if (const auto& ref_script = state->extraspace->current_script; ref_script && reinterpret_cast<std::uintptr_t>(ref_script) == script)
				{
					R->top->value.gc = (GCObject*)state->gt;
					R->top->tt = LUA_TTABLE;
					incr_top(R);
					return 1;
				}
			}
		} else {
			luaL_error(R, "Script is not running.");
		}

	} else {
		luaL_argerror(R, 1, "LocalScript/ModuleScript expected");
	}

	



	return 1;
}
#pragma optimize("", on)
                                                                                        // TODO: today: add getloadedmodules, fix getnilinstances, fix debug lib, reverse weakthreadref
int rbx::implement::getobjects(lua_State* R) {
	luaL_checktype(R, 1, LUA_TUSERDATA); // "game"
	luaL_checktype(R, 2, LUA_TSTRING); // "object", AKA rbxassetid://blahblahblah

	lua_getglobal(R, "game"); // self explanatory
	lua_getfield(R, -1, "GetService"); // find "GetService" in game
	lua_pushvalue(R, -2); // we're namecalling, so push "game" as first arg
	lua_pushstring(R, "InsertService"); // used to load assets
	lua_call(R, 2, 1); // this should never fail - we're OK to use lua_call.
	lua_remove(R, -2); // pop "game" since it won't be used again

	lua_getfield(R, -1, "LoadLocalAsset");

	lua_pushvalue(R, -2); /* InsertService */
	lua_pushvalue(R, 2); // push "object"
	lua_pcall(R, 2, 1, 0); // call that bitch!

	if (lua_type(R, -1) == LUA_TSTRING) { // it could error! EEK!
		luaL_error(R, lua_tostring(R, -1));
	}

	lua_createtable(R, 1, 0); // self explanatory x2
	lua_pushvalue(R, -2); // result from LoadLocalAsset
	lua_rawseti(R, -2, 1); // insert to table

	return 1;
}


int rbx::implement::getthreadidentity(lua_State* R) {
	lua_pushnumber(R, lua_getidentity(R));
	return 1;
}

int rbx::implement::setthreadidentity(lua_State* R) {
	luaL_checknumber(R, 1);

	if (!lua_isnumber(R, 1))
		luaL_argerror(R, 1, "identity expected");

	lua_setidentity(R, lua_tonumber(R, 1));

	return 0;
}

int rbx::implement::getcallingscript(lua_State* R) {
	if (R->extraspace->current_script) {
		reinterpret_cast<void(__cdecl*)(lua_State*, Instance**)>(push_inst)(R, &R->extraspace->current_script /* i kid you not this took like 6 hours of debugging */);
	} else {
		lua_pushnil(R);
	}

	return 1;
}

int rbx::implement::is_luau(lua_State* R) {
	lua_pushboolean(R, 1);
	return 1;
}

int rbx::implement::get_reg(lua_State* R) {
	lua_pushvalue(R, LUA_REGISTRYINDEX);
	return 1;
}

int rbx::implement::get_hidden_ui(lua_State* R) { // todo: internal recursive findfirstchild hook
	lua_getglobal(R, "cloneref");

	lua_getglobal(R, "game");
	lua_getfield(R, -1, "GetService");
	lua_pushvalue(R, -2);
	lua_pushstring(R, "CoreGui");
	lua_call(R, 2, 1);
	lua_remove(R, -2);

	lua_call(R, 1, 1);

	return 1;
}

int rbx::implement::set_fps_cap(lua_State* R) {
	luaL_checknumber(R, 1);

	const auto fps = lua_tonumber(R, 1);

	rbx::scheduler->set_fps_cap(fps);

	return 0;
}
