#include "../../src/pch.hpp"

#include "text_box.hpp"

#include "../../libraries/imgui/imgui.h"

#include "util.hpp"

ig::text_box::text_box() :
	_title(random()),
	_multi_line(false)
{
	set_visible(true);
	set_same_line(false);

	set_name(random());
	set_class_name(_STR("TextBox"));
}

void ig::text_box::render() {
	if (is_visible()) {
		if (is_same_line()) {
			ImGui::SameLine();
		}

		if (_multi_line) {
			if (ImGui::InputTextMultiline(_title.c_str(), _text.data(), _text.size(), ImVec2(0, 0), ImGuiInputTextFlags_AllowTabInput | ImGuiInputTextFlags_EnterReturnsTrue) && _callback) {
				_callback();
			}
		}
		else {
			if (ImGui::InputText(_title.c_str(), _text.data(), _text.size(), ImGuiInputTextFlags_AllowTabInput | ImGuiInputTextFlags_EnterReturnsTrue) && _callback) {
				_callback();
			}
		}

		if (get_tool_tip() != nullptr) {
			get_tool_tip()->render();
		}
	}
}

std::string ig::text_box::get_text() const {
	return _text.data();
}

std::string ig::text_box::get_title() const {
	return _title;
}

bool ig::text_box::is_multi_line() {
	return _multi_line;
}

ig::callback_t ig::text_box::get_callback() const {
	return _callback;
}

void ig::text_box::set_text(const std::string& text) {
	strcpy_s(_text.data(), _text.size(), text.c_str());
}

void ig::text_box::set_title(const std::string& title) {
	_title = title;
}

void ig::text_box::set_multi_line(bool multi_line) {
	_multi_line = multi_line;
}

void ig::text_box::set_callback(const callback_t& callback) {
	_callback = callback;
}
