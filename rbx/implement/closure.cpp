#include "../../src/pch.hpp"

#include "closure.hpp"

#include "lualib.h"

#include "../../libraries/os/output.hpp"

#include "lua.h"
#include "lapi.h"
#include "lmem.h"
#include "ldo.h"
#include "lgc.h"
#include "lfunc.h"

#include "../sync/auxillary.hpp"

#include "../../libraries/cryptography/random.hpp"

#include "Luau/Compiler.h"

struct closure_hook_info;
std::unordered_map<Closure*, closure_hook_info> hook_info;
lua_State* closure_manager = nullptr;

struct closure_hook_info
{
	closure_hook_info() = default;

	closure_hook_info(Closure* cl)
	{
		if (!closure_manager)
		{
			closure_manager = rbx::newthread(rbx::base_state, true);
			lua_ref(rbx::base_state, -1); /* DON'T YOU FUCKING GC MY STATE! :< */
		}

		original = cl;
		isC = cl->isC;
	}

private:
	bool isC;
	Closure* original;
	std::vector<Closure*> hooks; 
	std::vector<Closure*> stubs;

public:
	Closure* add_hook(Closure* hook)
	{
		auto original = create_stub(stubs.back()); /* this is the original function */

		stubs.back()->isC = 1;
		stubs.back()->c.f = rbx::implement::cclosure_handler; /* let's adjust the previous stub to point to our new hook */
		newcclosure_map[stubs.back()] = hook;

		return original; // return the original so we can push it to the stack
	}

	static Closure* convert_lua_to_c(Closure* cl)
	{
		if (cl->isC)
			return cl;

		lua_getglobal(closure_manager, "newcclosure");
		closure_manager->top->tt = LUA_TFUNCTION;
		closure_manager->top->value.gc = (GCObject*)cl;
		incr_top(closure_manager)
		lua_call(closure_manager, 1, 1);

		auto converted = clvalue(luaA_toobject(closure_manager, -1));
		lua_pop(closure_manager, 1);

		return converted;
	}

	Closure* create_stub(Closure* cl) const {
		if (newcclosure_map.contains(cl)) {
			setclvalue(closure_manager, closure_manager->top, newcclosure_map[cl])
		} else {
			setclvalue(closure_manager, closure_manager->top, cl)
		}
		
		incr_top(closure_manager);
		if (isC && !newcclosure_map.contains(cl)) {
			lua_clonecfunction(closure_manager, -1);
		} else {
			lua_clonefunction(closure_manager, -1);

			if (newcclosure_map.contains(cl)) {
				lua_remove(closure_manager, -2);
				auto convert = convert_lua_to_c(clvalue(luaA_toobject(closure_manager, -1)));
				lua_pop(closure_manager, 1);
				return convert;
			}
		}
		lua_remove(closure_manager, -2); // push func that leads to cl

		auto converted = clvalue(luaA_toobject(closure_manager, -1));
		lua_pop(closure_manager, 1);

		converted->env = cl->env;

		return converted;
	}

	Closure* hook_single(Closure* hook)
	{
		auto stub = create_stub(original);

		if (isC && newcclosure_map.contains(original)) {
			newcclosure_map[original] = newcclosure_map[hook];
			newcclosure_map[hook] = stub;
		} else if (isC) {
			original->c.f = rbx::implement::cclosure_handler;
			newcclosure_map[original] = hook;
		} else {
			auto lua_hook = newcclosure_map[hook];
			original->l.p = (Proto*)lua_hook->l.p;
			original->env = lua_hook->env;
			original->nupvalues = lua_hook->nupvalues;
			original->stacksize = lua_hook->stacksize;

			for (int i = 0; i < lua_hook->nupvalues; i++) {
				auto& src = lua_hook->l.uprefs[i];
				auto& target = original->l.uprefs[i];

				memcpy(&target, &src, sizeof(TValue));
			} // ive also tried setobj2n(R, &src, &target)
		}

		hooks.push_back(hook); /* set the initial hook */
		stubs.push_back(stub); /* & its stub */

		return stub;
	}

	void restore()
	{
		if (isC) {
			/* keep in mind that C closures don't get GC'd, obviously. */
		}
		else {
			// blah
		}
	}
};

