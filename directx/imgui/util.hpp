#pragma once

#include "../../src/pch.hpp"

namespace ig {
	using callback_t = std::function<void()>;

	std::string random();
	void initialize_font();
}
