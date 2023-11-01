#include "../../src/pch.hpp"

#include "button.hpp"
#include "util.hpp"

#include "../../libraries/imgui/imgui.h"

ig::button::button() :
	_text(random())
{
	set_visible(true);
	set_same_line(false);

	set_name(random());
	set_class_name(_STR("Button"));
}

void ig::button::render() {
	if (is_same_line()) {
		ImGui::SameLine();
	}

	if (is_visible()) {
		if (ImGui::Button(_text.c_str()) && _callback) {
			_callback();
		}

		if (get_tool_tip() != nullptr) {
			get_tool_tip()->render(_text.c_str());
		}
	}
}

std::string ig::button::get_text() const {
	return _text;
}

ig::callback_t ig::button::get_callback() const {
	return _callback;
}

void ig::button::set_text(const std::string& text) {
	_text = text;
}

void ig::button::set_callback(const callback_t& callback) {
	_callback = callback;
}
