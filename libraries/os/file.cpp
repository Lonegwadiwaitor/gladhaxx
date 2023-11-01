#include "../../src/pch.hpp"

#include "file.hpp"

static std::filesystem::path root_dir;

void sys::set_root_dir(const std::string& path) {
	root_dir = path;
}

std::filesystem::path sys::get_root_dir() {
	return root_dir;
}
