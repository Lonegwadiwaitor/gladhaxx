#pragma once

#include <string>

namespace base64 {
	std::string enc(const std::string &text);
	std::string dec(const std::string &text);
}
