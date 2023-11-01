#include "../src/pch.hpp"

namespace util::string {
	std::vector<std::string> split(std::string string, char delimiter);
	std::string trim(const std::string &string);
	bool is_hex(const std::string& string);
}
