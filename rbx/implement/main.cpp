#include "../../src/pch.hpp"

#include "../../rbx/sync/rapi.hpp"
#include "../luau/VM/include/lua.h"

#include "../libraries/imgui.hpp"
#include "../libraries/draw.hpp"
#include "../libraries/aes.hpp"
#include "../libraries/debug.hpp"
#include "../libraries/smh.hpp"
#include "../libraries/base64.hpp"

#include "main.hpp"

#include "closure.hpp"
#include "env.hpp"
#include "input.hpp"
#include "instance.hpp"
#include "hooks.hpp"
#include "io.hpp"
#include "lapi.h"
#include "lualib.h"
#include "script.hpp"
#include "table.hpp"
#include "console.hpp"

#include "Luau/Compiler.h"

#include "../scheduler/task_scheduler.hpp"

int step_frame(lua_State* R) {
	rbx::our_task_caller();
	return 0;
}

void rbx::implement::init(lua_State* R) {
	//ig::init(R);
	draw::init(R);
	smh::init(R);
	aes::init(R);
	base64::init(R);
	debug::init(R);

	lua_getglobal(R, "require");
	orequire = (void*)lua_tocfunction(R, -1);
	lua_pop(R, 1);

	constexpr std::array<std::pair<const char*, lua_CFunction>, 63> function_map{ {
			// closure.hpp
			{ "newcclosure", newcclosure },
			{ "hookfunction", hookfunction },
			{ "hookfunc", hookfunction },
			{ "checkcaller", checkcaller },
			{ "islclosure", islclosure },
			{ "iscclosure", iscclosure },
			{ "hookmetamethod", hook_metamethod },
			{ "clonefunction", clone_function },
			{ "cloneref", clone_ref },
			{ "filter_gc", filter_gc },
			{ "isourclosure", is_our_closure },

			// env.hpp
			{ "getgenv", getgenv },
			{ "getrenv", getrenv },
			{ "getgc", getgc },
			{ "getreg", get_reg },
			{ "getsenv", getsenv },
			{ "setthreadidentity", setthreadidentity },
			{ "getthreadidentity", getthreadidentity },
			{ "isluau", is_luau },
			{ "gethui", get_hidden_ui },
			{ "setfpscap", set_fps_cap },

			// console.hpp
			{ "rconsoleprint", console::rconsoleprint },
			{ "rconsoleclear", console::rconsoleclear },
			{ "rconsoleerr", console::rconsoleerr },
			{ "rconsolewarn", console::rconsolewarn },
			{ "rconsoletitle", console::rconsoletitle },
			{ "rconsoleinfo", console::rconsoleinfo },

			// input.hpp
			{ "mousemoverel", mousemoverel },
			{ "mouse1click", mouse1click },
			{ "mouse1down", mouse1down },
			{ "mouse1up", mouse1up },
			{ "mouse2click", mouse2click },
			{ "mouse2down", mouse2down },
			{ "mouse2up", mouse2up },

			// io.hpp
			{ "readfile", read_file, },
			{ "writefile", write_file },
			{ "appendfile", append_file },
			{ "loadfile", load_file },
			{ "listfiles", list_files },
			{ "isfile", is_file },
			{ "isfolder", is_folder },
			{ "makefolder", make_folder },
			{ "deletefolder", delete_folder },
			{ "deletefile", delete_file },


			// script.hpp
			{ "getscripthash", get_script_hash },
			{ "getscriptbytecode", get_script_bytecode },
			{ "getscriptclosure", get_script_closure },
			{ "loadstring", load_string },

			// instance.hpp
			// TODO: fully fix for UWP
			{ "require", require },
			//{ "getsimulationradius", getsimulationradius },
			//{ "setsimulationradius", setsimulationradius },
			{ "getcallingscript", getcallingscript },
			{ "fireclickdetector", fireclickdetector },
			//{ "firetouchinterest", firetouchinterest },
			{ "fireproximityprompt", fireproximityprompt },
			{ "getinstances", getinstances },
			{ "getnilinstances", getnilinstances },
			//{ "getsignalfunctions", getsignalfunctions },
			//{ "getconnections", getconnections },

			// table.hpp
			{ "getrawmetatable", getrawmetatable },
			{ "getnamecallmethod", getnamecallmethod },
			{ "setnamecallmethod", setnamecallmethod },
			{ "setreadonly", setreadonly },
			{ "isreadonly", isreadonly },
			{ "isluau", is_luau },
			{ "identifyexecutor", identify_executor},
			{ "getuserdata", [](lua_State* R) {
				std::printf("Userdata: %p\n", *reinterpret_cast<void**>(lua_touserdata(R, 1)));
				return 0;
			}}
	}
	};

	for (auto& [name, function] : function_map) {
		std::cout << name << '\n';
		lua_pushcclosure(R, function, name, 0);
		lua_setglobal(R, name);
	}

	hooks::init(R);

	lua_getfield(R, -10002, "game");
	lua_getfield(R, -1, "GetService");
	lua_pushvalue(R, -2);
	lua_remove(R, -3);
	lua_pushstring(R, "RunService");
	lua_pcall(R, 2, 1, 0);
	lua_getfield(R, -1, "RenderStepped");
	lua_getfield(R, -1, "Connect");
	lua_pushvalue(R, -2);
	lua_remove(R, -3);
	lua_pushcclosure(R, step_frame, "", 0);
	lua_pcall(R, 2, 0, 0);

	lua_pop(R, 1);


	// UNC ""aliases""

#define add_alias(original, alias) lua_getglobal(R, original); lua_setglobal(R, alias);

	lua_getglobal(R, "smh");
	lua_getfield(R, -1, "crypt");
	lua_setglobal(R, "crypt");
	lua_pop(R, 1);

	lua_getglobal(R, "crypt");
	lua_getfield(R, -1, "base64");

	lua_getfield(R, -1, "encode");
	lua_setfield(R, -3, "base64encode");

	lua_getfield(R, -1, "decode");
	lua_setfield(R, -3, "base64decode");

	lua_pop(R, 1);

	lua_getfield(R, -1, "base64encode");
	lua_setfield(R, -2, "base64_encode");

	lua_getfield(R, -1, "base64decode");
	lua_setfield(R, -2, "base64_decode");

	lua_pop(R, 1);

	lua_getglobal(R, "smh");
	lua_getfield(R, -1, "request");
	lua_setglobal(R, "request");
	lua_pop(R, 1);

	add_alias("hookfunction", "replaceclosure");

	add_alias("getthreadidentity", "getidentity");
	add_alias("getthreadidentity", "getthreadcontext");

	add_alias("setthreadidentity", "setidentity");
	add_alias("setthreadidentity", "setthreadcontext");
}
