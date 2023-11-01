#include "../../src/pch.hpp"

#include "debug.hpp"

#include "lmem.h"
#include "lualib.h"
#include "util.hpp"

#include "../sync/rapi.hpp"
#include "../../libraries/os/output.hpp"

#include "../luau/VM/src/lapi.h"
#include "../luau/VM/src/lstate.h"
#include "../luau/VM/src/ldebug.h"
#include "../luau/VM/src/lgc.h"
#include "../luau/VM/src/ltable.h"
#include "../luau/VM/src/lfunc.h"

static int getmetatable(lua_State* R) {
	if (lua_getmetatable(R, 1)) {
		lua_setreadonly(R, -1, 0);
	}
	else {
		lua_pushnil(R);
	}

	return 1;
}

__forceinline Closure* get_closure(lua_State* R) {
	int level;

	if (lua_isnumber(R, 1)) {
		level = lua_tointeger(R, 1);
		luaL_argcheck(R, level >= 0, 1, "negative level");
	}
	else if (lua_isfunction(R, 1))
		level = -lua_gettop(R);
	else
		luaL_argerror(R, 1, "function or level expected");

	lua_Debug ar{};

	if (!lua_getinfo(R, level, "f", &ar))
		luaL_argerror(R, 1, "invalid level");

	auto&& closure = clvalue(index2addr(R, -1));

	lua_pop(R, 1);

	return closure;
}

static int setmetatable(lua_State* L) {
	int t = lua_type(L, 2);
	luaL_argcheck(L, t == LUA_TNIL || t == LUA_TTABLE, 2,
		"nil or table expected");
	lua_settop(L, 2);
	lua_pushboolean(L, lua_setmetatable(L, 1));

	return 1;
}

static int getupvalues(lua_State* R) {
	luaL_argcheck(R, lua_isfunction(R, 1) || lua_isnumber(R, 1), 1, "function or number expected");

	const auto closure = get_closure(R);

	if (closure->isC) {
		std::printf("%s\n", (const char*)closure->c.debugname);
	}

	lua_createtable(R, 0, 0);

	setclvalue(R, R->top, closure);
	incr_top(R);

	for (auto i = 0; i < closure->nupvalues;)
	{
		lua_getupvalue(R, -1, ++i);
		lua_rawseti(R, -3, i);
	}

	lua_pop(R, 1);

	return 1;
}

static int getupvalue(lua_State* R) {
	const auto closure = get_closure(R);

	const auto index = lua_tointeger(R, 2);

	setclvalue(R, R->top, closure);
	incr_top(R);

	if (index < 1 || index > closure->nupvalues)
		luaL_error(R, "index out of range");

	lua_getupvalue(R, -1, index);

	lua_remove(R, -2);

	return 1;
}

static int ssetupvalue(lua_State* R) { // TODO: potentially fix luau optimization bug
	luaL_argcheck(R, lua_isfunction(R, 1) || lua_isnumber(R, 1), 1, "function or number expected");
	luaL_argcheck(R, lua_isnumber(R, 2), 2, "index expected");

	const auto closure = get_closure(R);
	auto obj = index2addr(R, 3);

	if (closure->isC)
		luaL_argerror(R, 1, "lua function expected");

	if (!closure->isC)
	{
		const auto index = lua_tointeger(R, 2);

		if (index < 1 || index > closure->nupvalues)
			luaL_argerror(R, 2, "index out of range");

		/*lua_pushvalue(R, 3);
		lua_setupvalue(R, -2, index);*/

		setobj(R, &closure->l.uprefs[index - 1], obj);
	}

	return 0;
}

static int getconstants(lua_State* R) {
	if (lua_gettop(R) == 0)
		luaL_argerror(R, 1, "function or level expected");

	const auto closure = get_closure(R);

	if (closure->isC)
		luaL_argerror(R, 1, "lua function expected");

	const Proto* cl_proto = closure->l.p;

	lua_createtable(R, cl_proto->sizek, 0);

	for (int i = 0; i < cl_proto->sizek;) {
		auto&& k = &cl_proto->k[i];

		if (k->tt == LUA_TTABLE || k->tt == LUA_TFUNCTION) {
			lua_pushnil(R);
		}
		else
		{
			setobj(R, R->top, k)
			incr_top(R);
		}

		lua_rawseti(R, -2, ++i);
	}

	return 1;
}

static int getconstant(lua_State* R) {
	const auto closure = get_closure(R);

	if (closure->isC)
		luaL_argerror(R, 1, "lua function expected");

	if (!lua_isnumber(R, 2))
		luaL_argerror(R, 2, "number expected");

	const int idx = lua_tonumber(R, 2);

	luaL_argcheck(R, idx > 0, 2, "negative index");

	const Proto* cl_proto = closure->l.p;

	luaL_argcheck(R, idx, 2, "index out of range");
	luaL_argcheck(R, idx <= cl_proto->sizek, 2, "index out of range");

	const auto& k = cl_proto->k[idx - 1];

	setobj(R, R->top, &k)
	incr_top(R)

	return 1;
}

