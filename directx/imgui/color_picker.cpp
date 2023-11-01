#include "../../src/pch.hpp"

#include "color_picker.hpp"

#include "util.hpp"

ig::color_picker::color_picker() :
	_text(random()),
	_colors({ 1, 1, 1, 1 })
{
	set_visible(true);
	set_same_line(false);

	set_name(random());
	set_class_name(_STR("ColorPicker"));
}

void ig::color_picker::render() {
	if (is_same_line()) {
		ImGui::SameLine();
	}

	if (is_visible()) {
		if (ImGui::ColorEdit4(_text.c_str(), _colors.data()) && _callback) {
			_callback();
		}

		if (get_tool_tip() != nullptr) {
			get_tool_tip()->render(_text.c_str());
		}
	}
}

ImVec4 ig::color_picker::get_color() const {
	return { _colors[0], _colors[1], _colors[2], _colors[3] };
}

std::string ig::color_picker::get_text() const {
	return _text;
}

ig::callback_t ig::color_picker::get_callback() const {
	return _callback;
}

void ig::color_picker::set_color(const ImVec4& color) {
	_colors = { color.x, color.y, color.z, color.w };
}

void ig::color_picker::set_text(const std::string& text) {
	_text = text;
}

void ig::color_picker::set_callback(const callback_t &callback) {
	_callback = callback;
}
