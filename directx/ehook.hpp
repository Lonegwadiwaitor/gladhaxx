#pragma once
#include <Windows.h>
#include <cstdint>

// pupper lib owo

namespace ehook
{
	class ehook
	{
	private:
		uint8_t* _original_bytes;
		uint8_t _hook_bytes[5] = { 0xE9 };
	public:
		bool is_hooked;
		uintptr_t _address, _hook;
	private:
		static FORCEINLINE uint8_t* read_memory_raw(void* address, const size_t size)
		{
			const auto result = new uint8_t[size];
			DWORD old_protect;
			VirtualProtect(address, size, PAGE_EXECUTE_READWRITE, &old_protect);
			memcpy(reinterpret_cast<void*>(result), address, size);
			VirtualProtect(address, size, old_protect, &old_protect);
			return result;
		}

		template<typename T>
		static FORCEINLINE T read_memory(void* address) {
			return read_memory_raw(address, sizeof(T));
		}

		static FORCEINLINE void write_memory_raw(void* address, const void* to_write, const size_t size)
		{
			DWORD old_protect;
			VirtualProtect(address, size, PAGE_EXECUTE_READWRITE, &old_protect);
			memcpy(reinterpret_cast<void*>(address), to_write, size);
			VirtualProtect(address, size, old_protect, &old_protect);
		}

		template<typename T>
		static FORCEINLINE void write_memory(void* address, const T to_write) {
			write_memory_raw(address, reinterpret_cast<void*>(to_write), sizeof(T));
		}

	public:
		ehook(const uintptr_t address, const uintptr_t hook, uint8_t* original_bytes = nullptr) : _address(address), _hook(hook), is_hooked(false) {
			const auto relative = (hook - address) - 5;
			memcpy(_hook_bytes + 1, &relative, sizeof(relative));
			_original_bytes = original_bytes ? original_bytes : read_memory_raw(reinterpret_cast<void*>(address), sizeof(_hook_bytes));
		}

		void apply() {
			if (!is_hooked) {
				write_memory_raw(reinterpret_cast<void*>(_address), _hook_bytes, sizeof(_hook_bytes));
				is_hooked = true;
			}
		}

		void undo() {
			if (is_hooked) {
				is_hooked = false;
				write_memory_raw(reinterpret_cast<void*>(_address), _original_bytes, sizeof(_hook_bytes));
			}
		}

		~ehook() {
			undo();
			delete _original_bytes;
		}
	};
}