static int setconstant(lua_State* R) {
	switch (lua_gettop(R)) {
	case 0:
		luaL_argerror(R, 1, "function or level expected");
	case 1:
		luaL_argerror(R, 2, "index expected");
	case 2:
		luaL_argerror(R, 3, "new constant expected");
	}

	const auto closure = get_closure(R);

	if (closure->isC)
		luaL_argerror(R, 1, "lua function expected");

	if (!closure->isC)
	{
		const int idx = lua_tonumber(R, 2);

		luaL_argcheck(R, idx > 0, 2, "negative index");

		const Proto* cl_proto = closure->l.p;

		luaL_argcheck(R, idx, 2, "index out of range");
		luaL_argcheck(R, idx <= cl_proto->sizek, 2, "index out of range");

		setobj(R, &cl_proto->k[idx - 1], index2addr(R, 3))
	}

	return 0;
}

static int getprotos(lua_State* R) {
	const auto closure = get_closure(R);

	if (closure->isC)
		luaL_argerror(R, 1, "lua function expected");

	Proto* original = closure->l.p;

	lua_createtable(R, original->sizep, 0);

	for (int i = 0; i < original->sizep;) {
		Proto* p = luaF_clonebrutallyfuckedproto(R, original->p[i]);
		setclvalue(R, R->top, luaF_newLclosure(R, closure->nupvalues, closure->env, p))
		incr_top(R)
		lua_rawseti(R, -2, ++i);
	}

	return 1;
}

static int getproto(lua_State* R) {
	const auto closure = get_closure(R);

	if (closure->isC)
		luaL_argerror(R, 1, "lua function expected");

	if (!lua_isnumber(R, 2))
		luaL_argerror(R, 2, "number expected");

	bool activated = luaL_optboolean(R, 3, false);

	const int idx = lua_tonumber(R, 2);

	luaL_argcheck(R, idx, 2, "index out of range");
	luaL_argcheck(R, idx <= closure->l.p->sizep, 2, "index out of range");

	Proto* original = closure->l.p->p[idx - 1];

	if (activated) {
		luaL_error(R, "Not supported."); // TODO: fix this
		auto cur_page = R->global->allgcopages;

		lua_createtable(R, 0, 0);

		int idx = 0;

		while (cur_page)
		{
			char* start = nullptr;
			char* end = nullptr;
			auto block = 0;
			auto size = 0;

			luaM_getpagewalkinfo(cur_page, &start, &end, &block, &size);

			for (auto pos = start; pos != end; pos += size)
			{
				if (const auto gco = reinterpret_cast<GCObject*>(pos); gco && (gco->gch.tt == LUA_TFUNCTION) && !isdead(R->global, gco)) {
					auto cl = &gco->cl;
					if (cl == closure) {
						auto newcl = luaF_newLclosure(R, cl->nupvalues, cl->env, closure->l.p->p[idx - 1]);
						setclvalue(R, R->top, newcl);
						incr_top(R);
						lua_rawseti(R, -2, ++idx);
					}
				}
			}

			cur_page = cur_page->gcolistnext;
		}

		return 1;
	}

	Proto* cl_proto = luaF_clonebrutallyfuckedproto(R, original); // this was funny

	setclvalue(R, R->top, luaF_newLclosure(R, closure->nupvalues, closure->env, cl_proto))
	incr_top(R)

	return 1;
}

static int getinfo(lua_State* R) {
	if (lua_gettop(R) == 0)
		luaL_argerror(R, 1, "function or level expected");

	size_t arg_sz;
	const auto args = luaL_optlstring(R, 2, "flnas", &arg_sz);

	int level;

	if (lua_isnumber(R, 1)) {
		level = lua_tointeger(R, 1);
		luaL_argcheck(R, level >= 0, 1, "negative level");
	}
	else if (lua_isfunction(R, 1))
		level = -lua_gettop(R);
	else
		luaL_argerror(R, 1, "function or level expected");

	lua_Debug ar{};

	if (!lua_getinfo(R, level, args, &ar))
		luaL_argerror(R, 1, "invalid level");

	const auto& closure = clvalue(index2addr(R, -1));

	lua_createtable(R, 0, 0);

	lua_pushboolean(R, closure->isC);
	lua_setfield(R, -2, "is_c");

	for (const unsigned char c : std::string(args)) {
		switch (c) {
		case 'a': {
			lua_pushnumber(R, ar.isvararg);
			lua_setfield(R, -2, "is_vararg");

			lua_pushinteger(R, ar.nparams);
			lua_setfield(R, -2, "numparams");

			lua_pushnumber(R, ar.nupvals);
			lua_setfield(R, -2, "nups");

			break;
		}
		case 'f': {
			lua_pushvalue(R, -2);
			lua_remove(R, -3);
			lua_setfield(R, -2, "func");

			break;
		}
		case 'l': {
			lua_pushnumber(R, ar.currentline);
			lua_setfield(R, -2, "currentline");
			break;
		}
		case 'n': {
			lua_pushstring(R, ar.name ? ar.name : "");
			lua_setfield(R, -2, "name");
			break;
		}
		case 's': {
			lua_pushstring(R, ar.source);
			lua_setfield(R, -2, "source");

			lua_pushstring(R, ar.short_src);
			lua_setfield(R, -2, "short_src");

			lua_pushstring(R, ar.what);
			lua_setfield(R, -2, "what");
			break;
		}
		default: {
			luaL_argerror(R, 2, "Invalid debug arguments");
		}
		}
	}
	

	return 1;
}

