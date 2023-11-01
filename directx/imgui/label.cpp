#include "../../src/pch.hpp"

#include "label.hpp"
#include "util.hpp"

#include "../../libraries/imgui/imgui.h"

ig::label::label() :
	_text(random()),
	_color({ 1, 1, 1, 1 })
{
	set_visible(true);
	set_same_line(false);

	set_name(random());
	set_class_name(_STR("Label"));
}

void ig::label::render() {
	if (is_same_line()) {
		ImGui::SameLine();
	}

	if (is_visible()) {
		ImGui::TextColored(_color, _text.c_str());

		if (get_tool_tip() != nullptr) {
			get_tool_tip()->render(_text.c_str());
		}
	}
}

std::string ig::label::get_text() const {
	return _text;
}

ImVec4 ig::label::get_color() const {
	return _color;
}

void ig::label::set_text(const std::string& text) {
	_text = text;
}

void ig::label::set_color(const ImVec4& color) {
	_color = color;
}
