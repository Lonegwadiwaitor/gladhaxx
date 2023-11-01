#pragma once

#include "container.hpp"

#include "../../libraries/imgui/imgui.h"

namespace ig {
	class window : public container {
	private:
		std::string _text;
		ImVec2 _size;
		ImVec2 _position;
	public:
		void render() override;

		window();

		static window* create();

		ImVec2 get_size() const;
		ImVec2 get_position() const;
		std::string get_text() const;

		void set_size(const ImVec2& size);
		void set_position(const ImVec2& position);
		void set_text(const std::string &text);
	};

	extern std::vector<ig::window> windows;
}
