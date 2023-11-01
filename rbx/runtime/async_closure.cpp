#include "../../src/pch.hpp"

#include "async_closure.hpp"
#include "../sync/rapi.hpp"

#include "../../libraries/os/output.hpp"
#include "../scheduler/task_scheduler.hpp"

#include "../../libraries/cryptography/random.hpp"

void __fastcall ResumeInvoker2(std::uintptr_t SC, int* thread, uintptr_t args, uintptr_t resumeWithErrorMessage, uintptr_t ErrorMessage) {
	static int oebp, oesp;

	static int* dunno = new int[4];

	__asm {
		mov oebp, ebp // cba manually adjusting for stack corruption
		mov oesp, esp

		push edi
		push ecx
		push edx

		mov ecx, a1
		mov edx, a2
		push a6
		push a5
		push a4
		push a3

		call func

		pop edx
		pop ecx
		pop edi

		mov ebp, oebp
		mov esp, oesp
	}
}



// most of this was taken from syn

rbx::runtime::yielding::yielding(lua_State* R): L(R) {}

int rbx::runtime::yielding::execute(const std::function<yieldable_cfunc()>& yielding_func) const {
	auto state = L;

	static std::map<lua_State*, std::string> yield_map;

	auto& YMS = yield_map[state];
	if (YMS.empty())
		YMS = random::string(16);

	lua_pushthread(L);
	lua_setfield(L, LUA_REGISTRYINDEX, YMS.c_str());

	std::thread([yielding_func, state]
		{
			auto& Sched = *rbx::scheduler;

			yieldable_cfunc ReturnedFunc;
			try
			{
				ReturnedFunc = yielding_func();
			}
			catch (std::exception& Ex)
			{
				Sched.push_smh([state, Ex]
					{
						std::printf("Imagine erroring you fucking pussy!\n");

						// yeah i was pretty annoyed trying to debug this
					});

				return;
			}

			Sched.push_smh([ReturnedFunc, state]
			{
				const auto Returns = ReturnedFunc(state);
				auto rf = ThreadRef(state);
				int threads[] = { (int)&rf };
				//std::printf("ScriptContext: %p\n", (void*)rbx::script_context);

				ResumeInvoker2(rbx::script_context, threads, Returns, 0, 0);
			});
		}).detach();

	return lua_yield(L, 0);
}
