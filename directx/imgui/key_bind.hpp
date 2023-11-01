#pragma once

#include "../../src/pch.hpp"

#include "child.hpp"
#include "util.hpp"
#include "../../libraries/imgui/imgui.h"

namespace ig {
	class key_bind : public child {
	private:
		std::string _key_code;
		std::string _text;
	public:
		void render() override;

		key_bind();

		std::string get_text() const;
		std::string get_key_code() const;

		void set_text(const std::string& text);
		void set_key_code(const std::string& key_code);
	};
}

