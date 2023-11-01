#include "../src/pch.hpp"

#include "string.hpp"

bool util::string::is_hex(const std::string& string) {
	for (const auto& digit : string) {
		if (!std::isxdigit(digit)) {
			return false;
		}
	}

	return true;
}

std::vector<std::string> util::string::split(std::string s, char delimiter) {
	size_t pos_start = 0, pos_end{};
	std::string token;
	std::vector<std::string> res;

	if (s.find(delimiter) == std::string::npos) {
		res.push_back(s);
		return res;
	}

	if (s.back() == delimiter) {
		s.pop_back();
	}

	while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
		token = s.substr(pos_start, pos_end - pos_start);
		pos_start = pos_end + 1;
		res.push_back(token);
	}

	res.push_back(s.substr(pos_start));
	return res;
}

std::string util::string::trim(const std::string &string) {
	try {
		std::string result = string;

		if (!result.empty() && (result.front() == ' ' || result.back() == ' ')) {
			while (result.find(' ') == 0) {
				result.erase(0, 1);
			}

			size_t len = result.size();

			while (result.rfind(' ') == --len) {
				result.erase(len, len + 1);
			}
		}

		return result;
	}
	catch (const std::exception& e) {
		return string;
	}
}
