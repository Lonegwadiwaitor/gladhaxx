#pragma once

#include "../../src/pch.hpp"

#include "child.hpp"
#include "../../libraries/imgui/imgui.h"

namespace ig {
	class label : public child {
	private:
		std::string _text;
		ImVec4 _color;
	public:
		void render() override;

		label();

		std::string get_text() const;
		ImVec4 get_color() const;

		void set_text(const std::string& text);
		void set_color(const ImVec4& color);
	};
}
