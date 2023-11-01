#pragma once

#include <functional>
#include <optional>

#include "../../src/singleton.hpp"

struct lua_State;

namespace rbx {
	void our_task_caller();
	class task_scheduler;
	inline lua_State* execution_state;
	inline task_scheduler* scheduler;

	class job {
	public:
		void* vtable;
		uint8_t _[12];
		std::string name;
	};

	class task_scheduler : public singleton<task_scheduler> {
	private:
		uintptr_t _scheduler = (uintptr_t)this;
		uint16_t _job_begin, _job_end;

		static void hook_vtable(job* job, uintptr_t func, size_t size);

		template<typename T>
		T iterate_jobs(const std::function<std::optional<T>(rbx::job*)>& callback) {
			class TaskScheduler {
			private:
				using job_iterator = std::vector<job*>::iterator;
			private:
				std::uint8_t _[308];
				job_iterator _begin;
				job_iterator _end;
			public:
				job_iterator begin() const {
					return _begin;
				}

				job_iterator end() const {
					return _end;
				}
			};

			for (const auto& job : std::ranges::reverse_view(*reinterpret_cast<TaskScheduler*>(_scheduler))) {
				if (auto result = callback(job)) {
					return result.value();
				}
			}

			return {};
		}
	public:
		task_scheduler();

		void hook();
		void set_fps_cap(const double& fps) const;
		static void push(const std::function<void()>& function, const char* identifier = nullptr);
		static void push_smh(const std::function<void()>& function, const char* identifier = nullptr);
		void print_jobs();

		uintptr_t get_script_context(int idx);

	};
}