// pushes a table full of values of *type* from the GC to the stack
static __forceinline void internal_getgc(lua_State* R, lua_Type type) {
	auto cur_page = R->global->allgcopages;

	lua_createtable(R, 0, 0);

	int idx = 1;

	while (cur_page)
	{
		char* start = nullptr;
		char* end = nullptr;
		auto block = 0;
		auto size = 0;

		luaM_getpagewalkinfo(cur_page, &start, &end, &block, &size);

		for (auto pos = start; pos != end; pos += size)
		{
			if (const auto gco = reinterpret_cast<GCObject*>(pos); gco && (gco->gch.tt == type) && !isdead(R->global, gco)) {
				lua_pushnumber(R, (idx++));
				incr_top(R)
				setpvalue(R->top, pos);
				lua_settable(R, -3);
			}
		}

		cur_page = cur_page->gcolistnext;
	}
}

int rbx::implement::checkcaller(lua_State* R) {
	/*std::printf("Thread: %p\n", R);
	std::printf("R->extraspace->script_ptr: %p\n", R->extraspace->script_ptr);
	std::printf("Result: %u", !R->extraspace->script_ptr);*/

	lua_pushboolean(R, R->extraspace->identity == 8);
	return 1;
}

//#define clone_l_upvalues(R, target, src) for (int i = 0; i < src->nupvalues; ++i) \
//	setobj2n(L, &target->l.uprefs[i], &src->l.uprefs[i]);
//
//#define clone_c_upvalues(R, target, src) for (int i = 0; i < src->nupvalues; ++i) \
//	setobj2n(L, &target->l.uprefs[i], &src->l.uprefs[i]);

int rbx::implement::hookfunction(lua_State* R) {
	luaL_checktype(R, 1, LUA_TFUNCTION);
	luaL_checktype(R, 2, LUA_TFUNCTION);

	auto original = clvalue(index2addr(R, 1));
	auto hook = clvalue(index2addr(R, 2));

	if (!hook->isC) { // this is bad! we don't wanna get xpcall'd!
		if (!closure_manager)
		{
			closure_manager = newthread(base_state, true);
			lua_ref(base_state, -1);
		}
		lua_getglobal(closure_manager, "newcclosure");
		closure_manager->top->tt = LUA_TFUNCTION;
		closure_manager->top->value.gc = (GCObject*)hook;
		incr_top(closure_manager);
		lua_call(closure_manager, 1, 1);

		hook = clvalue(luaA_toobject(closure_manager, -1));
		lua_pop(closure_manager, 1);
	}

	

	if (original->nupvalues >= hook->nupvalues) {
		Closure* stub;

		if (!hook_info.contains(original))
		{
			hook_info[original] = closure_hook_info{ original };
			stub = hook_info[original].hook_single(hook);
		}
		else
		{
			stub = hook_info[original].add_hook(hook);
		}

		R->top->tt = LUA_TFUNCTION;
		R->top->value.gc = (GCObject*)stub;
		incr_top(R);

		return 1;
	}

	luaL_error(R, "Failed to hook because original->nupvalues < hook->nupvalues");
}

int rbx::implement::iscclosure(lua_State* R) {
	luaL_checktype(R, 1, LUA_TFUNCTION);

	lua_pushboolean(R, clvalue(index2addr(R, 1))->isC);

	return 1;
}

int rbx::implement::islclosure(lua_State* R) {
	luaL_checktype(R, 1, LUA_TFUNCTION);

	lua_pushboolean(R, !clvalue(index2addr(R, 1))->isC);

	return 1;
}

