#include "pch.hpp"
#include "src.hpp"

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/roles/server_endpoint.hpp>

#include "lualib.h"
#include "../libraries/os/output.hpp"
#include "../libraries/os/file.hpp"
#include "../libraries/os/bios.hpp"

#include "../libraries/cryptography/aes.hpp"
#include "../libraries/cryptography/base64.hpp"
#include "../libraries/cryptography/sha512.hpp"

#include "../libraries/memory/stealth.hpp"

#include "../libraries/string.hpp"

#include "../rbx/bypass/console.hpp"

#include "../rbx/scheduler/task_scheduler.hpp"

#include "../rbx/implement/main.hpp"
#include "../rbx/bypass/obfuscation.hpp"

#include "../libraries/cryptography/base64.hpp"

#include "../directx/imgui/window.hpp"
#include "../directx/imgui/button.hpp"

#include "../directx/hook.hpp"

#include "../util/socket_client.hpp"

#include "../../VM/src/lstate.h"
#include "../../Compiler/include/Luau/Compiler.h"

#include "../../VM/src/lstring.h"

#include "../rbx/runtime/async_closure.hpp"

#include "../rbx/sync/auxillary.hpp"

DWORD YieldExceptionHandler(DWORD dwExceptionCode, PEXCEPTION_POINTERS lpException) {
	std::array<char, MAX_PATH> lpExceptionCode;
	auto dwExceptionAddress = reinterpret_cast<DWORD>(lpException->ExceptionRecord->ExceptionAddress);

	sprintf(lpExceptionCode.data(), _STR("Error Code: %x\nException Address: %x"), dwExceptionCode, mem::unbase(dwExceptionAddress, 0x400000));
	MessageBoxA(NULL, lpExceptionCode.data(), _STR("CallCheck Error"), MB_ICONERROR);

	return EXCEPTION_EXECUTE_HANDLER;
}

void wrong_obfuscation(std::string_view struct_name, std::string_view offset_name) {
	std::cout << "Obfuscations wrong for " << struct_name << " \"" << offset_name << "\", hanging here...\n";
}

void right_obfuscation(std::string_view struct_name, std::string_view offset_name) {
	std::cout << "Obfuscations correct for " << struct_name << " \"" << offset_name << "\", carrying on\n";
}

::LONG __stdcall rtl_exception_handler(::EXCEPTION_POINTERS* a/* no use for this atm */)
{
	std::cout << "crashed at 0x" << std::hex << mem::unbase((uintptr_t)a->ExceptionRecord->ExceptionAddress, 0x400000) << std::dec << '\n';
	std::cout << "callee: 0x" << std::hex << mem::unbase((uintptr_t) * reinterpret_cast<DWORD*>(a->ContextRecord->Ebp + 4) - 5, 0x400000) << std::dec << '\n';

	MessageBoxA(nullptr, "Roblox has crashed, press 'OK' to exit safely.", "SirMeme Hub", 0);
	exit(0);
}

void exception_handler() {
	const auto& proc_address = ::GetProcAddress(::GetModuleHandle("ntdll.dll"), "RtlSetUnhandledExceptionFilter");

	reinterpret_cast<::LPTOP_LEVEL_EXCEPTION_FILTER(__stdcall*)(::LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter)>(proc_address)(rtl_exception_handler);
}



