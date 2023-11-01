#pragma once

#include "../../src/pch.hpp"

namespace sys {
	void set_root_dir(const std::string& path);
	std::filesystem::path get_root_dir();

	enum class file_format {
		text,
		binary
	};

	template<file_format format = file_format::text, typename T>
	std::string read_file(const T& path) {
		if constexpr (format == file_format::text) {
			std::ifstream t(path);
			std::string data((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
			t.close();
			return data;
		}
		else if constexpr (format == file_format::binary) {
			std::ifstream t(path, std::ios::binary);
			std::string data((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
			t.close();
			return data;
		}
	}

	template<file_format format = file_format::text, typename T>
	void write_file(const T& path, const std::string& data) {
		if constexpr (format == file_format::text) {
			std::ofstream t(path);
			t << data;
			t.close();
		}
		else if constexpr (format == file_format::binary) {
			std::ofstream t(path, std::ios::binary);
			t << data;
			t.close();
		}
	}
}