int rbx::implement::hook_metamethod(lua_State* R) {
	luaL_checktype(R, 2, LUA_TSTRING);
	luaL_checktype(R, 3, LUA_TFUNCTION);

	STR_ENCRYPT_START

	// Add support for multiple hooks of the same function (possibly restorefunction)
	// ^^ DONE, see "closure_hook_info" 

	const TValue* obj = index2addr(R, 1);

	Table* object = nullptr;

	if (lua_getmetatable(R, 1)) {
		object = hvalue(index2addr(R, -1));
		lua_pop(R, 1);
	}

	if (!object)
		luaL_typeerror(R, 1, "Object does not have a metatable");

	const auto metamethod = tsvalue(index2addr(R, 2));

	sethvalue(R, R->top, object)
	incr_top(R)
	lua_rawgetfield(R, -1, metamethod->data); /* shitty method but it'll do for now */
	const auto tm = index2addr(R, -1);
	lua_pop(R, 2);

	if (!tm->tt)
		luaG_runerror(R, "Object does not have any metamethods called \"%d\"", metamethod->data);

	auto original_metamethod = clvalue(tm); // guaranteed to be a Roblox C-Closure

 	Closure* hook = clvalue(index2addr(R, 3));

	if (!hook->isC) {
		std::printf("stk: %i\n", lua_gettop(R));
		lua_getglobal(R, "newcclosure");
		lua_pushvalue(R, 3);
		lua_call(R, 1, 1);

		hook = clvalue(luaA_toobject(R, -1));
		lua_pop(R, 1);
		std::printf("stk: %i\n", lua_gettop(R));
	}

	// stack contains stub pointing to tm
	// TODO: crashes on 2nd hook, fix tomorrow
	// TODO: suggested fix: chain all stubs


	Closure* stub;

	if (!hook_info.contains(original_metamethod))
	{
		hook_info[original_metamethod] = closure_hook_info{ original_metamethod };
		stub = hook_info[original_metamethod].hook_single(hook);
	} else
	{
		stub = hook_info[original_metamethod].add_hook(hook);
	}

	R->top->tt = LUA_TFUNCTION;
	R->top->value.gc = (GCObject*)stub;
	incr_top(R);

	STR_ENCRYPT_END

	//if (original_metamethod->c.f != cclosure_handler)
	//{

	//}

	//setclvalue(R, R->top, original_metamethod)
	//incr_top(R);
	//lua_clonecfunction(R, -1);
	//lua_remove(R, -2); // push stub (func that leads to original_metamethod)

	//if (hook->c.f != cclosure_handler)
	//	luaL_argerror(R, 3, "Not a SirMeme Hub closure (use newcclosure)");

	////original->c.f = reinterpret_cast<lua_CFunction>(bypass::callcheck_flag);

	//if (original_metamethod->c.f != cclosure_handler)
	//{
	//	// hasn't been hooked yet
	//	newcclosure_map[original_metamethod] = newcclosure_map[hook];
	//	original_metamethod->c.f = cclosure_handler;
	//} else
	//{
	//	newcclosure_map[newcclosure_map[original_metamethod]] = newcclosure_map[hook];
	//}


	return 1;
}

int rbx::implement::filter_gc(lua_State* R) {
	return 0;
}

int rbx::implement::is_our_closure(lua_State* R) {
	luaL_checktype(R, 1, LUA_TFUNCTION);

	auto cl = clvalue(index2addr(R, 1));

	for (const auto & our_closure : our_closures) {
		if (cl == our_closure) {
			lua_pushboolean(R, true);
			return 1;
		}
	}

	lua_pushboolean(R, false);

	return 1;
}

int rbx::implement::clone_ref(lua_State* R) {
	luaL_checktype(R, 1, LUA_TUSERDATA);

	const auto original_userdata = uvalue(index2addr(R, 1));
	lua_newuserdatatagged(R, original_userdata->len, original_userdata->tag);

	const auto cloned_userdata = uvalue(index2addr(R, -1));
	cloned_userdata->metatable = (Table*)original_userdata->metatable;

	*reinterpret_cast<std::uintptr_t*>(cloned_userdata->data) = *reinterpret_cast<std::uintptr_t*>(original_userdata->data);
	cloned_userdata->dummy.u = original_userdata->dummy.u;

	return 1;
}

int rbx::implement::clone_function(lua_State* R) {
	luaL_checktype(R, 1, LUA_TFUNCTION);

	if (iscfunction(index2addr(R, 1))) {
		lua_clonecfunction(R, 1);
	}
	else
	lua_clonefunction(R, 1);

	return 1;
}


int rbx::implement::cclosure_handler(lua_State* R) {
	const auto args = lua_gettop(R);
	
	const auto closure = newcclosure_map[clvalue(R->ci->func)];

	STR_ENCRYPT_START

	setclvalue(R, R->top, closure)
	incr_top(R)

	lua_insert(R, 1);

	if (const auto res = lua_pcall(R, args, LUA_MULTRET, 0); res && res != LUA_YIELD)
	{
		std::size_t error_len;
		const char* errmsg = luaL_checklstring(R, -1, &error_len);
		std::string error(errmsg);

		if (error == std::string("attempt to yield across metamethod/C-call boundary"))
			return lua_yield(R, 0);

		luaL_errorL(R, luaL_checklstring(R, -1, nullptr));
	}

	STR_ENCRYPT_END

	return lua_gettop(R);
}

int rbx::implement::newcclosure(lua_State* R) {
	luaL_checktype(R, 1, LUA_TFUNCTION);

 	lua_pushvalue(R, 1);
	lua_setfield(R, LUA_REGISTRYINDEX, random::string(32).data()); /* prevent GC */

	const auto lua = clvalue(index2addr(R, 1));
	lua_pushcclosure(R, cclosure_handler, "", 0);
	const auto c = clvalue(index2addr(R, -1));

	newcclosure_map[c] = lua;
	
	return 1;
}