int main() {
	std::thread([]() {
		while (true)
		{
			std::string Script;

			std::getline(std::cin, Script);


			if (Script != "")
			{
				if (Script.starts_with("http"))
				{
					Script = cpr::Get(cpr::Url{ Script }).text;
				}

				rbx::task_scheduler::push_smh([Script]() {
					auto thread = rbx::execution_state;
					bool error = false;

					const auto bytecode = Luau::compile(Script, { 2, 1, 0 });

					if (error)
					{
						lua_getglobal(thread, "warn");
						lua_pushstring(thread, bytecode.data());
						lua_pcall(thread, 1, 0, 0);
						return;
					}

					lua_getglobal(thread, "spawn");
					if (luau_load(thread, "bytecode", bytecode, 0))
					{
						size_t len;
						const char* msg = lua_tolstring(thread, -1, &len);

						std::string error(msg, len);
						lua_pop(thread, 2);

						lua_getglobal(thread, "warn");
						lua_pushstring(thread, error.data());
					}

					lua_pcall(thread, 1, 0, 0);

				});
				
				
			}
		}
		}).detach();



		exception_handler();



		rbx::scheduler = reinterpret_cast<rbx::task_scheduler*>(scheduler_ref);


		//rbx::bypass::memcheck(scheduler);

		//auto script_context = scheduler.get_script_context();

		rbx::scheduler->hook();



		rbx::scheduler->set_fps_cap(144);

		auto loadfile = [=](std::string base64) {
			sys::cout("executing script via loadfile...\n");

			auto bytecode = base64::dec(base64);

			lua_getglobal(rbx::base_state, "spawn");
			luau_load(rbx::base_state, "smh", bytecode, 0);
			lua_pcall(rbx::base_state, 1, 0, 0);
			sys::cout("executed!\n");
		};



		rbx::task_scheduler::push([=] {
			using namespace rbx::bypass;


			const auto script_context = rbx::scheduler->get_script_context(312);

			sys::cout("getting lua state\n");

			// now, we need to borrow a few things.

			rbx::roblox_state = deobfuscate<lua_State*, SUB, RIGHT>(script_context, 276);
			std::printf("%i\n", lua_gettop(rbx::roblox_state));
			std::cout << script_context << '\n';
			std::cout << (int)rbx::roblox_state->marked << '\n';
			std::cout << (int)rbx::roblox_state->stacksize << '\n';
			global_State* global = rbx::roblox_state->global;
			std::printf("global: %p\n", global);
			std::cout << (int)global->currentwhite << '\n';

			volatile auto tm = global->tmname[0]->data;
			volatile auto tt = global->tmname[0]->data;

			std::cout << tm << '\n';
			std::cout << tt << '\n';



			//if (!test_obfuscations(rbx::roblox_state))
				//return;

			sys::cout("creating new thread\n");
			rbx::base_state = rbx::newthread(rbx::roblox_state, true);
			lua_ref(rbx::roblox_state, -1);
			lua_pop(rbx::roblox_state, 1);

			rbx::execution_state = rbx::newthread(rbx::base_state, true);
			lua_pop(rbx::base_state, 1);
			/*lua_pushvalue(rbx::roblox_state, -1);
			sys::cout("caching thread\n");
			rbx::ref(rbx::roblox_state, LUA_REGISTRYINDEX);

			sys::cout("caching require\n");
			rbx::getglobal(rbx::base_state, "require");
			rbx::setregistry(rbx::base_state, "require");*/

			sys::cout("loading libraries and functions\n");
			rbx::implement::init(rbx::base_state);

			if constexpr (sys::DEBUG == 0) {
				sys::cout("loading functions and libraries for debugging\n");
				sys::cout("loading getsignalfunctions\n");
				lua_getglobal(rbx::base_state, "getconnections");
				lua_setfield(rbx::base_state, -10003, "getconnections");

				sys::cout("loading imgui\n");
				lua_getglobal(rbx::base_state, "ImGui");
				lua_setfield(rbx::base_state, -10003, "ImGui");

				sys::cout("loading hookfunction\n");
				lua_getglobal(rbx::base_state, "hookfunction");
				lua_setfield(rbx::base_state, -10003, "hookfunction");

				sys::cout("loading newcclosure\n");
				lua_getglobal(rbx::base_state, "newcclosure");
				lua_setfield(rbx::base_state, -10003, "newcclosure");

				sys::cout("done loading debugging functions and libraries\n");
			}

			sys::cout("executing init\n");

			//loadfile(sys::read_file(sys::get_root_dir() / "workspace/init.bin"));

			/*auto vm = (PVOID)luau_execute_address;

			MH_CreateHook(vm, luau_execute, (LPVOID*)&oExecute);
			MH_EnableHook(vm);*/

			/*lua_getglobal(rbx::base_state, "spawn");
			auto bytecode = Luau::compile("print'hello from CLVM!'");
			luau_load(rbx::base_state, "", bytecode.data(), bytecode.size(), 0);
			lua_pcall(rbx::base_state, 1, 0, 0);*/

			sys::cout("init executed\n");

			//lua_settop(rbx::base_state, 0);
			//lua_settop(rbx::roblox_state, 0);

			}, "init function");

		/*static security::anti_tamper antitamper;
		std::thread([] {
			while (true) {
				std::this_thread::sleep_for(35ms);
				antitamper.check();
			}
		}).detach();*/

		/*std::thread([]() {
			std::string script{R"(print("WHAT THE FRICK MAN"))"};

			while (true) {
				std::getline(std::cin, script);

				rbx::task_scheduler::get_singleton().push([=]() {
					const auto bytecode = Luau::compile(script);

					lua_getglobal(rbx::base_state, "spawn");
					luau_load(rbx::base_state, "bytecode", bytecode.c_str(), bytecode.size(), 0);

					if (lua_type(rbx::base_state, -1) != LUA_TFUNCTION) {
						std::cout << "not func: " << lua_typename(rbx::base_state, -1) << '\n';

						if (lua_type(rbx::base_state, -1) == LUA_TSTRING) {
							std::cout << lua_tostring(rbx::base_state, -1) << '\n';
						}
					}
					else {
						std::cout << "is func\n";
					}

					if (lua_pcall(rbx::base_state, 1, 0, 0) != 0) {
						if (lua_type(rbx::base_state, -1) == LUA_TSTRING) {
							std::cout << lua_tostring(rbx::base_state, -1) << '\n';
						}

						lua_pop(rbx::base_state, 1);
					}
				});
			}
		}).detach();*/



		/*std::thread([&] {
			std::this_thread::sleep_for(2s);

			auto data = sys::read_file(sys::get_root_dir() / "../workspace/modules/universal.bin");

			loadfile(data);
		}).detach();*/

		/*MH_CreateHook(reinterpret_cast<unsigned int(__thiscall*)(int16_t, int, DWORD)>(mem::rebase(0x689040, 0x400000)), epic,
						  &original);

			MH_EnableHook(reinterpret_cast<unsigned int(__thiscall*)(int16_t, int, DWORD)>(mem::rebase(0x689040, 0x400000)));*/

			//sys::cout("the_epic\n");

			//auto deserialize = mem::rebase(0x15F26C0, 0x400000);
			//
			//rbx::bypass::add_hook(reinterpret_cast<int(__cdecl*)(lua_State*, const char*, char*, int, int)>(deserialize), des_hook,
			//              &original_des);

		return 0;
}