static int getstack(lua_State* R) {
	if (!lua_gettop(R))
		luaL_argerror(R, 1, "no level specified");

	luaL_checktype(R, 1, LUA_TNUMBER);

	int level = lua_tonumber(R, 1);

	const unsigned index = luaL_optinteger(R, 2, -1);

	lua_newtable(R);

	if (level >= R->ci - R->base_ci || level < 0)
		luaL_argerror(R, 1, "level out of range");

	const auto frame = R->ci - level; // god roblox why do you have to make everything retarded for
	const std::size_t top = (frame->top - frame->base); // ^

	if (clvalue(frame->func)->isC)
		luaL_argerror(R, 1, "level of Lua function expected");

	if (index == -1)
	{
		lua_createtable(R, 0, 0);

		for (unsigned i = 0; i < top;)
		{
			setobj(R, R->top, &frame->base[i])
			incr_top(R);

			lua_rawseti(R, -2, ++i);
		}
	}
	else
	{
		if (index < 1 || index > top)
			luaL_argerror(R, 2, "index out of range");

		setobj(R, R->top, &frame->base[index - 1])
		incr_top(R);
	}

	return 1;
}

static int setstack(lua_State* R) {
	luaL_checktype(R, 1, LUA_TNUMBER);
	luaL_checktype(R, 2, LUA_TNUMBER);
	luaL_checkany(R, 3);

	const int level = lua_tointeger(R, 1);
	const unsigned index = lua_tointeger(R, 2);

	if (level >= R->ci - R->base_ci || level < 0)
		luaL_argerror(R, 1, "level out of range");

	const auto frame = R->ci - level;
	const std::size_t top = (frame->top - frame->base);

	if (clvalue(frame->func)->isC)
		luaL_argerror(R, 1, "level of Lua function expected");

	if (index < 1 || index > top)
		luaL_argerror(R, 2, "index out of range");

	auto src = index2addr(R, 3);
	auto target = &frame->base[index - 1];

	if (src->tt != target->tt)
		luaL_error(R, "src and target types must be the same");

	setobj(R, target, src)

	return 1;
}

int getreg(lua_State* R) {
	// ReSharper disable once CppUnreachableCode
	/*if (sys::DEBUG == 0 && !is_smh_state(R)) {
		return 0;
	}*/

	lua_pushvalue(R, LUA_REGISTRYINDEX);
	return 1;
}

static constexpr std::array<std::pair<const char*, lua_CFunction>, 14> functions{ {
	{ "getmetatable", ::getmetatable },
	{ "getconstant", ::getconstant },
	{ "getconstants", ::getconstants },
	{ "getupvalue", ::getupvalue },
	{ "getupvalues", ::getupvalues },
	{ "getproto", ::getproto },
	{ "getinfo", ::getinfo },
	{ "getprotos", ::getprotos },
	{ "setmetatable", ::setmetatable },
	{ "setconstant", ::setconstant },
	{ "setupvalue", ::ssetupvalue },
	{ "getstack", ::getstack },
	{ "setstack", ::setstack },
	{ "getreg", ::getreg }
} };

void rbx::debug::init(lua_State* R) {
	lua_getglobal(R, "debug");
	lua_setreadonly(R, -1, 0);

	for (const auto & [name, function] : functions) {
		lua_pushcclosure(R, function, name, 0);
		lua_setfield(R, -2, name);
	}

	lua_setreadonly(R, -1, 1);
	lua_pop(R, 1);

	lua_pushcclosure(R, ::getupvalue, "getupvalue", 0);
	lua_setglobal(R, "getupvalue");

	lua_pushcclosure(R, ::getupvalues, "getupvalues", 0);
	lua_setglobal(R, "getupvalues");

	lua_pushcclosure(R, ::ssetupvalue, "setupvalue", 0);
	lua_setglobal(R, "setupvalue");
}