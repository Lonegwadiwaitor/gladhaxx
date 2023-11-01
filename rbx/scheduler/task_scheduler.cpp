#include "../../src/pch.hpp"

#include "../../libraries/os/output.hpp"
#include "../../libraries/memory/main.hpp"


#include "task_scheduler.hpp"

#include <array>
#include <deque>
#include <functional>
#include <iostream>
#include <optional>

#include "lua.h"

#include "../sync/rapi.hpp"

#include "../../util/safe_queue.hpp"

using namespace std::string_literals;

uintptr_t rbx::script_context;

struct call {
	std::function<void()> function;
	const char* name;
};

SafeQueue<call> smh_call_stack;
std::deque<call> call_stack;


uintptr_t(*oTaskCaller)(uintptr_t);
//void(*oTaskCaller1)();

DWORD TaskSchedulerExceptionHandler(DWORD dwExceptionCode, PEXCEPTION_POINTERS lpException, const char* identifier) {
	std::array<char, MAX_PATH> lpExceptionCode;
	auto dwExceptionAddress = reinterpret_cast<DWORD>(lpException->ExceptionRecord->ExceptionAddress);

	const char* filtered_identifier = identifier == nullptr ? _STR("Not found") : identifier;

	sprintf(lpExceptionCode.data(), _STR("Identifier: %s\nError Code: %x\nException Address: %x"), filtered_identifier,
		dwExceptionCode, mem::unbase(dwExceptionAddress, 0x400000));
	MessageBoxA(NULL, lpExceptionCode.data(), _STR("TaskScheduler Error"), MB_ICONERROR);

	return EXCEPTION_EXECUTE_HANDLER;
}

uintptr_t task_caller(uintptr_t poo) {

	__try {
		if (!call_stack.empty()) {
			const auto& [function, _] = call_stack.front();
			function();
			call_stack.pop_front();
		}

		return oTaskCaller(poo);
	}
	__except (TaskSchedulerExceptionHandler(GetExceptionCode(), GetExceptionInformation(), nullptr)) {
		exit(0);
	}
}

void task_caller_1() {
	if (!call_stack.empty()) {
		call_stack.front().function();
		call_stack.pop_front();
	}
}

void rbx::our_task_caller() {
	if (smh_call_stack.size() > 0)
		smh_call_stack.dequeue().function();
}

void rbx::task_scheduler::set_fps_cap(const double& fps) const {
	MUTATE_START

	std::array<std::uint8_t, 0x100> buffer{};
    const auto& search_offset = 0x100;
	std::uint16_t delay_offset{};

	memcpy(buffer.data(), reinterpret_cast<std::uint8_t*>(_scheduler + search_offset), buffer.size());

	for (std::size_t i = 0; i < buffer.size() - sizeof(double); i += 4) {
		constexpr double frame_delay = 1.0 / 60.0;
		auto difference = *reinterpret_cast<double*>(buffer.data() + i) - frame_delay;

		difference = difference < 0 ? -difference : difference;

		if (difference < std::numeric_limits<double>::epsilon()) {
			delay_offset = search_offset + i;
		}
	}

	*reinterpret_cast<double*>(_scheduler + delay_offset) = 1.0 / fps; // set FPS to 144
	std::cout << "Set FPS to " << std::to_string((int)(1.0 / *reinterpret_cast<double*>(_scheduler + delay_offset))) << '\n';

	MUTATE_END
}

rbx::task_scheduler::task_scheduler() : _scheduler(reinterpret_cast<std::uintptr_t>(this)) {
	_job_begin = 308;
	_job_end = 312;
}

void rbx::task_scheduler::hook_vtable(rbx::job* job, uintptr_t func, size_t size) {
	MUTATE_START

	auto new_vtable = new uint32_t[size];

	if (new_vtable != nullptr) {
		memcpy(new_vtable, job->vtable, size);
		oTaskCaller = reinterpret_cast<decltype(oTaskCaller)>(new_vtable[1]);

		new_vtable[1] = func;
		DWORD old;
		VirtualProtect(job, sizeof(uintptr_t), PAGE_READWRITE, &old);
		job->vtable = new_vtable;
		VirtualProtect(job, sizeof(uintptr_t), old, &old);
	}

	MUTATE_END
}

void rbx::task_scheduler::hook() {
	iterate_jobs<uintptr_t>([=](rbx::job* job) -> std::optional<uintptr_t> {
		if (job->name == _STR("WaitingHybridScriptsJob")) {
			sys::cout(_STR("hooking script job\n"));
			hook_vtable(job, reinterpret_cast<uintptr_t>(task_caller), 40);
			sys::cout(_STR("script job hooked\n"));
			return 0;
		}

		return {};
	});
}

uintptr_t rbx::task_scheduler::get_script_context(int idx) {
	return iterate_jobs<uintptr_t>([=](rbx::job* job) -> std::optional<uintptr_t> {
		if (job->name == _STR("WaitingHybridScriptsJob")) {
			script_context = *reinterpret_cast<uintptr_t*>(reinterpret_cast<uintptr_t>(job) + idx);
			return script_context;
		}

		return {};
	});
}

void rbx::task_scheduler::push(const std::function<void()>& function, const char* identifier) {
	call_stack.emplace_back(function, identifier);
}

void rbx::task_scheduler::push_smh(const std::function<void()>& function, const char* identifier) {
	smh_call_stack.enqueue({ function, identifier });
}

void rbx::task_scheduler::print_jobs() {
	iterate_jobs<int>([](rbx::job* job) -> std::optional<int> {
		std::cout << job->name << '\n';
		return {};
		});
}
