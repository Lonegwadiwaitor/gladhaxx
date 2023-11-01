#pragma once

#include "child.hpp"
#include "util.hpp"

#include "../../libraries/imgui/imgui.h"

namespace ig {
	class color_picker : public child {
	private:
		std::array<float, 4> _colors;
		std::string _text;
		callback_t _callback;
	public:
		void render() override;

		color_picker();

		ImVec4 get_color() const;
		std::string get_text() const;
		callback_t get_callback() const;

		void set_color(const ImVec4& color);
		void set_text(const std::string& text);
		void set_callback(const callback_t& callback);
	};
}
