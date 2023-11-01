#include "../../src/pch.hpp"

#include "check_box.hpp"
#include "util.hpp"

#include "../../libraries/imgui/imgui.h"

ig::check_box::check_box() :
	_text(random())
{
	set_visible(true);
	set_same_line(false);

	set_name(random());
	set_class_name(_STR("CheckBox"));
}

void ig::check_box::render() {
	if (is_same_line()) {
		ImGui::SameLine();
	}

	if (is_visible()) {
		if (ImGui::Checkbox(_text.c_str(), &_toggled) && _callback) {
			_callback();
		}

		if (get_tool_tip() != nullptr) {
			get_tool_tip()->render(_text.c_str());
		}
	}
}

std::string ig::check_box::get_text() const {
	return _text;
}

bool ig::check_box::is_toggled() const {
	return _toggled;
}

ig::callback_t ig::check_box::get_callback() const {
	return _callback;
}

void ig::check_box::set_text(const std::string& text) {
	_text = text;
}

void ig::check_box::set_toggled(bool toggled) {
	_toggled = toggled;
}

void ig::check_box::set_callback(const callback_t& callback) {
	_callback = callback;
}
