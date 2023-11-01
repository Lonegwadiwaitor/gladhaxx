#pragma once

#include "../../src/pch.hpp"

struct global_State;
struct Closure;

namespace rbx::bypass {
	enum obfuscation {
		ADD,
		SUB,
		XOR
	};

	enum direction {
		LEFT,
		RIGHT
	};

	using obfuscation_t = std::tuple<obfuscation, obfuscation, direction, direction>;

	template<typename T, obfuscation obf, direction dir>
	FORCEINLINE static constexpr T deobfuscate(const uintptr_t address, const uint16_t offset) {
		if constexpr (dir == LEFT) {
			if constexpr (obf == ADD) {
				return (T)(*reinterpret_cast<uintptr_t*>(address + offset) + (address + offset));
			}
			if constexpr (obf == SUB) {
				return (T)(*reinterpret_cast<uintptr_t*>(address + offset) - (address + offset));
			}
			if constexpr (obf == XOR) {
				return (T)(*reinterpret_cast<uintptr_t*>(address + offset) ^ (address + offset));
			}
		} else {
			if constexpr (obf == ADD) {
				return (T)((address + offset) + *reinterpret_cast<uintptr_t*>(address + offset));
			}
			if constexpr (obf == SUB) {
				return (T)((address + offset) - *reinterpret_cast<uintptr_t*>(address + offset));
			}
			if constexpr (obf == XOR) {
				return (T)((address + offset) ^ *reinterpret_cast<uintptr_t*>(address + offset));
			}
		}

		return T();
	}

	template<typename T, obfuscation obf, direction dir>
	FORCEINLINE static constexpr T obfuscate(const uintptr_t address, const uint16_t offset, const std::uintptr_t data) {
		if constexpr (dir == LEFT) {
			if constexpr(obf == ADD) {
				return (T)((uintptr_t)(data)+(address + offset));
			}
			if constexpr(obf == SUB) {
				return (T)((uintptr_t)(data)-(address + offset));
			}
			if constexpr(obf == XOR) {
				return (T)((uintptr_t)(data) ^ (address + offset));
			}
		}
		else {
			if constexpr (obf == ADD) {
				return (T)((address + offset) + (uintptr_t)(data));
			}
			if constexpr (obf == SUB) {
				return (T)((address + offset) - (uintptr_t)(data));
			}
			if constexpr (obf == XOR) {
				return (T)((address + offset) ^ (uintptr_t)(data));
			}
		}
	}
}
